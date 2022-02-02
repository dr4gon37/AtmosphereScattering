#pragma once
#include "ENGINE/WindowClass.h"
#include <algorithm>

class SunDirection
{
private:
	GLFWwindow * window;
	double kPi = 3.1415926;
public:
	double sunZenithAngleRadians;
	double sunAzimuthAngleRadians;

	int previousMouseX;
	int previousMouseY;
public:
	SunDirection(GLFWwindow * window, double sunZenithAngleRadians, double sunAzimuthAngleRadians);
	~SunDirection() = default;

	void handleMouseClickEvent(int button, int action, int mods);
	void handleMouseDragEvent(double mouseX, double mouseY);
};

