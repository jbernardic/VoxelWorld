#include "ResourceManager.h"
#include <SDL_image.h>
#include "Application.h"

void ResourceManager::LoadShader(std::string path, const char* name)
{
    Shaders[name] = std::make_unique<Shader>(path);
}

Shader* ResourceManager::GetShader(const char* name)
{
    return Shaders[name].get();
}
