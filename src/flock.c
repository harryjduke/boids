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

    *flockState = (struct FlockState){
        .boids = boids,
        .boidsCount = config.numberOfBoids,
        .steeringForces = steeringVectors,
        .config = config,
        .collisionTime = 0.F,
        .collisionTimeStart = (float)GetTime(),
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

// Internal function that calculates the steering vector (total separation, alignment and cohesion) for the given boid.
// NOTE: This function also has an out parameter that will be set to the collision time for the given boid.
static struct TargetVelocities {
    Vector2 separation;
    Vector2 alignment;
    Vector2 cohesion;
#ifdef DEBUG
    float collisionTime;
#endif /* ifdef DEBUG */
} CalculateTargetVelocities(int boidIndex, const struct FlockState *flockState) {
    const Boid *boid = &flockState->boids[boidIndex];

    struct TargetVelocities targetVelocities = {0};
    int boidsInSeparationRange = 0;
    int boidsInAlignmentRange = 0;
    int boidsInCohesionRange = 0;

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
            Vector2 separationOffset = Vector2Subtract(boid->position, otherBoid->position);
            // Magnitude starts at 0 at the edge of the range and scales towards infinity
            float magnitude = (flockState->config.separationRange / distanceToOtherBoid) - 1;
            // Magnitude gets exponentially higher as the distance closes
            // magnitude *= magnitude;
            targetVelocities.separation =
                Vector2Add(targetVelocities.separation, Vector2Scale(Vector2Normalize(separationOffset), magnitude));
            boidsInSeparationRange++;
        }

        // Alignment
        // Adjusts the velocity towards the average velocity of the boids within range.
        if (distanceToOtherBoid < flockState->config.alignmentRange) {
            targetVelocities.alignment = Vector2Add(targetVelocities.alignment, otherBoid->velocity);
            boidsInAlignmentRange++;
        }

        // Cohesion
        // A force towards the centre of the boids within range.
        if (distanceToOtherBoid < flockState->config.cohesionRange) {
            targetVelocities.cohesion = Vector2Add(targetVelocities.cohesion, otherBoid->position);
            boidsInCohesionRange++;
        }

#ifdef DEBUG
        if (distanceToOtherBoid < 5.F) {
            targetVelocities.collisionTime += GetFrameTime();
        }
#endif /* ifdef DEBUG */
    }

    if (boidsInAlignmentRange > 0) {
        targetVelocities.alignment = Vector2Scale(targetVelocities.alignment, 1.F / (float)boidsInAlignmentRange);
    }

    if (boidsInCohesionRange > 0) {
        targetVelocities.cohesion = Vector2Scale(targetVelocities.cohesion, 1.F / (float)boidsInCohesionRange);
        targetVelocities.cohesion = Vector2Subtract(targetVelocities.cohesion, boid->position);
    }

    if (flockState->config.normalizeForces) {
        targetVelocities.separation = Vector2Normalize(targetVelocities.separation);
        targetVelocities.alignment = Vector2Normalize(targetVelocities.alignment);
        targetVelocities.cohesion = Vector2Normalize(targetVelocities.cohesion);
        // separationVector = Vector2ClampValue(separationVector, 0.F, 1.F);
        // alignmentVector = Vector2ClampValue(alignmentVector, 0.F, 1.F);
        // cohesionVector = Vector2ClampValue(cohesionVector, 0.F, 1.F);
    }

    // Scale each velocity by its weight
    targetVelocities.separation = Vector2Scale(targetVelocities.separation, flockState->config.separationFactor);
    targetVelocities.alignment = Vector2Scale(targetVelocities.alignment, flockState->config.alignmentFactor);
    targetVelocities.cohesion = Vector2Scale(targetVelocities.cohesion, flockState->config.cohesionFactor);

    return targetVelocities;
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
        struct TargetVelocities targetVelocities = CalculateTargetVelocities(i, flockState);

        Vector2 totalTargetVelocity = Vector2Add(targetVelocities.separation, targetVelocities.alignment);
        totalTargetVelocity = Vector2Add(totalTargetVelocity, targetVelocities.cohesion);

        totalTargetVelocity = Vector2ClampValue(totalTargetVelocity, 0.F, flockState->config.maximumSpeed);

        Vector2 steeringForce = Vector2Subtract(totalTargetVelocity, flockState->boids[i].velocity);

        flockState->steeringForces[i] = steeringForce;
#ifdef DEBUG
        flockState->boids[i].separationVector = targetVelocities.separation;
        flockState->boids[i].alignmentVector = targetVelocities.alignment;
        flockState->boids[i].cohesionVector = targetVelocities.cohesion;
        totalCollisionTime += targetVelocities.collisionTime;
#endif /* ifdef DEBUG */
    }

#ifdef DEBUG
    flockState->collisionTime += totalCollisionTime;
#endif /* ifdef DEBUG */

    for (int i = 0; i < flockState->boidsCount; i++) {
        flockState->boids[i].velocity = Vector2Add(flockState->boids[i].velocity, flockState->steeringForces[i]);
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
