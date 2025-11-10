#include "boid.h"

#include <raylib.h>
#include <raymath.h>

void DrawBoid(const Boid boid) {
    const Vector2 forwardVector = Vector2Normalize(boid.velocity);
    Vector2 vertex1 = Vector2Scale(forwardVector, BOID_LENGTH / 2.f);
    Vector2 vertex2 = Vector2Add(Vector2Negate(vertex1),
                                 Vector2Scale((Vector2) {forwardVector.y, -forwardVector.x}, BOID_WIDTH / 2.f));
    Vector2 vertex3 = Vector2Add(Vector2Negate(vertex1),
                                 Vector2Scale((Vector2) {-forwardVector.y, forwardVector.x}, BOID_WIDTH / 2.f));

    // Transform vertices from local space to world space
    vertex1 = Vector2Add(vertex1, boid.position);
    vertex2 = Vector2Add(vertex2, boid.position);
    vertex3 = Vector2Add(vertex3, boid.position);

    DrawTriangle(vertex1, vertex2, vertex3, BLUE);
}

void SpawnBoids(Boid *boidsArray, const int numberOfBoids, const Rectangle spawnBounds) {
    for (int i = 0; i < numberOfBoids; i++) {
        boidsArray[i] = (Boid) {
                (Vector2) {(float) GetRandomValue((int) spawnBounds.x, (int) (spawnBounds.x + spawnBounds.width)),
                           (float) GetRandomValue((int) spawnBounds.y, (int) (spawnBounds.y + spawnBounds.height))},
                Vector2Scale(
                        Vector2Normalize((Vector2) {(float) GetRandomValue(0, 100), (float) GetRandomValue(0, 100)}),
                        (BOID_MIN_SPEED + BOID_MAX_SPEED) / 2.f)};
    }
}
