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

template<class UnitTag, class Rep, class Ratio = std::ratio<1>>
struct Unit
{
	using unit_tag = UnitTag;
	using rep = Rep; // representation
	using period = Ratio; // Ratio to 1m (S.I. reference units)

	constexpr Unit() = default;
	constexpr explicit Unit(Rep _x) : x(_x) {}
	
	template<class Rep2>
	constexpr explicit Unit(Rep2 _x) : x(_x) {}

	template<class Rep2, class Period2, class relative_ratio = std::ratio_divide<Period2, Ratio>>
	constexpr explicit Unit(Unit<UnitTag, Rep2, Period2> d)
		: x(d.count() * relative_ratio::num / relative_ratio::den)
	{
	}

	Unit& operator=(const Unit& other) = default;
	
	constexpr auto count() const { return x; }

	static constexpr Unit zero() noexcept { Unit d; d.x = 0; return d; }

	constexpr auto operator+(Unit d) const
	{
		Unit result;
		result.x = x + d.x;
		return result;
	}

	constexpr auto operator-(Unit d) const
	{
		Unit result;
		result.x = x - d.x;
		return result;
	}

	constexpr auto operator*(Rep k) const
	{
		Unit result;
		result.x *= k;
		return result;
	}

	constexpr auto operator/(Rep k) const
	{
		Unit result;
		result.x /= k;
		return result;
	}

	auto& operator+=(Unit d)
	{
		x += d.x;
		return *this;
	}

	auto& operator-=(Unit d)
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

struct DistanceUnitTag {};
struct RevolutionUnitTag {};
struct TimeUnitTag {};

template<class NumT, class DenT>
struct UnitRatioTag {
	using num = NumT;
	using den = DenT;
};

using SpeedUnitTag = UnitRatioTag<DistanceUnitTag, TimeUnitTag>;

template<class UnitT>
struct unit_traits
{
	using tag = UnitT::unit_tag;
};

// Unit adaptor to be able to use std::chrono::duration in unit operations
template<class Rep, class Ratio>
struct unit_traits<std::chrono::duration<Rep, Ratio>>
{
	using tag = TimeUnitTag;
};

// Distance unit (S.I. units), ratio relative to 1 meter
template<class Rep, class Ratio = std::ratio<1>>
using Distance = Unit<DistanceUnitTag, Rep, Ratio>;

// Revolution units, ratio relative to one full revolution
template<class Rep, class Ratio = std::ratio<1>>
using Revolutions = Unit<RevolutionUnitTag, Rep, Ratio>;

// Speed unit (S.I. units), ratio relative to 1 meter/second
template<class Rep, class Ratio = std::ratio<1>>
using Speed = Unit<SpeedUnitTag, Rep, Ratio>;

// Define common units and literal suffixes
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

constexpr auto operator""_rev(unsigned long long s) {
	return Revolutions<int32_t>(s);
}

// Operations with mixed units

template<class Dist_t, class Time_t>
constexpr auto operator/(Dist_t dist, Time_t t)
{
	using SpeedRep = decltype(dist.count() / t.count());
	using SpeedPeriod = std::ratio_divide<Dist_t::period, Time_t::period>;
	return Speed<SpeedRep, SpeedPeriod>(dist.count() / t.count());
}

template<
	class DivT, class DenT,
	class Rep1 = DivT::rep, class Period1 = DivT::period,
	class Rep2 = DenT::rep, class Period2 = DenT::period,
	class NumTag = unit_traits<DivT>::tag::num, class DenTag = unit_traits<DenT>::tag
>
constexpr auto operator*(
	DivT a,
	DenT b)
{
	using ResultRep = decltype(a.count() * b.count());
	using ResultPeriod = std::ratio_multiply<Period1, Period2>;
	return Unit<NumTag, ResultRep, ResultPeriod>(a.count() * b.count());
}

template< // Num/Mid * Mid/Den -> Num/Den
	class NumTag, class Rep1, class Period1,
	class DenTag, class Rep2, class Period2,
	class MidTag
>
constexpr auto operator*(
	Unit<UnitRatioTag<NumTag, MidTag>, Rep1, Period1> a,
	Unit<UnitRatioTag<MidTag, DenTag>, Rep2, Period2> b)
{
	using ResultRep = decltype(a.count()* b.count());
	using ResultPeriod = std::ratio_multiply<Period1, Period2>;
	return Unit<UnitRatioTag<NumTag,DenTag>, ResultRep, ResultPeriod>(a.count() * b.count());
}

template<class NumT, class DenT,
	class NumTag = unit_traits<NumT>::tag, class NumRep = NumT::rep, class NumPeriod = NumT::period,
	class DenTag = unit_traits<DenT>::tag, class DenRep = DenT::rep, class DenPeriod = DenT::period
>
constexpr auto operator/(Unit<NumTag, NumRep, NumPeriod> a, Unit<DenTag, DenRep, DenPeriod> b)
{
	using ResultRep = decltype(a.count() / b.count());
	using ResultPeriod = std::ratio_divide<NumPeriod, DenPeriod>;
	return Unit<UnitRatioTag<NumTag, DenTag>, ResultRep, ResultPeriod>(a.count() / b.count());
}