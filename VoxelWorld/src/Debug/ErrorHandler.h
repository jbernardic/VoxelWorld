#pragma once

#define ASSERT(x) if(!(x)) __debugbreak();
#define lcall(x) clearErrors();\
    x;\
    ASSERT(erLog(#x, __FILE__, __LINE__))

void clearErrors();

bool erLog(const char* func, const char* file, int line);

