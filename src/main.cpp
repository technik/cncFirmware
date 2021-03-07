//-------------------------------------------------------------
// Copyright 2021 Carmelo J Fdez-Aguera
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define _INC_TIME // Disable unintentional include of time.h in WIN32, which redefines clock
#include <Arduino.h>
#include <hal/gpio_pin.h>
#include <staticRingBuffer.h>
#include <assert.h>
#include <chrono>
#include <utility>
#include <hal/boards/arduinomega2560.h>
#include "AnalogJoystick.h"
#include "motionController.h"
#include "GCode.h"
#include "clock.h"

using namespace etl::hal;
using namespace etl;
using namespace std::chrono_literals;

using clock = SystemClock;
using time_point = clock::time_point;
using duration = clock::duration;

LedPin gLed;

AnalogJoystick<A5, A10, Pin44> gLeftStick;

etl::FixedRingBuffer<char,128> pendingMessage;

etl::FixedRingBuffer<GCodeOperation, 8> operationsBuffer;
MotionController<SystemClock> gMotionController;

void signalError()
{
	pendingMessage.clear();
	gMotionController.stop();
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
			{
				m_state = State::comment; // Comment out the rest of the line
				gMotionController.start();
				gLed.setHigh();
			}
			break;
		}
		case State::message:
		{
			switch (c)
			{
			case '%':
				m_state = State::outOfProgram;
				gMotionController.stop();
				gLed.setLow();
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

void setup() {
	// Setup scheduler
	// Setup serial port
	Serial.begin(9600);
	Serial.println("ready");
	gLed.setLow();
}

bool moving = false;

constexpr int32_t XstepsPerMM = 200 * 16 / 2;
constexpr int32_t YstepsPerMM = int32_t(200 * 16 / (13*2*3.14159f));
constexpr int32_t ZstepsPerMM = 200 * 16 / 2;

constexpr int32_t kMaxSpeedX = 10; // mm/s
constexpr duration kMinPeriodX = std::chrono::microseconds(int32_t(1'000'000.f/(kMaxSpeedX * XstepsPerMM) + 0.5f)); // mm/s

void loop()
{
	// Consume data from the serial port
	gCodeParser.parseInput();

	if (moving)
	{
		if (gMotionController.finished())
			moving = false;
		else
		{
			gMotionController.step();
		}
	}
	else
	{
		if (!operationsBuffer.empty())
		{
			auto op = operationsBuffer.front();
			operationsBuffer.pop_front();

			if (op.address == 'G')
			{
				if (op.opCode == 30) // GO to reference
				{
					gMotionController.goHome();
					moving = true;
				}
				else if(op.opCode == 1) // Move
				{
					auto targetPos = gMotionController.getMotorPositions();
					auto srcPos = targetPos;
					if (op.argument[0] != MotionController<SystemClock>::kUnknownPos)
						targetPos.x() = op.argument[0] * XstepsPerMM;

					if (op.argument[1] != MotionController<SystemClock>::kUnknownPos)
						targetPos.y() = op.argument[1];

					if (op.argument[2] != MotionController<SystemClock>::kUnknownPos)
						targetPos.x() = op.argument[2];

					auto dt = kMinPeriodX * abs(max(1, targetPos.x() - srcPos.x()));
					gMotionController.setLinearTarget(targetPos, dt);
					moving = true;
				}
			}


		}
	}

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
	// Reset system clock
	SystemClock::now();
	setup();
	for(;;)
		loop();
	return 0;
}

#endif // SITL