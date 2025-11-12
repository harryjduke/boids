#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

#include "boid.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1600;
    const int screenHeight = 900;

    int numberOfBoids = 100;

    float separationFactor = 1.f;
    float alignmentFactor = 0.01f;
    float cohesionFactor = 0.005f;

    float separationRange = 50.f;
    float alignmentRange = 100.f;
    float cohesionRange = 100.f;

    bool clampSpeed = true;
    float minimumSpeed = 10.f;
    float maximumSpeed = 50.f;

    bool showRanges = false;
    bool showFPS = false;

    // Debug values
    float collisionRate = 0.f;
    float collisionRateMeasurementStart = 0.f;

    Boid *boids = SpawnBoids(numberOfBoids, (Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight},
                             (minimumSpeed + maximumSpeed) / 2.f);

    // GUI
    bool separationFactorSpinnerEditMode = false;
    bool alignmentFactorSpinnerEditMode = false;
    bool cohesionFactorSpinnerEditMode = false;

    bool separationRangeSpinnerEditMode = false;
    bool alignmentRangeSpinnerEditMode = false;
    bool cohesionRangeSpinnerEditMode = false;

    bool minimumSpeedSpinnerEditMode = false;
    bool maximumSpeedSpinnerEditMode = false;

    bool numberOfBoidsSpinnerEditMode = false;

    const float padding = 5.f;
    const float panelWidth = 200.f;
    const float panelHeight = (float) screenHeight;
    const float headingHeight = 15.f;
    const float spinnerHeight = 20.f;
    const float spinnerWidth = 90.f;
    const float checkboxHeight = 10.f;
    const float buttonHeight = 20.f;


    InitWindow(screenWidth, screenHeight, "Boids");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update

        for (int i = 0; i < numberOfBoids; i++) {
            Vector2 separationForce = Vector2Zero();
            Vector2 alignmentForce = Vector2Zero();
            int boidsInAlignmentRange = 0;
            Vector2 cohesionForce = Vector2Zero();
            int boidsInCohesionRange = 0;

            // Calculate
            for (int j = 0; j < numberOfBoids; j++) {
                if (i == j)
                    continue;
                const float distanceToOtherBoid = Vector2Distance(boids[i].position, boids[j].position);

                // Separation
                // A force pushing away from other boids, the smaller distance between the boids, the
                // stronger the force.
                if (distanceToOtherBoid < separationRange && distanceToOtherBoid > 0.001f) {
                    Vector2 separationOffset = Vector2Subtract(boids[i].position, boids[j].position);
                    separationForce = Vector2Add(separationForce,
                                                 Vector2Scale(separationOffset, 1.f / powf(distanceToOtherBoid, 2.f)));
                }

                // Alignment
                // Adjusts the velocity towards the average velocity of the boids within range.
                if (distanceToOtherBoid < alignmentRange) {
                    alignmentForce = Vector2Add(alignmentForce, boids[j].velocity);
                    boidsInAlignmentRange++;
                }

                // Cohesion
                // A force towards the centre of the boids within range.
                if (distanceToOtherBoid < cohesionRange) {
                    cohesionForce = Vector2Add(cohesionForce, boids[j].position);
                    boidsInCohesionRange++;
                }

                // Debug values
                if (distanceToOtherBoid < 5.f) {
                    collisionRate += GetFrameTime();
                }
            }

            // Apply

            // Separation
            boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(separationForce, separationFactor));

            // Alignment
            if (boidsInAlignmentRange > 0) {
                alignmentForce = Vector2Scale(alignmentForce, 1.f / (float) boidsInAlignmentRange);
                alignmentForce = Vector2Subtract(alignmentForce, boids[i].velocity);
                boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(alignmentForce, alignmentFactor));
            }

            // Cohesion
            if (boidsInCohesionRange > 0) {
                cohesionForce = Vector2Scale(cohesionForce, 1.f / (float) boidsInCohesionRange);
                cohesionForce = Vector2Subtract(cohesionForce, boids[i].position);
                boids[i].velocity = Vector2Add(boids[i].velocity, Vector2Scale(cohesionForce, cohesionFactor));
            }

            // Clamp boid speed
            if (clampSpeed) {
                float speed = Vector2Length(boids[i].velocity);
                if (speed > maximumSpeed) {
                    boids[i].velocity = Vector2Scale(boids[i].velocity, (1.f / speed) * maximumSpeed);
                } else if (speed < minimumSpeed) {
                    boids[i].velocity = Vector2Scale(boids[i].velocity, (1.f / speed) * minimumSpeed);
                }
            }

            // Update position
            boids[i].position.x += boids[i].velocity.x * GetFrameTime();
            boids[i].position.y += boids[i].velocity.y * GetFrameTime();

            // Loop around screen edges
            if (boids[i].position.x < 0.f)
                boids[i].position.x = (float) screenWidth;
            if (boids[i].position.x > (float) screenWidth)
                boids[i].position.x = 0.f;
            if (boids[i].position.y < 0.f)
                boids[i].position.y = (float) screenHeight;
            if (boids[i].position.y > (float) screenHeight)
                boids[i].position.y = 0.f;
        }

        // Draw
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Debug - draw ranges on the first boid
        if (showRanges) {
            DrawCircleV(boids[0].position, separationRange, Fade(GRAY, 0.5f));
        DrawCircleV(boids[0].position, alignmentRange, Fade(GRAY, 0.5f));
            DrawCircleV(boids[0].position, cohesionRange, Fade(GRAY, 0.5f));
        }

        for (int i = 0; i < numberOfBoids; i++) {
            DrawBoid(boids[i]);
        }

        // GUI
        GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
        GuiSetStyle(SPINNER, TEXT_PADDING, (int) padding);

        GuiPanel((Rectangle) {0.f, 0.f, panelWidth, panelHeight}, "Boid Parameters");
        float heightOffset = 25.f + padding;

        GuiDrawText("Force Factors", (Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), headingHeight},
                    TEXT_ALIGN_LEFT, DARKGRAY);
        heightOffset += headingHeight + padding;

        int separationFactorValue = (int) (separationFactor * 100.f);
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Separation",
                       &separationFactorValue, 0, 10000, separationFactorSpinnerEditMode))
            separationFactorSpinnerEditMode = !separationFactorSpinnerEditMode;
        separationFactor = (float) separationFactorValue / 100.f;
        heightOffset += spinnerHeight + padding;

        int alignmentFactorValue = (int) (alignmentFactor * 10000.f);
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Alignment",
                       &alignmentFactorValue, 0, 10000, alignmentFactorSpinnerEditMode))
            alignmentFactorSpinnerEditMode = !alignmentFactorSpinnerEditMode;
        alignmentFactor = (float) alignmentFactorValue / 10000.f;
        heightOffset += spinnerHeight + padding;

        int cohesionFactorValue = (int) (cohesionFactor * 10000.f);
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Cohesion",
                       &cohesionFactorValue, 0, 10000, cohesionFactorSpinnerEditMode))
            cohesionFactorSpinnerEditMode = !cohesionFactorSpinnerEditMode;
        cohesionFactor = (float) cohesionFactorValue / 10000.f;
        heightOffset += spinnerHeight + padding;

        GuiDrawText("Force Ranges", (Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), headingHeight},
                    TEXT_ALIGN_LEFT, DARKGRAY);
        heightOffset += headingHeight + padding;

        int separationRangeValue = (int) separationRange;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Separation",
                       &separationRangeValue, 0, 1000, separationRangeSpinnerEditMode))
            separationRangeSpinnerEditMode = !separationRangeSpinnerEditMode;
        separationRange = (float) separationRangeValue;
        heightOffset += spinnerHeight + padding;

        int alignmentRangeValue = (int) alignmentRange;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Alignment",
                       &alignmentRangeValue, 0, 1000, alignmentRangeSpinnerEditMode))
            alignmentRangeSpinnerEditMode = !alignmentRangeSpinnerEditMode;
        alignmentRange = (float) alignmentRangeValue;
        heightOffset += spinnerHeight + padding;

        int cohesionRangeValue = (int) cohesionRange;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Cohesion",
                       &cohesionRangeValue, 0, 1000, cohesionRangeSpinnerEditMode))
            cohesionRangeSpinnerEditMode = !cohesionRangeSpinnerEditMode;
        cohesionRange = (float) cohesionRangeValue;
        heightOffset += spinnerHeight + padding;

        GuiCheckBox((Rectangle) {padding, heightOffset, checkboxHeight, checkboxHeight}, "Show Ranges", &showRanges);
        heightOffset += checkboxHeight + padding;

        GuiDrawText("Speed", (Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), headingHeight},
                    TEXT_ALIGN_LEFT, DARKGRAY);
        heightOffset += headingHeight + padding;

        GuiCheckBox((Rectangle) {padding, heightOffset, checkboxHeight, checkboxHeight}, "Clamp Speed", &clampSpeed);
        heightOffset += checkboxHeight + padding;

        if (!clampSpeed)
            GuiDisable();

        int minimumSpeedValue = (int) minimumSpeed;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Minimum Speed",
                       &minimumSpeedValue, 0, 1000, minimumSpeedSpinnerEditMode))
            minimumSpeedSpinnerEditMode = !minimumSpeedSpinnerEditMode;
        minimumSpeed = (float) minimumSpeedValue;
        heightOffset += spinnerHeight + padding;

        int maximumSpeedValue = (int) maximumSpeed;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Maximum Speed",
                       &maximumSpeedValue, 0, 1000, maximumSpeedSpinnerEditMode))
            maximumSpeedSpinnerEditMode = !maximumSpeedSpinnerEditMode;
        maximumSpeed = (float) maximumSpeedValue;
        heightOffset += spinnerHeight + padding;

        GuiEnable();

        GuiDrawText("Boids", (Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), headingHeight},
                    TEXT_ALIGN_LEFT, DARKGRAY);
        heightOffset += headingHeight + padding;

        int newNumberOfBoids = numberOfBoids;
        if (GuiSpinner((Rectangle) {padding, heightOffset, spinnerWidth, spinnerHeight}, "Number of Boids",
                       &newNumberOfBoids, 0, 10000, numberOfBoidsSpinnerEditMode))
            numberOfBoidsSpinnerEditMode = !numberOfBoidsSpinnerEditMode;
        if (newNumberOfBoids != numberOfBoids) {
            free(boids);
            boids = SpawnBoids(newNumberOfBoids, (Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight},
                               (minimumSpeed + maximumSpeed) / 2.f);
            numberOfBoids = newNumberOfBoids;
        }
        heightOffset += spinnerHeight + padding;

        if (GuiButton((Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), buttonHeight}, "Reset Boids")) {
            boids = SpawnBoids(numberOfBoids, (Rectangle) {0.f, 0.f, (float) screenWidth, (float) screenHeight},
                               (minimumSpeed + maximumSpeed) / 2.f);
            collisionRate = 0.f;
            collisionRateMeasurementStart = (float) GetTime();
        }
        heightOffset += buttonHeight + padding;

        GuiCheckBox((Rectangle) {padding, heightOffset, checkboxHeight, checkboxHeight}, "Show FPS", &showFPS);
        heightOffset += checkboxHeight + padding;

        GuiDrawText(TextFormat("Collision Rate: %f", collisionRate / GetTime()),
                    (Rectangle) {padding, heightOffset, panelWidth - (padding * 2.f), headingHeight}, TEXT_ALIGN_LEFT,
                    DARKGRAY);

        if (showFPS) {
            DrawText(TextFormat("FPS: %d", GetFPS()), screenWidth - 40 - (int) padding, (int) padding, 10, MAROON);
        }

        EndDrawing();
    }

    free(boids);
    CloseWindow();
    return EXIT_SUCCESS;
}
