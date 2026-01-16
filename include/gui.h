#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

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

struct PanelState {
    struct GuiConfig *config;

    int currentId;
    int activeId;

    float heightOffset;
};

struct GuiState {
    struct PanelState parametersPanelState;
    // Current config for GUI
    struct GuiConfig config;

    // GUI element toggles
    bool showFPS;
#ifdef DEBUG
    bool showRanges;
    int inspectedBoidIndex;
#endif /* ifdef DEBUG */
};

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

void InitializeGui(struct GuiState *guiState, struct GuiConfig config);

struct ParametersPanelResult {
    bool resetBoids;
    struct FlockConfig newFlockConfig;
};
struct ParametersPanelResult DrawParametersPanel(struct GuiState *guiState, const struct FlockState *flockState);

#ifdef DEBUG
void DrawGuiBoidOverlay(const struct GuiState *guiState, const struct FlockState *flockState, int boidIndex);
#endif /* ifdef DEBUG */

struct GuiResult {
    struct ParametersPanelResult parametersPanelResult;
};
struct GuiResult DrawGui(struct GuiState *state, const struct FlockState *flockState);

#endif // !GUI_H
