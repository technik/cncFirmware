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
#include "motionController.h"
#include <Arduino.h>

MotionController::MotionController()
{
	// Make sure we start with motors disabled
	MotorX.disable();
	MotorY.disable();
	MotorZ.disable();
}

void MotionController::start()
{
	// Make sure we start with motors disabled
	MotorX.enable();
	MotorY.enable();
	MotorZ.enable();
}

void MotionController::stop()
{
	// Make sure we start with motors disabled
	MotorX.disable();
	MotorY.disable();
	MotorZ.disable();
}

void MotionController::step()
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
	Vec3i dPos = (m_targetPosition - m_srcPosition) * dt.count() / m_dt.count();
	
	auto instantTarget = m_srcPosition + dPos;
	// Do I need to move?
	if (instantTarget != m_curPosition)
	{
		// step X
		if (m_curPosition.x() >= 0)
		{
			if (instantTarget.x() > m_curPosition.x())
			{
				MotorX.step();
				m_curPosition.x()++;
#ifdef SITL
				std::cout << "X++\n";
#endif
			}
			else if (instantTarget.x() < m_curPosition.x())
			{
				MotorX.step();
				m_curPosition.x()--;
#ifdef SITL
				std::cout << "X--\n";
#endif
			}
		}
	}
}

void MotionController::setLinearTarget(const Vec3i& targetPos, duration dt)
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
	Serial.print("G1 X");
	Serial.print(m_targetPosition.x());
	Serial.print(";cx:");
	Serial.println(m_curPosition.x());
}

void MotionController::goHome()
{
	m_targetPosition = Vec3i(0, 0, 0);
	m_curPosition = Vec3i(0,0,0);
	m_srcPosition = m_curPosition;
}