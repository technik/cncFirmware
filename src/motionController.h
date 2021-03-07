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
	void setLinearTarget(const Vec3i& targetPos, duration dt);
	void goHome();
	// TODO: Arc movements

	//std::vector<std::pair<typename clock_t::duration,int>> tlog;

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

	// Check end stops
	if (EndStopMinX.isHigh())
	{
		//m_curPosition.x() = 0;
	}

	// Compute instant target
	auto t = clock::now();
	auto dt = t - m_t0;
	auto arc = m_targetPosition - m_srcPosition;
	Vec3i dPos = arc * dt.count() / m_dt.count();

	auto instantTarget = m_srcPosition + dPos;
	// Do I need to move?
	//tlog.push_back({ dt,instantTarget.x() });
	if (instantTarget != m_curPosition)
	{
		//std::cout << "dt:" << dt.count() << ",ix:" << instantTarget.x() << ",x:" << m_curPosition.x() << std::endl;
		// step X
		if (m_curPosition.x() >= 0)
		{
			if (instantTarget.x() > m_curPosition.x())
			{
				MotorX.step();
				m_curPosition.x()++;
			}
			else if (instantTarget.x() < m_curPosition.x())
			{
				MotorX.step();
				m_curPosition.x()--;
			}
		}
	}
}

template<class clock_t>
void MotionController<clock_t>::setLinearTarget(const Vec3i& targetPos, duration dt)
{
	m_targetPosition = targetPos;
	m_targetPosition.x() = max(m_targetPosition.x(), 0);
	m_targetPosition.y() = max(m_targetPosition.y(), 0);
	m_targetPosition.z() = max(m_targetPosition.z(), 0);
	m_srcPosition = m_curPosition;
	MotorX.setDir(m_targetPosition.x() >= m_curPosition.x());
	MotorY.setDir(m_targetPosition.y() >= m_curPosition.y());
	MotorZ.setDir(m_targetPosition.z() >= m_curPosition.z());

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

