#pragma once

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const char SCREEN_TITLE[] = "Inzynierka";

class WindowClass
{
private:
	GLFWwindow* window;
private:
	void initializeWindow();
	bool checkWindow();
	bool checkGlad();
public:
	WindowClass();
	~WindowClass();
	GLFWwindow* getGlfwWindow();
};