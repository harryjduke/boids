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
    int numTabs = 1;
#ifdef DEBUG
    numTabs += 1;
#endif /* ifdef DEBUG */

    static const char *tabLabels[] = {
        [PARAMETERS_TAB] = "Parameters",
#ifdef DEBUG
        [DEBUG_INSPECTION_TAB] = "Inspect",
#endif /* ifdef DEBUG */
    };

    return (struct GuiConfig){
        .padding = 5.F,
        .panelWidth = 200.F,
        .panelHeight = screenHeight,
        .headingHeight = 15.F,
        .spinnerHeight = 20.F,
        .spinnerWidth = 90.F,
        .checkboxHeight = 10.F,
        .buttonHeight = 20.F,
        .numTabs = numTabs,
        .tabLabels = tabLabels,
    };
}

void InitializeGui(struct GuiState *guiState, const struct GuiConfig config) {
    // NOTE: We can skip config validation because an invalid config will only result in a visually broken GUI and not
    // cause any errors or crashes, this may change in the future.

    *guiState = (struct GuiState){
        .activeTab = PARAMETERS_TAB,
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
        .debug_inspectionPanelState =
            (struct PanelState){
                .currentId = 0,
                .activeId = 0,
                .heightOffset = 0,
                .config = &guiState->config,
            },
        .debug_inspectedBoidIndex = 0,
        .debug_showRanges = false,
        .debug_showVelocity = false,
#endif /* ifdef DEBUG */
    };
}

static void PanelTabBar(Rectangle bounds, const char **text, int count, int *active) {
    Rectangle tabBounds = {bounds.x, bounds.y, bounds.width / (float)count, bounds.height};

    if (*active < 0) {
        *active = 0;
    } else if (*active > count - 1) {
        *active = count - 1;
    }

    bool toggle = false; // Required for individual toggles

    // Draw control
    for (int i = 0; i < count; i++) {
        tabBounds.x = bounds.x + ((bounds.width / (float)count) * (float)i);

        int textAlignment = GuiGetStyle(TOGGLE, TEXT_ALIGNMENT);
        int textPadding = GuiGetStyle(TOGGLE, TEXT_PADDING);
        GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        GuiSetStyle(TOGGLE, TEXT_PADDING, 8);

        if (i == (*active)) {
            toggle = true;
            GuiToggle(tabBounds, text[i], &toggle);
        } else {
            toggle = false;
            GuiToggle(tabBounds, text[i], &toggle);
            if (toggle) {
                *active = i;
            }
        }

        GuiSetStyle(TOGGLE, TEXT_PADDING, textPadding);
        GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, textAlignment);
    }

    // Draw tab-bar bottom line
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){bounds.x, bounds.y + bounds.height - 1, bounds.width, 1}, 0, BLANK,
                     GetColor(GuiGetStyle(TOGGLE, BORDER_COLOR_NORMAL)));
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
    struct ParametersPanelResult result = {
        .resetBoids = false,
        .hasFlockConfigChanged = true,
        .newFlockConfig = flockState->config,
    };

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

    return result;
}

#ifdef DEBUG
struct Debug_PanelInspectionButtonsResult {
    bool isFlockPaused;
    bool doFlockStep;
} Debug_PanelInspectionButtons(struct PanelState *state, const struct FlockState *flockState) {
    const struct GuiConfig *config = state->config;

    struct Debug_PanelInspectionButtonsResult result = {
        .isFlockPaused = flockState->isPaused,
        .doFlockStep = false,
    };

    Rectangle buttonBounds = {
        .x = config->padding,
        .y = state->heightOffset,
        .width = config->buttonHeight,
        .height = config->buttonHeight,
    };
    if (GuiButton(buttonBounds, GuiIconText((flockState->isPaused ? ICON_PLAYER_PLAY : ICON_PLAYER_PAUSE), NULL))) {
        result.isFlockPaused = !flockState->isPaused;
    }
    buttonBounds.x += buttonBounds.width + config->padding;
    result.doFlockStep = GuiButton(buttonBounds, GuiIconText(ICON_PLAYER_NEXT, NULL));

    state->heightOffset += buttonBounds.height + config->padding;

    return result;
}

struct Debug_InspectionPanelResult Debug_DrawInspectionPanel(struct GuiState *guiState,
                                                             const struct FlockState *flockState) {
    struct Debug_InspectionPanelResult result;
    struct PanelState *panelState = &guiState->debug_inspectionPanelState;
    if (panelState == NULL) {
        TraceLog(LOG_ERROR, "Debug_DrawInspectionPanel: Recieved NULL pointer to debug_inspectionPanelState.");
        return result;
    }
    Boid *boid = &flockState->boids[guiState->debug_inspectedBoidIndex];
    if (boid == NULL) {
        TraceLog(LOG_ERROR, "Debug_DrawInspectionPanel: Recieved index of invalid boid.");
        return result;
    }

    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiSetStyle(SPINNER, TEXT_PADDING, (int)guiState->config.padding);

    // Draw the panel
    GuiPanel((Rectangle){0.F, 0.F, guiState->config.panelWidth, guiState->config.panelHeight}, "Boid Inspection");
    // Reset the ID counter (it will be incremented as we draw each element)
    panelState->currentId = 0;
    // Set the initial height offset
    panelState->heightOffset = 25.F + guiState->config.padding;

    struct Debug_PanelInspectionButtonsResult inspectionPanelButtonsResult =
        Debug_PanelInspectionButtons(panelState, flockState);
    result.isFlockPaused = inspectionPanelButtonsResult.isFlockPaused;
    result.doStepFlock = inspectionPanelButtonsResult.doFlockStep;

    PanelHeader("Inspect Boid", panelState);
    PanelParameterInt("Boid Index", &guiState->debug_inspectedBoidIndex, 0, flockState->config.numberOfBoids - 1,
                      panelState);
    PanelValueVector2("Position", &boid->position, false, panelState);
    PanelValueVector2("Velocity", &boid->velocity, true, panelState);
    PanelParameterBool("Draw Velocity", &guiState->debug_showVelocity, panelState);
    PanelValueVector2("Separation Vector", &boid->separationVector, true, panelState);
    PanelParameterBool("Draw Separation", &guiState->debug_showSeparation, panelState);
    PanelValueVector2("Alignment Vector", &boid->alignmentVector, true, panelState);
    PanelParameterBool("Draw Alignment", &guiState->debug_showAlignment, panelState);
    PanelValueVector2("Cohesion Vector", &boid->cohesionVector, true, panelState);
    PanelParameterBool("Draw Cohesion", &guiState->debug_showCohesion, panelState);

    PanelParameterBool("Show Ranges", &guiState->debug_showRanges, panelState);

    return result;
}

static void Debug_DrawInspectionHighlight(const struct FlockState *flockState, const int boidIndex) {
    Boid *boid = &flockState->boids[boidIndex];
    if (boid == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved index of invalid boid.");
        return;
    }

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

    DrawTriangleLines(vertex1, vertex2, vertex3, YELLOW);
}

static void Debug_DrawBoidRanges(const struct FlockState *flockState, const int boidIndex) {
    Boid *boid = &flockState->boids[boidIndex];
    if (boid == NULL) {
        TraceLog(LOG_ERROR, "DrawBoidRanges: Recieved index of invalid boid.");
        return;
    }

    DrawCircleV(boid->position, flockState->config.separationRange, Fade(GRAY, 0.2F));
    DrawCircleV(boid->position, flockState->config.alignmentRange, Fade(GRAY, 0.2F));
    DrawCircleV(boid->position, flockState->config.cohesionRange, Fade(GRAY, 0.2F));
}

static void Debug_DrawVector2(Vector2 origin, Vector2 displacement, Color color) {
    const float headLength = 5.F;
    const float headHalfWidth = 2.5F;

    Vector2 tipPosition = Vector2Add(origin, displacement);
    Vector2 direction = Vector2Normalize(displacement);
    Vector2 perpenicular = {.x = -direction.y, .y = direction.x};
    Vector2 basePosition = Vector2Subtract(tipPosition, Vector2Scale(direction, headLength));
    Vector2 perpendicularOffset = Vector2Scale(perpenicular, headHalfWidth);
    Vector2 point1 = Vector2Add(basePosition, perpendicularOffset);
    Vector2 point2 = Vector2Subtract(basePosition, perpendicularOffset);

    DrawLineV(origin, tipPosition, color);
    DrawTriangle(tipPosition, point2, point1, color);
}

static void Debug_DrawGuiBoidOverlay(const struct GuiState *guiState, const struct FlockState *flockState,
                                     const int boidIndex) {
    Debug_DrawInspectionHighlight(flockState, boidIndex);
    if (guiState->debug_showRanges) {
        Debug_DrawBoidRanges(flockState, boidIndex);
    }
    if (guiState->debug_showVelocity) {
        Debug_DrawVector2(flockState->boids[boidIndex].position, flockState->boids[boidIndex].velocity, RED);
    }
    if (guiState->debug_showSeparation) {
        Debug_DrawVector2(flockState->boids[boidIndex].position, Vector2Scale(flockState->boids[boidIndex].separationVector, 1/flockState->config.separationFactor), RED);
    }
    if (guiState->debug_showAlignment) {
        Debug_DrawVector2(flockState->boids[boidIndex].position, Vector2Scale(flockState->boids[boidIndex].alignmentVector, 1/flockState->config.alignmentFactor), RED);
    }
    if (guiState->debug_showCohesion) {
        Debug_DrawVector2(flockState->boids[boidIndex].position, Vector2Scale(flockState->boids[boidIndex].cohesionVector, 1/flockState->config.cohesionFactor), RED);
    }
}
#endif /* ifdef DEBUG */

struct GuiResult DrawGui(struct GuiState *state, const struct FlockState *flockState) {
    struct GuiResult result;
    if (state == NULL) {
        TraceLog(LOG_ERROR, "DrawGui: Recieved NULL pointer to guiState.");
        return result;
    }
    if (flockState == NULL) {
        TraceLog(LOG_ERROR, "DrawGui: Recieved NULL pointer to flockState.");
        return result;
    }

    switch (state->activeTab) {
    case PARAMETERS_TAB:
        result = (struct GuiResult){
            .parametersPanelResult = DrawParametersPanel(state, flockState),
        };
        break;
#ifdef DEBUG
    case DEBUG_INSPECTION_TAB:
        result = (struct GuiResult){
            .debug_inspectionPanelResult = Debug_DrawInspectionPanel(state, flockState),
        };
        break;
#endif /* ifdef DEBUG */
    default:
        break;
    }

#ifdef DEBUG
    Debug_DrawGuiBoidOverlay(state, flockState, state->debug_inspectedBoidIndex);
#endif /* ifdef DEBUG */

    Rectangle tabBarBounds = {.x = 0.F, .y = 0.F, .width = state->config.panelWidth, .height = 25.F};
    PanelTabBar(tabBarBounds, state->config.tabLabels, state->config.numTabs, &state->activeTab);

    if (state->showFPS) {
        DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 40 - (int)state->config.padding,
                 (int)state->config.padding, 10, MAROON);
    }

    return result;
}
