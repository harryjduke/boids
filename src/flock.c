#include "flock.h"

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>
#include "boid.h"

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
                                       .collisionTime = 0.f,
                                       .collisionTimeStart = (float) GetTime()};

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

// Internal function that calculates the steering vector (total separation, alignment and cohesion) for the given boid.
// NOTE: This function also has an out parameter that will be set to the collision time for the given boid.
static Vector2 CalculateSteeringVector(int boidIndex, const struct FlockState *flockState, float *outCollisionTime) {
    const Boid *boid = &flockState->boids[boidIndex];

    Vector2 separationVector = Vector2Zero();
    Vector2 alignmentVector = Vector2Zero();
    int boidsInAlignmentRange = 0;
    Vector2 cohesionVector = Vector2Zero();
    int boidsInCohesionRange = 0;

    float collisionTime = 0.f;

    for (int i = 0; i < flockState->boidsCount; i++) {
        if (boidIndex == i)
            continue;

        const Boid *otherBoid = &flockState->boids[i];

        const float distanceToOtherBoid = Vector2Distance(boid->position, otherBoid->position);

        // Separation
        // A force pushing away from other boids, the smaller distance between the boids, the
        // stronger the force.
        if (distanceToOtherBoid < flockState->config.separationRange && distanceToOtherBoid > EPSILON) {
            Vector2 separationOffset = Vector2Subtract(boid->position, otherBoid->position);
            separationVector =
                    Vector2Add(separationVector, Vector2Scale(separationOffset, 1.f / powf(distanceToOtherBoid, 2.f)));
        }

        // Alignment
        // Adjusts the velocity towards the average velocity of the boids within range.
        if (distanceToOtherBoid < flockState->config.alignmentRange) {
            alignmentVector = Vector2Add(alignmentVector, otherBoid->velocity);
            boidsInAlignmentRange++;
        }

        // Cohesion
        // A force towards the centre of the boids within range.
        if (distanceToOtherBoid < flockState->config.cohesionRange) {
            cohesionVector = Vector2Add(cohesionVector, otherBoid->position);
            boidsInCohesionRange++;
        }

        // Debug values
        if (distanceToOtherBoid < 5.f) {
            collisionTime += GetFrameTime();
        }
    }

    Vector2 steeringVector = Vector2Zero();

    // Separation
    steeringVector = Vector2Add(steeringVector, Vector2Scale(separationVector, flockState->config.separationFactor));

    // Alignment
    if (boidsInAlignmentRange > 0) {
        alignmentVector = Vector2Scale(alignmentVector, 1.f / (float) boidsInAlignmentRange);
        alignmentVector = Vector2Subtract(alignmentVector, boid->velocity);
        steeringVector = Vector2Add(steeringVector, Vector2Scale(alignmentVector, flockState->config.alignmentFactor));
    }

    // Cohesion
    if (boidsInCohesionRange > 0) {
        cohesionVector = Vector2Scale(cohesionVector, 1.f / (float) boidsInCohesionRange);
        cohesionVector = Vector2Subtract(cohesionVector, boid->position);
        steeringVector = Vector2Add(steeringVector, Vector2Scale(cohesionVector, flockState->config.cohesionFactor));
    }

    *outCollisionTime = collisionTime;
    return steeringVector;
}

// Internal function that updates the given boid's position by applying its velocity (clamped by min/max speed).
static void UpdateBoidPosition(Boid *boid, const struct FlockState *flockState) {
    // Clamp boid speed
    if (flockState->config.clampSpeed) {
        float speed = Vector2Length(boid->velocity);
        if (speed > flockState->config.maximumSpeed) {
            boid->velocity = Vector2Scale(boid->velocity, (1.f / speed) * flockState->config.maximumSpeed);
        } else if (speed < flockState->config.minimumSpeed) {
            boid->velocity = Vector2Scale(boid->velocity, (1.f / speed) * flockState->config.minimumSpeed);
        }
    }

    // Update position
    boid->position.x += boid->velocity.x * GetFrameTime();
    boid->position.y += boid->velocity.y * GetFrameTime();

    // Loop around screen edges
    if (boid->position.x < flockState->config.flockBounds.x)
        boid->position.x = flockState->config.flockBounds.x + flockState->config.flockBounds.width;
    if (boid->position.x > flockState->config.flockBounds.x + flockState->config.flockBounds.width)
        boid->position.x = flockState->config.flockBounds.x;
    if (boid->position.y < flockState->config.flockBounds.y)
        boid->position.y = flockState->config.flockBounds.y + flockState->config.flockBounds.height;
    if (boid->position.y > flockState->config.flockBounds.y + flockState->config.flockBounds.height)
        boid->position.y = flockState->config.flockBounds.y;
}

void UpdateFlock(struct FlockState *flockState) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Recieved NULL pointer to flockState.");
        return;
    }

    Vector2 *steeringVectors = malloc(sizeof(Vector2) * flockState->boidsCount);
    if (steeringVectors == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Failed to allocate memory for the steering vectors of %d boids.",
                 flockState->boidsCount);
        return;
   }

    float totalCollisionTime = 0.f;

    for (int i = 0; i < flockState->boidsCount; i++) {
        float boidCollisionTime = 0.f;
        steeringVectors[i] = CalculateSteeringVector(i, flockState, &boidCollisionTime);
        totalCollisionTime += boidCollisionTime;
    }

    flockState->collisionTime += totalCollisionTime;

    for (int i = 0; i < flockState->boidsCount; i++) {
        flockState->boids[i].velocity = Vector2Add(flockState->boids[i].velocity, steeringVectors[i]);
        UpdateBoidPosition(&flockState->boids[i], flockState);
    }

    free(steeringVectors);
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
