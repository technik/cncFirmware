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
#include <cassert>
#include "../src/motionController.h"
#include <chrono>
#include <vector>

using namespace std::chrono_literals;

void testStartUnknown()
{
	MotionController<RealTimeClock> mc;
	mc.start();
	auto startPos = mc.getMotorPositions();
	assert(startPos.x() == MotionController<RealTimeClock>::kUnknownPos);
	assert(startPos.y() == MotionController<RealTimeClock>::kUnknownPos);
	assert(startPos.z() == MotionController<RealTimeClock>::kUnknownPos);
	// Also check there is no ongoing operation on start
	assert(mc.finished());
}

void testGoHome()
{
	MotionController<RealTimeClock> mc;
	mc.start();
	mc.goHome();
	while (!mc.finished())
	{
		mc.step();
	}
	auto homePos = mc.getMotorPositions();
	assert(homePos.x() == 0);
	assert(homePos.y() == 0);
	assert(homePos.z() == 0);
}

void testPositiveMotionX(int32_t steps, std::chrono::milliseconds deadline)
{
	using clock = RealTimeClock;
	MotionController<clock> mc;
	mc.start();
	mc.goHome();
	while (!mc.finished())
	{
		mc.step();
	}
	// Move a short distance along the X axis
	const auto targetPos = Vec3i(steps,0,0);
	mc.setLinearTarget(targetPos, deadline);
	auto t0 = clock::now();
	while (clock::now() - t0 <= deadline+1ms)
	{
		if (mc.finished())
			break;
		mc.step();
	}
	auto finalPos = mc.getMotorPositions();
	assert(targetPos == finalPos);
}

int main()
{
	//testStartUnknown();
	//testGoHome();
	// Initialize the real time clock for time sensitive tests
	RealTimeClock::now();
	testPositiveMotionX(1, 10ms);
	testPositiveMotionX(100, 1001ms);
	testPositiveMotionX(80000, 10'001ms);
	// Initialize the mock clock
	MockClockSrc::currentTime = MockClockSrc::time_point(1ms);
	MockClockSrc::now();
	// TODO Mock time tests
}