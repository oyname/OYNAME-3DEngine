#pragma once
#include <chrono>
#include <cstdint>

class Timer
{
public:
    // Kompatibel: VSYNC_ONLY bleibt als Alias für variable timestep
    enum class TimeMode : int
    {
        FIXED_TIMESTEP = 0,
        VARIABLE_TIMESTEP = 1,
    };

private:
    TimeMode timeMode = TimeMode::VARIABLE_TIMESTEP;

    double deltaTime = 0.0;        // für VARIABLE: raw dt, für FIXED: fixedStep (nur wenn steps > 0)
    double accumulator = 0.0;
    std::chrono::high_resolution_clock::time_point lastFrameTime;

    int fixedSteps = 0;            // wie viele FIXED steps sind diese Frame abzuarbeiten

    static constexpr double FIXED_TIMESTEP_SEC = 1.0 / 60.0;
    static constexpr double MAX_DELTA_CLAMP = 0.1;     // cap gegen massive Stutters
    static constexpr int    MAX_FIXED_STEPS_PER_FRAME = 5; // Spiral-of-death Schutz

    Timer();                    // privat (Singleton)
    ~Timer() = default;

public:
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // Singleton
    static Timer* GetInstance();

    // Core-API
    static void Init();         // idempotent
    static void Shutdown();     // bleibt leer (Kompatibilität)
    static void Tick();         // ruft Update() auf

    // Alt-API kompatibel lassen
    static void Initialize() { Init(); }

    // Update
    void Update();

    // Getter
    static double GetDeltaTime() { return GetInstance()->deltaTime; }
    static double GetAccumulator() { return GetInstance()->accumulator; }
    static TimeMode GetTimeMode() { return GetInstance()->timeMode; }
    static double GetFixedStep() { return FIXED_TIMESTEP_SEC; }
    static int    GetFixedSteps() { return GetInstance()->fixedSteps; } // NEU
    static double GetFPS();

    // Setter
    static void SetTimeMode(TimeMode mode) { GetInstance()->timeMode = mode; }

    // Helper
    static void Reset();

    // FIXED: Step-Verbrauch (NEU, sauber)
    // Nutzung: while (Timer::ConsumeFixedStep()) { FixedUpdate(Timer::GetFixedStep()); }
    static bool ConsumeFixedStep();
};