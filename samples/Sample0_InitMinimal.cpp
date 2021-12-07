#include "CurrentSample.h"

#if CURRENT_SAMPLE == 0

#include <GP.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	// Initialize window and graphics device
	// Params: HINSTANCE, WindowWidth, WindowHeight, WindowTitle(optional), FPS(optional)
	GP::Init(hInstance, 1024, 768, "Demo");

	// After everytihng is setup we call Run which will start rendering to the screen
	GP::Run();

	// Deinitialize on the end to free allocated memory
	GP::Deinit();
}

#endif // CURRENT_SAMPLE

