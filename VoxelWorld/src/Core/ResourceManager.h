#pragma once
#include <iostream>
#include <SDL.h>
#include <unordered_map>
#include "../Graphics/Shader.h"

#define RM ResourceManager::GetInstance()

class ResourceManager
{
private:
	ResourceManager() = default;
	~ResourceManager() = default;
public:
	static auto& GetInstance()
	{
		static ResourceManager instance;
		return instance;
	}

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	void LoadShader(std::string path, const char* name);
	Shader* GetShader(const char* name);
private:
	std::unordered_map<std::string, std::unique_ptr<Shader>> Shaders;
};