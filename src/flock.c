#include "flock.h"

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
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

enum FlockConfigValidationResult {
    FLOCK_CONFIG_VALID = 0,
    FLOCK_CONFIG_INVALID_NULL,
    FLOCK_CONFIG_INVALID_BOID_COUNT,
    FLOCK_CONFIG_INVALID_BOUNDS,
    FLOCK_CONFIG_INVALID_SPEED_RANGE,
    FLOCK_CONFIG_INVALID_RANGE
};

// Internal function that returns a human-readable error message for a flock config validation result
static const char *FlockConfigValidationMessage(enum FlockConfigValidationResult validationResult) {
    switch (validationResult) {
        case FLOCK_CONFIG_VALID:
            return "configuration is valid";
        case FLOCK_CONFIG_INVALID_NULL:
            return "configuration is NULL";
        case FLOCK_CONFIG_INVALID_BOID_COUNT:
            return "number of boids must be greater than 0";
        case FLOCK_CONFIG_INVALID_BOUNDS:
            return "flock bounds must have a positive width and height";
        case FLOCK_CONFIG_INVALID_SPEED_RANGE:
            return "speed range invalid: minimum must be <= maximum and both must be greater than 0";
        case FLOCK_CONFIG_INVALID_RANGE:
            return "force ranges must be non-negative";
        default:
            return "unknown validation error";
    }
}

// Internal function for validating flock configs
static enum FlockConfigValidationResult ValidateFlockConfig(const struct FlockConfig *config) {
    if (config == NULL)
        return FLOCK_CONFIG_INVALID_NULL;
    if (config->numberOfBoids <= 0)
        return FLOCK_CONFIG_INVALID_BOID_COUNT;
    if (config->flockBounds.width <= 0.f || config->flockBounds.height <= 0.f)
        return FLOCK_CONFIG_INVALID_BOUNDS;
    if (config->clampSpeed && (config->minimumSpeed > config->maximumSpeed || config->minimumSpeed < 0.f))
        return FLOCK_CONFIG_INVALID_SPEED_RANGE;
    if (config->separationRange < 0.f || config->alignmentRange < 0.f || config->cohesionRange < 0.f)
        return FLOCK_CONFIG_INVALID_RANGE;

    // NOTE: Negative flock factors are not considered invalid.

    return FLOCK_CONFIG_VALID;
}

bool InitializeFlock(struct FlockState *flockState, const struct FlockConfig config) {
    enum FlockConfigValidationResult validationResult = ValidateFlockConfig(&config);
    if (validationResult != FLOCK_CONFIG_VALID) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed due to invalid flock config, %s.",
                 FlockConfigValidationMessage(validationResult));
        return false;
    }

    Boid *boids =
            SpawnBoids(config.numberOfBoids, config.flockBounds, (config.minimumSpeed + config.maximumSpeed) / 2.f);

    if (boids == NULL) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed to allocate boids.");
        return false;
    }

    *flockState = (struct FlockState) {.boids = boids,
                                       .boidsCount = config.numberOfBoids,
                                       .config = config,
                                       .collisionRate = 0.f,
                                       .collisionRateMeasurementStart = (float) GetTime()};

    return true;
}

void ModifyFlockConfig(struct FlockState *flockState, struct FlockConfig newConfig) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "ModifyFlockConfig: Recieved NULL pointer to flockState.");
        return;
    }

    // NOTE: We don't check if the config has actually changed here because although this function will likely be called
    // every frame when a GUI is used, comparing the configs for an early exit will (probably?) not be much faster

    enum FlockConfigValidationResult validationResult = ValidateFlockConfig(&newConfig);
    if (validationResult != FLOCK_CONFIG_VALID) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed due to invalid flock config, %s.",
                 FlockConfigValidationMessage(validationResult));
        return;
    }

    flockState->config = newConfig;
}

void UpdateFlock(struct FlockState *flockState) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Recieved NULL pointer to flockState.");
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
            if (distanceToOtherBoid < flockState->config.separationRange && distanceToOtherBoid > 0.001f) {
                Vector2 separationOffset =
                        Vector2Subtract(flockState->boids[i].position, flockState->boids[j].position);
                separationForce = Vector2Add(separationForce,
                                             Vector2Scale(separationOffset, 1.f / powf(distanceToOtherBoid, 2.f)));
            }

            // Alignment
            // Adjusts the velocity towards the average velocity of the boids within range.
            if (distanceToOtherBoid < flockState->config.alignmentRange) {
                alignmentForce = Vector2Add(alignmentForce, flockState->boids[j].velocity);
                boidsInAlignmentRange++;
            }

            // Cohesion
            // A force towards the centre of the boids within range.
            if (distanceToOtherBoid < flockState->config.cohesionRange) {
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
        flockState->boids[i].velocity = Vector2Add(flockState->boids[i].velocity,
                                                   Vector2Scale(separationForce, flockState->config.separationFactor));

        // Alignment
        if (boidsInAlignmentRange > 0) {
            alignmentForce = Vector2Scale(alignmentForce, 1.f / (float) boidsInAlignmentRange);
            alignmentForce = Vector2Subtract(alignmentForce, flockState->boids[i].velocity);
            flockState->boids[i].velocity = Vector2Add(
                    flockState->boids[i].velocity, Vector2Scale(alignmentForce, flockState->config.alignmentFactor));
        }

        // Cohesion
        if (boidsInCohesionRange > 0) {
            cohesionForce = Vector2Scale(cohesionForce, 1.f / (float) boidsInCohesionRange);
            cohesionForce = Vector2Subtract(cohesionForce, flockState->boids[i].position);
            flockState->boids[i].velocity = Vector2Add(flockState->boids[i].velocity,
                                                       Vector2Scale(cohesionForce, flockState->config.cohesionFactor));
        }

        // Clamp boid speed
        if (flockState->config.clampSpeed) {
            float speed = Vector2Length(flockState->boids[i].velocity);
            if (speed > flockState->config.maximumSpeed) {
                flockState->boids[i].velocity =
                        Vector2Scale(flockState->boids[i].velocity, (1.f / speed) * flockState->config.maximumSpeed);
            } else if (speed < flockState->config.minimumSpeed) {
                flockState->boids[i].velocity =
                        Vector2Scale(flockState->boids[i].velocity, (1.f / speed) * flockState->config.minimumSpeed);
            }
        }

        // Update position
        flockState->boids[i].position.x += flockState->boids[i].velocity.x * GetFrameTime();
        flockState->boids[i].position.y += flockState->boids[i].velocity.y * GetFrameTime();

        // Loop around screen edges
        if (flockState->boids[i].position.x < flockState->config.flockBounds.x)
            flockState->boids[i].position.x = flockState->config.flockBounds.x + flockState->config.flockBounds.width;
        if (flockState->boids[i].position.x > flockState->config.flockBounds.x + flockState->config.flockBounds.width)
            flockState->boids[i].position.x = flockState->config.flockBounds.x;
        if (flockState->boids[i].position.y < flockState->config.flockBounds.y)
            flockState->boids[i].position.y = flockState->config.flockBounds.y + flockState->config.flockBounds.height;
        if (flockState->boids[i].position.y > flockState->config.flockBounds.y + flockState->config.flockBounds.height)
            flockState->boids[i].position.y = flockState->config.flockBounds.y;
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
