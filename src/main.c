#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "flock.h"
#include "gui.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1600;
    const int screenHeight = 900;

    struct FlockState flockState;
    if (!InitializeFlock(&flockState,
                         CreateDefaultFlockConfig((Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight}))) {
        TraceLog(LOG_FATAL, "Failed to initialise flock. Exiting.");
        CloseWindow();
        return EXIT_FAILURE;
    }
    struct GuiConfig guiConfig = CreateDefaultGuiConfig((float) screenHeight);
    struct ParametersPanelState parametersPanelState;
    InitializeParametersPanel(&parametersPanelState, CreateDefaultGuiConfig((float) screenHeight));

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        UpdateFlock(&flockState);

        // Draw
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Debug - draw ranges on the first boid
        DrawBoidRanges(&parametersPanelState, &flockState, &flockState.boids[0]);

        // Draw boids
        for (int i = 0; i < flockState.boidsCount; i++) {
            DrawBoid(flockState.boids[i]);
        }

        // Draw GUI
        const struct ParametersPanelResult parametersPanelResult =
                DrawParametersPanel(&parametersPanelState, &flockState);
        if (parametersPanelResult.resetBoids) {
            DestroyFlock(&flockState);
            if (!InitializeFlock(&flockState, parametersPanelResult.newFlockConfig)) {
                TraceLog(LOG_FATAL, "Failed to reinitialise flock. Exiting.");
            }
        } else {
            ModifyFlockConfig(&flockState, parametersPanelResult.newFlockConfig);
        }

        EndDrawing();
    }

    DestroyFlock(&flockState);
    CloseWindow();
    return EXIT_SUCCESS;
}
