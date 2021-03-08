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

#include <ratio>
#include <chrono> // for speed units

template<class Rep, class Ratio = std::ratio<1>>
struct Distance
{
	using rep = Rep; // representation
	using ratio = Ratio; // Ratio to 1m (S.I. reference units)

	constexpr Distance() = default;
	constexpr explicit Distance(Rep _x) : x(_x) {}
	
	template<class Rep2>
	constexpr explicit Distance(Rep2 _x) : x(_x) {}

	template<class Rep2, class Period2, class relative_ratio = std::ratio_divide<Period2, Ratio>>
	constexpr Distance(const Distance<Rep2, Period2>& d)
		: x(d.x * relative_ratio::num / relative_ratio::den)
	{
	}

	Distance& operator=(const Distance& other) = default;
	
	constexpr auto count() const { return x; }

	static constexpr Distance zero() noexcept { Distance d; d.x = 0; return d; }

	constexpr auto operator+(Distance d) const
	{
		Distance result;
		result.x = x + d.x;
		return result;
	}

	constexpr auto operator-(Distance d) const
	{
		Distance result;
		result.x = x - d.x;
		return result;
	}

	constexpr auto operator*(Rep k) const
	{
		Distance result;
		result.x *= k;
		return result;
	}

	constexpr auto operator/(Rep k) const
	{
		Distance result;
		result.x /= k;
		return result;
	}

	auto& operator+=(Distance d)
	{
		x += d.x;
		return *this;
	}

	auto& operator-=(Distance d)
	{
		x -= d.x;
		return *this;
	}

	auto& operator*=(Rep k)
	{
		x *= k;
		return *this;
	}

	auto& operator/=(Rep k)
	{
		x /= k;
		return *this;
	}

private:
	Rep x;
};

using micrometers = Distance<long, std::micro>;
using millimeters = Distance<long, std::milli>;
using centimeters = Distance<long, std::ratio<1, 100>>;
using meters = Distance<long>;

constexpr auto operator""_um(unsigned long long s) {
	return micrometers(s);
}

constexpr auto operator""_mm(unsigned long long s) {
	return millimeters(s);
}

constexpr auto operator""_cm(unsigned long long s) {
	return centimeters(s);
}

constexpr auto operator""_m(unsigned long long s) {
	return meters(s);
}

template<class Rep, class Ratio = std::ratio<1>>
struct Revolutions
{
	using rep = Rep; // representation
	using ratio = Ratio; // Ratio to 1 revolution

	constexpr Revolutions() = default;
	constexpr explicit Revolutions(Rep _x) : x(_x) {}

	template<class Rep2>
	constexpr explicit Revolutions(Rep2 _x) : x(_x) {}

	template<class Rep2, class Period2, class relative_ratio = std::ratio_divide<Period2, Ratio>>
	constexpr Revolutions(const Revolutions<Rep2, Period2>& d)
		: x(d.x* relative_ratio::num / relative_ratio::den)
	{
	}

	Revolutions& operator=(const Revolutions& other) = default;

	constexpr auto count() const { return x; }

	static constexpr Revolutions zero() noexcept { Revolutions d; d.x = 0; return d; }

	constexpr auto operator+(Revolutions d) const
	{
		Revolutions result;
		result.x = x + d.x;
		return result;
	}

	constexpr auto operator-(Revolutions d) const
	{
		Revolutions result;
		result.x = x - d.x;
		return result;
	}

	constexpr auto operator*(Rep k) const
	{
		Revolutions result;
		result.x *= k;
		return result;
	}

	constexpr auto operator/(Rep k) const
	{
		Distance result;
		result.x /= k;
		return result;
	}

	auto& operator+=(Revolutions d)
	{
		x += d.x;
		return *this;
	}

	auto& operator-=(Revolutions d)
	{
		x -= d.x;
		return *this;
	}

	auto& operator*=(Revolutions k)
	{
		x *= k;
		return *this;
	}

	auto& operator/=(Revolutions k)
	{
		x /= k;
		return *this;
	}

private:
	Rep x;
};

constexpr auto operator""_rev(unsigned long long s) {
	return Revolutions(s);
}