#pragma once
#include <SDL.h>
#include <iostream>
#include "Game.h"

using namespace std;

int main(int argc, char* argv[])
{
	Game game;
	while (game.Tick());
	return 0;
}