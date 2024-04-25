#pragma once
#include <SDL.h>
#include "Core/Application.h"

class Game
{
public:
	Game();
	~Game();
	bool Tick();
private:
	bool ShouldQuit = false;
	void Init();
	void Draw();
	void Update();
};