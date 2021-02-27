// Disable unintentional include of time.h in WIN32, which redefines clock
#define _INC_TIME
#include <Arduino.h>
#include <hal/gpio_pin.h>
#include <staticRingBuffer.h>
#include <assert.h>
#include <chrono>
#include <utility>
#include <hal/boards/arduinomega2560.h>
#include "AnalogJoystick.h"

using namespace etl::hal;
using namespace etl;

using clock = std::chrono::steady_clock;
using time_point = clock::time_point;
using duration = clock::duration;

template<class StepPin, class DirPin, class EnablePin>
struct StepperDriver
{
	void enable() { enablePin.setLow(); }
	void disable() { enablePin.setHigh(); }

	void setDir(bool sign)
	{
		if(sign)
			dirPin.setHigh();
		else
			dirPin.setLow();
	}

	void step()
	{
		stepPin.setHigh();
		delayMicroseconds(20);
		stepPin.setLow();
	}

	typename StepPin::Out stepPin;
	typename DirPin::Out dirPin;
	typename EnablePin::Out enablePin;
};

// Ramps 1.4 definitions
using XAxisStepper = StepperDriver<Pin54,Pin55,Pin38>;
// using YAxisStepper = StepperDriver<Pin60,Pin61,Pin56>; // Original RAMPS mapping
using YAxisStepper = StepperDriver<Pin26,Pin28,Pin24>; // Remapping due to a few burnt traces
using ZAxisStepper = StepperDriver<Pin46,Pin48,Pin62>;

XAxisStepper gMotorX;
YAxisStepper gMotorY;
ZAxisStepper gMotorZ;

struct StepperController
{
	bool Finished();
	bool Continue();

	template<int axis>
	void move(int32_t steps, duration time)
	{
		auto& motor = motorState[axis];
		motor.pendingSteps = steps;
		motor.totalSteps = abs(steps);
	}

	template<int axis>
	void resetPosition()
	{
		motorState[axis].pendingSteps = 0;
		motorState[axis].position = 0;
	}

private:
	struct StepperState
	{
		duration stepPeriod;
		int32_t pendingSteps;
		int32_t totalSteps;
		int32_t position;
	} motorState[3];
};

LedPin gLed;

using XMinEndStop = Pin3::In;

XMinEndStop gEndStopMinX;

void setup() {
	// Setup scheduler
	// Setup serial port
	Serial.begin(9600);
	// Start motor control
	gMotorX.enable();
	gMotorY.enable();
	gMotorZ.enable();
}

AnalogJoystick<A5, A10, Pin44> gLeftStick;

etl::FixedRingBuffer<char,128> pendingMessage;

struct GCodeOperation
{
	// Arguments first for more compact alignment
	static constexpr int32_t kEmptyArg = int32_t(1ul<<31);
	int32_t argument[4] = { kEmptyArg, kEmptyArg, kEmptyArg, kEmptyArg }; // X,Y,Z,F
	// Instruction
	uint8_t address;
	uint8_t opCode; // [0,99] -> G, [100,199] -> M

	static constexpr uint8_t CodeOffsetG = 0;
	static constexpr uint8_t CodeOffsetM = 100;
};

etl::FixedRingBuffer<GCodeOperation, 8> operationsBuffer;

void signalError()
{
	pendingMessage.clear();
	gMotorX.disable();
	gMotorY.disable();
	gMotorZ.disable();
	Serial.println("error");
}

class OpCodeParser
{
public:
	void parseOpCode()
	{
		// We shouldn't be processing empty lines.
		// That could lead to us pushing garbage to the execution queue, or acknowleding commands prematurely
		assert(!pendingMessage.empty());

		operationsBuffer.push_back({});
		GCodeOperation& instruction = operationsBuffer.back();
		instruction.address = {};
		instruction.opCode = {};
		int8_t argSign = 1;
		int8_t argPos = 0;

		for (size_t i = 0; i < pendingMessage.size(); ++i)
		{
			char c = pendingMessage[i];
			switch (m_state)
			{
			case State::address:
				switch (c)
				{
				case ' ':
					break; // Ignore white space at the start of the line
				case 'G':
				case 'M':
					m_state = State::code;
					instruction.address = c;
					break;
				default:
					signalError();
					return;
				}
				break;
			case State::code:
				if (c >= '0' && c <= '9')
					instruction.opCode = 10 * instruction.opCode + (c - '0');
				else if (c == ' ')
				{
					m_state = State::arguments;
				}
				else
				{
					signalError();
					return;
				}
				break;
			case State::arguments:
				if (c == ' ')
					break; // Ignore extra spaces
				switch (c)
				{
				case 'X':
					argPos = 0;
					break;
				case 'Y':
					argPos = 1;
					break;
				case 'Z':
					argPos = 2;
					break;
				case 'F':
					argPos = 3;
					break;
				default:
					signalError();
					return;
				}
				m_state = State::integer;
				argSign = 1;
				instruction.argument[argPos] = 0;
				break;
			case State::integer:
				if (c == '-')
					argSign = -1;
				else if (c >= '0' && c <= '9')
				{
					instruction.argument[argPos] *= 10;
					instruction.argument[argPos] += (c - '0') * argSign;
				}
				else if (c == '.')
				{
					signalError(); // Decimal points not yet supported
					m_state = State::decimal;
				}
				else if (c == ' ')
					m_state = State::arguments;
				else
				{
					signalError();
					return;
				}
				break;
			case State::decimal:
				signalError(); // Not yet supported
				return;
			}
		}
		// Clear message and acknowledge
		pendingMessage.clear();
		Serial.println("ok");
	}

private:
	enum class State
	{
		address,
		code,
		arguments,
		integer,
		decimal,
	} m_state = State::address;
};


class GCodeParser
{
public:
	void parseInput()
	{
		if (m_state == State::full)
		{
			if (operationsBuffer.full())
				return;
			m_state = State::message; // No longer full, can proceed to parse input messages
		}
		// Parse one character at a time and yield to increase motor control throughput
		if (!Serial.available())
			return;
		char c;
		Serial.readBytes(&c, 1);
		switch (m_state)
		{
		case State::outOfProgram:
		{
			if (c == '%')
				m_state = State::comment; // Comment out the rest of the line
			break;
		}
		case State::message:
		{
			switch (c)
			{
			case '%':
				m_state = State::outOfProgram;
				break;
			case '\r': // Same as comment, just ignore till the end of the line
			case ';':
				m_state = State::comment;
				break;
			case '\n':
			{
				OpCodeParser parser;
				parser.parseOpCode();
				break;
			}
			default:
				pendingMessage.push_back(c);
				break;
			}
			break;
		}
		case State::comment:
		{
			if (c == '\n')
			{
				if (!pendingMessage.empty())
				{
					OpCodeParser parser;
					parser.parseOpCode();
				}
				m_state = State::message;
			}
			break;
		}
		}
	}
private:
	enum class State
	{
		outOfProgram,
		message,
		comment,
		full,
	} m_state = State::outOfProgram;
} gCodeParser;

void loop()
{
	// Consume data from the serial port
	gCodeParser.parseInput();

	// Control motors
	/*gLeftStick.read();
	if(gLeftStick.xAxis.m_pos < 60)
	{
		gLed.setHigh();
		gMotorX.setDir(true);
	}
	else
	{
		gLed.setLow();
		gMotorX.setDir(false);
	}

	gMotorX.step();
	//gMotorY.step();
	//gMotorZ.step();
	delay(1);
	if(gEndStopMinX)
		gMotorY.setDir(true);*/
}

#ifdef SITL

int main(int argc, char** argv)
{
	if (argc > 1)
		Serial.InitFromFile(argv[1]);
	setup();
	for(;;)
		loop();
	return 0;
}

#endif // SITL