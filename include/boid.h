#ifndef BOID_H
#define BOID_H

#include <raylib.h>

typedef struct {
    Vector2 position;
    Vector2 velocity;
} Boid;

#define BOID_LENGTH 10.f
#define BOID_WIDTH 7.5f

#define BOID_MAX_SPEED 50.f
#define BOID_MIN_SPEED 10.f

void DrawBoid(Boid boid);

void SpawnBoids(Boid *boidsArray, int numberOfBoids, Rectangle spawnBounds);

#endif // !BOID_H
