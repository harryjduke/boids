#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

#include "boid.h"
#include "flock.h"

// Used to keep a consistent style for the whole GUI
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

// TODO: Move definition to c file and only forward declare here
struct PanelState {
    int currentId;
    int activeId;

    float heightOffset;

    struct GuiConfig *config;
};

struct GuiState {
    // GUI element toggles
    bool showRanges;
    bool showFPS;
    int inspectedBoidIndex;

    struct PanelState parametersPanelState;

    // Current config for GUI
    struct GuiConfig config;
};

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

void InitializeGui(struct GuiState *guiState, struct GuiConfig config);

struct ParametersPanelResult {
    bool resetBoids;
    struct FlockConfig newFlockConfig;
};

void DrawBoidRanges(const struct GuiState *guiState, const struct FlockState *flockState, const Boid *boid);

struct ParametersPanelResult DrawParametersPanel(struct GuiState *guiState, const struct FlockState *flockState);

#endif // !GUI_H
