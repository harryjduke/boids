#include "flock.h"

#include "boid.h"

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

struct FlockConfig CreateDefaultFlockConfig(const Rectangle flockBounds) {
    return (struct FlockConfig){
        .flockBounds = flockBounds,
        .numberOfBoids = 100,

        .separationFactor = 1.F,
        .alignmentFactor = 1.F,
        .cohesionFactor = 1.F,

        .separationRange = 50.F,
        .alignmentRange = 100.F,
        .cohesionRange = 100.F,

        .normalizeForces = false,

        .clampSpeed = true,
        .minimumSpeed = 50.F,
        .maximumSpeed = 100.F,
    };
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
static enum FlockConfigValidationResult validateFlockConfig(const struct FlockConfig *config) {
    if (config == NULL) {
        return FLOCK_CONFIG_INVALID_NULL;
    }
    if (config->numberOfBoids <= 0) {
        return FLOCK_CONFIG_INVALID_BOID_COUNT;
    }
    if (config->flockBounds.width <= 0.F || config->flockBounds.height <= 0.F) {
        return FLOCK_CONFIG_INVALID_BOUNDS;
    }
    if (config->clampSpeed && (config->minimumSpeed > config->maximumSpeed || config->minimumSpeed < 0.F)) {
        return FLOCK_CONFIG_INVALID_SPEED_RANGE;
    }
    if (config->separationRange < 0.F || config->alignmentRange < 0.F || config->cohesionRange < 0.F) {
        return FLOCK_CONFIG_INVALID_RANGE;
    }

    // NOTE: Negative flock factors are not considered invalid.

    return FLOCK_CONFIG_VALID;
}

// Internal function to spawn a set number of boids at random positions in the given bounds.
static Boid *SpawnBoids(const int numberOfBoids, const Rectangle spawnBounds, const float startSpeed) {
    Boid *boids = malloc(sizeof(Boid) * numberOfBoids);
    if (boids == NULL) {
        TraceLog(LOG_ERROR, "SpawnBoids: Failed to allocate memory for %d boids.", numberOfBoids);
        return NULL;
    }
    for (int i = 0; i < numberOfBoids; i++) {
        float x = (float)GetRandomValue((int)spawnBounds.x, (int)(spawnBounds.x + spawnBounds.width));
        float y = (float)GetRandomValue((int)spawnBounds.y, (int)(spawnBounds.y + spawnBounds.height));
        Vector2 randomDirection = (Vector2){
            .x = (float)GetRandomValue(-100, 100),
            .y = (float)GetRandomValue(-100, 100),
        };

        boids[i] = (Boid){
            .position = (Vector2){.x = x, .y = y},
            .velocity = Vector2Scale(Vector2Normalize(randomDirection), startSpeed),
        };
    }
    return boids;
}

bool InitializeFlock(struct FlockState *flockState, const struct FlockConfig config) {
    enum FlockConfigValidationResult validationResult = validateFlockConfig(&config);
    if (validationResult != FLOCK_CONFIG_VALID) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed due to invalid flock config, %s.",
                 FlockConfigValidationMessage(validationResult));
        return false;
    }

    Boid *boids =
        SpawnBoids(config.numberOfBoids, config.flockBounds, (config.minimumSpeed + config.maximumSpeed) / 2.F);
    if (boids == NULL) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed to spawn boids.");
        return false;
    }

    // Pre-allocate memory for calculating the steering vectors
    Vector2 *steeringVectors = malloc(sizeof(Vector2) * config.numberOfBoids);
    if (steeringVectors == NULL) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed to allocate memory for the steering vectors for %d boids.",
                 config.numberOfBoids);
        // Initialisation failed, clean up.
        if (boids != NULL) {
            free(boids);
            boids = NULL;
        }
        return false;
    }

#ifdef DEBUG
    // Allocate memory for storing boid debug data
    struct Debug_BoidData *debug_boidData = malloc(sizeof(struct Debug_BoidData) * config.numberOfBoids);
    if (debug_boidData == NULL) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed to allocate memory for the debug data for %d boids.",
                 config.numberOfBoids);
        // Initialisation failed, clean up.
        if (boids != NULL) {
            free(boids);
            boids = NULL;
        }
        if (steeringVectors != NULL) {
            free(steeringVectors);
            steeringVectors = NULL;
        }
        return false;
    }
#endif /* ifdef DEBUG */

    *flockState = (struct FlockState){
        .boids = boids,
        .boidsCount = config.numberOfBoids,
        .steeringForces = steeringVectors,
        .config = config,
        .collisionTime = 0.F,
        .collisionTimeStart = (float)GetTime(),
#ifdef DEBUG
        .debug_boidData = debug_boidData,
#endif /* ifdef DEBUG */
    };

    return true;
}

void ModifyFlockConfig(struct FlockState *flockState, struct FlockConfig newConfig) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "ModifyFlockConfig: Recieved NULL pointer to flockState.");
        return;
    }

    // NOTE: We don't check if the config has actually changed here because although this function will likely be called
    // every frame when a GUI is used, comparing the configs for an early exit will (probably?) not be much faster

    enum FlockConfigValidationResult validationResult = validateFlockConfig(&newConfig);
    if (validationResult != FLOCK_CONFIG_VALID) {
        TraceLog(LOG_ERROR, "InitializeFlock: Failed due to invalid flock config, %s.",
                 FlockConfigValidationMessage(validationResult));
        return;
    }

    flockState->config = newConfig;
}

// Internal function that calculates the steering force (total separation, alignment and cohesion) for the given boid.
static Vector2 CalculateSteeringForce(int boidIndex, const struct FlockState *flockState) {
    const Boid *boid = &flockState->boids[boidIndex];

    // Accumulators
    Vector2 separationAccumulator = Vector2Zero();
    Vector2 averageVelocity = Vector2Zero();
    Vector2 centerOfMass = Vector2Zero();

    int boidsInSeparationRange = 0;
    int boidsInAlignmentRange = 0;
    int boidsInCohesionRange = 0;

#ifdef DEBUG
    float collisionTime = 0;
#endif /* ifdef DEBUG */

    for (int i = 0; i < flockState->boidsCount; i++) {
        if (boidIndex == i) {
            continue;
        }

        const Boid *otherBoid = &flockState->boids[i];
        const float distanceToOtherBoid = Vector2Distance(boid->position, otherBoid->position);

        // Separation
        // A force pushing away from other boids, the smaller distance between the boids, the
        // stronger the force.
        if (distanceToOtherBoid < flockState->config.separationRange && distanceToOtherBoid > EPSILON) {
            Vector2 offset = Vector2Subtract(boid->position, otherBoid->position);
            // Magnitude starts at 0 at the edge of the range and scales towards infinity
            float speed = (flockState->config.separationRange / distanceToOtherBoid) - 1;
            // Magnitude gets exponentially higher as the distance closes
            speed *= flockState->config.maximumSpeed;
            separationAccumulator = Vector2Add(separationAccumulator, Vector2Scale(Vector2Normalize(offset), speed));
            boidsInSeparationRange++;
        }

        // Alignment
        // Adjusts the velocity towards the average velocity of the boids within range.
        if (distanceToOtherBoid < flockState->config.alignmentRange) {
            averageVelocity = Vector2Add(averageVelocity, otherBoid->velocity);
            boidsInAlignmentRange++;
        }

        // Cohesion
        // A force towards the centre of the boids within range.
        if (distanceToOtherBoid < flockState->config.cohesionRange) {
            centerOfMass = Vector2Add(centerOfMass, otherBoid->position);
            boidsInCohesionRange++;
        }

#ifdef DEBUG
        if (distanceToOtherBoid < 5.F) {
            collisionTime += GetFrameTime();
        }
#endif /* ifdef DEBUG */
    }

    // Calculate steering forces
    Vector2 desiredSeparation = Vector2Zero();
    Vector2 desiredAlignment = Vector2Zero();
    Vector2 desiredCohesion = Vector2Zero();

    Vector2 separationSteeringForce = Vector2Zero();
    Vector2 alignmentSteeringForce = Vector2Zero();
    Vector2 cohesionSteeringForce = Vector2Zero();

    // Separation
    if (boidsInSeparationRange > 0) {
        desiredSeparation = Vector2ClampValue(separationAccumulator, 0.F, flockState->config.maximumSpeed);
        separationSteeringForce = Vector2Subtract(desiredSeparation, boid->velocity);
    }

    // Alignment
    if (boidsInAlignmentRange > 0) {
        averageVelocity = Vector2Scale(averageVelocity, 1.F / (float)boidsInAlignmentRange);
        desiredAlignment = Vector2ClampValue(averageVelocity, 0.F, flockState->config.maximumSpeed);
        alignmentSteeringForce = Vector2Subtract(desiredAlignment, boid->velocity);
    }

    // Cohesion
    if (boidsInCohesionRange > 0) {
        centerOfMass = Vector2Scale(centerOfMass, 1.F / (float)boidsInCohesionRange);
        desiredCohesion = Vector2Subtract(centerOfMass, boid->position);
        desiredCohesion = Vector2ClampValue(desiredCohesion, 0.F, flockState->config.maximumSpeed);
        cohesionSteeringForce = Vector2Subtract(desiredCohesion, boid->velocity);
    }

#ifdef DEBUG
    flockState->debug_boidData[boidIndex].separationVector = desiredSeparation;
    flockState->debug_boidData[boidIndex].alignmentVector = desiredAlignment;
    flockState->debug_boidData[boidIndex].cohesionVector = desiredCohesion;

    flockState->debug_boidData[boidIndex].collisionTime = collisionTime;
#endif /* ifdef DEBUG */

    // Scale each steering force by its weight
    separationSteeringForce = Vector2Scale(separationSteeringForce, flockState->config.separationFactor);
    alignmentSteeringForce = Vector2Scale(alignmentSteeringForce, flockState->config.alignmentFactor);
    cohesionSteeringForce = Vector2Scale(cohesionSteeringForce, flockState->config.cohesionFactor);

    Vector2 steeringForce = Vector2Add(separationSteeringForce, alignmentSteeringForce);
    steeringForce = Vector2Add(steeringForce, cohesionSteeringForce);

    return steeringForce;
}

// Internal function that updates the given boid's position by applying its velocity (clamped by min/max speed).
static void UpdateBoidPosition(Boid *boid, const struct FlockState *flockState) {
    // Clamp boid speed
    if (flockState->config.clampSpeed) {
        float speed = Vector2Length(boid->velocity);
        // TODO: Handle speed = 0 to avoid division by 0
        if (speed > flockState->config.maximumSpeed) {
            boid->velocity = Vector2Scale(boid->velocity, (1.F / speed) * flockState->config.maximumSpeed);
        } else if (speed < flockState->config.minimumSpeed) {
            boid->velocity = Vector2Scale(boid->velocity, (1.F / speed) * flockState->config.minimumSpeed);
        }
    }

    // Update position
    boid->position.x += boid->velocity.x * GetFrameTime();
    boid->position.y += boid->velocity.y * GetFrameTime();

    // Loop around screen edges
    if (boid->position.x < flockState->config.flockBounds.x) {
        boid->position.x = flockState->config.flockBounds.x + flockState->config.flockBounds.width;
    }
    if (boid->position.x > flockState->config.flockBounds.x + flockState->config.flockBounds.width) {
        boid->position.x = flockState->config.flockBounds.x;
    }
    if (boid->position.y < flockState->config.flockBounds.y) {
        boid->position.y = flockState->config.flockBounds.y + flockState->config.flockBounds.height;
    }
    if (boid->position.y > flockState->config.flockBounds.y + flockState->config.flockBounds.height) {
        boid->position.y = flockState->config.flockBounds.y;
    }
}

void UpdateFlock(struct FlockState *flockState) {
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "UpdateFlock: Recieved NULL pointer to flockState.");
        return;
    }

#ifdef DEBUG
    if (flockState->isPaused) {
        if (flockState->doStep) {
            flockState->doStep = false;
        } else {
            return;
        }
    }

    float totalCollisionTime = 0.F;
#endif /* ifdef DEBUG */

    for (int i = 0; i < flockState->boidsCount; i++) {
        Vector2 steeringForce = CalculateSteeringForce(i, flockState);

        flockState->steeringForces[i] = steeringForce;

        #ifdef DEBUG
            totalCollisionTime += flockState->debug_boidData[i].collisionTime;
        #endif /* ifdef DEBUG */
    }

#ifdef DEBUG
    flockState->collisionTime += totalCollisionTime;
#endif /* ifdef DEBUG */

    for (int i = 0; i < flockState->boidsCount; i++) {
        flockState->boids[i].velocity =
            Vector2Add(flockState->boids[i].velocity, Vector2Scale(flockState->steeringForces[i], GetFrameTime()));
        UpdateBoidPosition(&flockState->boids[i], flockState);
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

    if (flockState->steeringForces != NULL) {
        free(flockState->steeringForces);
        flockState->steeringForces = NULL;
    }
}
