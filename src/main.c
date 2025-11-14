#include <raylib.h>
#include <stdlib.h>

#include "flock.h"
#include "gui.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1600;
    const int screenHeight = 900;

    struct FlockConfig flockConfig =
            CreateDefaultFlockConfig((Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight});
    struct FlockState flockState;
    if (!InitializeFlock(&flockState, &flockConfig)) {
        TraceLog(LOG_FATAL, "Could not initialise flock. Exiting.");
        CloseWindow();
        return EXIT_FAILURE;
    }

    struct GuiConfig guiConfig = CreateDefaultGuiConfig((float) screenHeight);
    struct ParametersPanelState parametersPanelState;
    InitializeParametersPanel(&parametersPanelState);

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        UpdateFlock(&flockState, &flockConfig);

        // Draw
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Debug - draw ranges on the first boid
        DrawBoidRanges(&parametersPanelState, &guiConfig, &flockConfig, &flockState.boids[0]);

        // Draw boids
        for (int i = 0; i < flockState.boidsCount; i++) {
            DrawBoid(flockState.boids[i]);
        }

        // Draw GUI
        const struct ParametersPanelResult parametersPanelResult =
                DrawParametersPanel(&parametersPanelState, &guiConfig, &flockState, &flockConfig);
        flockConfig = parametersPanelResult.newFlockConfig;
        if (parametersPanelResult.resetBoids) {
            DestroyFlock(&flockState);
            InitializeFlock(&flockState, &flockConfig);
        }

        EndDrawing();
    }

    DestroyFlock(&flockState);
    CloseWindow();
    return EXIT_SUCCESS;
}
