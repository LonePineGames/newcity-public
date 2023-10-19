#include "plansPanel.hpp"

#include "../icons.hpp"
#include "../plan.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"

#include "../parts/button.hpp"
#include "../parts/label.hpp"

#include "road.hpp"

bool toggleShowPlans(Part* part, InputEvent event) {
  setShowPlans(!isShowPlans());
  return true;
}

bool buyAllPlans(Part* part, InputEvent event) {
  buyAllPlans();
  reportTutorialUpdate(CompletedPlansInTT);
  return true;
}

bool discardAllPlans(Part* part, InputEvent event) {
  discardAllPlans();
  return true;
}

void plansPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1, strdup_s("Planner Mode")));
  float width = panel->dim.end.x - panel->padding*2;

  r(panel, button(vec2(width-1,0), iconX, vec2(1,1), togglePlanPanel, 0));

  if (getNumGraphPlans() > 0) {
    money cost = getTotalPlansCost();
    char* costStr = printMoneyString(cost);

    r(panel, superButton(vec2(0.f, 1.5f), vec2(width,1.2),
          iconYes, sprintf_o("Complete All for %s", costStr),
          buyAllPlans, 0, ActPlansComplete, false));
    free(costStr);

    r(panel, superButton(vec2(0.f, 3.f), vec2(width,1.2),
          iconNo, strdup_s("Discard All"),
          discardAllPlans, 0, ActPlansDiscard, false));

    //const char* showPlansButtText = isShowPlans() ?
      //"Hide Plan Widgets" : "Show Plan Widgets";
    r(panel, superButton(vec2(0.f, 4.5f), vec2(width,1.2),
          iconMessage, strdup_s("Plan Widgets"),
          toggleShowPlans, 0, ActPlansShowWidgets, isShowPlans()));

  } else {
    float ySize = 0;
    r(panel, multiline(vec2(0,2), vec2(width, 0.8),
          strdup_s("Roads will now be planned instead of built. "
            "You can complete or discard plans at any time. "
            "Plans are saved until you complete "
            "or discard them, or exit Planner mode."), &ySize));
  }
}

