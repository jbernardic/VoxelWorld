#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
enum class KeyState
{
	KeyUp,
	KeyDown,
	TriggerPressed,
	TriggerReleased,
};
class InputHandler
{
public:
	~InputHandler();
	void InitState();
	void HandleEvent(SDL_Event e);
	KeyState GetKeyState(int sdl_scancode);
	bool KeyPressed(int sdl_scancode) const 
	{ 
		return m_KeyStates[sdl_scancode] == (int)KeyState::TriggerPressed;
	}
	bool KeyReleased(int sdl_scancode) const
	{
		return m_KeyStates[sdl_scancode] == (int)KeyState::TriggerReleased;
	}
	bool KeyDown(int sdl_scancode) const
	{
		return m_KeyStates[sdl_scancode] == (int)KeyState::TriggerPressed || 
			m_KeyStates[sdl_scancode] == (int)KeyState::KeyDown;
	}
	bool MouseButtonDown(int sdl_button) const
	{
		return m_MouseState & SDL_BUTTON(sdl_button);
	}
	bool MouseButtonPressed(int sdl_button) const
	{
		return (m_MouseState & SDL_BUTTON(sdl_button)) && !(m_PrevMouseState & SDL_BUTTON(sdl_button));
	}
	const glm::vec2& GetMousePosition() const
	{
		return m_MousePosition;
	}
private:
	Uint8* m_KeyStates = nullptr;
	Uint8 m_MouseState = 0;
	Uint8 m_PrevMouseState = 0;
	glm::vec2 m_MousePosition;
};