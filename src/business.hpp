#pragma once

#include "building/building.hpp" // For IssueIcon
#include "person.hpp"
#include "serialize.hpp"

#include <vector>
using namespace std;

const int _businessExists = 1 << 0;
//const int _businessIsRetail = 1 << 1;
//const int _businessIsGovernment = 1 << 2;
const int _businessOpen = 1 << 3;
const int _businessComeToWork = 1 << 4;
const int _businessIsHotel = 1 << 5;
const int _businessTypeShift = 24;
const int _businessTypeMask = 255 << _businessTypeShift;

enum BusinessType {
  Retail, Office, Farm, Factory, Institution,
  numBusinessTypes
};

struct Position {
  item employee;
  EducationLevel minEducation;
};

struct Business {
  int flags;
  item building;
  float foundedTime;
  float lastFreightTime;
  float lastCustomerTime;
  char* name;
  vector<Position> positions;
};

item addBusiness(item buildingNdx, BusinessType type);
Business* getBusiness(item businessNdx);
void employeeQuit(item personNdx);
void employeeHired(item bussNdx, item personNdx);
char* getBusinessDescriptor(item ndx);
IssueIcon getBusinessIssue(item ndx);
BusinessType getBusinessType(item ndx);
bool longTimeSinceFreight(item ndx);
bool longTimeSinceCustomer(item ndx);

float retailRatio();
item getRandomRetail();
item getRandomBusiness();
item getRandomPosition(EducationLevel edu);
item numBusinesses(item econ);
item sizeBusinesses();
float getBusinessTypeDemand(item econ, BusinessType type);
float getBusinessTypeGrowth(item econ, BusinessType type);
const char* getBusinessTypeName(BusinessType type);

void updateBusiness(item ndx, float duration, vec3 loc);
void removeBusiness(item businessNdx);
void updateBusinesses(double duration);
void resetBusinesses();
void rebuildBusinessStats();
void writeBusinesses(FileBuffer* file);
void readBusinesses(FileBuffer* file, int version);

