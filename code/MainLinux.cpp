#include "Core.h"

#if PLATFORM_LINUX
#include "Main.h"

void* CreateRGBATexture(uint32 width, uint32 height, const void* data)
{
    ASSERT(0); // TODO
    return nullptr;
}

void  DestroyTexture(void* handle)
{
    ASSERT(0); // TODO
}

int main(int argc, const char * argv[])
{
    ASSERT(0);  // TODO
    return 1;
}

#endif // PLATFORM_LINUX