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

using namespace std::chrono_literals;

void testStartUnknown()
{
	MotionController mc;
	mc.start();
	auto startPos = mc.getMotorPositions();
	assert(startPos.x() == MotionController::kUnknownPos);
	assert(startPos.y() == MotionController::kUnknownPos);
	assert(startPos.z() == MotionController::kUnknownPos);
	// Also check there is no ongoing operation on start
	assert(mc.finished());
}

void testGoHome()
{
	MotionController mc;
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

void testPositiveShortMotionX()
{
	MotionController mc;
	mc.start();
	mc.goHome();
	while (!mc.finished())
	{
		mc.step();
	}
	// Move a short distance along the X axis
	const auto targetPos = Vec3i(1,0,0);
	mc.setLinearTarget(targetPos, 1s);
	auto t0 = MotionController::clock::now();
	while (!mc.finished() && (MotionController::clock::now() - t0 < 1001ms))
	{
		mc.step();
	}
	auto finalPos = mc.getMotorPositions();
	assert(targetPos == finalPos);
}

void testPositiveLongMotionX()
{
	MotionController mc;
	mc.start();
	mc.goHome();
	while (!mc.finished())
	{
		mc.step();
	}
	// Move a short distance along the X axis
	const auto targetPos = Vec3i(100, 0, 0);
	mc.setLinearTarget(targetPos, 1s);
	auto t0 = MotionController::clock::now();
	while (!mc.finished() && (MotionController::clock::now() - t0 < 1001ms))
	{
		mc.step();
	}
	auto finalPos = mc.getMotorPositions();
	assert(targetPos == finalPos);
}

int main()
{
	testStartUnknown();
	testGoHome();
	testPositiveShortMotionX();
	testPositiveLongMotionX();
}