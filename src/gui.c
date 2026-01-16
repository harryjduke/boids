#include "gui.h"

#ifdef DEBUG
#include "boid.h"
#endif /* ifdef DEBUG */
#include "flock.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>
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

void InitializeGui(struct GuiState *guiState, const struct GuiConfig config) {
    // NOTE: We can skip config validation because an invalid config will only result in a visually broken GUI and not
    // cause any errors or crashes, this may change in the future.

    *guiState = (struct GuiState){
        .parametersPanelState =
            (struct PanelState){
                .currentId = 0,
                .activeId = 0,

                .heightOffset = 0,

                .config = &guiState->config,
            },
        .config = config,

        .showFPS = false,
#ifdef DEBUG
        .showRanges = false,
        .inspectedBoidIndex = 0,
#endif /* ifdef DEBUG */
    };
}

static void PanelHeader(const char *text, struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->panelWidth - (config->padding * 2.F),
        .height = config->headingHeight,
    };

    GuiDrawText(text, bounds, TEXT_ALIGN_LEFT, DARKGRAY);
    state->heightOffset += bounds.height + config->padding;
}

static void PanelParameterFloat(const char *label, float *value, float scale, int minValue, int maxValue,
                                struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    int intValue = (int)roundf(*value * scale);
    int previousIntValue = intValue;
    int id = ++state->currentId;

    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->spinnerWidth,
        .height = config->spinnerHeight,
    };

    if (GuiSpinner(bounds, label, &intValue, minValue, maxValue, id == state->activeId)) {
        state->activeId = id;
    }

    if (intValue != previousIntValue) {
        *value = (float)intValue / scale;
    }

    state->heightOffset += bounds.height + config->padding;
}

static void PanelParameterInt(const char *label, int *value, int minValue, int maxValue, struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    int id = ++state->currentId;

    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->spinnerWidth,
        .height = config->spinnerHeight,
    };

    if (GuiSpinner(bounds, label, value, minValue, maxValue, id == state->activeId)) {
        state->activeId = id;
    }

    state->heightOffset += bounds.height + config->padding;
}

static void PanelParameterBool(const char *label, bool *value, struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->checkboxHeight,
        .height = config->checkboxHeight,
    };

    GuiCheckBox(bounds, label, value);

    state->heightOffset += bounds.height + config->padding;
}

static bool PanelButton(const char *label, struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->panelWidth - (config->padding * 2.F),
        .height = config->buttonHeight,
    };

    bool result = GuiButton(bounds, label);

    state->heightOffset += bounds.height + config->padding;

    return result;
}

static void PanelValueFloat(const char *label, const float *value, struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    const char *text = TextFormat("%s: %f", label, *value);
    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->panelWidth - (config->padding * 2.F),
        .height = config->headingHeight,
    };

    GuiDrawText(text, bounds, TEXT_ALIGN_LEFT, DARKGRAY);

    state->heightOffset += bounds.height + config->padding;
}

static void PanelValueVector2(const char *label, const Vector2 *value, bool displayMagnitude,
                              struct PanelState *state) {
    const struct GuiConfig *config = state->config;
    const char *text;
    int lines;
    if (displayMagnitude) {
        float magnitude = Vector2Length(*value);
        text = TextFormat("%s:\n  x: % 9.4f\n  y: % 9.4f\n  magnitude: % 9.4f", label, value->x, value->y, magnitude);
        lines = 4;
    } else {
        text = TextFormat("%s:\n  x: % 9.4f\n  y: % 9.4f", label, value->x, value->y);
        lines = 3;
    }
    Rectangle bounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->panelWidth - (config->padding * 2.F),
        .height = config->headingHeight * (float)lines,
    };

    GuiDrawText(text, bounds, TEXT_ALIGN_LEFT, DARKGRAY);

    state->heightOffset += bounds.height + config->padding;
}

struct ParametersPanelResult DrawParametersPanel(struct GuiState *guiState, const struct FlockState *flockState) {
    struct PanelState *panelState = &guiState->parametersPanelState;
    struct ParametersPanelResult result = {.resetBoids = false, .newFlockConfig = flockState->config};

    if (guiState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recieved NULL pointer to parametersPanelState.");
        return result;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawParametersPanel: Recieved NULL pointer to flockState.");
        return result;
    }

    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiSetStyle(SPINNER, TEXT_PADDING, (int)guiState->config.padding);

    // Draw the panel
    GuiPanel((Rectangle){0.F, 0.F, guiState->config.panelWidth, guiState->config.panelHeight}, "Boid Parameters");

    // Reset the ID counter (it will be incremented as we draw each element)
    guiState->parametersPanelState.currentId = 0;

    // Set the initial height offset
    guiState->parametersPanelState.heightOffset = 25.F + guiState->config.padding;

    PanelHeader("Force Factors", panelState);
    PanelParameterFloat("Separation", &result.newFlockConfig.separationFactor, 100.F, 0, 10000, panelState);
    PanelParameterFloat("Alignment", &result.newFlockConfig.alignmentFactor, 100.F, 0, 10000, panelState);
    PanelParameterFloat("Cohesion", &result.newFlockConfig.cohesionFactor, 100.F, 0, 10000, panelState);

    PanelHeader("Force Ranges", panelState);
    PanelParameterFloat("Separation", &result.newFlockConfig.separationRange, 1.F, 0, 1000, panelState);
    PanelParameterFloat("Alignment", &result.newFlockConfig.alignmentRange, 1.F, 0, 1000, panelState);
    PanelParameterFloat("Cohesion", &result.newFlockConfig.cohesionRange, 1.F, 0, 1000, panelState);

#ifdef DEBUG
    PanelParameterBool("Show Ranges", &guiState->showRanges, panelState);
#endif /* ifdef DEBUG */

    PanelParameterBool("Normalise Forces", &result.newFlockConfig.normalizeForces, panelState);

    PanelHeader("Speed", panelState);
    PanelParameterBool("Clamp Speed", &result.newFlockConfig.clampSpeed, panelState);
    if (!result.newFlockConfig.clampSpeed) {
        GuiDisable();
    }
    PanelParameterFloat("Minimum Speed", &result.newFlockConfig.minimumSpeed, 1.F, 0, 1000, panelState);
    PanelParameterFloat("Maximum Speed", &result.newFlockConfig.maximumSpeed, 1.F, 0, 1000, panelState);
    GuiEnable();

    PanelHeader("Boids", panelState);
    PanelParameterInt("Number of Boids", &result.newFlockConfig.numberOfBoids, 1, 10000, panelState);
    // Although the minimum is 1, deleting all the digits in the spinner still returns 0
    if (result.newFlockConfig.numberOfBoids <= 0) {
        result.newFlockConfig.numberOfBoids = 1;
    }
    // Reset boids when the number is changed to prevent buffer overflow.
    // TODO: Change to dynamically resize the buffer.
    if (flockState->config.numberOfBoids != result.newFlockConfig.numberOfBoids) {
        result.resetBoids = true;
    }

    if (PanelButton("Reset Boids", panelState)) {
        result.resetBoids = true;
    }

    PanelParameterBool("Show FPS", &guiState->showFPS, panelState);

    float collisionRate = (float)(flockState->collisionTime / (GetTime() - flockState->collisionTimeStart));
    PanelValueFloat("Collision Rate", &collisionRate, panelState);

#ifdef DEBUG
    PanelHeader("Inspect Boid", panelState);
    PanelParameterInt("Boid Index", &guiState->inspectedBoidIndex, 0, flockState->config.numberOfBoids - 1, panelState);
    PanelValueVector2("Position", &flockState->boids[guiState->inspectedBoidIndex].position, false, panelState);
    PanelValueVector2("Velocity", &flockState->boids[guiState->inspectedBoidIndex].velocity, true, panelState);
    PanelValueVector2("Separation Vector", &flockState->boids[guiState->inspectedBoidIndex].separationVector, true,
                      panelState);
    PanelValueVector2("Alignment Vector", &flockState->boids[guiState->inspectedBoidIndex].alignmentVector, true,
                      panelState);
    PanelValueVector2("Cohesion Vector", &flockState->boids[guiState->inspectedBoidIndex].cohesionVector, true,
                      panelState);
#endif /* ifdef DEBUG */

    return result;
}

#ifdef DEBUG
static void DrawBoidRanges(const struct GuiState *guiState, const struct FlockState *flockState, const int boidIndex) {
    if (guiState == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved NULL pointer to parametersPanelState.");
        return;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved NULL pointer to flockState.");
        return;
    }
    Boid *boid = &flockState->boids[boidIndex];
    if (boid == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved index of invalid boid.");
        return;
    }

    if (!guiState->showRanges) {
        return;
    }
    DrawCircleV(boid->position, flockState->config.separationRange, Fade(GRAY, 0.2F));
    DrawCircleV(boid->position, flockState->config.alignmentRange, Fade(GRAY, 0.2F));
    DrawCircleV(boid->position, flockState->config.cohesionRange, Fade(GRAY, 0.2F));
}

void DrawGuiBoidOverlay(const struct GuiState *guiState, const struct FlockState *flockState, const int boidIndex) {
    DrawBoidRanges(guiState, flockState, boidIndex);
}
#endif /* ifdef DEBUG */

struct GuiResult DrawGui(struct GuiState *state, const struct FlockState *flockState) {
    struct GuiResult result;
    result = (struct GuiResult){
        .parametersPanelResult = DrawParametersPanel(state, flockState),
    };

    #ifdef DEBUG
    DrawGuiBoidOverlay(state, flockState, state->inspectedBoidIndex);
    #endif /* ifdef DEBUG */

    if (state->showFPS) {
        DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 40 - (int)state->config.padding,
                 (int)state->config.padding, 10, MAROON);
    }

    return result;
}
