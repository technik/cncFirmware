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

#include "clock.h"
#include "stepperDriver.h"
#include "vector.h"

// Control motor stepping for all three axes and keep track of their estimated position
class MotionController
{
public:
	using clock = SystemClock;
	using duration = clock::duration;
	using time = clock::time_point;

	using XMinEndStop = Pin3::In;

	static constexpr int32_t kUnknownPos = int32_t(1ul << 31);

	MotionController();
	void start(); // Engage motors
	void stop(); // Disengate motors

	void step();
	bool finished() const { return m_targetPosition == m_curPosition; }
	const Vec3i& getMotorPositions() const { return m_curPosition; }

	// Motion operations
	void setLinearTarget(const Vec3i& targetPos, duration dt);
	void goHome();
	// TODO: Arc movements

private:
	time m_t0; // Motion start time
	duration m_dt{};
	Vec3i m_curPosition = { kUnknownPos, kUnknownPos , kUnknownPos };
	Vec3i m_targetPosition = { kUnknownPos, kUnknownPos , kUnknownPos };
	Vec3i m_srcPosition = { kUnknownPos, kUnknownPos , kUnknownPos };

	XAxisStepper MotorX;
	YAxisStepper MotorY;
	ZAxisStepper MotorZ;

	XMinEndStop EndStopMinX;
};

