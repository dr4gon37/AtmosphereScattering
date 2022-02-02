#pragma once
#include <functional>
#include <vector>
#include <map>
#include "EngineInputFunctions.h"
#include "GLFW/glfw3.h"

template <typename T>
class InputEngine
{
public:
	EngineInputFunctions *functions;
	
	explicit InputEngine::InputEngine(GLFWwindow* window)
	{
		this->functions = new EngineInputFunctions(window);
		addBinding(GLFW_KEY_ESCAPE, &EngineInputFunctions::escapeKey);
		addBinding(GLFW_KEY_P, &EngineInputFunctions::mouseCursor);
	}
	
	InputEngine::~InputEngine()
	{
		std::cout << "Input Engine destroyed" << std::endl;
		delete functions;
	}
	
	using Callback = std::function<void(T*)>;
	
	void onKeyPress(int key, T* object)
	{
		for (Callback &callback : mCallbacks[key])
			callback(object);
	}
private:
	std::map<int, std::vector<Callback>> mCallbacks;

	void addBinding(int key, const Callback& callback)
	{
		mCallbacks[key].push_back(callback);
	}
};
