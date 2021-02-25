// Disable unintentional include of time.h in WIN32, which redefines clock
#define _INC_TIME
#include <Arduino.h>
#include <hal/gpio_pin.h>
#include <staticRingBuffer.h>
#include <chrono>
#include <utility>
#include <hal/boards/arduinomega2560.h>

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

	enum class Axis {
		X, Y, Z
	};

	void setStepDir(bool sign);
	void resetPosition();
	void configMicroStepping();

	struct StepperState
	{
		duration stepPeriod;
		int32_t pendingSteps;
		int32_t position;
	};
};

template<class Pin>
struct InputButton
{
	void read()
	{
		m_state = (m_state<<1) & FullStateMask;
		m_state |= m_pin?1:0;
	}
	bool pressed() const { return m_state == CurStateMask; }
	bool held() const { return m_state == FullStateMask; }
	bool released() const { return m_state == LastStateMask; };

private:
	static constexpr uint8_t CurStateMask = 1;
	static constexpr uint8_t LastStateMask = (1<<1);
	static constexpr uint8_t FullStateMask = CurStateMask | LastStateMask;
	// bit 0: current state; bit 1: past state
	uint8_t m_state{};
	typename Pin::In m_pin;
};

template<int pinX, int pinY, class ButtonPin>
struct AnalogJoystick
{
	struct Axis
	{
		Axis(int pin)
			: m_pin(pin)
		{
			pinMode(pin, INPUT);
			m_center = analogRead(pin);
		}

		void read()
		{
			m_pos = analogRead(m_pin);
		}

		int m_pin;
		int m_pos;
		uint16_t m_center;
		uint16_t m_max;
		uint16_t m_min;
	};

	void read()
	{
		xAxis.read();
		yAxis.read();
		button.read();
	}

	Axis xAxis{pinX};
	Axis yAxis{pinY};
	InputButton<ButtonPin> button;
};

LedPin gLed;

using XMinEndStop = Pin3::In;

XMinEndStop gEndStopMinX;

void setup() {
	// Setup scheduler
	// Setup serial port
	Serial.begin(9600);
	// Start motor control
	gMotorY.setDir(false);
	gMotorX.enable();
	gMotorY.enable();
	gMotorZ.enable();
	// Start message processing
	// executeGCode();
}

AnalogJoystick<A5, A10, Pin44> gLeftStick;

void loop() {
	
	/*delay(1000);
	Serial.print("X:");
	Serial.print(gLeftStick.xAxis.m_pos);
	Serial.print(" Y:");
	Serial.println(gLeftStick.yAxis.m_pos);
	*/
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

	gLeftStick.read();
	gMotorX.step();
	//gMotorY.step();
	//gMotorZ.step();
	delay(1);
	if(gEndStopMinX)
		gMotorY.setDir(true);
  // Consume work from the task queue
  // Global work flow looks like:
  // - Interrupt from hardware stops
  // - Interrupt from motor control
  // - Consume data from the serial port. Interpret the G-Code and write ack to it
}

#ifdef SITL

int main()
{
	setup();
	for(;;)
		loop();
	return 0;
}

#endif // SITL