#include "Engine.h"
#include <glad/glad.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <future>

bool firstMouse;
bool canMovingMouse;
double lastX;
double lastY;

constexpr double kPi = 3.1415926;
constexpr double kSunAngularRadius = 0.00935 / 2.0;
constexpr double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
constexpr double kLengthUnitInMeters = 1000.0;


const char kVertexShader[] = R"(
    #version 330
    uniform mat4 model_from_view;
    uniform mat4 view_from_clip;
    layout(location = 0) in vec4 vertex;
    out vec3 view_ray;
    void main() {
      view_ray =
          (model_from_view * vec4((view_from_clip * vertex).xyz, 0.0)).xyz;
      gl_Position = vertex;
    })";

#include "App.glsl.inc"


static std::map<int, Engine*> INSTANCES;


void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (canMovingMouse)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = (float)xpos - lastX;
		float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;
		glfwSetCursorPos(window, lastX, lastY);
		Pointers* pointers = reinterpret_cast<Pointers*>(glfwGetWindowUserPointer(window));
		pointers->sunDirection->handleMouseDragEvent(lastX, lastY);
	}
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		Pointers* pointers = reinterpret_cast<Pointers*>(glfwGetWindowUserPointer(window));
		pointers->inputEngine->onKeyPress(key, pointers->inputEngine->functions);
		firstMouse = pointers->inputEngine->functions->getFirstMouseValue();
		canMovingMouse = pointers->inputEngine->functions->getCanMovingMouseValue();
	}
}

Engine::Engine() :
	useConstantSolarSpectrum(false),
	useOzone(true),
	useCombinedTextures(true),
	useHalfPrecision(true),
	useLuminance(NONE),
	doWhiteBalance(false),
	programId(0),
	viewDistanceMeters(9000.0),
	viewZenithAngleRadians(1.47),
	viewAzimuthAngleRadians(-0.1),
	exposure(10.0)
{

	WindowClass windowClass;
	this->window = windowClass.getGlfwWindow();
	int width, height;
	glfwGetWindowSize(this->window, &width, &height);
	this->imguiClass = new ImguiClass(this->window);

	this->inputEngine = new InputEngine<EngineInputFunctions>(this->window);
	this->sunDirection = new SunDirection(this->window, 1.3, 2.9);
	getSystemInfo();

	pointers.inputEngine = this->inputEngine;
	pointers.sunDirection = this->sunDirection;

	canMovingMouse = pointers.inputEngine->functions->getCanMovingMouseValue();
	
	glfwSetWindowUserPointer(this->window, &pointers); //magic
}

Engine::~Engine()
{
	delete inputEngine;
	delete sunDirection;
	delete imguiClass;
	glfwDestroyWindow(this->window);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(programId);
	glDeleteBuffers(1, &fullScreenQuadVBO);
	glDeleteVertexArrays(1, &fullScreenQuadVAO);
	INSTANCES.erase(windowId);

}

void Engine::getSystemInfo() noexcept
{
	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;
	char CPUBrandString[0x40];
	// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	for (i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	this->CPU = CPUBrandString;
	this->GPU = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	this->memory = std::to_string((statex.ullTotalPhys / 1024) / 1024);
}

void Engine::refreshMemoryInfo() noexcept
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	this->usingMemory = std::to_string((statex.ullTotalPageFile - statex.ullAvailPageFile) / 1024 / 1024);
}


void Engine::initializeObjects()
{
	glfwMakeContextCurrent(window);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	this->sunDirection->previousMouseY = ((double)(mode->height)) / 1.4;
	this->sunDirection->previousMouseX = ((double)(mode->width)) / 2.0;

	glfwSetCursorPos(this->window, this->sunDirection->previousMouseX, this->sunDirection->previousMouseY);


	//window = glfwCreateWindow(mode->width, mode->height, SCREEN_TITLE, NULL, NULL);

	glfwSetFramebufferSizeCallback(this->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(this->window, mouse_callback);
	glfwSetKeyCallback(this->window, key_callback);
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
		Pointers* pointers = reinterpret_cast<Pointers*>(glfwGetWindowUserPointer(win));
		pointers->sunDirection->handleMouseClickEvent(button, action, mods);
	});

	glfwMakeContextCurrent(this->window);

	glViewport(0, 0, mode->width, mode->height);

	glGenVertexArrays(1, &fullScreenQuadVAO);
	glBindVertexArray(fullScreenQuadVAO);
	glGenBuffers(1, &fullScreenQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadVBO);
	const GLfloat vertices[] = {
	  -1.0, -1.0, 0.0, 1.0,
	  +1.0, -1.0, 0.0, 1.0,
	  -1.0, +1.0, 0.0, 1.0,
	  +1.0, +1.0, 0.0, 1.0,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	constexpr GLuint kAttribIndex = 0;
	constexpr int kCoordsPerVertex = 4;
	glVertexAttribPointer(kAttribIndex, kCoordsPerVertex, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(kAttribIndex);
	glBindVertexArray(0);

	textRenderer.reset(new TextRenderer);
	density = 1;
	topHeight = 6420000.0;
	rayleigh = 1.24062e-6;
	mie = 5.328e-3;
}

void Engine::updateModel()
{
	modelInit(density, topHeight, rayleigh, mie);
}

void Engine::run()
{
	initializeObjects();
	modelInit(density, topHeight, rayleigh, mie);

	while (!glfwWindowShouldClose(this->window)) {
		handleRedisplayEvent();
	}	
}

/*
<p>The "real" initialization work, which is specific to  atmosphere model,
is done in the following method. It starts with the creation of an atmosphere
<code>Model</code> instance, with parameters corresponding to the Earth
atmosphere:
*/

void Engine::modelInit(double density, double kTop, double kRay, double kMie)
{
	// Values from "Reference Solar Spectral Irradiance: ASTM G-173", ETR column
	// (see http://rredc.nrel.gov/solar/spectra/am1.5/ASTMG173/ASTMG173.html),
	// summed and averaged in each bin (e.g. the value for 360nm is the average
	// of the ASTM G-173 values for all wavelengths between 360 and 370nm).
	// Values in W.m^-2.
	constexpr int kLambdaMin = 360;
	constexpr int kLambdaMax = 830;
	constexpr double kSolarIrradiance[48] = {
	  1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
	  1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
	  1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
	  1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
	  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
	  1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
	};
	// Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
	// referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
	// each bin (e.g. the value for 360nm is the average of the original values
	// for all wavelengths between 360 and 370nm). Values in m^2.
	constexpr double kOzoneCrossSection[48] = {
	  1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
	  8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
	  1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
	  4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
	  2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
	  6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
	  2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};
	// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
	constexpr double kDobsonUnit = 2.687e20;
	// Maximum number density of ozone molecules, in m^-3 (computed so at to get
	// 300 Dobson units of ozone - for this we divide 300 DU by the integral of
	// the ozone density profile defined below, which is equal to 15km).
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
	// Wavelength independent solar irradiance "spectrum" (not physically
	// realistic, but was used in the original implementation).
	constexpr double kConstantSolarIrradiance = 1.5;
	constexpr double kBottomRadius = 6360000.0;
	double kTopRadius = kTop;
	double kRayleigh = kRay;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	double kMieAngstromBeta = kMie;
	constexpr double kMieSingleScatteringAlbedo = 0.9;
	constexpr double kMiePhaseFunctionG = 0.8;
	constexpr double kGroundAlbedo = 0.1;
	const double maxSunZenithAngle = (useHalfPrecision ? 102.0 : 120.0) / 180.0 * kPi;

	DensityProfileLayer
		rayleigh_layer(0.0, density, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	DensityProfileLayer mie_layer(0.0, density, -1.0 / kMieScaleHeight, 0.0, 0.0);
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	std::vector<DensityProfileLayer> ozone_density;
	ozone_density.push_back(
		DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	ozone_density.push_back(
		DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	std::vector<double> wavelengths;
	std::vector<double> solarIrradiance;
	std::vector<double> rayleighScattering;
	std::vector<double> mieScattering;
	std::vector<double> mieExtinction;
	std::vector<double> absorptionExtinction;
	std::vector<double> groundAlbedo;
	for (int l = kLambdaMin; l <= kLambdaMax; l += 10) {
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie =
			kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		if (useConstantSolarSpectrum) {
			solarIrradiance.push_back(kConstantSolarIrradiance);
		}
		else {
			solarIrradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		}
		rayleighScattering.push_back(kRayleigh * pow(lambda, -4));
		mieScattering.push_back(mie * kMieSingleScatteringAlbedo);
		mieExtinction.push_back(mie);
		absorptionExtinction.push_back(useOzone ?
			kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] :
			0.0);
		groundAlbedo.push_back(kGroundAlbedo);
	}

	modelPointer.reset(new Model1(wavelengths, solarIrradiance, kSunAngularRadius,
		kBottomRadius, kTopRadius, { rayleigh_layer }, rayleighScattering,
		{ mie_layer }, mieScattering, mieExtinction, kMiePhaseFunctionG,
		ozone_density, absorptionExtinction, groundAlbedo, maxSunZenithAngle,
		kLengthUnitInMeters, useLuminance == PRECOMPUTED ? 15 : 3,
		useCombinedTextures, useHalfPrecision));
	modelPointer->Init();

	/*
	<p>Then, it creates and compiles the vertex and fragment shaders used to render
	our App scene, and link them with the <code>Model</code>'s atmosphere shader
	to get the final scene rendering program:
	*/

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* const vertex_shader_source = kVertexShader;
	glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertexShader);

	const std::string fragment_shader_str =
		"#version 330\n" +
		std::string(useLuminance != NONE ? "#define USE_LUMINANCE\n" : "") +
		"const float kLengthUnitInMeters = " +
		std::to_string(kLengthUnitInMeters) + ";\n" +
		demo_glsl;
	const char* fragment_shader_source = fragment_shader_str.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragmentShader);

	if (programId != 0) {
		glDeleteProgram(programId);
	}
	programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glAttachShader(programId, modelPointer->shader());
	glLinkProgram(programId);
	glDetachShader(programId, vertexShader);
	glDetachShader(programId, fragmentShader);
	glDetachShader(programId, modelPointer->shader());

	/*
	<p>Finally, it sets the uniforms of this program that can be set once and for
	all (in our case this includes the <code>Model</code>'s texture uniforms,
	because our App app does not have any texture of its own):
	*/

	glUseProgram(programId);
	modelPointer->setProgramUniforms(programId, 0, 1, 2, 3);
	double whitePointR = 1.0;
	double whitePointG = 1.0;
	double whitePointB = 1.0;
	if (doWhiteBalance) {
		Model1::ConvertSpectrumToLinearSrgb(wavelengths, solarIrradiance,
			&whitePointR, &whitePointG, &whitePointB);
		double white_point = (whitePointR + whitePointG + whitePointB) / 3.0;
		whitePointR /= white_point;
		whitePointG /= white_point;
		whitePointB /= white_point;
	}
	glUniform3f(glGetUniformLocation(programId, "white_point"),
		whitePointR, whitePointG, whitePointB);
	glUniform3f(glGetUniformLocation(programId, "earth_center"),
		0.0, 0.0, -kBottomRadius / kLengthUnitInMeters);
	glUniform2f(glGetUniformLocation(programId, "sun_size"),
		tan(kSunAngularRadius),
		cos(kSunAngularRadius));

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);

	handleReshapeEvent(mode->width, mode->height);

}

/*
<p>The scene rendering method simply sets the uniforms related to the camera
position and to the Sun direction, and then draws a full screen quad (and
optionally a help screen).
*/

void Engine::handleRedisplayEvent()
{
	// Unit vectors of the camera frame, expressed in world space.
	float cosZ = cos(viewZenithAngleRadians);
	float sinZ = sin(viewZenithAngleRadians);
	float cosA = cos(viewAzimuthAngleRadians);
	float sinA = sin(viewAzimuthAngleRadians);
	float ux[3] = { -sinA, cosA, 0.0 };
	float uy[3] = { -cosZ * cosA, -cosZ * sinA, sinZ };
	float uz[3] = { sinZ * cosA, sinZ * sinA, cosZ };
	float l = viewDistanceMeters / kLengthUnitInMeters;

	// Transform matrix from camera frame to world space (i.e. the inverse of a
	// GL_MODELVIEW matrix).
	float modelFromView[16] = {
	  ux[0], uy[0], uz[0], uz[0] * l,
	  ux[1], uy[1], uz[1], uz[1] * l,
	  ux[2], uy[2], uz[2], uz[2] * l,
	  0.0, 0.0, 0.0, 1.0
	};

	glUniform3f(glGetUniformLocation(programId, "camera"),
		modelFromView[3],
		modelFromView[7],
		modelFromView[11]);
	glUniform1f(glGetUniformLocation(programId, "exposure"),
		useLuminance != NONE ? exposure * 1e-5 : exposure);
	glUniformMatrix4fv(glGetUniformLocation(programId, "model_from_view"),
		1, true, modelFromView);
	glUniform3f(glGetUniformLocation(programId, "sun_direction"),
		cos(this->sunDirection->sunAzimuthAngleRadians) * sin(this->sunDirection->sunZenithAngleRadians),
		sin(this->sunDirection->sunAzimuthAngleRadians) * sin(this->sunDirection->sunZenithAngleRadians),
		cos(this->sunDirection->sunZenithAngleRadians));

	glBindVertexArray(fullScreenQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	double dummyDensity = density;
	double dummyTopHeight = topHeight;
	double dummyRayleigh = rayleigh;
	double dummyMie = mie;
	
	imguiClass->renderDrawData(GPU, CPU, memory, usingMemory, density, topHeight, rayleigh, mie); //always at the end

	if(density != dummyDensity || dummyMie != mie || dummyRayleigh != rayleigh || dummyTopHeight != topHeight)
	{
		modelInit(density, topHeight, rayleigh, mie);
	}
	
    glfwSwapBuffers(this->window);
	glfwPollEvents();	
}

/*
<p>The other event handling methods are also straightforward, and do not
interact with the atmosphere model:
*/

void Engine::handleReshapeEvent(int viewport_width, int viewport_height)
{
	glViewport(0, 0, viewport_width, viewport_height);

	const float kFovY = 50.0 / 180.0 * kPi;
	const float kTanFovY = tan(kFovY / 2.0);
	float aspect_ratio = static_cast<float>(viewport_width) / viewport_height;

	// Transform matrix from clip space to camera space (i.e. the inverse of a
	// GL_PROJECTION matrix).
	float viewFromClip[16] = {
	  kTanFovY * aspect_ratio, 0.0, 0.0, 0.0,
	  0.0, kTanFovY, 0.0, 0.0,
	  0.0, 0.0, 0.0, -1.0,
	  0.0, 0.0, 1.0, 1.0
	};
	glUniformMatrix4fv(glGetUniformLocation(programId, "view_from_clip"), 1, true, viewFromClip);
} 


void Engine::setView(double viewDistanceMeters,
	double viewZenithAngleRadians, double viewAzimuthAngleRadians,
	double sunZenithAngleRadians, double sunAzimuthAngleRadians,
	double exposure)
{
	viewDistanceMeters = viewDistanceMeters;
	viewZenithAngleRadians = viewZenithAngleRadians;
	viewAzimuthAngleRadians = viewAzimuthAngleRadians;
	this->sunDirection->sunZenithAngleRadians = sunZenithAngleRadians;
	this->sunDirection->sunAzimuthAngleRadians = sunAzimuthAngleRadians;
	exposure = exposure;
}
