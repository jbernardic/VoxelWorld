#pragma once

#include <SDL.h>
#include "Input.h"
#include "../Vulkan/VkContext.h"

class Application
{
public:
    static const unsigned int W_WIDTH = 1280;
    static const unsigned int W_HEIGHT = 720;
    static void Create();
    static void Destroy();
    static void HideCursor();
    static void ShowCursor();
    static void SetMousePosition(int x, int y);

    static InputHandler Input;
    static SDL_Window* SDLWindow;
    static VkContext Vulkan;

};