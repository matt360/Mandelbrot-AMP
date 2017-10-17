#pragma once
#include <amp_math.h>

// using our own structure as Complex function not available in the Concurrency namespace
struct Complex 
{
	float x;
	float y;
};

// c_add
Complex c_add(Complex c1, Complex c2) restrict(cpu, amp) // restrict keyword - able to execute this function on the GPU and CPU
{
	Complex tmp;

	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a + c;
	tmp.y = b + d;

	return tmp;
}

// c_abs
float c_abs(Complex c) restrict(cpu, amp)
{
	return concurrency::fast_math::sqrt(c.x*c.x + c.y*c.y);
}

// c_mul
Complex c_mul(Complex c1, Complex c2) restrict(cpu, amp)
{
	Complex tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a*c - b*d;
	tmp.y = b*c + a*d;
	return tmp;
}
