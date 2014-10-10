#pragma once
#ifdef _DEBUG
#include <crtdbg.h>

#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif