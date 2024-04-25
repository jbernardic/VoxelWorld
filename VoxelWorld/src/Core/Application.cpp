#pragma once
#include "Application.h"
#include <iostream>
#include <glad/glad.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

SDL_Renderer* Application::SDLRenderer = nullptr;
SDL_Window* Application::SDLWindow = nullptr;
SDL_GLContext Application::GLContext = nullptr;
InputHandler Application::Input;

void Application::Create()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error("SDL_Init(SDL_INIT_VIDEO)");
		return;
	}
	SDL_CreateWindowAndRenderer(W_WIDTH, W_HEIGHT, SDL_WINDOW_OPENGL, &SDLWindow, &SDLRenderer);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	GLContext = SDL_GL_CreateContext(SDLWindow);
	if (GLContext == nullptr)
	{
		throw std::runtime_error("SDL_GL_CreateContext(Window)");
		return;
	}

	gladLoadGL();

	//Use Vsync
	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		throw std::runtime_error("Unable to set up VSync");
		return;
	}

	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		throw std::runtime_error("SDL_image could not initialize");
		return;
	}

}

void Application::Destroy()
{
	SDL_DestroyRenderer(SDLRenderer);
	SDL_DestroyWindow(SDLWindow);
	SDL_Quit();
	IMG_Quit();
}

void Application::HideCursor()
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Application::ShowCursor()
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Application::SetMousePosition(int x, int y)
{
	SDL_WarpMouseInWindow(SDLWindow, x, y);
}
