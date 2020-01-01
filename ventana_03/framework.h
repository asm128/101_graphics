#pragma once

#include <cstdint>
#include <cmath>

#pragma pack(push, 1)
// Use this structure to simplify the use of 2d values.
struct SCoord {
	int							x;
	int							y;

	// This operator allows us to scale the vector components in a single step
	SCoord						operator /				(double scalar)		const	{ return SCoord{(int)(x / scalar), (int)(y / scalar)}; }

	// This method allows us to obtain the length of the vector, in order to calculate later the distance between two points (pixels in this case)
	double						Length					()					const	{
		double							sqlen					= x * (double)x + y * (double)y;	// squared length
		return sqrt(sqlen);	// square root of sqlen (square length)
	}
};

// Use this structure to simplify the use of color values.
struct SColor {
	uint8_t						b;
	uint8_t						g;
	uint8_t						r;
	uint8_t						a;

	// These operators allow us to scale the colors in a single step
	SColor						operator /				(double scalar)		const	{ return SColor{(uint8_t)(b / scalar), (uint8_t)(g / scalar), (uint8_t)(r / scalar)}; }
	SColor						operator *				(double scalar)		const	{ return SColor{(uint8_t)(b * scalar), (uint8_t)(g * scalar), (uint8_t)(r * scalar)}; }
};
#pragma pack(pop)

// This function copies the in-memory pixels to the window's client area.
int							drawBuffer				(::HDC hdc, const SColor* sourcePixels, SCoord sizeTarget);
