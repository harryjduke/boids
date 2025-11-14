#include "flock.h"

#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

struct FlockConfig CreateDefaultFlockConfig(const Rectangle flockBounds) {
    return (struct FlockConfig) {.flockBounds = flockBounds,
                                 .numberOfBoids = 100,

                                 .separationFactor = 1.f,
                                 .alignmentFactor = 0.01f,
                                 .cohesionFactor = 0.005f,

                                 .separationRange = 50.f,
                                 .alignmentRange = 100.f,
                                 .cohesionRange = 100.f,

                                 .clampSpeed = true,
                                 .minimumSpeed = 50.f,
                                 .maximumSpeed = 100.f};
}

static Boid *SpawnBoids(const int numberOfBoids, const Rectangle spawnBounds, const float startSpeed) {
    Boid *boids = malloc(sizeof(Boid) * numberOfBoids);
    if (boids == NULL) {
        TraceLog(LOG_ERROR, "SpawnBoids: Failed to allocate memory for %d boids.", numberOfBoids);
        return NULL;
    }
    for (int i = 0; i < numberOfBoids; i++) {
        boids[i] = (Boid) {
                (Vector2) {(float) GetRandomValue((int) spawnBounds.x, (int) (spawnBounds.x + spawnBounds.width)),
                           (float) GetRandomValue((int) spawnBounds.y, (int) (spawnBounds.y + spawnBounds.height))},
                Vector2Scale(Vector2Normalize(
                                     (Vector2) {(float) GetRandomValue(-100, 100), (float) GetRandomValue(-100, 100)}),
                             startSpeed)};
    }
    return boids;
}

bool InitializeFlock(struct FlockState *flockState, const struct FlockConfig *flockConfig) {
    if (flockConfig == NULL) {
        TraceLog(LOG_ERROR, "InititializeFlock: Recieved NULL pointer to flockConfig.");
        return false;
    }

    *flockState = (struct FlockState) {.boids = SpawnBoids(flockConfig->numberOfBoids, flockConfig->flockBounds,
                                                           (flockConfig->minimumSpeed + flockConfig->maximumSpeed) / 2.f),
                                       .boidsCount = flockConfig->numberOfBoids,
                                       .collisionRate = 0.f,
                                       .collisionRateMeasurementStart = (float) GetTime()};

    if (flockState->boids == NULL) {
        flockState = NULL;
        return false;
    }

    return true;
}

void UpdateFlock(struct FlockState *flockState, const struct FlockConfig *flockConfig) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Recieved NULL pointer to flockState.");
        return;
    }
    if (flockConfig == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Recieved NULL pointer to flockConfig.");
        return;
    }

    for (int i = 0; i < flockState->boidsCount; i++) {
        Vector2 separationForce = Vector2Zero();
        Vector2 alignmentForce = Vector2Zero();
        int boidsInAlignmentRange = 0;
        Vector2 cohesionForce = Vector2Zero();
        int boidsInCohesionRange = 0;

        // Calculate
        for (int j = 0; j < flockState->boidsCount; j++) {
            if (i == j)
                continue;
            const float distanceToOtherBoid =
                    Vector2Distance(flockState->boids[i].position, flockState->boids[j].position);

            // Separation
            // A force pushing away from other boids, the smaller distance between the boids, the
            // stronger the force.
            if (distanceToOtherBoid < flockConfig->separationRange && distanceToOtherBoid > 0.001f) {
                Vector2 separationOffset =
                        Vector2Subtract(flockState->boids[i].position, flockState->boids[j].position);
                separationForce = Vector2Add(separationForce,
                                             Vector2Scale(separationOffset, 1.f / powf(distanceToOtherBoid, 2.f)));
            }

            // Alignment
            // Adjusts the velocity towards the average velocity of the boids within range.
            if (distanceToOtherBoid < flockConfig->alignmentRange) {
                alignmentForce = Vector2Add(alignmentForce, flockState->boids[j].velocity);
                boidsInAlignmentRange++;
            }

            // Cohesion
            // A force towards the centre of the boids within range.
            if (distanceToOtherBoid < flockConfig->cohesionRange) {
                cohesionForce = Vector2Add(cohesionForce, flockState->boids[j].position);
                boidsInCohesionRange++;
            }

            // Debug values
            if (distanceToOtherBoid < 5.f) {
                flockState->collisionRate += GetFrameTime();
            }
        }

        // Apply

        // Separation
        flockState->boids[i].velocity =
                Vector2Add(flockState->boids[i].velocity, Vector2Scale(separationForce, flockConfig->separationFactor));

        // Alignment
        if (boidsInAlignmentRange > 0) {
            alignmentForce = Vector2Scale(alignmentForce, 1.f / (float) boidsInAlignmentRange);
            alignmentForce = Vector2Subtract(alignmentForce, flockState->boids[i].velocity);
            flockState->boids[i].velocity = Vector2Add(flockState->boids[i].velocity,
                                                       Vector2Scale(alignmentForce, flockConfig->alignmentFactor));
        }

        // Cohesion
        if (boidsInCohesionRange > 0) {
            cohesionForce = Vector2Scale(cohesionForce, 1.f / (float) boidsInCohesionRange);
            cohesionForce = Vector2Subtract(cohesionForce, flockState->boids[i].position);
            flockState->boids[i].velocity =
                    Vector2Add(flockState->boids[i].velocity, Vector2Scale(cohesionForce, flockConfig->cohesionFactor));
        }

        // Clamp boid speed
        if (flockConfig->clampSpeed) {
            float speed = Vector2Length(flockState->boids[i].velocity);
            if (speed > flockConfig->maximumSpeed) {
                flockState->boids[i].velocity =
                        Vector2Scale(flockState->boids[i].velocity, (1.f / speed) * flockConfig->maximumSpeed);
            } else if (speed < flockConfig->minimumSpeed) {
                flockState->boids[i].velocity =
                        Vector2Scale(flockState->boids[i].velocity, (1.f / speed) * flockConfig->minimumSpeed);
            }
        }

        // Update position
        flockState->boids[i].position.x += flockState->boids[i].velocity.x * GetFrameTime();
        flockState->boids[i].position.y += flockState->boids[i].velocity.y * GetFrameTime();

        // Loop around screen edges
        if (flockState->boids[i].position.x < flockConfig->flockBounds.x)
            flockState->boids[i].position.x = flockConfig->flockBounds.x + flockConfig->flockBounds.width;
        if (flockState->boids[i].position.x > flockConfig->flockBounds.x + flockConfig->flockBounds.width)
            flockState->boids[i].position.x = flockConfig->flockBounds.x;
        if (flockState->boids[i].position.y < flockConfig->flockBounds.y)
            flockState->boids[i].position.y = flockConfig->flockBounds.y + flockConfig->flockBounds.height;
        if (flockState->boids[i].position.y > flockConfig->flockBounds.y + flockConfig->flockBounds.height)
            flockState->boids[i].position.y = flockConfig->flockBounds.y;
    }
}

void DestroyFlock(struct FlockState *flockState) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DestroyFlock: Recieved NULL pointer to flockState.");
        return;
    }
    if (flockState->boids != NULL) {
        free(flockState->boids);
        flockState->boids = NULL;
    }
    flockState->boidsCount = 0;
}
