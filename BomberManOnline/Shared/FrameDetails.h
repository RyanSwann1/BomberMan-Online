#pragma once

struct FrameDetails
{
	FrameDetails(int height, int width, int y, int x, int ID)
		: height(height),
		width(width),
		y(y),
		x(x),
		ID(ID)
	{}

	int height;
	int width;
	int y;
	int x;
	int ID;
};