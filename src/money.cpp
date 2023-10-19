#include "money.hpp"

#include "building/building.hpp"
#include "business.hpp"
#include "economy.hpp"
#include "error.hpp"
#include "icons.hpp"
#include "game/game.hpp"
#include "person.hpp"
#include "platform/event.hpp"
#include "string_proxy.hpp"
#include "time.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "spdlog/spdlog.h"
#include <math.h>

static float taxRate[5] = {0.00, 0.01, 0.00, 0.00, 0.00};
static bool taxEnabled[5] = {false, true, true, false, true};
static bool taxLocked[5] = {false, false, false, false, false};
static double loanRepaymentTime = 1.0;
vector<Budget> budgetHistory;
Budget estimateBudget;
Budget estimateBudgetNext;

double yearDurationSum = 0;
float lastAssetValueCalc = 0;
double assetValue = 0;

void clearBudget(Budget* budget) {
  budget->year = 0;
  budget->flags = 0;
  for (int i = 0; i < numBudgetLines; i++) {
    budget->line[i] = 0;
    budget->control[i] = 1;
  }
}

void resetMoney() {
  loanRepaymentTime = 1;
  lastAssetValueCalc = 0;
  assetValue = 0;
  yearDurationSum = 0;

  budgetHistory.clear();
  clearBudget(&estimateBudget);
  clearBudget(&estimateBudgetNext);

  for (int i = 0; i < 5; i++) {
    taxRate[i] = i == 1 ? 0.01f : 0.f;
    taxEnabled[i] = i == PropertyTax || i == SalesTax ? true : false;
    taxLocked[i] = false;
  }
}

const char* lineName[] = {
  "Null Budget",
  "Property Tax",
  "Sales Tax",
  "Fines and Fees",
  "Gas Tax",
  "Amenity Income",
  "Transit Income",
  "Asset Sales",
  "Road Construction",
  "Expwy Construction",
  "Transit Construction",
  "Pillar Construction",
  "Eminent Domain",
  "Infrastructure Repairs",
  "Building Construction",
  "Misc",
  "Education",
  "Parks and Recreation",
  "Community Services",
  "University",
  "Transit Operations",
  "Total Income",
  "Total Expenses",
  "Total Discretionary",
  "Total Mandatory",
  "Total Earnings",
  "Assets",
  "Line of Credit",
  "Loan Interest",
  "Cashflow",
  "Balance",
};

const char* lineCode[] = {
  "BudgetNull",
  "BudgetPropertyTax",
  "BudgetSalesTax",
  "BudgetFinesAndFeesIncome",
  "BudgetFuelTaxIncome",
  "BudgetAmenityIncome",
  "BudgetTransitIncome",
  "BudgetAssetSalesIncome",
  "BudgetRoadBuildExpenses",
  "BudgetExpwyBuildExpenses",
  "BudgetTransitBuildExpenses",
  "BudgetPillarBuildExpenses",
  "BudgetEminentDomainExpenses",
  "BudgetRepairExpenses",
  "BudgetBuildingBuildExpenses",
  "BudgetMiscDiscExpenses",
  "BudgetEducationExpenses",
  "BudgetRecreationExpenses",
  "BudgetServicesExpenses",
  "BudgetUniversityExpenses",
  "BudgetTransitExpenses",
  "BudgetTotalIncome",
  "BudgetTotalExpenses",
  "BudgetTotalDiscretionary",
  "BudgetTotalMandatory",
  "BudgetTotalEarnings",
  "BudgetAssets",
  "BudgetLineOfCredit",
  "BudgetLoanInterest",
  "BudgetCashflow",
  "BudgetBalance",
  "BudgetCashToSpend",
};

const char* getBudgetLineName(BudgetLine l) {
  return lineName[l];
}

const char* getBudgetLineCode(BudgetLine l) {
  return lineCode[l];
}

const vec3 lineIcon[] = {
  iconNull,
  iconZoneMono[ResidentialZone],
  iconZoneMono[RetailZone],
  iconDocument,
  iconFuel,
  iconZoneMono[OfficeZone],
  iconBus,
  iconZoneMono[GovernmentZone],

  iconRoad,
  iconExpressway,
  iconBus,
  iconBridgePillar[0],
  iconZoneMono[ResidentialZone],
  iconWrench,
  iconZoneMono[GovernmentZone],
  iconNull,
  iconBuildingCategory[0],
  iconBuildingCategory[1],
  iconBuildingCategory[2],
  iconBuildingCategory[3],
  iconBus,

  iconNull,
  iconNull,
  iconNull,
  iconNull,
  iconCash,
  iconZoneMono[GovernmentZone],
  iconDocument,
  iconNull,
  iconCash,
  iconNull,
};

bool isTaxLocked(item tax) {
  return taxLocked[tax];
}

void lockTax(item tax) {
  SPDLOG_INFO("Locking tax {}", tax);
  taxRate[tax] = 0.04;
  taxLocked[tax] = true;
}

void unlockTaxes() {
  for (int i = 0; i < 5; i++) {
    taxLocked[i] = false;
  }
}

bool isTaxEnabled(item tax) {
  return taxEnabled[tax];
}

void enableTax(item tax) {
  SPDLOG_INFO("Enabling tax {}", tax);
  taxEnabled[tax] = true;
}

vec3 getBudgetLineIcon(BudgetLine l) {
  return lineIcon[l];
}

void setLoanRepaymentTime(float time) {
  loanRepaymentTime = time;
}

double getLoanRepaymentTime() {
  return loanRepaymentTime;
}

money getCredit() {
  Budget currentBudget = getBudget(0);
  return currentBudget.line[BudgetBalance] + currentBudget.line[LineOfCredit];
}

money getEarnings() {
  Budget currentBudget = getBudget(1);
  return currentBudget.line[TotalEarnings];
}

void makeRetailTransaction(item econNdx, int eduLevel, bool isTourist) {
  money value = (eduLevel > 1) ? c(CCollegeEduRetailSpending) :
    ((eduLevel == 1) ? c(CHSEduRetailSpending) : c(CNoEduRetailSpending));
  value *= getInflation();

  adjustStat(econNdx, RetailTransactions, value);
  if (isTourist) {
    adjustStat(econNdx, TouristTransactions, value);
  }

  Econ* econ = getEcon(econNdx);
  if (isTaxEnabled(SalesTax) &&
      (econ->type == ChunkEcon || econ->type == OurCityEcon)) {
    transaction(SalesTax, value*getTaxRate(SalesTax));
  }
}

Budget* getCurrentBudget() {
  int historySize = budgetHistory.size();
  //SPDLOG_INFO("getCurrentBudget {}", historySize);

  if (historySize == 0) {
    Budget b;
    b.flags = _budgetIsYTD | _budgetIsValid;
    b.year = getBaseYear();
    for (int l = 0; l < numBudgetLines; l ++) {
      b.line[l] = 0;
      b.control[l] = 1;
    }
    b.line[BudgetBalance] = c(CStartingMoney);
    budgetHistory.push_back(b);
    historySize++;
    //SPDLOG_INFO("created first budget {}", b.line[BudgetBalance],
        //c(CStartingMoney));
    //logStacktrace();
  } else {
    //SPDLOG_INFO("existing first budget {}",
        //budgetHistory[historySize-1].line[BudgetBalance]);
  }

  return &budgetHistory[historySize-1];
}

double getInterestRate() {
  return getNationalInterestRate() + 0.002 * loanRepaymentTime;
}

double compound(double interestRate, double time) {
  return pow(2.7183, interestRate*time);
}

void applyInterest(Budget* b, double duration) {
  double balance = b->line[BudgetBalance];
  double interestRate = getInterestRate();

  if (balance < 0) {
    double interest = (compound(interestRate, duration)-1) * -balance;
    if (interest > 1e30 || interest < -1e30 || interest != interest) {
      handleError("Infinite Interest Rate\n");
    }
    transaction(b, LoanInterest, -interest);
  }
}

int getNumHistoricalBudgets() {
  return budgetHistory.size() - 1;
}

Budget getBudget(int yearRel) {
  int computedYear = yearRel + getBaseYear();
  if (yearRel > 0) computedYear --;

  if (yearRel == 2) {
    return estimateBudgetNext;

  } else if (yearRel == 1) {
    return estimateBudget;

  } else if (yearRel > 1) {
    Budget b;
    b.flags = 0;
    b.year = computedYear;
    for (int i = 0; i < numBudgetLines; i++) {
      b.line[i] = 0;
      b.control[i] = 1;
    }
    return b;

  } else if (-yearRel < budgetHistory.size()) {
    return budgetHistory[budgetHistory.size() + yearRel - 1];

  } else {
    Budget b;
    b.flags = 0;
    b.year = computedYear;
    for (int i = 0; i < numBudgetLines; i++) {
      b.line[i] = 0;
      b.control[i] = 1;
    }
    b.line[LineOfCredit] = c(CGoodFaithLOC);
    return b;
  }
}

void runBudget(Budget* budget, float dur) {
  // Property Tax
  makeBuildingPayments(budget, dur);

  // Sales Tax
  // Calculate Retail GDP
  TimeSeries retailStat = getTimeSeries(ourCityEconNdx(), RetailTransactions);
  money retailGDP = 0;
  int numSteps = oneYear/retailStat.timeStep;
  int endStep = retailStat.values.size();
  int startStep = endStep - numSteps;
  startStep = startStep < 0 ? 0 : startStep;
  for (int i = startStep; i < endStep; i++) {
    retailGDP += retailStat.values[i];
  }

  //SPDLOG_INFO("retail GDP {} {} {}", retailGDP, numSteps, startStep);
  transaction(budget, SalesTax, getTaxRate(SalesTax) * retailGDP * dur);

  // Fines and Fees
  float finesAndFees = numPeople(ourCityEconNdx()) * c(CFinesPerPerson);
  finesAndFees += numBusinesses(ourCityEconNdx()) * c(CFinesPerBusiness);
  finesAndFees *= getTaxRate(FinesAndFeesIncome);
  transaction(budget, FinesAndFeesIncome, dur*finesAndFees);

  // Park lots
  item numParkLots = getStatistic(ourCityEconNdx(), ParkLots);
  float control = getBudgetControl(RecreationExpenses);
  transaction(budget, RecreationExpenses,
      -numParkLots * dur * c(CParkLotMaint) * getInflation() * control);

  // Road Repair
  /*
  float pavement = getStatistic(ourCityEconNdx(), PavementMSq);
  transaction(budget,RepairExpenses,
      -numRoads(ourCityEconNdx()) * dur * c(CRoadRepairCost) * getInflation());
      */

  applyInterest(budget, dur);
}

void computeBudgetLOC(Budget* budget, float estEarnings) {
  // Compute line of credit
  //double estEarnings = budget->line[TotalEarnings];
  //estEarnings = estEarnings < 0 ? 0 : estEarnings;
  double interestRate = getInterestRate();
  double compoundedInterest = compound(interestRate, loanRepaymentTime);
  double loc = (budget->line[Assets]*c(CAssetLOC)
     + c(CGoodFaithLOC)*getInflation() + estEarnings) *
    loanRepaymentTime / compoundedInterest;
  loc = loc < 0 ? 0 : loc;
  budget->line[LineOfCredit] = loc;
}

void updateMoney(double duration) {
  int historySize = budgetHistory.size();
  Budget* currentBudget = getCurrentBudget();
  Budget previousBudget = getBudget(-1);

  if (currentBudget->year < getBaseYear()) {
    if (currentBudget != 0) {
      currentBudget->flags = _budgetIsValid;
    }

    fireEvent(EventBudget);

    Budget b;
    b.flags = _budgetIsYTD | _budgetIsValid;
    b.year = getBaseYear();
    for (int l = 0; l < numBudgetLines; l ++) {
      b.line[l] = 0;
      b.control[l] = currentBudget->control[l];
    }

    b.line[BudgetBalance] = currentBudget->line[BudgetBalance];
    budgetHistory.push_back(b);
    currentBudget = &budgetHistory[historySize];
    historySize++;
    yearDurationSum = 0;
  }

  float time = getCurrentDateTime();
  double yearDur = duration / gameDayInRealSeconds / oneYear;
  double yearFractional = time / oneYear;
  double yearRemaining = 1 - (yearFractional-int(yearFractional));
  yearDurationSum += yearDur;

  if (time - lastAssetValueCalc > oneHour*0.25) {
    assetValue = getCityAssetValue();
    lastAssetValueCalc = time;
  }

  currentBudget->line[Assets] = assetValue;

  runBudget(currentBudget, yearDur);

  estimateBudget.flags = _budgetIsValid | _budgetIsEstimate;
  estimateBudget.year = currentBudget->year;
  for (int l = 0; l < numBudgetLines; l ++) {
    estimateBudget.line[l] = currentBudget->line[l];
  }

  runBudget(&estimateBudget, yearRemaining);
  estimateBudgetNext.flags = _budgetIsValid | _budgetIsEstimate;
  estimateBudgetNext.year = currentBudget->year + 1;
  for (int l = 0; l < numBudgetLines; l ++) {
    estimateBudgetNext.line[l] = 0;
  }

  estimateBudgetNext.line[Assets] = assetValue;
  estimateBudgetNext.line[BudgetBalance] += estimateBudget.line[BudgetBalance];
  runBudget(&estimateBudgetNext, 1);

  float transitIncome = currentBudget->line[TransitIncome];
  transitIncome *= 1/(1-yearRemaining);
  transaction(&estimateBudget, TransitIncome, transitIncome * yearRemaining);
  transaction(&estimateBudgetNext, TransitIncome, transitIncome);

  float transitOps = currentBudget->line[TransitExpenses];
  transitOps *= 1/(1-yearRemaining);
  transaction(&estimateBudget, TransitExpenses, transitOps * yearRemaining);
  transaction(&estimateBudgetNext, TransitExpenses, transitOps);

  float repairEx = currentBudget->line[RepairExpenses];
  repairEx *= 1/(1-yearRemaining);
  transaction(&estimateBudget, RepairExpenses, repairEx * yearRemaining);
  transaction(&estimateBudgetNext, RepairExpenses, repairEx);

  float fuelTax = currentBudget->line[FuelTaxIncome];
  fuelTax *= 1/(1-yearRemaining);
  transaction(&estimateBudget, FuelTaxIncome, fuelTax * yearRemaining);
  transaction(&estimateBudgetNext, FuelTaxIncome, fuelTax);

  float earningsEst = estimateBudgetNext.line[TotalEarnings];
  computeBudgetLOC(currentBudget, earningsEst);
  estimateBudget.line[LineOfCredit] = currentBudget->line[LineOfCredit];
  estimateBudgetNext.line[LineOfCredit] = currentBudget->line[LineOfCredit];
  computeBudgetLOC(&estimateBudgetNext, earningsEst);
}

bool canBuy(BudgetLine line, money amount) {
  return canBuy(getCurrentBudget(), line, amount);
}

bool canBuy(Budget* b, BudgetLine line, money amount) {
  return (budgetLineFlags[line] & _budgetIsMandatory) ||
    (budgetLineFlags[line] & _budgetIsIncome) ||
    getGameMode() == ModeTest || amount <= 0 ||
    amount <= b->line[BudgetBalance] + b->line[LineOfCredit];
}

void subtransaction(Budget* b, BudgetLine line, money amount) {
  b->line[line] += amount;
  if (b == getCurrentBudget()) {
    int flags = budgetLineFlags[line];
    if (!(flags & _budgetNotAggregate)) {
      bool inverted = line != Cashflow && !(flags & _budgetIsIncome);
      float statAmount = inverted ? -amount : amount;
      adjustStat(ourCityEconNdx(), PropertyTaxStat + line - 1, statAmount);
    }
  }
}

bool forceTransaction(Budget* b, BudgetLine line, money amount) {
  int flags = budgetLineFlags[line];
  subtransaction(b, line, amount);
  subtransaction(b, Cashflow, amount);
  subtransaction(b, BudgetBalance, amount);
  if (flags & _budgetIsIncome) {
    subtransaction(b, TotalIncome, amount);
    if (flags & _budgetIsMandatory) {
      subtransaction(b, TotalEarnings, amount);
    }

  } else {
    subtransaction(b, TotalExpenses, amount);

    if (flags & _budgetIsMandatory) {
      subtransaction(b, TotalMandatory, amount);
      subtransaction(b, TotalEarnings, amount);

    } else {
      subtransaction(b, TotalDiscretionary, amount);
    }
  }

  return true;
}

bool forceTransaction(BudgetLine line, money amount) {
  return forceTransaction(getCurrentBudget(), line, amount);
}

bool transaction(Budget* b, BudgetLine line, money amount) {
  int flags = budgetLineFlags[line];
  if (getGameMode() != ModeGame) return true;
  if (!(flags & _budgetIsIncome) && amount > 0) return false;
  if (!(flags & _budgetIsMandatory) && !canBuy(b, line, -amount)) return false;

  return forceTransaction(b, line, amount);
}

bool transaction(BudgetLine line, money amount) {
  return transaction(getCurrentBudget(), line, amount);
}

void setTaxRate(BudgetLine t, float amount) {
  taxRate[t] = amount;
}

float getTaxRate(BudgetLine t) {
  if (!isTaxEnabled(t)) return 0;
  return taxRate[t];
}

float getBudgetControlEffect(BudgetLine l) {
  return pow(getCurrentBudget()->control[l], c(CBudgetControlFactor));
}

float getBudgetControl(BudgetLine l) {
  return getCurrentBudget()->control[l];
}

void setBudgetControl(BudgetLine l, float val) {
  getCurrentBudget()->control[l] = val;
}

char* printPaddedPositiveMoneyString(money amount) {
  amount *= c(CMoneyMultiplier);
  const char* pre  = "$";
  const char* post = "";
  char* result;

  if (amount >= 1000000000) { // 1B
    result = sprintf_o("%s%5.1fB%s", pre, amount/1000000000, post);
  } else if (amount >= 1000000) {
    result = sprintf_o("%s%5.1fM%s", pre, amount/1000000, post);
  } else if (amount >= 1000) {
    result = sprintf_o("%s%5.1fK%s", pre, amount/1000, post);
  } else {
    result = sprintf_o("%s%5.2f%s", pre, amount, post);
  }

  return result;
}

char* printPaddedMoneyString(money amount) {
  amount *= c(CMoneyMultiplier);
  bool isNegative = amount < 0;
  const char* pre  = isNegative ? "($" : " $";
  const char* post = isNegative ? ")" : " ";
  char* result;
  amount = isNegative ? -amount : amount;

  if (amount >= 1000000000) { // 1B
    result = sprintf_o("%s%5.1fB%s", pre, amount/1000000000, post);
  } else if (amount >= 1000000) {
    result = sprintf_o("%s%5.1fM%s", pre, amount/1000000, post);
  } else if (amount >= 1000) {
    result = sprintf_o("%s%5.1fK%s", pre, amount/1000, post);
  } else {
    result = sprintf_o("%s%3d   %s", pre, int(amount), post);
  }

  return result;
}

char* printMoneyString(money amount) {
  amount *= c(CMoneyMultiplier);
  char* result;
  const char* neg = (amount <= -0.01) ? "-" : "";
  amount = abs(amount);

  if (amount >= 1000000000) { // 1B
    result = sprintf_o("%s$%.2fB", neg, amount/1000000000);
  } else if (amount >= 100000000) { // 100M
    result = sprintf_o("%s$%.0fM", neg, amount/1000000);
  } else if (amount >= 10000000) { // 10M
    result = sprintf_o("%s$%.1fM", neg, amount/1000000);
  } else if (amount >= 1000000) { // 1M
    result = sprintf_o("%s$%.2fM", neg, amount/1000000);
  } else if (amount >= 10000) { // 10K
    result = sprintf_o("%s$%.0fK", neg, amount/1000);
  } else if (amount >= 1000) {
    result = sprintf_o("%s$%d,%03d", neg, int(amount/1000), int(amount)%1000);
  } else if (amount >= 100) {
    result = sprintf_o("%s$%d", neg, int(amount));
  } else if (amount >= 1) {
    result = sprintf_o("%s$%1.2f", neg, amount);
  } else if (amount > 0.01) {
    result = sprintf_o("%s%d cents", neg, int(amount*100));
  } else if (amount > 0.0001) {
    result = sprintf_o("%s%.2f cents", neg, amount*100);
  } else {
    result = strdup_s("Free");
  }

  return result;
}

void writeBudget(FileBuffer* file, Budget* b) {
  fwrite_int(file, b->year);
  fwrite_int(file, b->flags);
  for (int i = 0; i < numBudgetLines; i++) {
    fwrite_money(file, b->line[i]);
    fwrite_float(file, b->control[i]);
  }
}

void readBudget(FileBuffer* file, Budget* b, int version) {
  b->year = fread_int(file);
  if (version < 50) b->year -= c(CStartYear);
  b->flags = fread_int(file);
  for (int i = 0; i < numBudgetLines; i++) {
    b->line[i] = fread_money(file, version);
    if (version < 51) {
      b->control[i] = 1;
    } else {
      b->control[i] = fread_float(file);
    }
  }
}

void writeMoney(FileBuffer* file) {
  for (int i = 1; i <= 3; i++) {
    fwrite_float(file, taxRate[i]);
  }
  fwrite_float(file, loanRepaymentTime);

  fwrite_int(file, budgetHistory.size());
  for (int i = 0; i < budgetHistory.size(); i++) {
    writeBudget(file, &budgetHistory[i]);
  }
  estimateBudget.control[FuelTaxIncome] = taxRate[4];
  writeBudget(file, &estimateBudget);
  writeBudget(file, &estimateBudgetNext);
}

void readMoney(FileBuffer* file, int version) {
  if (version >= 28) {
    for (int i = 1; i <= 3; i++) {
      taxRate[i] = fread_float(file);
    }
    loanRepaymentTime = fread_float(file);

  } else {
    for (int i = 1; i <= 3; i++) {
      taxRate[i] = 0.01;
    }
    loanRepaymentTime = 5.0;
  }

  if (version >= 21) {
    int numBudgets = fread_int(file);
    for (int i = 0; i < numBudgets; i++) {
      Budget b;
      budgetHistory.push_back(b);
      readBudget(file, &budgetHistory[i], version);
    }
    readBudget(file, &estimateBudget, version);
    taxRate[4] = estimateBudget.control[FuelTaxIncome];
    if (version >= 51) {
      readBudget(file, &estimateBudgetNext, version);
    }

  } else {
    getCurrentBudget()->line[BudgetBalance] = fread_money(file, version);
  }

  if (getGameMode() != ModeGame && version <= 58) {
    resetMoney();
  }
}

void fwrite_money(FileBuffer* file, money amount) {
  fwrite_double(file, amount);
}

money fread_money(FileBuffer* file, int version) {
  return version >= 23 ? fread_double(file) : fread_float(file);
}

