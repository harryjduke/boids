#include <raylib.h>
#include <stdlib.h>

#include "flock.h"
#include "gui.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1600;
    const int screenHeight = 900;

    struct FlockConfig flockConfig =
            CreateDefaultFlockConfig((Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight});
    struct FlockState flockState = InitializeFlock(flockConfig);

    struct GuiConfig guiConfig = CreateDefaultGuiConfig((float) screenHeight);

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        UpdateFlock(&flockState, &flockConfig);

        // Draw
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Debug - draw ranges on the first boid
        DrawBoidRanges(&guiConfig, &flockConfig, &flockState.boids[0]);

        // Draw boids
        for (int i = 0; i < flockConfig.numberOfBoids; i++) {
            DrawBoid(flockState.boids[i]);
        }

        // Draw GUI
        const struct ParametersPanelResult parametersPanelResult = DrawParametersPanel(&guiConfig, &flockState, &flockConfig);
        flockConfig = parametersPanelResult.newFlockConfig;
        if (parametersPanelResult.resetBoids) {
            free(flockState.boids);
            flockState.boids = SpawnBoids(flockConfig.numberOfBoids, flockConfig.flockBounds,
                                          (flockConfig.minimumSpeed + flockConfig.maximumSpeed) / 2.f);
            flockState.collisionRate = 0.f;
            flockState.collisionRateMeasurementStart = (float) GetTime();
        }

        EndDrawing();
    }

    free(flockState.boids);
    CloseWindow();
    return EXIT_SUCCESS;
}
