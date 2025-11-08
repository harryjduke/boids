#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

void spawnBoids(Vector2 *boidsArray, int numberOfBoids, Rectangle spawnBounds) {
    for (int i = 0; i < numberOfBoids; i++) {
        boidsArray[i] = (Vector2){(float)GetRandomValue((int)spawnBounds.x, (int)spawnBounds.width),
                                  (float)GetRandomValue((int)spawnBounds.y, (int)spawnBounds.height)};
    }
}

int main(int argc, char *argv[]) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int numberOfBoids = 10;

    Vector2 boids[numberOfBoids];
    spawnBoids(boids, numberOfBoids, (Rectangle){0.f, 0.f, (float)screenWidth, (float)screenHeight});

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < numberOfBoids; i++) {
            DrawCircle((int)boids[i].x, (int)boids[i].y, 5.f, MAROON);
        }

        DrawText("Boids", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
