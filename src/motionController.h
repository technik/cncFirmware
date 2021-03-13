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
//#include <vector>

// Control motor stepping for all three axes and keep track of their estimated position
template<class clock_t>
class MotionController
{
public:
	using clock = clock_t;
	using duration = typename clock::duration;
	using time = typename clock::time_point;

	using XMinEndStop = Pin3::In;

	static constexpr int32_t kUnknownPos = int32_t(1ul << 31);

	MotionController();
	void start(); // Engage motors
	void stop(); // Disengate motors

	void step();
	bool finished() const { return m_targetPosition == m_curPosition; }
	const Vec3i& getMotorPositions() const { return m_curPosition; }

	// Motion operations
	void setLinearTarget(const Vec3i& targetPos, std::chrono::milliseconds dt);
	void goHome();
	// TODO: Arc movements

	//std::vector<std::pair<typename clock_t::duration,int>> tlog;

private:
	time m_t0; // Motion start time
	std::chrono::milliseconds m_dt{};
	Vec3i m_curPosition = { kUnknownPos, kUnknownPos , kUnknownPos };
	Vec3i m_targetPosition = { kUnknownPos, kUnknownPos , kUnknownPos };
	Vec3i m_srcPosition = { kUnknownPos, kUnknownPos , kUnknownPos };
	Vec3i m_arc = { 0, 0, 0 };

	template<size_t axis_, typename Motor>
	void stepAxis(Motor& motor, int goal)
	{
		if (m_curPosition.element<axis_>() >= 0)
		{
			if (goal > m_curPosition.element<axis_>())
			{
				motor.step();
				m_curPosition.element<axis_>()++;
			}
			else if (goal < m_curPosition.element<axis_>())
			{
				motor.step();
				m_curPosition.element<axis_>()--;
			}
		}
	}

	XAxisStepper MotorX;
	YAxisStepper MotorY;
	ZAxisStepper MotorZ;

	XMinEndStop EndStopMinX;
};

template<class clock_t>
MotionController<clock_t>::MotionController()
{
	// Make sure we start with motors disabled
	MotorX.disable();
	MotorY.disable();
	MotorZ.disable();
}

template<class clock_t>
void MotionController<clock_t>::start()
{
	// Make sure we start with motors disabled
	MotorX.enable();
	MotorY.enable();
	MotorZ.enable();
}

template<class clock_t>
void MotionController<clock_t>::stop()
{
	// Make sure we start with motors disabled
	MotorX.disable();
	MotorY.disable();
	MotorZ.disable();
}

template<class clock_t>
void MotionController<clock_t>::step()
{
	// Am I there yet?
	if (finished())
		return;

	// Compute instant target
	auto t = clock::now();
	auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t - m_t0);
	Vec3i dPos = m_arc * dt.count() / m_dt.count();

	auto instantTarget = m_srcPosition + dPos;
	// Do I need to move?
	//tlog.push_back({ dt,instantTarget.x() });
	if (instantTarget != m_curPosition)
	{
		// step X
		stepAxis<0>(MotorX, instantTarget.x());
		// step Y
		stepAxis<1>(MotorY, instantTarget.y());
		// step Z
		stepAxis<2>(MotorZ, instantTarget.z());
	}
}

template<class clock_t>
void MotionController<clock_t>::setLinearTarget(const Vec3i& targetPos, std::chrono::milliseconds dt)
{
	if (dt.count() == 0)
		return; // Avoid impossible operations and division by 0
	m_targetPosition = targetPos;
	m_targetPosition.x() = max(m_targetPosition.x(), 0);
	m_targetPosition.y() = max(m_targetPosition.y(), 0);
	m_targetPosition.z() = max(m_targetPosition.z(), 0);
	m_srcPosition = m_curPosition;
	m_arc = m_targetPosition - m_srcPosition;
	MotorX.setDir(m_arc.x() >= 0);
	MotorY.setDir(m_arc.y() >= 0);
	MotorZ.setDir(m_arc.z() >= 0);

	/*Serial.print("tx:");
	Serial.print(m_targetPosition.x());
	Serial.print(",cx:");
	Serial.print(m_curPosition.x());
	Serial.print(",ax:");
	Serial.print(m_arc.x());
	Serial.print(",dt:");
	Serial.println(dt.count());*/

	m_t0 = clock::now();
	m_dt = dt;
}

template<class clock_t>
void MotionController<clock_t>::goHome()
{
	m_targetPosition = Vec3i(0, 0, 0);
	m_curPosition = Vec3i(0, 0, 0);
	m_srcPosition = m_curPosition;
}

