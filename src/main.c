#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

int main(int argc, char *argv[])
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "Boids");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {

		// Draw
		BeginDrawing();

		ClearBackground(RAYWHITE);
		DrawText("Boids", 10, 10, 20, DARKGRAY);
		
		EndDrawing();
	}

	CloseWindow();
	return EXIT_SUCCESS;
}
