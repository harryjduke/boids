#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

#include "boid.h"
#include "flock.h"

// GUI config
struct GuiConfig {
    float padding;
    float panelWidth;
    float panelHeight;
    float headingHeight;
    float spinnerHeight;
    float spinnerWidth;
    float checkboxHeight;
    float buttonHeight;
};

// GUI state
struct ParametersPanelState {
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

    // Current config for GUI
    struct GuiConfig config;
};

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

void InitializeParametersPanel(struct ParametersPanelState *parametersPanelState, struct GuiConfig config);

struct ParametersPanelResult {
    bool resetBoids;
    struct FlockConfig newFlockConfig;
};

void DrawBoidRanges(const struct ParametersPanelState *parametersPanelState, const struct FlockState *flockState,
                    const Boid *boid);

struct ParametersPanelResult DrawParametersPanel(struct ParametersPanelState *parametersPanelState,
                                                 const struct FlockState *flockState);

#endif // !GUI_H
