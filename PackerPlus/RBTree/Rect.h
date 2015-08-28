#pragma once
template <class T>
struct Point
{
	T x, y;

	Point(T x, T y)
		: x(x),
		  y(y)
	{
	}


	Point() :x(), y()
	{
	}

	Point(const Point<T> p) :x(p->x), y(p->y)
	{
	}
};

template <class T>
struct Rect
{
	T xMin, yMin, xMax, yMax;

	Rect() :xMin(), yMin(), xMax(), yMax()
	{
	}

	Rect(Point<T> p, T width, T height) : xMin(p.x), yMin(p.y), xMax(p.x + width), yMax(p.y + height)
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

	Point<T> center()
	{
		return new Point<T>(xMin + width() / 2, yMin + height() / 2);
	}
};
