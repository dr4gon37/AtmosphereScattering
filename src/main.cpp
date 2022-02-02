#include <stdio.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include "ENGINE/Engine.h"


#include <glad/glad.h>
#include <GL/freeglut.h>
#include <memory>



int main()
{
	try
	{
		Engine engine;
		engine.run();
		
		glfwTerminate();
		return EXIT_SUCCESS;		
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
}





