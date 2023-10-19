
void handleInputTask(double duration) {
  updateInput(duration);
  finishGraph();
  renderGraph();
  updatePlans();
  updateCamera(duration);
}

void collectEntities(double duration) {
  collectEntities_g();
}

bool testCollectEntities(double duration) {
  return c(CCullingAlgo) > 0;
}

void finishLanes(double duration) {
  finishLanes();
}

void finishLots(double duration) {
  finishLots();
}

void applyBlueprintZones(double duration) {
  applyBlueprintZones();
}

void finishTransit(double duration) {
  finishTransit();
}

void updatePlans(double duration) {
  updatePlans();
}

void assignHeights(double duration) {
  assignHeights();
}

bool testAssignHeights(double duration) {
  return numHeightsToAssign() > 0;
}

void fireEventFrame(double duration) {
  fireEvent(EventFrame);
}

void fireEventGameUpdate(double duration) {
  fireEvent(EventGameUpdate);
}

void renderLandStep(double duration) {
  renderLandStep();
}

bool testRenderLandStep(double duration) {
  //SPDLOG_INFO("numRenderChunksQueued() {}", numRenderChunksQueued());
  return numRenderChunksQueued() > 0;
}

void renderGraph(double duration) {
  renderGraph();
}

bool testRenderGraph(double duration) {
  return graphElementsToRender() > 0;
}

void updateSoundEnvironment(double duration) {
  updateSoundEnvironment();
}

void updateGraphVisuals(double duration) {
  updateGraphVisuals(false);
}

void partialVehicleSwap(double duration) {
  swapVehiclesCommand(false);
}

void fullVehicleSwap(double duration) {
  swapVehiclesCommand(true);
}

void updateTutorial(double duration) {
  TutorialState* ptr = getTutorialStatePtr();

  // Not null checking here for performance, 
  // also TutorialState is a struct -supersoup
  // if(ptr == 0)
    // return;

  ptr->tick(duration);
}

