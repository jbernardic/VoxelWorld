#include "Application.h"
#include <iostream>
#include <stdexcept>

SDL_Window* Application::SDLWindow = nullptr;
InputHandler Application::Input;
VkContext Application::Vulkan;

void Application::Create()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        throw std::runtime_error("SDL_Init(SDL_INIT_VIDEO) failed");
        return;
    }

    SDLWindow = SDL_CreateWindow(
        "Vulkan Engine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        W_WIDTH,
        W_HEIGHT,
        SDL_WINDOW_VULKAN
    );

    Vulkan.Init(SDLWindow, W_WIDTH, W_HEIGHT);
}

void Application::Destroy()
{
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
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