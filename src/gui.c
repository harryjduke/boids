#include "gui.h"
#include <raylib.h>
#include <stdbool.h>
#include "flock.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

struct GuiConfig CreateDefaultGuiConfig(float screenHeight) {
    return (struct GuiConfig) {.padding = 5.f,
                               .panelWidth = 200.f,
                               .panelHeight = screenHeight,
                               .headingHeight = 15.f,
                               .spinnerHeight = 20.f,
                               .spinnerWidth = 90.f,
                               .checkboxHeight = 10.f,
                               .buttonHeight = 20.f};
}

void InitializeParametersPanel(struct ParametersPanelState *parametersPanelState, const struct GuiConfig guiConfig) {
    // TODO: Validate config
    *parametersPanelState = (struct ParametersPanelState) {.separationFactorSpinnerEditMode = false,
                                                           .alignmentFactorSpinnerEditMode = false,
                                                           .cohesionFactorSpinnerEditMode = false,

                                                           .separationRangeSpinnerEditMode = false,
                                                           .alignmentRangeSpinnerEditMode = false,
                                                           .cohesionRangeSpinnerEditMode = false,

                                                           .minimumSpeedSpinnerEditMode = false,
                                                           .maximumSpeedSpinnerEditMode = false,

                                                           .showRanges = false,
                                                           .showFPS = false,

    .guiConfig = guiConfig};
}

void DrawBoidRanges(const struct ParametersPanelState *parametersPanelState, const struct FlockState *flockState,
                    const Boid *boid) {
    if (parametersPanelState == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved NULL pointer to parametersPanelState.");
        return;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved NULL pointer to flockState.");
        return;
    }
    if (boid == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved NULL pointer to boid.");
        return;
    }

    if (!parametersPanelState->showRanges)
        return;
    DrawCircleV(boid->position, flockState->flockConfig.separationRange, Fade(GRAY, 0.5f));
    DrawCircleV(boid->position, flockState->flockConfig.alignmentRange, Fade(GRAY, 0.5f));
    DrawCircleV(boid->position, flockState->flockConfig.cohesionRange, Fade(GRAY, 0.5f));
}

struct ParametersPanelResult DrawParametersPanel(struct ParametersPanelState *parametersPanelState,
                                                 const struct FlockState *flockState) {
    struct ParametersPanelResult result = {.resetBoids = false, .newFlockConfig = flockState->flockConfig};

    if (parametersPanelState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recived NULL pointer to parametersPanelState.");
        return result;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recived NULL pointer to flockState.");
        return result;
    }

    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiSetStyle(SPINNER, TEXT_PADDING, (int) parametersPanelState->guiConfig.padding);

    GuiPanel((Rectangle) {0.f, 0.f, parametersPanelState->guiConfig.panelWidth,
                          parametersPanelState->guiConfig.panelHeight},
             "Boid Parameters");
    float heightOffset = 25.f + parametersPanelState->guiConfig.padding;

    GuiDrawText(
            "Force Factors",
            (Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                         parametersPanelState->guiConfig.panelWidth - (parametersPanelState->guiConfig.padding * 2.f),
                         parametersPanelState->guiConfig.headingHeight},
            TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->guiConfig.headingHeight + parametersPanelState->guiConfig.padding;

    int separationFactorValue = (int) (result.newFlockConfig.separationFactor * 100.f);
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Separation", &separationFactorValue, 0, 10000,
                   parametersPanelState->separationFactorSpinnerEditMode))
        parametersPanelState->separationFactorSpinnerEditMode = !parametersPanelState->separationFactorSpinnerEditMode;
    result.newFlockConfig.separationFactor = (float) separationFactorValue / 100.f;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    int alignmentFactorValue = (int) (result.newFlockConfig.alignmentFactor * 10000.f);
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Alignment", &alignmentFactorValue, 0, 10000, parametersPanelState->alignmentFactorSpinnerEditMode))
        parametersPanelState->alignmentFactorSpinnerEditMode = !parametersPanelState->alignmentFactorSpinnerEditMode;
    result.newFlockConfig.alignmentFactor = (float) alignmentFactorValue / 10000.f;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    int cohesionFactorValue = (int) (result.newFlockConfig.cohesionFactor * 10000.f);
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Cohesion", &cohesionFactorValue, 0, 10000, parametersPanelState->cohesionFactorSpinnerEditMode))
        parametersPanelState->cohesionFactorSpinnerEditMode = !parametersPanelState->cohesionFactorSpinnerEditMode;
    result.newFlockConfig.cohesionFactor = (float) cohesionFactorValue / 10000.f;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    GuiDrawText(
            "Force Ranges",
            (Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                         parametersPanelState->guiConfig.panelWidth - (parametersPanelState->guiConfig.padding * 2.f),
                         parametersPanelState->guiConfig.headingHeight},
            TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->guiConfig.headingHeight + parametersPanelState->guiConfig.padding;

    int separationRangeValue = (int) result.newFlockConfig.separationRange;
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Separation", &separationRangeValue, 0, 1000, parametersPanelState->separationRangeSpinnerEditMode))
        parametersPanelState->separationRangeSpinnerEditMode = !parametersPanelState->separationRangeSpinnerEditMode;
    result.newFlockConfig.separationRange = (float) separationRangeValue;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    int alignmentRangeValue = (int) result.newFlockConfig.alignmentRange;
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Alignment", &alignmentRangeValue, 0, 1000, parametersPanelState->alignmentRangeSpinnerEditMode))
        parametersPanelState->alignmentRangeSpinnerEditMode = !parametersPanelState->alignmentRangeSpinnerEditMode;
    result.newFlockConfig.alignmentRange = (float) alignmentRangeValue;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    int cohesionRangeValue = (int) result.newFlockConfig.cohesionRange;
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Cohesion", &cohesionRangeValue, 0, 1000, parametersPanelState->cohesionRangeSpinnerEditMode))
        parametersPanelState->cohesionRangeSpinnerEditMode = !parametersPanelState->cohesionRangeSpinnerEditMode;
    result.newFlockConfig.cohesionRange = (float) cohesionRangeValue;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    GuiCheckBox((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                             parametersPanelState->guiConfig.checkboxHeight,
                             parametersPanelState->guiConfig.checkboxHeight},
                "Show Ranges", &parametersPanelState->showRanges);
    heightOffset += parametersPanelState->guiConfig.checkboxHeight + parametersPanelState->guiConfig.padding;

    GuiDrawText(
            "Speed",
            (Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                         parametersPanelState->guiConfig.panelWidth - (parametersPanelState->guiConfig.padding * 2.f),
                         parametersPanelState->guiConfig.headingHeight},
            TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->guiConfig.headingHeight + parametersPanelState->guiConfig.padding;

    GuiCheckBox((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                             parametersPanelState->guiConfig.checkboxHeight,
                             parametersPanelState->guiConfig.checkboxHeight},
                "Clamp Speed", &result.newFlockConfig.clampSpeed);
    heightOffset += parametersPanelState->guiConfig.checkboxHeight + parametersPanelState->guiConfig.padding;

    if (!result.newFlockConfig.clampSpeed)
        GuiDisable();

    int minimumSpeedValue = (int) result.newFlockConfig.minimumSpeed;
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Minimum Speed", &minimumSpeedValue, 0, 1000, parametersPanelState->minimumSpeedSpinnerEditMode))
        parametersPanelState->minimumSpeedSpinnerEditMode = !parametersPanelState->minimumSpeedSpinnerEditMode;
    result.newFlockConfig.minimumSpeed = (float) minimumSpeedValue;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    int maximumSpeedValue = (int) result.newFlockConfig.maximumSpeed;
    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Maximum Speed", &maximumSpeedValue, 0, 1000, parametersPanelState->maximumSpeedSpinnerEditMode))
        parametersPanelState->maximumSpeedSpinnerEditMode = !parametersPanelState->maximumSpeedSpinnerEditMode;
    result.newFlockConfig.maximumSpeed = (float) maximumSpeedValue;
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    GuiEnable();

    GuiDrawText(
            "Boids",
            (Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                         parametersPanelState->guiConfig.panelWidth - (parametersPanelState->guiConfig.padding * 2.f),
                         parametersPanelState->guiConfig.headingHeight},
            TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->guiConfig.headingHeight + parametersPanelState->guiConfig.padding;

    if (GuiSpinner((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                                parametersPanelState->guiConfig.spinnerWidth,
                                parametersPanelState->guiConfig.spinnerHeight},
                   "Number of Boids", &result.newFlockConfig.numberOfBoids, 0, 10000,
                   parametersPanelState->numberOfBoidsSpinnerEditMode))
        parametersPanelState->numberOfBoidsSpinnerEditMode = !parametersPanelState->numberOfBoidsSpinnerEditMode;
    // Reset boids when the number is changed to prevent buffer overflow.
    // TODO: Chang to dynamically resize the buffer.
    if (flockState->flockConfig.numberOfBoids != result.newFlockConfig.numberOfBoids) {
        result.resetBoids = true;
    }
    heightOffset += parametersPanelState->guiConfig.spinnerHeight + parametersPanelState->guiConfig.padding;

    if (GuiButton((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                               parametersPanelState->guiConfig.panelWidth -
                                       (parametersPanelState->guiConfig.padding * 2.f),
                               parametersPanelState->guiConfig.buttonHeight},
                  "Reset Boids")) {
        result.resetBoids = true;
    }
    heightOffset += parametersPanelState->guiConfig.buttonHeight + parametersPanelState->guiConfig.padding;

    GuiCheckBox((Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                             parametersPanelState->guiConfig.checkboxHeight,
                             parametersPanelState->guiConfig.checkboxHeight},
                "Show FPS", &parametersPanelState->showFPS);
    heightOffset += parametersPanelState->guiConfig.checkboxHeight + parametersPanelState->guiConfig.padding;

    GuiDrawText(
            TextFormat("Collision Rate: %f", flockState->collisionRate / GetTime()),
            (Rectangle) {parametersPanelState->guiConfig.padding, heightOffset,
                         parametersPanelState->guiConfig.panelWidth - (parametersPanelState->guiConfig.padding * 2.f),
                         parametersPanelState->guiConfig.headingHeight},
            TEXT_ALIGN_LEFT, DARKGRAY);

    if (parametersPanelState->showFPS) {
        DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 40 - (int) parametersPanelState->guiConfig.padding,
                 (int) parametersPanelState->guiConfig.padding, 10, MAROON);
    }

    return result;
}
