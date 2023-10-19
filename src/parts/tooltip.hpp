#ifndef TOOLTIP_HPP
#define TOOLTIP_HPP
// Doing both guards for any old compiler for any new team member
#pragma once

#include "part.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "../util.hpp"
#include "../string_proxy.hpp"


const float ttTxtSize = 0.5f;
const float ttPadding = 0.1f;
const float ttWidth = 4.0f;
const float ttYOffset = 1.0f;
const float ttTimeToShow = 0.25f;
const double ttSafeZoneCoeff = 0.9;

enum TooltipType {

  // General
  GenBudget = 0,
  GenMsg,
  GenGraphs,
  GenMenu,
  GenInfo,
  GenSpdPause,
  GenSpd1,
  GenSpd2,
  GenSpd3,
  GenSpd4,
  GenSpd5,
  GenSpd6,
  GenSpd7,
  GenSeedH,
  GenSeedRandom,
  GenLandReset,

  // Toolbar
  TbQuery,
  TbRoad,
  TbZone,
  TbAmen,
  TbDozer,
  TbMsg,
  TbBlu,
  TbNewspaper,
  TbFPS,
  TbEye,

  // Amenity Types
  AmeEdu,
  AmeRec,
  AmeServ,
  AmeUniv,
  AmePlop,

  // Blueprint Tool
  BluLowerEle,
  BluNew,
  BluRaiseEle,
  BluExport,
  BluImport,
  BluRotate,
  BluFlip,
  BluSave,
  BluPlan,

  // Budget
  BudNullBudget, BudPropertyTax, BudSalesTax, BudFinesAndFeesIncome,
  BudFuelTaxIncome, BudAmenityIncome, BudTransitIncome, BudAssetSalesIncome,
  BudRoadBuildExpenses, BudExpwyBuildExpenses, BudTransitBuildExpenses,
  BudPillarBuildExpenses, BudEminentDomainExpenses, BudRepairExpenses,
  BudBuildingBuildExpenses, BudMiscDiscExpenses,
  BudEducationExpenses, BudRecreationExpenses, BudServicesExpenses,
  BudUniversityExpenses, BudTransitExpenses,
  BudTotalIncome, BudTotalExpenses, BudTotalDiscretionary, BudTotalMandatory,
  BudTotalEarnings, BudAssets, BudLineOfCredit, BudLoanInterest, BudCashflow,
  BudBudgetBalance, BudCashToSpend,

  // Infoview
  InfUnder,
  InfZone,
  InfRoad,
  InfTransit,
  InfLabels,

  // Query Tool
  QueSubInner,
  QueSubHeatmap,
  QueSubRoute,
  QueSubLabel,

  QueSelHMPol,
  QueSelHMVal,
  QueSelMDen,
  QueSelHMCri,
  QueSelHMEdu,
  QueSelHMPro,
  QueSelHMCom,
  QueSelHMHea,

  QueDemo,
  QueIssues,
  QuePollu,
  QueCrime,
  QueEduca,
  QueProsp,
  QueValue,
  QueDensity,
  QueCommunity,
  QueHealth,
  QueTraffic,

  // Road Tool
  RoadButtRoad,
  RoadStreet,
  RoadAvenue,
  RoadBlvd,
  RoadOneWay2,
  RoadOneWay4,
  RoadButtExpress,
  RoadExpress1,
  RoadExpress2,
  RoadExpress3,
  RoadExpress4,
  RoadExpress5,
  RoadButtRail,
  RoadRail1,
  RoadRail2,
  RoadRailStation,
  RoadRail3,
  RoadRail4,
  RoadButtRepair,
  RoadButtCut,
  RoadButtTransit,
  RoadButtPillar,
  RoadButtPlanner,
  RoadLowerEle,
  RoadRaiseEle,
  RoadViaduct,
  RoadTunnel,
  RoadButtBuild,
  RoadGridMode,
  RoadLinkMode,

  // Transit Tool
  TransBus,
  TransNewLine,
  TransAddStops,

  // Zone Tool
  ZoneDensity,
  ZoneDezone,
  ZoneRes,
  ZoneRetail,
  ZoneAgri,
  ZoneGov,
  ZoneOffice,
  ZoneIndus,
  ZoneMixed,
  ZonePark,
  ZoneBrushEdge,
  ZoneBrushSmall,
  ZoneBrushMed,
  ZoneBrushLarge,
  ZoneBrushPoint,
  ZoneOverzone,

  // Building Designer
  DesignerSelect,
  DesignerStruct,
  DesignerDeco,
  DesignerDelete,
  DesignerDozer,
  DesignerUndo,
  DesignerRedo,
  DesignerGrid,
  DesignerStructGable,
  DesignerStructHip,
  DesignerStructFlat,
  DesignerStructBarrel,
  DesignerStructSlant,
  DesignerStructGambrel,
  DesignerStructChangeGable,
  DesignerStructChangeHip,
  DesignerStructChangeFlat,
  DesignerStructChangeBarrel,
  DesignerStructChangeSlant,
  DesignerStructChangeGambrel,
  DesignerStructRoofOnly,
  DesignerStructuresVisible,
  DesignerDecoGrab,
  DesignerDecoGroupVisible,
  DesignerConfigSelect,
  DesignerConfigVisual,
  DesignerConfigGame,
  EditInDesigner,
  DesignerOpenInWorkshop,
  DesignerFixBuilding,

  // Graphs
  GraphCityStats,
  GraphAchievements,
  GraphAmenityEffects,
  GraphExpand,
  GraphPeriodAll,
  GraphPeriod20Y,
  GraphPeriod5Y,
  GraphPeriodYear,
  GraphPeriodDay,
  GraphChooseStat,
  GraphPinMessage,

  // Docs
  DocsCitipedia,

  // Util
  NumTooltipTypes
};

const char* getDefaultTooltipText();
const char* getTooltipText(int type);
const char* getTooltipText(TooltipType type);
void setActiveTooltipType(int type);
const char* getActiveTooltipText();
float getTooltipLastHover();
void setTooltipShow(bool show);
bool getTooltipShow();
void updateTooltip(Part* part);
void tooltip_onInput(InputEvent event);
Part* tooltip(vec2 pos, const char* txt);

#endif  // TOOLTIP_HPP
