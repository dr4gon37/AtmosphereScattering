#include "WindowClass.h"

void WindowClass::initializeWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor *primary = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(primary);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	this->window = glfwCreateWindow(mode->width, mode->height, SCREEN_TITLE, NULL, NULL);
	glfwSetWindowAspectRatio(window, 16, 9);
	//this->window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE, NULL, NULL);







	if (checkWindow())
	{
		glfwMakeContextCurrent(this->window);
		//glfwSwapInterval(1);
	}
	if (!checkGlad())
	{
		exit(2);
	}
	glViewport(0, 0, mode->width, mode->height);
	//glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

}

bool WindowClass::checkWindow()
{
	if (this->window == NULL)
	{
		std::cout << "Failed to create GLFW window " << std::endl;
		glfwTerminate();
		return false;
	}
	return true;
}

bool WindowClass::checkGlad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	return true;
}


WindowClass::WindowClass()
{
	initializeWindow();
}


WindowClass::~WindowClass()
{
	//glfwDestroyWindow(this->window);
}

GLFWwindow * WindowClass::getGlfwWindow()
{
	return this->window;
}
