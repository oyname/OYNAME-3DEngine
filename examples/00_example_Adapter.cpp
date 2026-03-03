#include "gidx.h"
#include "core.h"

// OYNAME Engine
// 
// This example demonstrates how the graphics properties of a system can be queried.
// The queried data will be output in the debug window.

int monitors();
int adapter();
int grafikadapter();
int grafikmodus();
void directxversion();
int grafikformats();

int main()
{
    grafikformats();
    directxversion();
    grafikmodus();
    grafikadapter();
    adapter();
    monitors();

    Engine::Graphics(1024,768);

    Engine::SetGfxDriver(0);
    Engine::SetOutput(1);

    Engine::SetVSync(1);

    Debug::Log("X: ", Engine::GetWidth(), " Y: ", Engine::GetHeight(), " Hz: ", Engine::GetMaxFrequency(1920,1080));

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) // Main loop
    {
        //Core::BeginFrame();
        //const float dt = static_cast<float>(Core::GetDeltaTime());
        //
        //
        //Engine::Cls(0, 64, 128);
        //Engine::UpdateWorld();
        //Engine::RenderWorld();
        //Engine::Flip();
        //
        //Core::EndFrame();
    }

    // Shutdown the engine
    return 0;

}

int monitors()
{
    int count = Engine::CountGfxDrivers(); // Count the number of graphics drivers available

    for (int i = 0; i < count; i++)
    {
        Engine::SetGfxDriver(i); // Set the current graphics driver

        int numMonitors = Engine::CountOutputs(); // Count the number of connected monitors

        for (int j = 0; j < numMonitors; j++)
        {
            Engine::SetOutput(j); // Set the current monitor

            int numModes = Engine::CountGfxModes(j); // Count the number of available resolutions for the current monitor
            Debug::Log("Number of possible resolutions for monitor ", j + 1, ": ", numModes);

            for (int k = 0; k < numModes; k++)
            {
                Debug::Log("X: ", Engine::GfxModeWidth(k), " Y: ", Engine::GfxModeHeight(k), " @ ", Engine::GetGfxModeFrequency(k), " Frequency");
            }
        }
    }

    return 0;
}

int adapter()
{
    int count = Engine::CountGfxDrivers(); // Count the number of graphics drivers available

    for (int i = 0; i < count; i++)
    {
        Engine::SetGfxDriver(i); // Set the current graphics driver

        Debug::Log("Graphics card name: ", Engine::GfxDriverName());

        int numMonitors = Engine::CountOutputs(); // Count the number of connected monitors
        Debug::Log("Number of connected monitors: ", numMonitors);
    }

    return 0;
}

int grafikadapter()
{
    int count = Engine::CountGfxDrivers(); // Count the number of graphics drivers available
    Debug::Log("Number of adapters: ", count);

    return 0;
}

int grafikmodus()
{
    if (Engine::GfxModeExists(1280, 1024, 120)) // Check if a specific mode exists
        Debug::Log("This mode exists");
    else
        Debug::Log("This mode does not exist");

    return 0;

}

void directxversion()
{
    std::string text = "";
    unsigned int version = Engine::GfxGetDirectXVersion();

    switch (version)
    {
    case 1:
        text = "DirectX 9";
        break;
    case 2:
        text = "DirectX 10";
        break;
    case 3:
        text = "DirectX 10.1";
        break;
    case 4:
        text = "DirectX 11";
        break;
    case 5:
        text = "DirectX 11.1";
        break;
    case 6:
        text = "DirectX 12";
        break;
    case 7:
        text = "DirectX 12.1";
        break;
    default:
        text = "Unknown";
        break;
    }

    Debug::Log("DirectX Version : ", text);
}

int grafikformats()
{
    if (Engine::GfxFormatSupported(GXFORMAT::R8G8B8A8_UNORM))
        Debug::Log("The tested format is supported");
    else
        Debug::Log("The tested format is not supported");

    return 0;
}

