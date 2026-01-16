#include "boid.h"
#include "flock.h"
#include "gui.h"

#include <raylib.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const int screenWidth = 1600;
    const int screenHeight = 900;

    const Rectangle flockBounds = {
        .x = 0.F,
        .y = 0.F,
        .width = (float)screenWidth,
        .height = (float)screenHeight,
    };
    struct FlockState flockState;
    if (!InitializeFlock(&flockState, CreateDefaultFlockConfig(flockBounds))) {
        TraceLog(LOG_FATAL, "Failed to initialise flock. Exiting.");
        CloseWindow();
        return EXIT_FAILURE;
    }

    struct GuiState guiState;
    InitializeGui(&guiState, CreateDefaultGuiConfig((float)screenHeight));

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        UpdateFlock(&flockState);

        // Draw
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Draw boids
        for (int i = 0; i < flockState.boidsCount; i++) {
            DrawBoid(flockState.boids[i]);
        }

        // Draw GUI
        const struct GuiResult guiResult = DrawGui(&guiState, &flockState);
        if (guiResult.parametersPanelResult.resetBoids) {
            DestroyFlock(&flockState);
            if (!InitializeFlock(&flockState, guiResult.parametersPanelResult.newFlockConfig)) {
                TraceLog(LOG_FATAL, "Failed to reinitialise flock. Exiting.");
            }
        } else {
            ModifyFlockConfig(&flockState, guiResult.parametersPanelResult.newFlockConfig);
        }

        EndDrawing();
    }

    DestroyFlock(&flockState);
    CloseWindow();
    return EXIT_SUCCESS;
}
