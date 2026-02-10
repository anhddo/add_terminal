#ifdef _WIN32
// This tells the linker to use main() even if we are in Windows Subsystem mode
#pragma comment(linker, "/entry:mainCRTStartup")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // first

#include "renderer.h"
#include "data_api/ikbr/ibkr.h"
//#include "data_api/ikbr/MainClientDemo.h"


int main() {
	//readfile();
	Renderer renderer;
	renderer.draw();
	//main1(0, nullptr);
	//IbkrClient client;
	//client.test();

}

