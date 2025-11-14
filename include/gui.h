#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

#include "boid.h"
#include "flock.h"

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
};

// GUI config
struct GuiConfig {
    const float padding;
    const float panelWidth;
    const float panelHeight;
    const float headingHeight;
    const float spinnerHeight;
    const float spinnerWidth;
    const float checkboxHeight;
    const float buttonHeight;
};

void InitializeParametersPanel(struct ParametersPanelState *parametersPanelState);

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

struct ParametersPanelResult {
    bool resetBoids;
    struct FlockConfig newFlockConfig;
};

struct ParametersPanelResult DrawParametersPanel(struct ParametersPanelState *parametersPanelState,
                                                 const struct GuiConfig *guiConfig, const struct FlockState *flockState,
                                                 const struct FlockConfig *flockConfig);

void DrawBoidRanges(const struct ParametersPanelState *parametersPanelState, const struct GuiConfig *guiConfig,
                    const struct FlockConfig *flockConfig, const Boid *boid);

#endif // !GUI_H
