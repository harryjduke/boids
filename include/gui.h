#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

#include "boid.h"
#include "flock.h"

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

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

struct ParametersPanelResult {
    bool resetBoids;
    struct FlockConfig newFlockConfig;
};

struct ParametersPanelResult DrawParametersPanel(const struct GuiConfig *guiConfig,
                                                 const struct FlockState *flockState, const struct FlockConfig *flockConfig);

void DrawBoidRanges(const struct GuiConfig *guiConfig, const struct FlockConfig *flockConfig, const Boid *boid);

#endif // !GUI_H
