#include "gui.h"

#include "boid.h"
#include "flock.h"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <stdbool.h>
#include <stddef.h>

struct GuiConfig CreateDefaultGuiConfig(float screenHeight) {
    return (struct GuiConfig){
        .padding = 5.F,
        .panelWidth = 200.F,
        .panelHeight = screenHeight,
        .headingHeight = 15.F,
        .spinnerHeight = 20.F,
        .spinnerWidth = 90.F,
        .checkboxHeight = 10.F,
        .buttonHeight = 20.F,
    };
}

void InitializeParametersPanel(struct ParametersPanelState *parametersPanelState, const struct GuiConfig config) {
    // NOTE: We can skip config validation because an invalid config will only result in a visually broken GUI and not
    // cause any errors or crashes, this may change in the future.
    *parametersPanelState = (struct ParametersPanelState){
        .separationFactorSpinnerEditMode = false,
        .alignmentFactorSpinnerEditMode = false,
        .cohesionFactorSpinnerEditMode = false,

        .separationRangeSpinnerEditMode = false,
        .alignmentRangeSpinnerEditMode = false,
        .cohesionRangeSpinnerEditMode = false,

        .minimumSpeedSpinnerEditMode = false,
        .maximumSpeedSpinnerEditMode = false,

        .showRanges = false,
        .showFPS = false,

        .config = config,
    };
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

    if (!parametersPanelState->showRanges) {
        return;
    }
    DrawCircleV(boid->position, flockState->config.separationRange, Fade(GRAY, 0.5F));
    DrawCircleV(boid->position, flockState->config.alignmentRange, Fade(GRAY, 0.5F));
    DrawCircleV(boid->position, flockState->config.cohesionRange, Fade(GRAY, 0.5F));
}

struct ParametersPanelResult DrawParametersPanel(struct ParametersPanelState *parametersPanelState,
                                                 const struct FlockState *flockState) {
    struct ParametersPanelResult result = {.resetBoids = false, .newFlockConfig = flockState->config};

    if (parametersPanelState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recieved NULL pointer to parametersPanelState.");
        return result;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recieved NULL pointer to flockState.");
        return result;
    }

    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiSetStyle(SPINNER, TEXT_PADDING, (int)parametersPanelState->config.padding);

    GuiPanel((Rectangle){0.F, 0.F, parametersPanelState->config.panelWidth, parametersPanelState->config.panelHeight},
             "Boid Parameters");
    float heightOffset = 25.F + parametersPanelState->config.padding;

    GuiDrawText("Force Factors",
                (Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                            parametersPanelState->config.headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->config.headingHeight + parametersPanelState->config.padding;

    int separationFactorValue = (int)(result.newFlockConfig.separationFactor * 100.F);
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Separation", &separationFactorValue, 0, 10000,
                   parametersPanelState->separationFactorSpinnerEditMode)) {
        parametersPanelState->separationFactorSpinnerEditMode = !parametersPanelState->separationFactorSpinnerEditMode;
    }
    result.newFlockConfig.separationFactor = (float)separationFactorValue / 100.F;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    int alignmentFactorValue = (int)(result.newFlockConfig.alignmentFactor * 10000.F);
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Alignment", &alignmentFactorValue, 0, 10000,
                   parametersPanelState->alignmentFactorSpinnerEditMode)) {
        parametersPanelState->alignmentFactorSpinnerEditMode = !parametersPanelState->alignmentFactorSpinnerEditMode;
    }
    result.newFlockConfig.alignmentFactor = (float)alignmentFactorValue / 10000.F;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    int cohesionFactorValue = (int)(result.newFlockConfig.cohesionFactor * 10000.F);
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Cohesion", &cohesionFactorValue, 0, 10000, parametersPanelState->cohesionFactorSpinnerEditMode)) {
        parametersPanelState->cohesionFactorSpinnerEditMode = !parametersPanelState->cohesionFactorSpinnerEditMode;
    }
    result.newFlockConfig.cohesionFactor = (float)cohesionFactorValue / 10000.F;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    GuiDrawText("Force Ranges",
                (Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                            parametersPanelState->config.headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->config.headingHeight + parametersPanelState->config.padding;

    int separationRangeValue = (int)result.newFlockConfig.separationRange;
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Separation", &separationRangeValue, 0, 1000,
                   parametersPanelState->separationRangeSpinnerEditMode)) {
        parametersPanelState->separationRangeSpinnerEditMode = !parametersPanelState->separationRangeSpinnerEditMode;
    }
    result.newFlockConfig.separationRange = (float)separationRangeValue;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    int alignmentRangeValue = (int)result.newFlockConfig.alignmentRange;
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Alignment", &alignmentRangeValue, 0, 1000, parametersPanelState->alignmentRangeSpinnerEditMode)) {
        parametersPanelState->alignmentRangeSpinnerEditMode = !parametersPanelState->alignmentRangeSpinnerEditMode;
    }
    result.newFlockConfig.alignmentRange = (float)alignmentRangeValue;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    int cohesionRangeValue = (int)result.newFlockConfig.cohesionRange;
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Cohesion", &cohesionRangeValue, 0, 1000, parametersPanelState->cohesionRangeSpinnerEditMode)) {
        parametersPanelState->cohesionRangeSpinnerEditMode = !parametersPanelState->cohesionRangeSpinnerEditMode;
    }
    result.newFlockConfig.cohesionRange = (float)cohesionRangeValue;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    GuiCheckBox((Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.checkboxHeight, parametersPanelState->config.checkboxHeight},
                "Show Ranges", &parametersPanelState->showRanges);
    heightOffset += parametersPanelState->config.checkboxHeight + parametersPanelState->config.padding;

    GuiDrawText("Speed",
                (Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                            parametersPanelState->config.headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->config.headingHeight + parametersPanelState->config.padding;

    GuiCheckBox((Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.checkboxHeight, parametersPanelState->config.checkboxHeight},
                "Clamp Speed", &result.newFlockConfig.clampSpeed);
    heightOffset += parametersPanelState->config.checkboxHeight + parametersPanelState->config.padding;

    if (!result.newFlockConfig.clampSpeed) {
        GuiDisable();
    }

    int minimumSpeedValue = (int)result.newFlockConfig.minimumSpeed;
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Minimum Speed", &minimumSpeedValue, 0, 1000, parametersPanelState->minimumSpeedSpinnerEditMode)) {
        parametersPanelState->minimumSpeedSpinnerEditMode = !parametersPanelState->minimumSpeedSpinnerEditMode;
    }
    result.newFlockConfig.minimumSpeed = (float)minimumSpeedValue;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    int maximumSpeedValue = (int)result.newFlockConfig.maximumSpeed;
    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Maximum Speed", &maximumSpeedValue, 0, 1000, parametersPanelState->maximumSpeedSpinnerEditMode)) {
        parametersPanelState->maximumSpeedSpinnerEditMode = !parametersPanelState->maximumSpeedSpinnerEditMode;
    }
    result.newFlockConfig.maximumSpeed = (float)maximumSpeedValue;
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    GuiEnable();

    GuiDrawText("Boids",
                (Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                            parametersPanelState->config.headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += parametersPanelState->config.headingHeight + parametersPanelState->config.padding;

    if (GuiSpinner((Rectangle){parametersPanelState->config.padding, heightOffset,
                               parametersPanelState->config.spinnerWidth, parametersPanelState->config.spinnerHeight},
                   "Number of Boids", &result.newFlockConfig.numberOfBoids, 1, 10000,
                   parametersPanelState->numberOfBoidsSpinnerEditMode)) {
        parametersPanelState->numberOfBoidsSpinnerEditMode = !parametersPanelState->numberOfBoidsSpinnerEditMode;
    }
    // Although the minimum is 1, deleting all the digits in the spinner still returns 0
    if (result.newFlockConfig.numberOfBoids <= 0) {
        result.newFlockConfig.numberOfBoids = 1;
    }
    // Reset boids when the number is changed to prevent buffer overflow.
    // TODO: Change to dynamically resize the buffer.
    if (flockState->config.numberOfBoids != result.newFlockConfig.numberOfBoids) {
        result.resetBoids = true;
    }
    heightOffset += parametersPanelState->config.spinnerHeight + parametersPanelState->config.padding;

    if (GuiButton((Rectangle){parametersPanelState->config.padding, heightOffset,
                              parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                              parametersPanelState->config.buttonHeight},
                  "Reset Boids")) {
        result.resetBoids = true;
    }
    heightOffset += parametersPanelState->config.buttonHeight + parametersPanelState->config.padding;

    GuiCheckBox((Rectangle){parametersPanelState->config.padding, heightOffset,
                            parametersPanelState->config.checkboxHeight, parametersPanelState->config.checkboxHeight},
                "Show FPS", &parametersPanelState->showFPS);
    heightOffset += parametersPanelState->config.checkboxHeight + parametersPanelState->config.padding;

    GuiDrawText(
        TextFormat("Collision Rate: %f", flockState->collisionTime / (GetTime() - flockState->collisionTimeStart)),
        (Rectangle){parametersPanelState->config.padding, heightOffset,
                    parametersPanelState->config.panelWidth - (parametersPanelState->config.padding * 2.F),
                    parametersPanelState->config.headingHeight},
        TEXT_ALIGN_LEFT, DARKGRAY);

    if (parametersPanelState->showFPS) {
        DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 40 - (int)parametersPanelState->config.padding,
                 (int)parametersPanelState->config.padding, 10, MAROON);
    }

    return result;
}
