#ifndef BOID_H
#define BOID_H

#include <raylib.h>

typedef struct {
    Vector2 position;
    Vector2 velocity;

    // Debug values
    Vector2 separationVector;
    Vector2 alignmentVector;
    Vector2 cohesionVector;
} Boid;

#define BOID_LENGTH 10.f
#define BOID_WIDTH 7.5f

void DrawBoid(Boid boid);

#endif // !BOID_H
