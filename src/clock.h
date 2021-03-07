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

#include <cstddef>
#include <chrono>

#ifdef WIN32
template<class baseClock>
struct AtmegaEmulatedClock
{
	using rep = int32_t;
	using period = std::chrono::microseconds::period;
	using duration = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<AtmegaEmulatedClock<baseClock>>;

	static constexpr bool is_steady = true;

	static time_point now() noexcept
	{
		// Emulate arduino's internal clock mechanism
		// This should be equivalent to how the micros() function reconstructs time,
		// and so reproduce the same kind of quicks and overflows
		constexpr uint32_t FCPU = 16'000'000; // 16MHz
		constexpr uint32_t clockCyclesPerMicrosecond = FCPU / 1'000'000;
		constexpr uint32_t timerDiv = 64;
		constexpr uint32_t timerCapacity = 256;

		// Use time from first call as an approximation to time from start in the device
		using implClock = std::chrono::steady_clock;
		static auto t0 = implClock::now();
		auto timeFromStart = implClock::now() - t0;

		// Emulate the state of internal variables used to track timer overflows in arduino
		const uint64_t nsFromStart = timeFromStart.count();
		const uint64_t ticksFromStart = nsFromStart * 16 / 1000; // 16 ticks per microsecond
		const uint32_t timer0_overflow_count = uint32_t(ticksFromStart / (timerDiv * timerCapacity));
		const uint8_t tcnt = uint8_t(ticksFromStart/timerDiv); // Timer counter register

		uint32_t result = ((timer0_overflow_count << 8) + tcnt) * (timerDiv / clockCyclesPerMicrosecond);

		return time_point(duration(result));
	}
};

using RealTimeClock = AtmegaEmulatedClock<std::chrono::steady_clock>;

struct MockClockSrc
{
	using rep = RealTimeClock::rep;
	using period = std::chrono::steady_clock::period;
	using duration = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<MockClockSrc>;

	static constexpr bool is_steady = true;

	static time_point now() noexcept
	{
		return currentTime;
	}

	inline static time_point currentTime;
};
using MockClock = AtmegaEmulatedClock<MockClockSrc>;

using SystemClock = AtmegaEmulatedClock<std::chrono::steady_clock>;;
#else
struct SystemClock
{
	using rep = int32_t;
	using period = std::chrono::microseconds::period;
	using duration = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<SystemClock>;

	static constexpr bool is_steady = true;

	static time_point now() noexcept
	{
		return time_point(duration(micros()));
	}
};
#endif