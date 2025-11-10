#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

#include "boid.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int numberOfBoids = 70;

    const float separationFactor = 10.f;
    const float separationRange = 50.f;

    const float alignmentFactor = 0.01f;
    const float alignmentRange = 100.f;

    const float cohesionFactor = 0.01f;
    const float cohesionRange = 100.f;

    Boid boids[numberOfBoids];
    SpawnBoids(boids, numberOfBoids, (Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight});

    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update

        for (int i = 0; i < numberOfBoids; i++) {
            Vector2 separationForce = Vector2Zero();
            Vector2 alignmentForce = Vector2Zero();
            int boidsInAlignmentRange = 0;
            Vector2 cohesionForce = Vector2Zero();
            int boidsInCohesionRange = 0;

            // Calculate
            for (int j = 0; j < numberOfBoids; j++) {
                if (i == j)
                    continue;
                const float distanceToOtherBoid = Vector2Distance(boids[i].position, boids[j].position);

                // Separation
                // A force pushing away from other boids, the smaller distance between the boids, the
                // stronger the force.
                if (distanceToOtherBoid < separationRange && distanceToOtherBoid > 0.001f) {
                    Vector2 separationOffset = Vector2Subtract(boids[i].position, boids[j].position);
                    separationForce = Vector2Add(separationForce,
                                                 Vector2Scale(separationOffset, 1.f / powf(distanceToOtherBoid, 2.f)));
                }

                // Alignment
                // Adjusts the velocity towards the average velocity of the boids within range.
                if (distanceToOtherBoid < alignmentRange) {
                    alignmentForce = Vector2Add(alignmentForce, boids[j].velocity);
                    boidsInAlignmentRange++;
                }

                // Cohesion
                // A force towards the centre of the boids within range.
                if (distanceToOtherBoid < cohesionRange) {
                    cohesionForce = Vector2Add(cohesionForce, boids[j].position);
                    boidsInCohesionRange++;
                }
            }

            // Apply

            // Separation
            boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(separationForce, separationFactor));

            // Alignment
            if (boidsInAlignmentRange > 0) {
                alignmentForce = Vector2Scale(alignmentForce, 1.f / (float) boidsInAlignmentRange);
                alignmentForce = Vector2Subtract(alignmentForce, boids[i].velocity);
                boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(alignmentForce, alignmentFactor));
            }

            // Cohesion
            if (boidsInCohesionRange > 0) {
                cohesionForce = Vector2Scale(cohesionForce, 1.f / (float) boidsInCohesionRange);
                cohesionForce = Vector2Subtract(cohesionForce, boids[i].position);
                boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(cohesionForce, cohesionFactor));
            }

            // Clamp boid speed
            // float speed = Vector2Length(boids[i].velocity);
            // if (speed > BOID_MAX_SPEED) {
            //     boids[i].velocity = Vector2Scale(boids[i].velocity, (1.f / speed) * BOID_MAX_SPEED);
            // } else if (speed < BOID_MIN_SPEED) {
            //     boids[i].velocity = Vector2Scale(boids[i].velocity, (1.f / speed) * BOID_MIN_SPEED);
            // }

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

        ClearBackground(DARKGRAY);

        // Debug - draw ranges on the first boid
        DrawCircleV(boids[0].position, alignmentRange, GRAY);
        DrawCircleV(boids[0].position, separationRange, LIGHTGRAY);

        for (int i = 0; i < numberOfBoids; i++) {
            DrawBoid(boids[i]);
        }

        // DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, MAROON);

        EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
