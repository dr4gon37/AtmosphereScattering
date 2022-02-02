#include "SunDirection.h"

SunDirection::SunDirection(GLFWwindow * window, double sunZenithAngleRadians, double sunAzimuthAngleRadians)
{
	this->window = window;
	this->sunZenithAngleRadians = sunZenithAngleRadians;
	this->sunAzimuthAngleRadians = sunAzimuthAngleRadians;
}


void SunDirection::handleMouseClickEvent(int button, int action, int mods) {
	double x;
	double y;

	glfwGetCursorPos(window, &x, &y);

	previousMouseX = x;
	previousMouseY = y;
}

void SunDirection::handleMouseDragEvent(double mouseX, double mouseY) {
	constexpr double kScale = 500.0;
	
	sunZenithAngleRadians -= (previousMouseY - mouseY) / kScale;
	sunZenithAngleRadians =
		std::max(0.0, std::min(kPi, sunZenithAngleRadians));
	sunAzimuthAngleRadians += (previousMouseX - mouseX) / kScale;

	previousMouseX = mouseX;
	previousMouseY = mouseY;
	
}