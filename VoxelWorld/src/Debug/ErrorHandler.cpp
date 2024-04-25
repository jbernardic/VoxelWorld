#include "ErrorHandler.h"
#include<glad/glad.h>
#include<iostream>

using namespace std;

void clearErrors() {
	while (glGetError());
}

bool erLog(const char* func, const char* file, int line) {
	while (GLenum error = glGetError()) {
		cout << "OPENGL ERROR: " << error << endl;
		cout << "FILE: " << file << endl;
		cout << "FUNCTION: " << func << endl;
		cout << "LINE: " << line << endl;
		return false;
	}
	return true;
}