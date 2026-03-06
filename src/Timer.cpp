#include "Timer.h"
#include <algorithm> // std::min
#include <cmath>     // std::floor

Timer::Timer()
    : lastFrameTime(std::chrono::high_resolution_clock::now())
{

}

Timer* Timer::GetInstance()
{
    static Timer instance; // kein new/delete
    return &instance;
}

void Timer::Init()
{
    (void)GetInstance();
}

void Timer::Shutdown()
{
    // absichtlich leer: statischer Singleton räumt sich automatisch auf
}

void Timer::Tick()
{
    GetInstance()->Update();
}

void Timer::Update()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    double rawDeltaTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();
    lastFrameTime = currentTime;

    // Clamp gegen massive Stutters (Alt-Tab, Breakpoints, Window drag)
    rawDeltaTime = std::min(rawDeltaTime, MAX_DELTA_CLAMP);

    fixedSteps = 0;

    if (timeMode == TimeMode::FIXED_TIMESTEP)
    {
        accumulator += rawDeltaTime;

        // Wie viele fixed steps sind abzuarbeiten?
        int steps = static_cast<int>(std::floor(accumulator / FIXED_TIMESTEP_SEC));

        // Spiral-of-death Schutz
        if (steps > MAX_FIXED_STEPS_PER_FRAME)
            steps = MAX_FIXED_STEPS_PER_FRAME;

        if (steps > 0)
        {
            fixedSteps = steps;
            accumulator -= static_cast<double>(steps) * FIXED_TIMESTEP_SEC;

            // In FIXED mode deltaTime is always fixedStep (not raw)
            deltaTime = FIXED_TIMESTEP_SEC;
        }
        else
        {
            // No step due -> deltaTime = 0 (steps=0 is the key indicator)
            deltaTime = 0.0;
        }
    }
    else
    {
        // VARIABLE (aka VSYNC_ONLY): einfach raw dt
        deltaTime = rawDeltaTime;
    }
}

bool Timer::ConsumeFixedStep()
{
    Timer* t = GetInstance();
    if (t->fixedSteps > 0)
    {
        --t->fixedSteps;
        // deltaTime bleibt FIXED_TIMESTEP_SEC während Steps > 0, das passt so.
        return true;
    }
    return false;
}

void Timer::Reset()
{
    Timer* t = GetInstance();
    t->deltaTime = 0.0;
    t->accumulator = 0.0;
    t->fixedSteps = 0;
    t->lastFrameTime = std::chrono::high_resolution_clock::now();
}

double Timer::GetFPS()
{
    const double dt = GetInstance()->deltaTime;
    if (dt > 0.0)
        return 1.0 / dt;

    // In FIXED mode with no step due: do not derive FPS from 0
    return 0.0;
}


