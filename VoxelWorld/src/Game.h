#pragma once
#include <SDL.h>
#include "Core/Application.h"
#include <chrono>

class Game
{
public:
	Game();
	~Game();
	bool Tick();
private:
	bool shouldQuit = false;
	std::chrono::steady_clock::time_point time;
	void Init();
	void Draw();
	void Update(double deltaTime);
};