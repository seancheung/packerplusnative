#pragma once
template <class T>
struct Rect
{
	T xMin, yMin, xMax, yMax;

	Rect() :xMin(), yMin(), xMax(), yMax()
	{
	}

	Rect(T x, T y, T width, T height) : xMin(x), yMin(y), xMax(x + width), yMax(y + height)
	{
	}

	T width()
	{
		return xMax - xMin;
	}

	T height()
	{
		return yMax - yMin;
	}

	~Rect()
	{
	}
};
