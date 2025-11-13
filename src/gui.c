#include "gui.h"
#include <stdbool.h>
#include "flock.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// GUI state
struct {
    // Edit mode flags for spinners
    bool separationFactorSpinnerEditMode;
    bool alignmentFactorSpinnerEditMode;
    bool cohesionFactorSpinnerEditMode;

    bool separationRangeSpinnerEditMode;
    bool alignmentRangeSpinnerEditMode;
    bool cohesionRangeSpinnerEditMode;

    bool minimumSpeedSpinnerEditMode;
    bool maximumSpeedSpinnerEditMode;

    bool numberOfBoidsSpinnerEditMode;

    // GUI element toggles
    bool showRanges;
    bool showFPS;
} state = {0};

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

void DrawBoidRanges(const struct GuiConfig *guiConfig, const struct FlockConfig *flockConfig, const Boid *boid) {
    if (!state.showRanges)
        return;
    DrawCircleV(boid->position, flockConfig->separationRange, Fade(GRAY, 0.5f));
    DrawCircleV(boid->position, flockConfig->alignmentRange, Fade(GRAY, 0.5f));
    DrawCircleV(boid->position, flockConfig->cohesionRange, Fade(GRAY, 0.5f));
}

struct ParametersPanelResult DrawParametersPanel(const struct GuiConfig *guiConfig, const struct FlockState *flockState,
                                                 const struct FlockConfig *flockConfig) {
    struct ParametersPanelResult result = {.resetBoids = false, .newFlockConfig = *flockConfig};
    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiSetStyle(SPINNER, TEXT_PADDING, (int) guiConfig->padding);

    GuiPanel((Rectangle) {0.f, 0.f, guiConfig->panelWidth, guiConfig->panelHeight}, "Boid Parameters");
    float heightOffset = 25.f + guiConfig->padding;

    GuiDrawText("Force Factors",
                (Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                             guiConfig->headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += guiConfig->headingHeight + guiConfig->padding;

    int separationFactorValue = (int) (result.newFlockConfig.separationFactor * 100.f);
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Separation", &separationFactorValue, 0, 10000, state.separationFactorSpinnerEditMode))
        state.separationFactorSpinnerEditMode = !state.separationFactorSpinnerEditMode;
    result.newFlockConfig.separationFactor = (float) separationFactorValue / 100.f;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    int alignmentFactorValue = (int) (result.newFlockConfig.alignmentFactor * 10000.f);
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Alignment", &alignmentFactorValue, 0, 10000, state.alignmentFactorSpinnerEditMode))
        state.alignmentFactorSpinnerEditMode = !state.alignmentFactorSpinnerEditMode;
    result.newFlockConfig.alignmentFactor = (float) alignmentFactorValue / 10000.f;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    int cohesionFactorValue = (int) (result.newFlockConfig.cohesionFactor * 10000.f);
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Cohesion", &cohesionFactorValue, 0, 10000, state.cohesionFactorSpinnerEditMode))
        state.cohesionFactorSpinnerEditMode = !state.cohesionFactorSpinnerEditMode;
    result.newFlockConfig.cohesionFactor = (float) cohesionFactorValue / 10000.f;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    GuiDrawText("Force Ranges",
                (Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                             guiConfig->headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += guiConfig->headingHeight + guiConfig->padding;

    int separationRangeValue = (int) result.newFlockConfig.separationRange;
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Separation", &separationRangeValue, 0, 1000, state.separationRangeSpinnerEditMode))
        state.separationRangeSpinnerEditMode = !state.separationRangeSpinnerEditMode;
    result.newFlockConfig.separationRange = (float) separationRangeValue;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    int alignmentRangeValue = (int) result.newFlockConfig.alignmentRange;
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Alignment", &alignmentRangeValue, 0, 1000, state.alignmentRangeSpinnerEditMode))
        state.alignmentRangeSpinnerEditMode = !state.alignmentRangeSpinnerEditMode;
    result.newFlockConfig.alignmentRange = (float) alignmentRangeValue;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    int cohesionRangeValue = (int) result.newFlockConfig.cohesionRange;
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Cohesion", &cohesionRangeValue, 0, 1000, state.cohesionRangeSpinnerEditMode))
        state.cohesionRangeSpinnerEditMode = !state.cohesionRangeSpinnerEditMode;
    result.newFlockConfig.cohesionRange = (float) cohesionRangeValue;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    GuiCheckBox((Rectangle) {guiConfig->padding, heightOffset, guiConfig->checkboxHeight, guiConfig->checkboxHeight},
                "Show Ranges", &state.showRanges);
    heightOffset += guiConfig->checkboxHeight + guiConfig->padding;

    GuiDrawText("Speed",
                (Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                             guiConfig->headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += guiConfig->headingHeight + guiConfig->padding;

    GuiCheckBox((Rectangle) {guiConfig->padding, heightOffset, guiConfig->checkboxHeight, guiConfig->checkboxHeight},
                "Clamp Speed", &result.newFlockConfig.clampSpeed);
    heightOffset += guiConfig->checkboxHeight + guiConfig->padding;

    if (!result.newFlockConfig.clampSpeed)
        GuiDisable();

    int minimumSpeedValue = (int) result.newFlockConfig.minimumSpeed;
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Minimum Speed", &minimumSpeedValue, 0, 1000, state.minimumSpeedSpinnerEditMode))
        state.minimumSpeedSpinnerEditMode = !state.minimumSpeedSpinnerEditMode;
    result.newFlockConfig.minimumSpeed = (float) minimumSpeedValue;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    int maximumSpeedValue = (int) result.newFlockConfig.maximumSpeed;
    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Maximum Speed", &maximumSpeedValue, 0, 1000, state.maximumSpeedSpinnerEditMode))
        state.maximumSpeedSpinnerEditMode = !state.maximumSpeedSpinnerEditMode;
    result.newFlockConfig.maximumSpeed = (float) maximumSpeedValue;
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    GuiEnable();

    GuiDrawText("Boids",
                (Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                             guiConfig->headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);
    heightOffset += guiConfig->headingHeight + guiConfig->padding;

    if (GuiSpinner((Rectangle) {guiConfig->padding, heightOffset, guiConfig->spinnerWidth, guiConfig->spinnerHeight},
                   "Number of Boids", &result.newFlockConfig.numberOfBoids, 0, 10000, state.numberOfBoidsSpinnerEditMode))
        state.numberOfBoidsSpinnerEditMode = !state.numberOfBoidsSpinnerEditMode;
    // Reset boids when the number is changed to prevent buffer overflow.
    // TODO: Chang to dynamically resize the buffer.
    if (flockConfig->numberOfBoids != result.newFlockConfig.numberOfBoids) {
        result.resetBoids = true;
    }
    heightOffset += guiConfig->spinnerHeight + guiConfig->padding;

    if (GuiButton((Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                               guiConfig->buttonHeight},
                  "Reset Boids")) {
        result.resetBoids = true;
    }
    heightOffset += guiConfig->buttonHeight + guiConfig->padding;

    GuiCheckBox((Rectangle) {guiConfig->padding, heightOffset, guiConfig->checkboxHeight, guiConfig->checkboxHeight},
                "Show FPS", &state.showFPS);
    heightOffset += guiConfig->checkboxHeight + guiConfig->padding;

    GuiDrawText(TextFormat("Collision Rate: %f", flockState->collisionRate / GetTime()),
                (Rectangle) {guiConfig->padding, heightOffset, guiConfig->panelWidth - (guiConfig->padding * 2.f),
                             guiConfig->headingHeight},
                TEXT_ALIGN_LEFT, DARKGRAY);

    if (state.showFPS) {
        DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 40 - (int) guiConfig->padding,
                 (int) guiConfig->padding, 10, MAROON);
    }

    return result;
}
