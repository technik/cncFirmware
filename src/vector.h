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

template<class T, int N>
struct Vector
{
	T x() const { static_assert(N > 0); return m[0]; }
	T y() const { static_assert(N > 1); return m[1]; }
	T z() const { static_assert(N > 2); return m[2]; }
	T w() const { static_assert(N > 3); return m[3]; }

	T& x() { static_assert(N > 0); return m[0]; }
	T& y() { static_assert(N > 1); return m[1]; }
	T& z() { static_assert(N > 2); return m[2]; }
	T& w() { static_assert(N > 3); return m[3]; }

	T& operator[](size_t i) { return m[i]; }
	T operator[](size_t i) const { return m[i]; }

	Vector() = default;
	Vector(const Vector&) = default;
	template<class T2>
	Vector(const Vector<T2, N>& other)
	{
		for (int i = 0; i < N; ++i)
		{
			m[i] = other[i];
		}
	}
	Vector(T _x, T _y) : m{ _x, _y } { static_assert(N > 1); }
	Vector(T _x, T _y, T _z) : m{ _x, _y,_z } { static_assert(N > 2); }
	Vector(T _x, T _y, T _z, T _w) : m{ _x, _y, _w, _w } { static_assert(N > 3); }

private:
	T m[N];
};

template<class T1, class T2, int N>
auto operator*(Vector<T1,N> v, T2 x)
{
	using MulT = decltype(v[0]*x);
	Vector<MulT, N> res;
	for (int i = 0; i < N; ++i)
	{
		res[i] = v[i]*x;
	}
	return res;
}

template<class T1, class T2, int N>
auto operator*(T1 x, Vector<T2, N> v)
{
	using MulT = decltype(x * v[0]);
	Vector<MulT, N> res;
	for (int i = 0; i < N; ++i)
	{
		res[i] = x * v[i];
	}
	return res;
}

template<class T1, class T2, int N>
auto operator/(Vector<T1, N> v, T2 x)
{
	using DivT = decltype(v[0] / x);
	Vector<DivT, N> res;
	for (int i = 0; i < N; ++i)
	{
		res[i] = v[i] / x;
	}
	return res;
}

template<class T, int N>
Vector<T, N> operator+(Vector<T, N> a, Vector<T, N> b)
{
	Vector<T, N> res;
	for (int i = 0; i < N; ++i)
	{
		res[i] = a[i] + b[i];
	}
	return res;
}

template<class T, int N>
Vector<T, N> operator-(Vector<T, N> a, Vector<T, N> b)
{
	Vector<T, N> res;
	for (int i = 0; i < N; ++i)
	{
		res[i] = a[i] - b[i];
	}
	return res;
}

template<class T, int N>
bool operator==(Vector<T, N> a, Vector<T, N> b)
{
	Vector<T, N> res;
	bool eq = true;
	for (int i = 0; i < N; ++i)
	{
		eq &= a[i] == b[i];
	}
	return eq;
}

template<class T, int N>
bool operator!=(Vector<T, N> a, Vector<T, N> b)
{
	Vector<T, N> res;
	bool dif = false;
	for (int i = 0; i < N; ++i)
	{
		dif |= a[i] != b[i];
	}
	return dif;
}

template<class T> using Vec2 = Vector<T, 2>;
template<class T> using Vec3 = Vector<T, 3>;
template<class T> using Vec4 = Vector<T, 4>;

using Vec2i = Vec2<int32_t>;
using Vec3i = Vec3<int32_t>;
using Vec4i = Vec4<int32_t>;
