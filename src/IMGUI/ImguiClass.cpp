#include "ImguiClass.h"
#include <iostream>

ImguiClass::ImguiClass(GLFWwindow *window)
{
	this->window = window;
	
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return;
	}

	const char* glsl_version = "#version 430";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();
}

ImguiClass::~ImguiClass()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImguiClass::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImguiClass::drawApplicationDataWindow(const std::string & GPU, const std::string & CPU, const std::string & memory, const std::string & usingMemory)
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::Begin("Application Data", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("CPU: %s", CPU.c_str());
	ImGui::Text("GPU: %s", GPU.c_str());
	ImGui::Text("Using memory: %s / %s MB", usingMemory.c_str(), memory.c_str());
	ImGui::End();
}

void ImguiClass::drawParametersSettingsWindow(double & density, double & topHeight, double & rayleigh, double & mie)
{
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth(), 0), ImGuiCond_Once);
	ImGui::Begin("Atmosphere settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	setCursorMode();
	setDensity(density);
	setTopHeight(topHeight);
	setRayleighConstant(rayleigh);
	setMieConstant(mie);
	ImGui::End();
}

void ImguiClass::setDensity(double & density)
{
	ImGui::Text("Set Density");
	const float sliderDensityInitialValue = density;
	float sliderDensity = sliderDensityInitialValue;
	ImGui::SliderFloat("Density", &sliderDensity, 0.5f, 2.5f);
	//if (sliderDensity != sliderDensityInitialValue) {
	if ((abs(sliderDensity - sliderDensityInitialValue) > 0.4)) {
		density = sliderDensity;
	}
}

void ImguiClass::setTopHeight(double & topHeight)
{
	ImGui::Text("Set top height");
	const float sliderkTopInitialValue = topHeight;
	float sliderkTop = sliderkTopInitialValue;
	ImGui::SliderFloat("Top height", &sliderkTop, 6400000.0f, 6500000.0f);
	//if (sliderkTop != sliderkTopInitialValue)
	if (abs(sliderkTop - sliderkTopInitialValue) > 20000.0) {
		topHeight = sliderkTop;
	}
}

void ImguiClass::setRayleighConstant(double & rayleigh)
{
	ImGui::Text("Set rayleigh constant");
	const float sliderkRayInitialValue = rayleigh * 10000.0;
	float sliderkRay = sliderkRayInitialValue;
	ImGui::SliderFloat("Rayleigh constant", &sliderkRay, 0.5e-2, 4.0e-2);
	//if (sliderkRay != sliderkRayInitialValue)
	if (abs(sliderkRay - sliderkRayInitialValue) > 0.005) {
		rayleigh = sliderkRay / 10000.0;
	}
}

void ImguiClass::setMieConstant(double & mie)
{
	ImGui::Text("Set Mie constant");
	const float sliderkMieInitialValue = mie;
	float sliderkMie = sliderkMieInitialValue;
	ImGui::SliderFloat("Mie constant", &sliderkMie, 1.328e-3, 9.328e-3);
	//if (sliderkMie != sliderkMieInitialValue)
	if (abs(sliderkMie - sliderkMieInitialValue) > 0.0015) {
		mie = sliderkMie;
	}
}

void ImguiClass::renderDrawData(const std::string & GPU, const std::string & CPU, const std::string & memory, const std::string & usingMemory,
								double & density, double & topHeight, double & rayleigh, double & mie)
{
	newFrame();
	drawParametersSettingsWindow(density, topHeight, rayleigh, mie);
	drawApplicationDataWindow(GPU, CPU, memory, usingMemory);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui::EndFrame();
}


void ImguiClass::setCursorMode()
{
	ImGui::Text("Press P to disable cursor");
	ImGui::Text("Use this option to change parameter of atmosphere");
	auto lambda = [](GLFWwindow * window) -> bool { return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? false : true; };
	bool checkBox = lambda(this->window);
	ImGui::Checkbox("Is P pressed", &checkBox);
}
