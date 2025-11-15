#ifndef FLOCK_H
#define FLOCK_H

#include <raylib.h>
#include <stdbool.h>

#include "boid.h"

// Configuration for boid flock
struct FlockConfig {
    // Bounds
    Rectangle flockBounds;

    // Boids
    int numberOfBoids;

    // Force factors
    float separationFactor;
    float alignmentFactor;
    float cohesionFactor;

    // Force ranges
    float separationRange;
    float cohesionRange;
    float alignmentRange;

    // Speed
    bool clampSpeed;
    float minimumSpeed;
    float maximumSpeed;
};

// State of boids flock
struct FlockState {
    Boid *boids;
    int boidsCount;

    struct FlockConfig flockConfig;

    // Debug values
    float collisionRate;
    float collisionRateMeasurementStart;
};

struct FlockConfig CreateDefaultFlockConfig(Rectangle flockBounds);

bool InitializeFlock(struct FlockState *flockState, struct FlockConfig flockConfig);

void ModifyFlockConfig(struct FlockState *flockState, struct FlockConfig newFlockConfig);

void UpdateFlock(struct FlockState *flockState);

void DestroyFlock(struct FlockState *flockState);

#endif // !BOID_FLOCK_H
