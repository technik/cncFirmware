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
#include "HardwareConfig.h"

using namespace std::chrono_literals;

// Control motor stepping for all three axes and keep track of their estimated position
template<class clock_t>
class MotionController
{
public:
	using clock = clock_t;
	using duration = typename clock::duration;
	using time = typename clock::time_point;

	using Vec3step = Vec3<MotorSteps>;

	using XMinEndStop = Pin3::In;

	static constexpr int32_t kUnknownPos = int32_t(1ul << 31);
	static constexpr auto UnkownStep = MotorSteps(kUnknownPos);

	MotionController();
	void start(); // Engage motors
	void stop(); // Disengate motors

	void step();
	bool finished() const { return m_targetPosition == m_curPosition; }
	const Vec3step& getMotorPositions() const { return m_curPosition; }

	// Motion operations
	void setLinearTarget(const Vec3step& targetPos);
	void goHome();
	// TODO: Arc movements

	void printState() const;

	template<class Dist>
	static std::chrono::milliseconds linearArcMinDuration(const Vec3<Dist>& arc);

private:
	time m_t0; // Motion start time
	std::chrono::milliseconds m_dt{};

	Vec3step m_curPosition = { UnkownStep, UnkownStep , UnkownStep };
	Vec3step m_targetPosition = { UnkownStep, UnkownStep , UnkownStep };
	Vec3step m_srcPosition = { UnkownStep, UnkownStep , UnkownStep };
	Vec3step m_arc = {};

	template<size_t axis_, typename Motor>
	void stepAxis(Motor& motor, MotorSteps goal)
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
	
	Vec3step dPos = m_arc * dt.count() / m_dt.count();
	// Clamp target
	if (abs(dPos.x()) > m_arc.x()) dPos.x() = m_arc.x();
	if (abs(dPos.y()) > m_arc.y()) dPos.y() = m_arc.y();
	if (abs(dPos.z()) > m_arc.z()) dPos.z() = m_arc.z();

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
void MotionController<clock_t>::setLinearTarget(const Vec3step& targetPos)
{
	m_targetPosition = targetPos;
	m_targetPosition.x() = max(m_targetPosition.x(), MotorSteps(0));
	m_targetPosition.y() = max(m_targetPosition.y(), MotorSteps(0));
	m_targetPosition.z() = max(m_targetPosition.z(), MotorSteps(0));
	m_srcPosition = m_curPosition;
	m_arc = m_targetPosition - m_srcPosition;

	MotorX.setDir(m_arc.x() >= MotorSteps(0));
	MotorY.setDir(m_arc.y() >= MotorSteps(0));
	MotorZ.setDir(m_arc.z() >= MotorSteps(0));

	m_dt = linearArcMinDuration(m_arc);

	printState();

	m_t0 = clock::now();
}

template<class clock_t>
void MotionController<clock_t>::goHome()
{

	m_targetPosition = Vec3i(0, 0, 0);
	if (m_curPosition.x() == kUnknownPos)
		m_curPosition.x() = MotorSteps(0);
	if (m_curPosition.y() == kUnknownPos)
		m_curPosition.y() = MotorSteps(0);
	if (m_curPosition.z() == kUnknownPos)
		m_curPosition.z() = MotorSteps(0);
	m_srcPosition = m_curPosition;
	m_arc = m_targetPosition - m_srcPosition;
	m_dt = linearArcMinDuration(m_arc);

	MotorX.setDir(false);
	MotorY.setDir(false);
	MotorZ.setDir(false);
}

namespace mc_impl
{
	inline void printAxis(MotorSteps target, MotorSteps current, MotorSteps arc)
	{
		Serial.print("tx:");
		Serial.print(target.count());
		Serial.print(",cx:");
		Serial.print(current.count());
		Serial.print(",ax:");
		Serial.println(arc.count());
	}
}

template<class clock_t>
void MotionController<clock_t>::printState() const
{
	mc_impl::printAxis(m_targetPosition.x(), m_curPosition.x(), m_arc.x());
	mc_impl::printAxis(m_targetPosition.y(), m_curPosition.y(), m_arc.y());
	mc_impl::printAxis(m_targetPosition.z(), m_curPosition.z(), m_arc.z());

	Serial.print("dt:");
	Serial.println(m_dt.count());
}

template<class clock_t>
template<class Dist>
std::chrono::milliseconds MotionController<clock_t>::linearArcMinDuration(const Vec3<Dist>& arc)
{
	auto stepsX = MotorSteps(abs(arc.x()));
	auto minTimeX = kMinStepPeriodX * stepsX;

	auto stepsY = MotorSteps(abs(arc.y()));
	auto minTimeY = kMinStepPeriodY * stepsY;

	auto stepsZ = MotorSteps((arc.z()));
	auto minTimeZ = kMinStepPeriodZ * stepsZ;

	auto minTravelDt = max(minTimeX, max(minTimeY, minTimeZ));
	auto minTravelMillis = std::chrono::duration_cast<std::chrono::milliseconds>(minTravelDt);
	return max(1ms, minTravelMillis);
}

