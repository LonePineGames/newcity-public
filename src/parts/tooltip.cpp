#include "tooltip.hpp"

const char* ttDefTxt = "Default Tooltip";
const char* ttUnk = "Unknown Tooltip Type";

static float ttLastHover = 0.0f;
static int ttActiveType = 0;
static vec3 ttActiveDim = vec3(0, 0, 0);
static bool ttShow = true;
static bool ttEnable = true;

const char* getDefaultTooltipText() {
  return ttDefTxt;
}

const char* getTooltipText(int type) {
  switch(type) {
    // General
    case TooltipType::GenBudget:
      return "Take a peek at your city's finances";
    case TooltipType::GenMsg:
      return "Pin a message with this info";
    case TooltipType::GenGraphs:
      return "View selected data graphs";
    case TooltipType::GenMenu:
      return "Open the Main Menu";
    case TooltipType::GenInfo:
      return "Toggle the Info panel";
    case TooltipType::GenSpdPause:
      return "Pause the Game";
    case TooltipType::GenSpd1:
      return "Real Time";
    case TooltipType::GenSpd2:
      return "Normal Speed";
    case TooltipType::GenSpd3:
      return "Fast Speed";
    case TooltipType::GenSpd4:
      return "Timelapse";
    case TooltipType::GenSpd5:
      return "Plaid";
    case TooltipType::GenSpd6:
      return "11";
    case TooltipType::GenSpd7:
      return "c";
    case TooltipType::GenSeedH:
      return "Terrain Seed - Determines the shape of the terrain";
    case TooltipType::GenSeedRandom:
      return "Randomize Terrain Seed";
    case TooltipType::GenLandReset:
      return "Regenerate Current City";

    // Toolbar
    case TooltipType::TbQuery:
      return "Query your city for more information";
    case TooltipType::TbRoad:
      return "Build roads and expressways";
    case TooltipType::TbZone:
      return "Zone homes, shops, and industry";
    case TooltipType::TbAmen:
      return "Build services and infrastructure";
    case TooltipType::TbDozer:
      return "Destroy roads and buildings";
    case TooltipType::TbMsg:
      return "Toggle messages";
    case TooltipType::TbBlu:
      return "Create Blueprints from existing roads/zones";
    case TooltipType::TbNewspaper:
      return "Read the newspaper";
    case TooltipType::TbFPS:
      return "Toggle FPS/Frame timing display";
    case TooltipType::TbEye:
      return "Toggle UI";

    // Amenity Types
    case TooltipType::AmeEdu:
      return "Build education amenities";
    case TooltipType::AmeRec:
      return "Build recreation amenities";
    case TooltipType::AmeServ:
      return "Build city-service amenities";
    case TooltipType::AmeUniv:
      return "Build a university";
    case TooltipType::AmePlop:
      return "Build any non-government building";

    // Blueprint Tool
    case TooltipType::BluLowerEle:
      return "Lower Blueprint";
    case TooltipType::BluNew:
      return "Create New Blueprint";
    case TooltipType::BluRaiseEle:
      return "Raise Blueprint";
    case TooltipType::BluExport:
      return "Export Blueprint";
    case TooltipType::BluImport:
      return "Import Blueprint";
    case TooltipType::BluRotate:
      return "Rotate Blueprint";
    case TooltipType::BluFlip:
      return "Flip Blueprint";
    case TooltipType::BluSave:
      return "Save Blueprint";
    case TooltipType::BluPlan:
      return "Planner Mode (Blueprints)";

    // Budget
    case TooltipType::BudNullBudget:
      return "";
    case TooltipType::BudPropertyTax:
      return "Income from property tax\nReduces residential demand";
    case TooltipType::BudSalesTax:
      return "Income from retail sales tax\nReduces retail demand";
    case TooltipType::BudFinesAndFeesIncome:
      return "Income from fines and fees\nIncreases crime";
    case TooltipType::BudFuelTaxIncome:
      return "Income from fuel tax\nReduces traffic";
    case TooltipType::BudAmenityIncome:
      return "Income from profitable amenities";
    case TooltipType::BudTransitIncome:
      return "Income from transit ticket sales";
    case TooltipType::BudAssetSalesIncome:
      return "Income from selling amenities";
    case TooltipType::BudRoadBuildExpenses:
      return "Money spent constructing new roads";
    case TooltipType::BudExpwyBuildExpenses:
      return "Money spent constructing new expressways";
    case TooltipType::BudTransitBuildExpenses:
      return "Money spent constructing mass transit";
    case TooltipType::BudPillarBuildExpenses:
      return "Money spent constructing bridge pillars";
    case TooltipType::BudEminentDomainExpenses:
      return "Money spent reimbursing property owners"
        "\nwhen demolishing their buildings to"
        "\nmake way for infrasture or amenities";
    case TooltipType::BudRepairExpenses:
      return "Money spent repairing infrastructure";
    case TooltipType::BudBuildingBuildExpenses:
      return "Money spent constructing new amenities";
    case TooltipType::BudMiscDiscExpenses:
      return "Miscellaneous spending";
    case TooltipType::BudEducationExpenses:
      return "Money spent on teacher's salaries,"
        "\nbooks, building maintenance, and more.";
    case TooltipType::BudRecreationExpenses:
      return "Money spent maintaining parks"
        "\nand recreation buildings";
    case TooltipType::BudServicesExpenses:
      return "Money spent on police salaries,"
        "\ndoctors, nurses, building"
        "\nmaintenance, and more";
    case TooltipType::BudUniversityExpenses:
      return "Money spent on professor's salaries,"
        "\nstudent amenities, building maintenance,"
        "\nand more";
    case TooltipType::BudTransitExpenses:
      return "Money spent on fuel, vehicle maintenance,"
        "\nbus and train driver salaries,"
        "\nand more";
    case TooltipType::BudTotalIncome:
      return "The sum of all city income"
        "\nfrom every source, including taxes,"
        "\namenity sales, and transit ticket sales";
    case TooltipType::BudTotalExpenses:
      return "The sum of all city spending,"
        "\nboth discretionary and mandatory";
    case TooltipType::BudTotalDiscretionary:
      return "The sum of all discretionary spending."
        "\nDiscretionary expenses are the result of"
        "\nplayer choice, including building roads"
        "\nand amenities.";
    case TooltipType::BudTotalMandatory:
      return "The sum of all mandatory spending."
        "\nMandatory expenses recur day after day,"
        "\nand continue even after the city's line"
        "\nof credit has been overdrawn.";
    case TooltipType::BudTotalEarnings:
      return "Total income minus total mandatory"
        "\nspending. Does not include"
        "\ndiscretionary spending.";
    case TooltipType::BudAssets:
      return "The total value of all the city's amenities";
    case TooltipType::BudLineOfCredit:
      return "The amount that the bank is"
        "\nwilling to lend the city."
        "\nThe bank determines this amount"
        "\nbased on total earnings, assets,"
        "\nand loan term.";
    case TooltipType::BudLoanInterest:
      return "The yearly interest payments on"
        "\nthe city's loan";
    case TooltipType::BudCashflow:
      return "Total income minus total expenses,"
        "\nincluding loan interest. The amount the"
        "\ncity's budget balance rose or fell this year.";
    case TooltipType::BudBudgetBalance:
      return "The bottom line - how much cash the city"
        "\nhas on hand, minus the city's loan";
    case TooltipType::BudCashToSpend:
      return "The total amount the city can spend"
        "\nwithout becoming overdrawn."
        "\nIf negative, this value represents"
        "\nthe amount of money the city must earn"
        "\n(or increase the loan by) before"
        "\ndiscretionary spending can resume.";

    // Infoview
    case TooltipType::InfUnder:
      return "Toggle the Underground view";
    case TooltipType::InfTransit:
      return "Open the Transit View";
    case TooltipType::InfRoad:
      return "Open the Road Map View";
    case TooltipType::InfZone:
      return "Open the Zone View";
    case TooltipType::InfLabels:
      return "Show or Hide Labels";

    // Query Tool
    case TooltipType::QueSubInner:
      return "Query Vehicles, Roads, Buildings, and More";
    case TooltipType::QueSubHeatmap:
      return "Observe the Aura of any Location";
    case TooltipType::QueSubRoute:
      return "Inspect Routes through Your City";
    case TooltipType::QueSubLabel:
      return "Label Locations in Your City";

    case TooltipType::QueSelHMPol:
      return "Pollution";
    case TooltipType::QueSelHMVal:
      return "Land Value";
    case TooltipType::QueSelMDen:
      return "Density";
    case TooltipType::QueSelHMCri:
      return "Crime";
    case TooltipType::QueSelHMEdu:
      return "Education";
    case TooltipType::QueSelHMPro:
      return "Prosperity";
    case TooltipType::QueSelHMCom:
      return "Community";
    case TooltipType::QueSelHMHea:
      return "Health";
      
    case TooltipType::QueDemo:
      return "Follow a citizen on a Sunday drive";
    case TooltipType::QueIssues:
      return "Open the Issue Icon Legend";
    case TooltipType::QuePollu:
      return "Open the Pollution heatmap";
    case TooltipType::QueCrime:
      return "Open the Crime heatmap";
    case TooltipType::QueEduca:
      return "Open the Education heatmap";
    case TooltipType::QueProsp:
      return "Open the Prosperity heatmap";
    case TooltipType::QueValue:
      return "Open the Land Value heatmap";
    case TooltipType::QueDensity:
      return "Open the Urban Density heatmap";
    case TooltipType::QueCommunity:
      return "Open the Community heatmap";
    case TooltipType::QueHealth:
      return "Open the Health heatmap";
    case TooltipType::QueTraffic:
      return "Open the Traffic heatmap";

    // Road Tool
    case TooltipType::RoadButtRoad:
      return "Build roads";
    case TooltipType::RoadButtBuild:
      return "Build infrastructure";
    case TooltipType::RoadStreet:
      return "Build a two-lane road";
    case TooltipType::RoadAvenue:
      return "Build a four-lane avenue";
    case TooltipType::RoadBlvd:
      return "Build a broad boulevard";
    case TooltipType::RoadOneWay2:
      return "Build a two-lane one-way road";
    case TooltipType::RoadOneWay4:
      return "Build a four-lane one-way road";
    case TooltipType::RoadButtExpress:
      return "Build expressways";
    case TooltipType::RoadExpress1:
      return "Build a single-lane expressway";
    case TooltipType::RoadExpress2:
      return "Build a two-lane expressway";
    case TooltipType::RoadExpress3:
      return "Build a three-lane expressway";
    case TooltipType::RoadExpress4:
      return "Build a four-lane expressway";
    case TooltipType::RoadExpress5:
      return "Build a five-lane expressway";
    case TooltipType::RoadButtRail:
      return "Build a rail system";
    case TooltipType::RoadRail1:
      return "Build a single-rail track";
    case TooltipType::RoadRail2:
      return "Build a double-rail track";
    case TooltipType::RoadRailStation:
      return "Build a station platform";
    case TooltipType::RoadRail3:
      return "Build a triple-rail track";
    case TooltipType::RoadRail4:
      return "Build a quad-rail track";
    case TooltipType::RoadButtRepair:
      return "Repair damaged roads";
    case TooltipType::RoadButtCut:
      return "Cut road into segments";
    case TooltipType::RoadButtTransit:
      return "Add a mass transit system";
    case TooltipType::RoadButtPillar:
      return "Build bridge pillars";
    case TooltipType::RoadButtPlanner:
      return "Toggle planner mode";
    case TooltipType::RoadLowerEle:
      return "Lower road elevation";
    case TooltipType::RoadRaiseEle:
      return "Raise road elevation";
    case TooltipType::RoadViaduct:
      return "Build viaducts instead of embankments";
    case TooltipType::RoadTunnel:
      return "Build tunnels instead of trenches";
    case TooltipType::RoadGridMode:
      return "Toggle Grid Snapping";
    case TooltipType::RoadLinkMode:
      return "Toggle Link Snapping";

    // Transit Tool
    case TooltipType::TransBus:
      return "Build a bus network";
    case TooltipType::TransNewLine:
      return "Add a New Transit Line";
    case TooltipType::TransAddStops:
      return "Add a Stops to Line";

    // Zone Tool
    case TooltipType::ZoneDensity:
      return "Toggle density control mode";
    case TooltipType::ZoneDezone:
      return "Dezone existing zones";
    case TooltipType::ZoneRes:
      return "Zone for residential";
    case TooltipType::ZoneMixed:
      return "Zone for mixed-use (residential/retail)";
    case TooltipType::ZoneRetail:
      return "Zone for retail";
    case TooltipType::ZoneOffice:
      return "Zone for offices";
    case TooltipType::ZoneAgri:
      return "Zone for agriculture";
    case TooltipType::ZoneIndus:
      return "Zone for industry";
    case TooltipType::ZoneGov:
      return "Zone for government";
    case TooltipType::ZonePark:
      return "Zone for parks";
    case TooltipType::ZoneBrushEdge:
      return "Add zones along street edges";
    case TooltipType::ZoneBrushSmall:
      return "Paint zones with a small brush";
    case TooltipType::ZoneBrushMed:
      return "Paint zones with a medium brush";
    case TooltipType::ZoneBrushLarge:
      return "Paint zones with a large brush";
    case TooltipType::ZoneBrushPoint:
      return "Place individual zones";
    case TooltipType::ZoneOverzone:
      return "Allow or prevent overzoning of existing zones";

    // Building Designer
    case TooltipType::DesignerSelect:
      return "Select a design element";
    case TooltipType::DesignerStruct:
      return "Add structures";
    case TooltipType::DesignerDeco:
      return "Add decorations";
    case TooltipType::DesignerDelete:
      return "Delete selected structure or decoration";
    case TooltipType::DesignerDozer:
      return "Remove structures or decorations";
    case TooltipType::DesignerUndo:
      return "Undo changes to the design";
    case TooltipType::DesignerRedo:
      return "Redo changes to the design";
    case TooltipType::DesignerGrid:
      return "Toggle grid snapping";
    case TooltipType::DesignerStructRoofOnly:
      return "Make only the roof the structure";
    case TooltipType::DesignerDecoGrab:
      return "Grab and move the selected decoration";
    case TooltipType::DesignerDecoGroupVisible:
      return "Hide these decorations";
    case TooltipType::DesignerStructuresVisible:
      return "Hide structures";
    case TooltipType::DesignerStructGable:
      return "Add a structure with a gable roof";
    case TooltipType::DesignerStructHip:
      return "Add a structure with a peaked roof";
    case TooltipType::DesignerStructFlat:
      return "Add a structure with a flat roof";
    case TooltipType::DesignerStructBarrel:
      return "Add a structure with a rounded roof";
    case TooltipType::DesignerStructSlant:
      return "Add a structure with a flat, slanted roof";
    case TooltipType::DesignerStructGambrel:
      return "Add a structure with a barn-like roof";
    case TooltipType::DesignerStructChangeGable:
      return "Change selected structure to a gable roof";
    case TooltipType::DesignerStructChangeHip:
      return "Change selected structure to a peaked roof";
    case TooltipType::DesignerStructChangeFlat:
      return "Change selected structure to a flat roof";
    case TooltipType::DesignerStructChangeBarrel:
      return "Change selected structure to a rounded roof";
    case TooltipType::DesignerStructChangeSlant:
      return "Change selected structure to a flat, slanted roof";
    case TooltipType::DesignerStructChangeGambrel:
      return "Change selected structure to a barn-like roof";
    case TooltipType::DesignerConfigSelect:
      return "Edit selected structure or decoration";
    case TooltipType::DesignerConfigVisual:
      return "Modify visuals for the design";
    case TooltipType::DesignerConfigGame:
      return "Set game-specific values for the design";
    case TooltipType::EditInDesigner:
      return "Edit in Building Designer [This will autosave your game.]";
    case TooltipType::DesignerOpenInWorkshop:
      return "Share this Design with the Steam Workshop";
    case TooltipType::DesignerFixBuilding:
      return "Edit the Stats for this Building Design";

    // Graphs
    case TooltipType::GraphCityStats:
      return "Show city statistics";
    case TooltipType::GraphAchievements:
      return "Show achievements";
    case TooltipType::GraphAmenityEffects:
      return "Show amenity affects";
    case TooltipType::GraphExpand:
      return "Toggle expand window to show more graphs";
    case TooltipType::GraphPeriodAll:
      return "Set time scale to show all history";
    case TooltipType::GraphPeriod20Y:
      return "Set time scale to 20 years";
    case TooltipType::GraphPeriod5Y:
      return "Set time scale to 5 years";
    case TooltipType::GraphPeriodYear:
      return "Set time scale to 1 year";
    case TooltipType::GraphPeriodDay:
      return "Set time scale to 24 hours";
    case TooltipType::GraphChooseStat:
      return "Choose the statistic to graph";
    case TooltipType::GraphPinMessage:
      return "Pin graph as message";

    case TooltipType::DocsCitipedia:
      return "Open the Citipedia to learn more";

    default:
      return ttUnk;
  }
}

const char* getTooltipText(TooltipType type) {
  return getTooltipText((int)type);
}

void setActiveTooltipType(int type) {
  ttActiveType = type;
}

const char* getActiveTooltipText() {
  return getTooltipText(ttActiveType);
}

float getTooltipLastHover() {
  return ttLastHover;
}

void resetTooltipLastHover() {
  ttLastHover = getCameraTime();
}

void setTooltipShow(bool show) {
  ttShow = show;
}

bool getTooltipShow() {
  return ttShow;
}

void tooltip_onInput(InputEvent event) {
  if(!event.isMouse) {
    return;
  }

  if(event.isButtonDown[0] 
    || event.isButtonDown[1]
    || event.isButtonDown[2]) {
    setTooltipShow(false);
    return;
  }
}

void updateTooltip(Part* part) {
  if(part == 0) {
    return;
  }

  int type = part->ttType;
  vec3 newDim = part->dim.start;

  if(newDim == ttActiveDim
    && ttActiveType == type) {
    return;
  }

  resetTooltipLastHover();

  ttActiveType = type;
  ttActiveDim = newDim;
  setTooltipShow(true);
}

Part* tooltip(vec2 pos, const char* txt) {
  float xVal = ttWidth;
  float yVal = 0.0f;
  float* yPtr = &yVal;
  Part* ttTxt = multiline(pos, vec2(xVal, ttTxtSize),
    strdup_s(txt), yPtr);
  Part* ttPanel = panel(pos, vec2(xVal, yVal));
  r(ttPanel, ttTxt);

  return ttPanel;
}
