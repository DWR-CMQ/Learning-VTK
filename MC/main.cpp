#include "va_mc_app.h"

// Á´½Ó OpenGL ¿â
#pragma comment(lib, "opengl32.lib")

int main(int argc, char* argv[])
{
	MCApp app;
	app.Init();
	app.Run();
	return 0;
}
