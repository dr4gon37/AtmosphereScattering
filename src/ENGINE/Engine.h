#pragma once
#include "ENGINE/WindowClass.h"
#include "ENGINE/InputEngine.h"
#include "ENGINE/EngineInputFunctions.h"
#include "IMGUI/ImguiClass.h"
#include <string>
#include "MODEL/model1.h"
#include "TEXT/text_renderer.h"
#include "MATHS/SunDirection.h"

#ifdef _WIN64
#include <intrin.h>

#include <glad/glad.h>
#include <memory>
#include "./MODEL/model1.h"
#include "./TEXT/text_renderer.h"

#endif

const int TICKS_PER_SECOND = 60;
const float SEC_PER_TICK = 1.0f / TICKS_PER_SECOND;
const float MS_PER_TICK = 1000.0f / TICKS_PER_SECOND;

 struct Pointers
{
	InputEngine<EngineInputFunctions> *inputEngine = nullptr;
	SunDirection *sunDirection = nullptr;
 	
	Pointers::~Pointers()
	{
		std::cout << "Pointers deleted" << std::endl;
	}
};


class Engine
{
private:
	GLFWwindow* window;
	InputEngine<EngineInputFunctions> *inputEngine;
	ImguiClass * imguiClass;
	SunDirection *sunDirection;
	Pointers pointers;
	std::string CPU;
	std::string GPU;
	std::string memory;
	std::string usingMemory;

private:
	void getSystemInfo() noexcept;
	void refreshMemoryInfo() noexcept;

	enum Luminance {
		// Render the spectral radiance at kLambdaR, kLambdaG, kLambdaB.
		NONE,
		// Render the sRGB luminance, using an approximate (on the fly) conversion

		APPROXIMATE,
		// Render the sRGB luminance, precomputed from 15 spectral radiance values

		PRECOMPUTED
	};
	void handleRedisplayEvent() ;
	void handleReshapeEvent(int viewport_width, int viewport_height);

	void setView(double viewDistanceMeters, double viewZenithAngleRadians,
				double viewAzimuthAngleRadians, double sunZenithAngleRadians,
				double sunAzimuthAngleRadians, double exposure);

	void modelInit(double density, double kTop, double kRay, double kMie);

	bool useConstantSolarSpectrum;
	bool useOzone;
	bool useCombinedTextures;
	bool useHalfPrecision;
	Luminance useLuminance;
	bool doWhiteBalance;

	std::unique_ptr<Model1> modelPointer;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programId;
	GLuint fullScreenQuadVAO;
	GLuint fullScreenQuadVBO;
	std::unique_ptr<TextRenderer> textRenderer;
	int windowId;

	double viewDistanceMeters;
	double viewZenithAngleRadians;
	double viewAzimuthAngleRadians;
	double exposure;


public:
	double density;
	double topHeight;
	double rayleigh;
	double mie;

	const Model1& model1() const { return *modelPointer; }
	const GLuint vertex_shader() const { return vertexShader; }
	const GLuint fragment_shader() const { return fragmentShader; }
	const GLuint program() const { return programId; }


	Engine();
	~Engine();

	void run();
	void initializeObjects();
	void updateModel();

};