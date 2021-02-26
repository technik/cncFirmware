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
#pragma once

template<class Pin>
struct InputButton
{
	void read()
	{
		m_state = (m_state << 1) & FullStateMask;
		m_state |= m_pin ? 1 : 0;
	}
	bool pressed() const { return m_state == CurStateMask; }
	bool held() const { return m_state == FullStateMask; }
	bool released() const { return m_state == LastStateMask; };

private:
	static constexpr uint8_t CurStateMask = 1;
	static constexpr uint8_t LastStateMask = (1 << 1);
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

	Axis xAxis{ pinX };
	Axis yAxis{ pinY };
	InputButton<ButtonPin> button;
};
