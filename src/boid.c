#include "boid.h"

#include <raylib.h>
#include <raymath.h>

void DrawBoid(const Boid *boid) {
    const Vector2 forwardVector = Vector2Normalize(boid->velocity);
    Vector2 perpRight = (Vector2){.x = forwardVector.y, .y = -forwardVector.x};
    Vector2 perpLeft = (Vector2){.x = -forwardVector.y, .y = forwardVector.x};
    Vector2 vertex1 = Vector2Scale(forwardVector, BOID_LENGTH / 2.F);
    Vector2 vertex2 = Vector2Add(Vector2Negate(vertex1), Vector2Scale(perpRight, BOID_WIDTH / 2.F));
    Vector2 vertex3 = Vector2Add(Vector2Negate(vertex1), Vector2Scale(perpLeft, BOID_WIDTH / 2.F));

    // Transform vertices from local space to world space
    vertex1 = Vector2Add(vertex1, boid->position);
    vertex2 = Vector2Add(vertex2, boid->position);
    vertex3 = Vector2Add(vertex3, boid->position);

    DrawTriangle(vertex1, vertex2, vertex3, BLUE);
}
