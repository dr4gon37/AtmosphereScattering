#pragma once

#include "IMGUI/ImguiHeader.h"
#include <cstdio>
#include <string>

class ImguiClass
{
private:
	GLFWwindow * window;
public:
	ImguiClass() = delete;
	explicit ImguiClass(GLFWwindow * window);
	~ImguiClass();
	void newFrame();
	void renderDrawData(const std::string & GPU, const std::string & CPU, const std::string & memory, const std::string & usingMemory, 
						double & density, double & topHeight, double & rayleigh, double & mie);

private:
	void inline drawApplicationDataWindow(const std::string & GPU, const std::string & CPU, const std::string & memory, const std::string & usingMemory);
	void inline drawParametersSettingsWindow(double & density, double & topHeight, double & rayleigh, double & mie);
	void inline setDensity(double & density);
	void inline setTopHeight(double & topHeight);
	void inline setRayleighConstant(double & rayleigh);
	void inline setMieConstant(double & mie);
	void inline setCursorMode();
};
