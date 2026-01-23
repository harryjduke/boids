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
    int numTabs;
    const char **tabLabels;
};

struct PanelState {
    struct GuiConfig *config;

    int currentId;
    int activeId;

    float heightOffset;
};

enum GuiTab {
    PARAMETERS_TAB,
#ifdef DEBUG
    DEBUG_INSPECTION_TAB,
#endif /* ifdef DEBUG */
};

struct GuiState {
    int activeTab;
    struct PanelState parametersPanelState;
    // Current config for GUI
    struct GuiConfig config;

    // GUI element toggles
    bool showFPS;
#ifdef DEBUG
    struct PanelState debug_inspectionPanelState;
    int debug_inspectedBoidIndex;
    bool debug_showRanges;
    bool debug_showVelocity;
    bool debug_showSeparation;
    bool debug_showAlignment;
    bool debug_showCohesion;
#endif /* ifdef DEBUG */
};

struct GuiConfig CreateDefaultGuiConfig(float screenHeight);

void InitializeGui(struct GuiState *guiState, struct GuiConfig config);

struct ParametersPanelResult {
    bool resetBoids;
    bool hasFlockConfigChanged;
    struct FlockConfig newFlockConfig;
};
struct ParametersPanelResult DrawParametersPanel(struct GuiState *guiState, const struct FlockState *flockState);

struct GuiResult {
    struct ParametersPanelResult parametersPanelResult;
#ifdef DEBUG
    struct Debug_InspectionPanelResult {
        bool isFlockPaused;
        bool doStepFlock;
    } debug_inspectionPanelResult ;
#endif /* ifdef DEBUG */
};
struct GuiResult DrawGui(struct GuiState *state, const struct FlockState *flockState);

#endif // !GUI_H
