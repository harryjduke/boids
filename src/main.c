#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

void spawnBoids(Vector2* boidsArray, int numberOfBoids, Vector2 spawnPoint, Vector2 spawnBounds) {
	for (int i = 0; i < numberOfBoids; i++) {
		boidsArray[i] = (Vector2){GetRandomValue(spawnPoint.x, spawnBounds.x), GetRandomValue(spawnPoint.y, spawnBounds.y)};
	}
}

int main(int argc, char *argv[])
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	const int numberOfBoids = 10;

	Vector2 boids[numberOfBoids];
	spawnBoids(boids, numberOfBoids, (Vector2){0, 0}, (Vector2){screenWidth, screenHeight});

	InitWindow(screenWidth, screenHeight, "Boids");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {

		// Draw
		BeginDrawing();

		ClearBackground(RAYWHITE);

		for (int i = 0; i < numberOfBoids; i++) {
			DrawCircle(boids[i].x, boids[i].y, 5.f, MAROON);
		}

		DrawText("Boids", 10, 10, 20, DARKGRAY);
		
		EndDrawing();
	}

	CloseWindow();
	return EXIT_SUCCESS;
}
