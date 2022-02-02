#pragma once
#include "GLFW/glfw3.h"
#include <ENGINE/Engine.h>
#include <algorithm>

class EngineInputFunctions
{
	explicit EngineInputFunctions::EngineInputFunctions(GLFWwindow*window)
	{
		this->window = window;
	}
public:
	bool getFirstMouseValue() const { return this->firstMouse; };
	bool getCanMovingMouseValue() const { return this->canMovingMouse; };
private:
	GLFWwindow*window;
	template<class T>
	friend class InputEngine;
	bool firstMouse = true;
	bool canMovingMouse = true;

	void escapeKey() const
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	
	void mouseCursor() 
	{
		canMovingMouse = !canMovingMouse;
		if (canMovingMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			firstMouse = true;
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = false;
		}
	}
};
