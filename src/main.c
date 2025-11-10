#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>

#include "boid.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int numberOfBoids = 10;

    Boid boids[numberOfBoids];
    SpawnBoids(boids, numberOfBoids, (Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight});

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update

        for (int i = 0; i < numberOfBoids; i++) {
            // Update position
            boids[i].position.x += boids[i].velocity.x * GetFrameTime();
            boids[i].position.y += boids[i].velocity.y * GetFrameTime();

            // Loop around screen edges
            if (boids[i].position.x < 0.f)
                boids[i].position.x = (float) screenWidth;
            if (boids[i].position.x > (float) screenWidth)
                boids[i].position.x = 0.f;
            if (boids[i].position.y < 0.f)
                boids[i].position.y = (float) screenHeight;
            if (boids[i].position.y > (float) screenHeight)
                boids[i].position.y = 0.f;
        }

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < numberOfBoids; i++) {
            DrawBoid(boids[i]);
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
