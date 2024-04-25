#pragma once
#include <SDL.h>
#include "Input.h"
class Application
{
public:
	static const unsigned int W_WIDTH = 1280;
	static const unsigned int W_HEIGHT = 720;
public:
	static void Create();
	static void Destroy();
	static void HideCursor();
	static void ShowCursor();
	static void SetMousePosition(int x, int y);
	static InputHandler Input;
	static SDL_Renderer* SDLRenderer;
	static SDL_Window* SDLWindow;
	static SDL_GLContext GLContext;
};