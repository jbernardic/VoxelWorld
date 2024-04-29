#include "Input.h"
#include <iostream>

InputHandler::~InputHandler()
{
	free(m_KeyStates);
}

void InputHandler::InitState()
{
	//Keyboard state
	free(m_KeyStates);
	m_KeyStates = (Uint8*)calloc(SDL_NUM_SCANCODES, sizeof(Uint8));
	m_PrevMouseState = m_MouseState;
	m_MouseState = SDL_GetMouseState(NULL, NULL);

	const Uint8* keyStates = SDL_GetKeyboardState(NULL);
	if (m_KeyStates)
	{
		SDL_memcpy(m_KeyStates, keyStates, SDL_NUM_SCANCODES * sizeof(Uint8));
	}

	//Mouse position
	int x, y;
	SDL_GetMouseState(&x, &y);
	m_MousePosition = glm::vec2(x, y);
}

void InputHandler::HandleEvent(SDL_Event e)
{
	switch (e.key.type)
	{
	case SDL_KEYDOWN:
		if (!e.key.repeat)
		{
			m_KeyStates[e.key.keysym.scancode] = 2;
		}
		break;
	case SDL_KEYUP:
		if (!e.key.repeat)
		{
			m_KeyStates[e.key.keysym.scancode] = 3;
		}
		break;
	default:
		break;
	}
}

KeyState InputHandler::GetKeyState(int sdl_scancode)
{
	return (KeyState)m_KeyStates[sdl_scancode];
}
