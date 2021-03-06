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

#include <chrono>
#include <cstdint>
#include "units.h"

using namespace std::chrono_literals;

// Stepper motor steps
constexpr int microSteppingFactor = 16;
constexpr int microStepsPerRevolution = 200 * microSteppingFactor;
using MotorSteps = Revolutions<int32_t,std::ratio<1, microStepsPerRevolution>>;

using us_step = RevolutionPeriod<long, std::ratio_divide<std::micro, MotorSteps::period>>;

constexpr auto operator""_steps(unsigned long long s) {
	return MotorSteps(s);
}

constexpr auto kSteps_mmX = MotorSteps(1_rev) / 8_mm;
constexpr auto kSteps_mmY = MotorSteps(int32_t(200 * 16 / 38.f + 0.5)) / 1_mm;
constexpr auto kSteps_mmZ = MotorSteps(1_rev) / 8_mm;
/*
constexpr int32_t XstepsPerMM = 200 * 16 / 2;
constexpr int32_t YstepsPerMM = int32_t(microStepsPerRevolution / (13 * 2 * 3.14159f));
constexpr int32_t ZstepsPerMM = 200 * 16 / 2;
*/
constexpr auto kMaxSpeedX = 5_mm / 1s; // mm/s
constexpr auto kMaxSpeedY = 50_mm / 1s; // mm/s
constexpr auto kMaxSpeedZ = 5_mm / 1s; // mm/s

constexpr auto kMaxSteps_secX = kMaxSpeedX * kSteps_mmX;
constexpr auto kMaxSteps_secY = kMaxSpeedY * kSteps_mmY;
constexpr auto kMaxSteps_secZ = kMaxSpeedZ * kSteps_mmZ;

constexpr auto kMinStepPeriodX = us_step(int32_t(1'000'000.f / kMaxSteps_secX.count() + 0.5f)); // us/step
constexpr auto kMinStepPeriodY = us_step(int32_t(1'000'000.f / kMaxSteps_secY.count() + 0.5f)); // us/step
constexpr auto kMinStepPeriodZ = us_step(int32_t(1'000'000.f / kMaxSteps_secZ.count() + 0.5f)); // us/step
