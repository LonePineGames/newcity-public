#ifndef PERSON_H
#define PERSON_H

#include "building/building.hpp" // For IssueIcon
#include "route/location.hpp"
#include "vehicle/vehicle.hpp"

const int _personExists = 1 << 0;
const int _personTraveling = 1 << 1;
const int _personIsWorker = 1 << 2;
const int _personIsFemale = 1 << 3;
const int _personIsChild = 1 << 4;

//const int _personIsHSEducated = 1 << 5;
//const int _personIsCollegeEducated = 1 << 6;
const int _personEducationShift = 5;
const int _personEducationMask = 3 << _personEducationShift;

const int _personWaitingForRoute = 1 << 8;
const int _personAwake = 1 << 9;
const int _personIsCollegeStudent = 1 << 10;
const int _personSick = 1 << 11;
const int _personIsTourist = 1 << 12;

const int _familyExists = 1 << 0;
const int _familyIsDormRoom = 1 << 1;
const int _familyIsTourists = 1 << 2;

struct Family {
  int flags;
  vector<item> members;
  item home;
  item econ;
  float lastStoreTime;
  float lastWorkTime;
  char* name;
};

enum Activity {
  DecisionActivity, SleepActivity, HomeActivity, WorkActivity,
  FriendsActivity, StoreActivity, FreightActivity, GovernmentActivity,
  InterviewActivity, DoctorActivity,
  TouristReturnActivity,
  //DrivingChildrenActivity, FollowParentActivity,
  numActivities
};

enum EducationLevel {
  NoEducation, HSDiploma, BachelorsDegree, Doctorate,
  numEducationLevels
};

struct Person {
  int flags;
  item family;
  item location;
  item activity;
  item lastActivity;
  item activityBuilding;
  item activityTarget;
  item employer;
  VehicleDescription vehicleDescription;
  float energy;
  float enterTime;
  float sleepTime;
  float birthday;
  char* name;
};

Person* getPerson(item personNdx);
void sleepPerson(item personNdx, float wakeTime);
void wakePerson(item personNdx);
void sleepPerson(item personNdx);
float getPersonWakeTime(item personNdx);
float getActivityScore(item personNdx, item activity);

void putPersonInTravelGroup_g(item personNdx, item groupNdx);
void putPersonInBuilding(item personNdx, item buildingNdx);
void removePersonFromLocation(item personNdx);
void evacutePerson(item personNdx);
void killPerson(item personNdx);

IssueIcon getFamilyIssue(item ndx);
char* getFamilyDescriptor(item ndx);
char* getPersonDescriptor(item ndx);
char* personName(item personNdx);
vec3 getPersonIcon(Person* p);
char* getActivityName(item personNdx);
const char* getRawActivityName(item activity);
void getJob(item personNdx);
void quitJob(item personNdx);
void findAnyJob(item personNdx);
void educatePerson(item personNdx, EducationLevel level);
void finishPersonRoute(item personNdx, Route* route);
void assignPersonToFreight(item pNdx);
void comeToWork(item pNdx);
EducationLevel getEducationForPerson(item personNdx);
void adjustPersonStats(item personNdx, int mult);

float getTouristTypicalStay(item econ);
float memorableExperiencesForPerson(item personNdx, float duration);
void haveMemories(item econNdx, item personNdx, float memories);

item numPeople(item econ);
item sizePeople();
item numFamilies(item econ);
int emergencyHousingBalance(item econ);
float unemploymentRate(item econ, EducationLevel edu);
float unemploymentRate(item econ);
void updatePeople(double duration);
void update1000People(double duration);
void resetPeople();
void rebuildPopulationStats();
void writePeople(FileBuffer* file);
void readPeople(FileBuffer* file, int version);

void addFamily(double data);
void addTouristFamily(double data);
void removeFamily(item familyNdx);
void evictFamily(item familyNdx);
void evictFamily(item familyNdx, bool findNew);
void handleHome(item familyNdx);
Family* getFamily(item familyNdx);
int sizeFamilies();

#endif
