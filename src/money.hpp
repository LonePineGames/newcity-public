#pragma once

#include "serialize.hpp"

typedef double money;

enum BudgetLine {
  // Income
  NullBudget, PropertyTax, SalesTax, FinesAndFeesIncome,
  FuelTaxIncome, AmenityIncome, TransitIncome, AssetSalesIncome,

  //Discretionary Expenses
  RoadBuildExpenses, ExpwyBuildExpenses, TransitBuildExpenses,
  PillarBuildExpenses, EminentDomainExpenses, RepairExpenses,
  BuildingBuildExpenses, MiscDiscExpenses,

  // Mandatory Expenses
  EducationExpenses, RecreationExpenses, ServicesExpenses, UniversityExpenses,
  TransitExpenses,

  // Financials
  TotalIncome, TotalExpenses, TotalDiscretionary, TotalMandatory,
  TotalEarnings, Assets, LineOfCredit, LoanInterest, Cashflow, BudgetBalance,

  numBudgetLines
};

struct Budget {
  int year;
  int flags;
  money line[numBudgetLines];
  float control[numBudgetLines];
};

const int _budgetIsValid = 1 << 0;
const int _budgetIsEstimate = 1 << 1;
const int _budgetIsYTD = 1 << 2;
const int _budgetIsMandatory = 1 << 3;
const int _budgetIsIncome = 1 << 4;
const int _budgetNotAggregate = 1 << 5;

const int budgetLineFlags[numBudgetLines] = {
  0, //NullBudget
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //ResidentialIncome
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //CommercialIncome
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //IndustrialIncome
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //FuelTaxIncome
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //AmenityIncome
  _budgetIsValid | _budgetIsIncome | _budgetIsMandatory, //TransitIncome
  _budgetIsValid | _budgetIsIncome, //AssetSalesIncome

  ////Discretionary Expenses
  _budgetIsValid, //RoadBuildExpenses
  _budgetIsValid, //ExpwyBuildExpenses
  _budgetIsValid, //RailBuildExpenses
  _budgetIsValid, //PillarBuildExpenses
  _budgetIsValid, //EminentDomainExpenses
  _budgetIsValid | _budgetIsMandatory, //RepairExpenses
  _budgetIsValid, //BuildingBuildExpenses
  _budgetIsValid, //MiscDiscExpenses

  //// Mandatory Expenses
  _budgetIsValid | _budgetIsMandatory, //SchoolsExpenses
  _budgetIsValid | _budgetIsMandatory, //ParksExpenses
  _budgetIsValid | _budgetIsMandatory, //PoliceExpenses
  _budgetIsValid | _budgetIsMandatory, //TransitExpenses
  _budgetIsValid | _budgetIsMandatory, //MiscMandExpenses

  //// Financials
  _budgetIsValid | _budgetIsIncome, //TotalIncome
  _budgetIsValid, //TotalExpenses
  _budgetIsValid, //TotalDiscretionary
  _budgetIsValid | _budgetIsMandatory, //TotalMandatory
  _budgetIsValid | _budgetIsIncome, //TotalEarnings
  _budgetIsValid | _budgetNotAggregate, //Assets
  _budgetIsValid | _budgetNotAggregate, //LineOfCredit
  _budgetIsValid | _budgetIsMandatory, //LoanInterest
  _budgetIsValid, //Cashflow
  _budgetIsValid | _budgetNotAggregate //BudgetBalance
};

double compound(double rate, double time);
money getCredit();
money getEarnings();
Budget* getCurrentBudget();
void makeRetailTransaction(item econNdx, int eduLevel, bool isTourist);
void setTaxRate(BudgetLine tax, float amount);
float getTaxRate(BudgetLine tax);
bool isTaxEnabled(item tax);
void enableTax(item tax);
void lockTax(item tax);
void unlockTaxes();
bool isTaxLocked(item tax);
money getInflation();
Budget getBudget(int yearRel);
bool canBuy(Budget* b, BudgetLine line, money amount);
bool canBuy(BudgetLine line, money amount);
bool transaction(Budget* b, BudgetLine line, money amount);
bool transaction(BudgetLine line, money amount);
bool forceTransaction(BudgetLine line, money amount);
double getLoanRepaymentTime();
void setLoanRepaymentTime(float time);
double getInterestRate();
int getNumHistoricalBudgets();
const char* getBudgetLineName(BudgetLine l);
const char* getBudgetLineCode(BudgetLine l);
vec3 getBudgetLineIcon(BudgetLine l);
float getBudgetControl(BudgetLine l);
void setBudgetControl(BudgetLine l, float val);
float getBudgetControlEffect(BudgetLine l);

char* printMoneyString(money amount);
char* printPaddedMoneyString(money amount);
char* printPaddedPositiveMoneyString(money amount); // For positive numbers only

void updateMoney(double duration);
void resetMoney();

void writeMoney(FileBuffer* file);
void readMoney(FileBuffer* file, int version);
void fwrite_money(FileBuffer* file, money amount);
money fread_money(FileBuffer* file, int version);

