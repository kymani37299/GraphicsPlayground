#include <Engine.h>

#define RUN_SAMPLE

#ifdef RUN_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    GP::Init(hInstance);
    GP::Run();
    GP::Deinit();
    return 0;
}
#endif // RUN_SAMPLE