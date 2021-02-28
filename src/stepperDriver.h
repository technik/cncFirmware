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

#include <hal/boards/arduinomega2560.h>
#include <Arduino.h>

template<class StepPin, class DirPin, class EnablePin>
struct StepperDriver
{
	void enable() { enablePin.setLow(); }
	void disable() { enablePin.setHigh(); }

	void setDir(bool sign)
	{
		if (sign)
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
using XAxisStepper = StepperDriver<Pin54, Pin55, Pin38>;
// using YAxisStepper = StepperDriver<Pin60,Pin61,Pin56>; // Original RAMPS mapping
using YAxisStepper = StepperDriver<Pin26, Pin28, Pin24>; // Remapping due to a few burnt traces
using ZAxisStepper = StepperDriver<Pin46, Pin48, Pin62>;