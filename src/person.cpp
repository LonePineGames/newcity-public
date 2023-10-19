#include "person.hpp"

#include "amenity.hpp"
#include "board.hpp"
#include "building/building.hpp"
#include "building/design.hpp"
#include "business.hpp"
#include "city.hpp"
#include "economy.hpp"
#include "game/game.hpp"
#include "game/task.hpp"
#include "heatmap.hpp"
#include "icons.hpp"
#include "lane.hpp"
#include "lot.hpp"
#include "name.hpp"
#include "option.hpp"
#include "pool.hpp"
#include "selection.hpp"
#include "string_proxy.hpp"
#include "route/broker.hpp"
#include "route/router.hpp"
#include "time.hpp"
#include "util.hpp"
#include "vehicle/travelGroup.hpp"
#include "zone.hpp"

#include "parts/messageBoard.hpp"

const float maleWorkforceParticipationRate = 0.8;
const float femaleWorkforceParticipationRate = 0.4;
const float singleRate = 0.5;
const float singleMotherRate = 0.1;
const float singleFatherRate = 0.05;
const float familiesWithChildrenRate = 0.4;
const float maxChildren = 6;
const float extraAdultRate = 0.5;

Pool<Person>* people = Pool<Person>::newPool(20000);
Pool<Family>* families = Pool<Family>::newPool(20000);
const int numWakeLists = 24*60;
Cup<float> wakeTimes;
Cup<float> evaluateTime;

void teleport(item personNdx, item building);

void setWakeTime(item ndx, float time) {
  wakeTimes.ensureSize(ndx+1);
  wakeTimes.set(ndx, time);
}

void sleepPerson(item ndx) {
  setWakeTime(ndx, getCurrentDateTime() + c(CPersonUpdateTime)*oneHour);
}

void wakePerson(item ndx) {
  setWakeTime(ndx, getCurrentDateTime());
}

void sleepPersonUntil(item ndx, float time) {
  setWakeTime(ndx, time);
}

void sleepPersonForever(item ndx) {
  setWakeTime(ndx, FLT_MAX);
}

float getPersonWakeTime(item ndx) {
  return wakeTimes[ndx];
}

float randomAge(float min, float max) {
  return oneYear*(min+(max-min)*(1 - pow(1-randFloat(), 0.5)));
}

float randomAge() {
  return randomAge(18, 80);
}

int emergencyHousingBalance(item econ) {
  float pop = getStatistic(econ, Population);
  float targetHomeless = unemploymentRate(econ) * c(CUnemploymentHomelessness) * pop;
  targetHomeless += zoneDemand(econ, ResidentialZone) * pop * c(CResDemandHomelessness);
  targetHomeless *= 1 - c(CCommunityHomelessness) * getStatistic(econ, CommunityStat);
  targetHomeless -= getStatistic(econ, EmergencyHousing);
  return targetHomeless;
}

int maxHomeless(item econ) {
  float targetHomeless = emergencyHousingBalance(econ);
  if (targetHomeless < 0) targetHomeless = 0;
  return targetHomeless;
}

EducationLevel getEducationForPerson(item personNdx) {
  int flags = getPerson(personNdx)->flags;
  int edu = (flags & _personEducationMask) >> _personEducationShift;
  return (EducationLevel)edu;
}

item getFamilyEcon(item familyNdx) {
  Family* fam = getFamily(familyNdx);
  if (fam->econ == 0) {
    return nationalEconNdx();
  } else {
    return fam->econ;
  }
}

item getPersonEcon(item personNdx) {
  Person* person = getPerson(personNdx);
  return getFamilyEcon(person->family);
}

void adjustPersonStats(item personNdx, int mult) {
  Person* person = getPerson(personNdx);
  EducationLevel edu = getEducationForPerson(personNdx);
  item econ = getPersonEcon(personNdx);

  if (person->flags & _personSick) {
    adjustStat(econ, PeopleSick, mult);
  }

  if (person->flags & _personIsTourist) {
    adjustStat(econ, NumTouristsNow, mult);
    return;
  }

  adjustStat(econ, Population, mult);

  if (edu == 0) {
    adjustStat(econ, (Statistic)(NumNoEdu+edu), mult);
  } else {
    for (int i = 1; i <= edu; i++) {
      adjustStat(econ, (Statistic)(NumNoEdu+i), mult);
    }
  }

  if (person->flags & _personIsWorker) {
    adjustStat(econ, (Statistic)(NumNoEduWorkers+edu), mult);
    adjustStat(econ, NumWorkers, mult);
    if (person->employer == 0) {
      adjustStat(econ, NumUnemployed, mult);
      adjustStat(econ, (Statistic)(NumNoEduUnemployed+edu), mult);
    } else {
      adjustStat(econ, NumEmployed, mult);
    }
  }

  if (person->flags & _personIsCollegeStudent) {
    adjustStat(econ, NumCollegeStudents, mult);
  }
}

void maybeConvertToWorker(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;
  if (person->flags & _personIsChild) return;
  if (person->flags & _personIsWorker) return;
  if (person->flags & _personIsCollegeStudent) return;
  if (person->flags & _personIsTourist) return;
  bool isFemale = person->flags & _personIsFemale;
  if ((!isFemale && randFloat() < maleWorkforceParticipationRate) ||
    (isFemale && randFloat() < femaleWorkforceParticipationRate)) {
    adjustPersonStats(personNdx, -1);
    person->flags |= _personIsWorker;
    adjustPersonStats(personNdx, 1);

    findAnyJob(personNdx);
  }
}

void findAnyJob(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;
  if (!(person->flags & _personIsWorker)) return;
  if (person->employer != 0) return; // nothing to do
  Family* family = getFamily(person->family);
  if (unemploymentRate(family->econ) < targetUnemploymentRate(family->econ)) return; // keep unemployment near target
  EducationLevel edu = getEducationForPerson(personNdx);

  // look for a brokered (routable) job
  if (family->home != 0) {
    Building* home = getBuilding(family->home);
    item laneBlock = home->graphLoc.lane;
    Supply supplies = (Supply)(SuppliesNoEduJob + edu);
    item brokered = routeBroker_p(laneBlock, supplies);
    if (brokered != 0) {
      employeeHired(brokered, personNdx);
      if (person->employer != 0) return;
    }
  }

  // get a job from anywhere
  for (int i = 0; i < 10; i++) {
    if (i > 5 && edu > 0) edu = (EducationLevel)(((item)edu)-1);
    if (person->employer != 0) break;
    item businessNdx = getRandomPosition(edu);
    if (businessNdx == 0) continue;
    Business* b = getBusiness(businessNdx);
    if (!(b->flags & _businessExists)) continue;
    employeeHired(businessNdx, personNdx);
    //if (person->employer == 0) {
      //SPDLOG_INFO("Employer didn't hire employee with edu {}", edu);
    //}
  }

  /*
  if (person->employer == 0) {
    edu = getEducationForPerson(personNdx);
    SPDLOG_INFO("Didn't find job for new worker {} edu{}",
        personNdx, edu);
    //setSelection(SelectionPerson, personNdx);
  }
  */
}

void retirePerson(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;
  if (person->flags & _personIsChild) return;
  if (!(person->flags & _personIsWorker)) return;
  adjustPersonStats(personNdx, -1);
  person->flags &= ~_personIsWorker;
  adjustPersonStats(personNdx, 1);
  //SPDLOG_INFO("person quits job due to retirement");
  quitJob(personNdx);
}

item addPerson(bool isFemale, float age, item familyNdx, int flags) {
  item personNdx = people->create();
  Person* person = getPerson(personNdx);
  person->flags = _personExists | flags;
  bool isStudent = flags & _personIsCollegeStudent;
  bool isTourist = flags & _personIsTourist;
  person->family = familyNdx;
  person->activity = DecisionActivity;
  person->lastActivity = DecisionActivity;
  person->activityTarget = 0;
  person->activityBuilding = 0;
  person->employer = 0;
  person->energy = randFloat(0.5, 2.0);
  float time = getCurrentDateTime();
  person->enterTime = time;
  person->sleepTime = time;
  person->birthday = time - age;
  person->vehicleDescription = createVehicleDescription();
  person->location = 0;
  person->name = intern(
      randomName(isFemale ? GivenFemaleName : GivenMaleName));
  bool isChild = age <= 16*oneYear;
  Family* family = getFamily(familyNdx);
  family->members.push_back(personNdx);
  item econ = getPersonEcon(personNdx);

  if (family->flags & _familyIsTourists) {
    person->flags |= _personIsTourist;
    isTourist = true;
  }

  if (family->flags & _familyIsDormRoom) {
    person->flags |= _personIsCollegeStudent;
    isStudent = true;
  }

  if (isFemale) {
    person->flags |= _personIsFemale;
  }

  if (isChild) {
    person->flags |= _personIsChild;
  }

  adjustStat(econ, PopulationGrowth, 1);
  adjustPersonStats(personNdx, 1);

  if (isTourist) {
    adjustStat(econ, NumTouristsArriving, 1);
  }

  if (isStudent) {
    educatePerson(personNdx, HSDiploma);

  } else if (!isChild) {
    float target = randFloat();
    for (int i = numEducationLevels-1; i > 0; i--) {
      float nat = i == 3 ? c(CNationalPhdEdu) :
                  i == 2 ? c(CNationalBclEdu) :
                           c(CNationalHSEdu);
      float city = getStatistic(econ, (Statistic)(NoEduPercent+i));
      float cityPossible = (econ == ourCityEconNdx()) ?
        nonZero(getEduLevelLimit(i)) / numPeople(econ) : nat*2;
      float cityTarget = mix(city, cityPossible, c(CEducationMix));
      float mixedTarget = mix(cityTarget, nat, c(CEducationNationalMix));
      if (isTourist) {
        mixedTarget *= (numEducationLevels-i)*0.2f/numEducationLevels*
          getStatistic(ourCityEconNdx(), TouristRating) + 0.1;
      } else {
        mixedTarget *= 1 -
          4 * getStatistic(econ, (Statistic)(NoEduUnemployment+i));
      }
      target -= mixedTarget;
      //SPDLOG_INFO("{} edu{} c{} cp{} ct{} mt{} t{}=>{}",
          //personNdx, i, city, cityPossible, cityTarget, mixedTarget,
          //mixedTarget+target, target);
      if (target <= 0) {
        educatePerson(personNdx, (EducationLevel)i);
        break;
      }
    }
  }

  maybeConvertToWorker(personNdx);

  //SPDLOG_INFO("addPerson:{} to family:{} isTourist:{} famIsTourist:{}",
      //personNdx, person->family, isTourist,
      //bool(family->flags & _familyIsTourists));

  wakePerson(personNdx);
  return personNdx;
}

Person* getPerson(item personNdx) {
  return people->get(personNdx);
}

char* personName(item personNdx) {
  Person* person = getPerson(personNdx);
  Family* family = getFamily(person->family);
  return sprintf_o("%s %s", person->name, family->name);
}

vec3 getPersonIcon(Person* p) {
  if (!(p->flags & _personExists)) {
    return iconNo;
  } else if (p->flags & _personIsChild) {
    return iconPersonChild;
  } else if (p->flags & _personIsTourist) {
    if (p->flags & _personIsFemale) {
      return iconTouristFemale;
    } else {
      return iconTouristMale;
    }
  } else if (p->flags & _personIsFemale) {
    return iconPersonWoman;
  } else {
    return iconPersonMan;
  }
}

void removePersonFromLocation(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;

  if (person->location != 0) {
    if (person->flags & _personTraveling) {
      removePersonFromTravelGroup_g(personNdx,
          locationNdx(person->location));

    } else {
      Building* building = getBuilding(person->location);
      for (int i = building->peopleInside.size() - 1; i >= 0; i--) {
        if (building->peopleInside[i] == personNdx) {
          building->peopleInside.erase(building->peopleInside.begin() + i);
        }
      }
    }
  }

  person->location = 0;
  person->flags &= ~_personTraveling;
  wakePerson(personNdx);
}

void educatePerson(item personNdx, EducationLevel level) {
  Person* person = getPerson(personNdx);
  EducationLevel prev = getEducationForPerson(personNdx);
  if (prev >= level) return;

  // Write new education level
  adjustPersonStats(personNdx, -1);
  person->flags &= ~_personEducationMask;
  person->flags |= level << _personEducationShift;
  adjustPersonStats(personNdx, 1);

  //quitJob(personNdx);
}

void quitJob(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists) || person->employer == 0) {
    return;
  }

  employeeQuit(personNdx);
  wakePerson(personNdx);
}

void removePerson(item personNdx, bool andFamily) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;
  removePersonFromLocation(personNdx);
  deselect(SelectionPerson, personNdx);
  removeMessageByObject(PersonMessage, personNdx);
  item econ = getPersonEcon(personNdx);

  if (person->flags & _personIsCollegeStudent) {
    adjustStat(econ, NumStudentsDroppingOut, 1);
  }

  if (person->flags & _personIsTourist) {
    adjustStat(econ, NumTouristsLeaving, 1);
  } else {
    adjustStat(econ, PopulationGrowth, -1);
  }

  if (person->employer != 0) {
    quitJob(personNdx);
  }

  EducationLevel edu = getEducationForPerson(personNdx);
  adjustPersonStats(personNdx, -1);

  sleepPersonForever(personNdx);
  person->flags = 0;
  //free(person->name);
  person->name = 0;
  people->free(personNdx);

  Family* fam = getFamily(person->family);
  removeFromVector(&fam->members, personNdx);
  if (fam->members.size() == 0 && andFamily) {
    removeFamily(person->family);
  }
}

void removePerson(item personNdx) {
  removePerson(personNdx, true);
}

bool allowNewCollegeStudents(item econNdx) {
  return boardSize(econNdx, DormBunks) > 0 &&
    getStatistic(econNdx, MaxCollegeStudents) >
    getStatistic(econNdx, NumCollegeStudents);
}

void moveFamilyEcon(item familyNdx, item econNdx) {
  Family* fam = getFamily(familyNdx);

  for (int i = 0; i < fam->members.size(); i++) {
    adjustPersonStats(fam->members[i], -1);
  }
  adjustStat(getFamilyEcon(familyNdx), NumFamilies, -1);

  //SPDLOG_INFO("moveFamilyEcon:{} econ:{}=>{}",
      //familyNdx, fam->econ, econNdx);
  fam->econ = econNdx;

  for (int i = 0; i < fam->members.size(); i++) {
    adjustPersonStats(fam->members[i], 1);
  }
  adjustStat(getFamilyEcon(familyNdx), NumFamilies, 1);
}

item addFamily(item econ, bool isTourists) {
  bool isCollege = !isTourists && allowNewCollegeStudents(econ);
  if (isCollege && boardSize(econ, Homes) >= 0 && randFloat() < 0.5) {
    isCollege = false;
  }

  item familyNdx = families->create();
  Family* family = getFamily(familyNdx);
  family->flags = _familyExists;
  if (isTourists) family->flags |= _familyIsTourists;
  family->home = 0;
  family->lastStoreTime = getCurrentDateTime();
  family->lastWorkTime = getCurrentDateTime();
  family->name = intern(randomName(FamilyName));
  family->members.clear();
  family->econ = econ;
  econ = getFamilyEcon(familyNdx);
  char* specialName = 0;

  if (randFloat() < c(CPersonContributorNameChance)) {
    //free(family->name);
    specialName = randomName(ContributorName);
    int l = strlen(specialName);
    for (; l >= 0 && specialName[l] != ' '; l --);
    family->name = intern(strdup_s(specialName + l + 1));
    if (l > 0) {
      specialName[l] = '\0';
    }
  }

  if (isCollege) {
    family->flags |= _familyIsDormRoom;
    addPerson(randFloat() < 0.5, randomAge(18, 24), familyNdx,
        _personIsCollegeStudent);
    adjustStat(econ, NumStudentsMovingIn, family->members.size());

  } else {
    float headAge = randomAge();
    float mothersAge = headAge;
    bool hasKids = false;
    uint32_t flags = isTourists ? _personIsTourist : 0;
    if (randFloat() < singleRate) {
      bool isFemale = randItem(2) == 0;
      hasKids = (isFemale && randFloat() < singleMotherRate) ||
        (!isFemale && randFloat() < singleFatherRate);
      addPerson(isFemale, headAge, familyNdx, flags);
    } else {
      addPerson(false,  headAge, familyNdx, flags);
      mothersAge = headAge + randFloat(-5,3)*oneYear;
      addPerson(true, mothersAge, familyNdx, flags);
      hasKids = randFloat() < familiesWithChildrenRate;
    }

    float kidsAgeMin = std::max(0.f, mothersAge-35*oneYear);
    float kidsAgeMax = std::min(mothersAge-18*oneYear, 20.f*oneYear);
    if (hasKids && kidsAgeMin+1 < kidsAgeMax) {
      float k = randFloat();
      float num = (k*k) * (maxChildren-1) + 1;
      num = std::min((kidsAgeMax-kidsAgeMin)/oneYear, num);
      for (int i = 0; i < num; i++) {
        float age = randFloat(kidsAgeMin, kidsAgeMax);
        addPerson(randFloat() < .5f, age, familyNdx, flags);
      }
    }

    while (randFloat() * family->members.size() < extraAdultRate) {
      addPerson(randItem(2) == 0, randomAge(), familyNdx, flags);
    }

    if (!isTourists) {
      adjustStat(econ, FamiliesMovingIn, 1);
    }
  }

  if (specialName != 0) {
    Person* main = getPerson(family->members[0]);
    //free(main->name);
    main->name = intern(strdup_s(specialName));
    free(specialName);
  }

  if (!isTourists) {
    adjustStat(econ, NumFamilies, 1);
    adjustStat(econ, NumHomeless, family->members.size());
  }

  handleHome(familyNdx);
  if (!(family->flags & _familyExists)) {
    return 0;
  }

  // Place Tourists in a random building in a neighboring city
  if (isTourists && numBuildings() > 0 && countNeighbors() > 0) {
    item buildingNdx = 0;
    item k = 0;
    while (buildingNdx == 0 && k < 1000) {
      buildingNdx = randItem(sizeBuildings())+1;
      Building* b = getBuilding(buildingNdx);
      if (!(b->flags & _buildingExists) || b->econ == ourCityEconNdx()) {
        buildingNdx = 0;
      }
      k ++;
    }

    if (buildingNdx != 0) {
      for (int i = 0; i < family->members.size(); i++) {
        item personNdx = family->members[i];
        teleport(personNdx, buildingNdx);
      }
    }
  }

  //SPDLOG_INFO("addFamily:{} econ:{}/{}, members:{}",
      //familyNdx, econ, getFamilyEcon(econ), family->members.size());

  return familyNdx;
}

Family* getFamily(item familyNdx) {
  return families->get(familyNdx);
}

int sizeFamilies() {
  return families->size();
}

item getRandomFamily() {
  int size = families->size();
  if (size == 0) {
    return 0;
  }
  item ndx = randItem(size)+1;
  if (!(getFamily(ndx)->flags & _familyExists)) {
    return 0;
  }
  return ndx;
}

void evictFamily(item familyNdx, bool findNew) {
  Family* family = getFamily(familyNdx);
  if (!(family->flags & _familyExists)) {
    SPDLOG_INFO("bad family evicted {}", familyNdx);
    return;
  }

  if (family->home != 0) {
    if (!(family->flags & _familyIsTourists)) {
      adjustStat(getFamilyEcon(familyNdx), NumHomeless,
          family->members.size());
    }
    item home = family->home;
    family->home = 0;
    removeFamilyFromHome(home, familyNdx);

    if (findNew) handleHome(familyNdx);
  }
}

void evictFamily(item familyNdx) {
  evictFamily(familyNdx, true);
}

void removeFamily(item familyNdx) {
  Family* family = getFamily(familyNdx);
  if (!(family->flags & _familyExists)) return;
  //SPDLOG_INFO("remove family:{} with {} members",
      //familyNdx, family->members.size());
  if (!(family->flags & _familyExists)) {
    SPDLOG_INFO("bad family removed {}", familyNdx);
    logStacktrace();
    return;
  }

  item econ = getFamilyEcon(familyNdx);
  adjustStat(econ, NumFamilies, -1);

  if (family->home == 0) {
    if (!(family->flags & _familyIsTourists)) {
      int numMembers = family->members.size();
      adjustStat(econ, NumHomeless, -numMembers);
    }
  } else {
    removeFamilyFromHome(family->home, familyNdx);
  }

  for(int i = family->members.size()-1; i >= 0; i--) {
    //SPDLOG_INFO("removing person:{} member:{}/{} of family:{}",
        //family->members[i], i, family->members.size(), familyNdx);
    removePerson(family->members[i], false);
  }

  vector<item> swap;
  family->members.swap(swap);

  adjustStat(econ, FamiliesMovingOut, 1);
  adjustStat(econ, TotalMoves, 1);
  family->flags = 0;
  //free(family->name);
  family->name = 0;
  families->free(familyNdx);
}

void putPersonInTravelGroup_g(item personNdx, item groupNdx) {
  Person* person = getPerson(personNdx);
  removePersonFromLocation(personNdx);
  person->flags |= _personTraveling;
  person->enterTime = getCurrentDateTime();
  person->location = travelGroupLocation(groupNdx);
  sleepPerson(personNdx);
}

void putPersonInBuilding(item personNdx, item buildingNdx) {
  Person* person = getPerson(personNdx);
  item econ = getPersonEcon(personNdx);
  bool isTourist = person->flags & _personIsTourist;
  if (!person->flags & _personExists) return;

  if (!(person->flags & _personTraveling) &&
      person->location == buildingNdx) {
    return;
  }

  wakePerson(personNdx);
  adjustStat(econ, TotalTrips, 1);
  if (person->activity == FreightActivity) {
    adjustStat(econ, FreightShipments, 1);
  } else if (person->activity == HomeActivity ||
      person->activity == SleepActivity) {
    adjustStat(econ, TripsHome, 1);
  } else if (person->activity == InterviewActivity) {
    adjustStat(econ, JobInterviews, 1);
  } else if (person->activity == DoctorActivity) {
    adjustStat(econ, DoctorsVisits, 1);
  } else if (person->activity == WorkActivity) {
    adjustStat(econ, WorkCommutes, 1);
  } else if (person->activity == FriendsActivity) {
    adjustStat(econ, SocialVisits, 1);
  } else if (person->activity == StoreActivity) {
    adjustStat(econ, StoreTrips, 1);
  }

  if (buildingNdx <= 0 ||
      !(getBuilding(buildingNdx)->flags & _buildingExists)) {
    removePersonFromLocation(personNdx);
    person->location = 0;
    person->enterTime = getCurrentDateTime();
    return;
  }

  Building* building = getBuilding(buildingNdx);
  float time = getCurrentDateTime();
  float score = c(CSuccessfulTripProsperity);
  float travelTime = time - person->enterTime;

  if ((person->flags & _personTraveling) && travelTime <= c(CMaxCommute)*oneHour) {
    adjustStat(ourCityEconNdx(), TimeSpentTraveling, travelTime);
    adjustStat(ourCityEconNdx(), RecordedTrips, 1);
  }

  if (isTourist) {
    Design* d = getDesign(building->design);
    haveMemories(building->econ, personNdx,
        c(CMemoriesPerTourismPoint) * d->effect[Tourism]);
  }

  //SPDLOG_INFO("travel time: {}/{} {}", (time-person->enterTime)/oneHour,
      //c(CMaxCommute), getActivityName(personNdx));
  if (travelTime > c(CMaxCommute)*oneHour) {
    if (person->activity == WorkActivity ||
        person->activity == InterviewActivity) {
      //SPDLOG_INFO("person quits job due to long commute {}", personNdx);
      quitJob(personNdx);
      if (randFloat() < c(CRejobRate)) {
        findAnyJob(personNdx);
      }
    }
    if (person->activity != FreightActivity) {
      adjustStat(econ, LongTrips, 1);
      score = c(CLongTripProsperity);
    }

  } else {
    if (person->activity == StoreActivity) {
      heatMapAdd(Density, building->location, c(CShoppingTripDensity));
      if (person->activityTarget != 0) {
        Business* business = getBusiness(person->activityTarget);
        if (business->building == buildingNdx &&
            business->flags & _businessOpen) {

          business->lastCustomerTime = time;
          getFamily(person->family)->lastStoreTime = time;
          makeRetailTransaction(econ,
              getEducationForPerson(personNdx),
              isTourist);
          score = c(CShoppingTripProsperity);
        }
      }

    } else if (person->activity == WorkActivity) {
      Family* fam = getFamily(person->family);
      if (fam->flags & _familyIsTourists) {
        //SPDLOG_WARN("Tourists Working");
        //setSelection(SelectionPerson, personNdx);
        //setGameSpeed(0);
      } else {
        fam->lastWorkTime = time;
      }

    } else if (person->activity == InterviewActivity) {
      if (person->activityTarget != 0) {
        Family* fam = getFamily(person->family);
        if (fam->flags & _familyIsTourists) {
          //SPDLOG_WARN("Tourists Interviewing");
          //setSelection(SelectionPerson, personNdx);
          //setGameSpeed(0);
        } else {
          fam->lastWorkTime = time;
        }
        employeeHired(person->activityTarget, personNdx);
      }

    } else if (person->activity == DoctorActivity) {
      if (person->activityTarget != 0) {
        if (person->flags & _personSick) {
          adjustStat(econ, PeopleSick, -1);
          person->flags &= ~_personSick;
        }
      }

    } else if (person->activity == FreightActivity) {
      if (person->activityTarget != 0) {
        Business* business = getBusiness(person->activityTarget);
        if (business->building == buildingNdx &&
            (business->flags & _businessExists)) {

          business->lastFreightTime = time;
          score = c(CFreightDeliveryPropserity);
          if (person->employer != 0) {
            Business* b = getBusiness(person->employer);
            b->lastCustomerTime = time;
          }
        }
      }

    } else if (person->activity == GovernmentActivity) {
      if (person->activityBuilding != 0) {
        Building* b = getBuilding(person->activityBuilding);
        Design* d = getDesign(b->design);
        if (isDesignEducation(b->design)) {
          int edu = getEducationForPerson(personNdx);
          if (edu < Doctorate) {
            int nextEdu = edu+1;
            float numEdu = getStatistic(b->econ,
                (Statistic)(NumNoEdu+nextEdu));
            float maxEdu = getEduLevelLimit(nextEdu);
            if (numEdu < maxEdu &&
                d->flags & (_designProvidesHSDiploma << (nextEdu-1))) {
              educatePerson(personNdx, (EducationLevel)nextEdu);
            }
          }
        }
      }
    }
  }

  heatMapAdd(Prosperity, building->location, score);
  removePersonFromLocation(personNdx);
  person->location = buildingNdx;
  person->enterTime = getCurrentDateTime();
  building->peopleInside.push_back(personNdx);
  sleepPerson(personNdx);
}

#include "activity.cpp"

const char* activityName[] = {
  "Thinking", "Asleep", "At Home", "At Work",
  "Visiting Friend", "At Store", "Delivering Freight",
  "Visiting Amenity", "Interviewing for a Job", "Visiting a Doctor",
  "Returing Home",
};

const char* activityTransitName[] = {
  "Leaving", "Going Home to Sleep", "Going Home", "Going to Work",
  "Going to Visit Friend", "Going to Store", "Delivering Freight",
  "Going to Amenity", "Heading to a Job Interview", "Visiting a Doctor",
  "Returing Home",
};

const char* getRawActivityName(item activity) {
  return activityTransitName[activity];
}

char* getActivityName(item personNdx) {
  Person* person = getPerson(personNdx);
  if (person->activity == GovernmentActivity &&
      person->activityBuilding != 0) {
    Building* b = getBuilding(person->activityBuilding);
    if (b->flags & _businessExists && b->design != 0) {
      Design* d = getDesign(b->design);
      if (person->flags & _personTraveling) {
        return sprintf_o("Going to %s", d->displayName);
      } else if (d->category == EducationCategory) {
        return sprintf_o("At %s", d->displayName);
      } else {
        return sprintf_o("Visiting %s", d->displayName);
      }
    }
  }

  bool isCorrectBuilding = !(person->flags & _personTraveling) &&
    person->activityBuilding != 0 &&
    person->location == person->activityBuilding;
  if (isCorrectBuilding) {
    return strdup_s(activityName[person->activity]);
  } else {
    return strdup_s(activityTransitName[person->activity]);
  }
}

void teleport(item personNdx, item building) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;

  removePersonFromLocation(personNdx);
  adjustStat(getPersonEcon(personNdx), Teleports, 1);
  person->enterTime = getCurrentDateTime();
  putPersonInBuilding(personNdx, person->activityBuilding);
}

void sendPerson(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;

  item econ = getPersonEcon(personNdx);

  if (person->flags & _personTraveling) {
    return;
  } else if (person->flags & _personWaitingForRoute) {
    SPDLOG_INFO("sendPerson but waiting for route");
    return;
  } else if (person->activityBuilding == 0) {
    setDeciding(personNdx);
    return;
  } else if (person->location == person->activityBuilding) {
    return;
  } else if (person->location == 0 ||
      person->location == person->activityBuilding) {
    teleport(personNdx, person->activityBuilding);

  } else {
    GraphLocation source = getBuilding(person->location)->graphLoc;
    GraphLocation dest = getBuilding(person->activityBuilding)->graphLoc;
    float time = laneRoutingEstimate_r(source.lane, dest.lane);
    if (person->activityBuilding != getFamily(person->family)->home &&
        time > c(CMaxCommute)*oneHour) {
      adjustStat(econ, FailedDistanceTests, 1);
      setDeciding(personNdx);

    } else if (vecDistance(getLocation(source), getLocation(dest)) <
        c(CStopRadius)) {
      teleport(personNdx, person->activityBuilding);

    } else {
      if (person->activity == HomeActivity ||
          person->activity == SleepActivity) {
        adjustStat(econ, HomeRoutes, 1);
      } else if (person->activity == WorkActivity) {
        adjustStat(econ, WorkRoutes, 1);
      } else if (person->activity == InterviewActivity) {
        adjustStat(econ, InterviewRoutes, 1);
      } else if (person->activity == DoctorActivity) {
        adjustStat(econ, DoctorRoutes, 1);
      } else if (person->activity == StoreActivity) {
        adjustStat(econ, StoreRoutes, 1);
      } else if (person->activity == FreightActivity) {
        adjustStat(econ, FreightRoutes, 1);
      } else if (person->activity == GovernmentActivity) {
        Building* building = getBuilding(person->activityBuilding);
        Design* design = getDesign(building->design);
        if (design->category == EducationCategory
            || design->category == UniversityCategory) {
          adjustStat(econ, SchoolCommutes, 1);
        } else if (design->category == ParksCategory) {
          adjustStat(econ, ParkTrips, 1);
        } else {
          adjustStat(econ, CommunityEngagements, 1);
        }
      } else {
        adjustStat(econ, OtherRoutes, 1);
      }

      person->flags |= _personWaitingForRoute;
      routePerson_g(personNdx, source, dest);
    }
  }
}

void handleHome(item familyNdx) {
  Family* family = getFamily(familyNdx);
  if (!(family->flags & _familyExists)) return;
  if (family->home != 0) return;
  bool isDorm = family->flags & _familyIsDormRoom;
  bool isTourists = family->flags & _familyIsTourists;
  item econ = getFamilyEcon(familyNdx);
  item targetEcon = econ;

  if (randFloat() < c(CMoveEconRate)) {
    targetEcon = randItem(sizeEcons())+1;
  }

  if (isTourists ||
      getStatistic(targetEcon, NumHomeless) >= maxHomeless(targetEcon)*.5f) {
    item newHome = boardTake(targetEcon, isDorm ? DormBunks :
        isTourists ? HotelRooms : Homes);
    if (isDorm && newHome == 0) newHome = boardTake(targetEcon, Homes);

    if (newHome != 0) {
      Building* building = getBuilding(newHome);
      building->families.push_back(familyNdx);
      family->home = newHome;

      int numMembers = family->members.size();
      if (!isTourists) {
        heatMapAdd(Prosperity, building->location, c(CMoveInProsperity));
        adjustStat(econ, NumHomeless, -numMembers);
        adjustStat(econ, TotalMoves, 1);
        supplyTableSuggest_g(building->graphLoc.lane,
            SuppliesFriend, familyNdx);
      }

      if (building->econ != econ) {
        adjustStat(building->econ, TotalMoves, 1);
        moveFamilyEcon(familyNdx, building->econ);
      }

    } else if (!isTourists) {
      float buffer = randFloat();
      buffer = buffer * buffer;
      buffer *= 20;

      if (getStatistic(econ, NumHomeless) > maxHomeless(econ)*2 + buffer) {
        removeFamily(familyNdx);
      }
      //SPDLOG_INFO("family:{} removed due to homelessness", familyNdx);
    }
  }
}

void killPerson(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;

  float time = getCurrentDateTime();
  float age = (time - person->birthday)/oneYear;
  adjustStat(getPersonEcon(personNdx), Deaths, 1);
  adjustStat(getPersonEcon(personNdx), DeathYears, age);
  removePerson(personNdx);
}

void handleAge(item personNdx, float dur) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;

  // Special handling for tourists; remove them once they leave the city
  float time = getCurrentDateTime();
  Family* family = getFamily(person->family);
  if (person->flags & _personIsTourist) {
    float stayTime = time - family->lastWorkTime;
    float typicalStay = getTouristTypicalStay(ourCityEconNdx());
    stayTime += 0.5f; // fudge factor
    if (stayTime > 1.5f*typicalStay) {
      removePerson(personNdx);
    } else if (stayTime > getTouristTypicalStay(ourCityEconNdx())) {
      if (countNeighbors() == 0 || person->location == 0) {
        removePerson(personNdx);
      } else if (!(person->flags & _personTraveling)) {
        Building* b = getBuilding(person->location);
        if (b->econ != ourCityEconNdx()) {
          removePerson(personNdx);
        }
      }
    }
    return;
  }

  bool isChild = person->flags & _personIsChild;
  bool isStudent = person->flags & _personIsCollegeStudent;
  float age = (time - person->birthday)/oneYear;

  float health = family->home == 0 ? 0.5 :
    heatMapGet(HealthHM, getBuilding(family->home)->location);
  float maxAge = c(CMaxAgeBase) + c(CMaxAgeHealthAdjust)*health;
  float probOfDeathYear = 1.0f/pow(2, (maxAge-age)/c(CDeathFactor));
  if (randFloat() < probOfDeathYear * dur/oneYear) {
    killPerson(personNdx);
    return;
  }

  if (age > c(CRetirementAge)) {
    retirePerson(personNdx);

  } else if (age > 20+personNdx%8 && isStudent) {
    //removePersonFromFamily(person->family);
    person->flags &= ~_personIsCollegeStudent;
    family->flags &= ~_familyIsDormRoom;
    evictFamily(person->family);
    educatePerson(personNdx, BachelorsDegree);
    adjustStat(getPersonEcon(personNdx), NumStudentsGraduating, 1);
    maybeConvertToWorker(personNdx);

  } else if (age > 16 && isChild) {
    person->flags &= ~_personIsChild;
    maybeConvertToWorker(personNdx);

  } else if (age <= 16 && !isChild) { // sanity check
    person->flags |= _personIsChild;
  }
}

void handleJob(item personNdx) {
  Person* person = getPerson(personNdx);
  Family* fam = getFamily(person->family);
  if (!(person->flags & _personExists)) return;
  if (person->flags & _personIsChild) return;
  bool isWorker = person->flags & _personIsWorker;

  /*
  float timeSinceWork = time - fam->lastWorkTime;
  if (timeSinceWork > c(CWorkDays) && !isWorker &&
      randFloat() < c(CWorkerTransitionRate)) {
    adjustPersonStats(personNdx, -1);
    person->flags |= _personIsWorker;
    adjustPersonStats(personNdx, 1);

  } else if (timeSinceWork < 0.5 && isWorker &&
      randFloat() < c(CWorkerTransitionRate)) {

    bool otherWorkers = false;
    for (int i = 0; i < fam->members.size(); i++) {
      item ndx = fam->members[i];
      if (ndx == personNdx) continue;
      Person* p = getPerson(ndx);
      if (p->flags & _personIsWorker) {
        otherWorkers = true;
        break;
      }
    }

    if (otherWorkers) {
      if (person->employer != 0) {
        quitJob(personNdx);
      }
      adjustPersonStats(personNdx, -1);
      person->flags &= ~_personIsWorker;
      adjustPersonStats(personNdx, 1);
    }
  }
  */

  if (person->employer != 0) {
    Business* b = getBusiness(person->employer);
    if (!(b->flags & _businessExists)) {
      //SPDLOG_INFO("person works for business that doesn't exist {}", personNdx);
      quitJob(personNdx);
    }
  }
}

float getActivityScore(item personNdx, item activity) {
  return scoreActivity[activity](personNdx);
}

Supply supplyForActivity(item personNdx, item activity) {
  switch (activity) {
    case FriendsActivity: return SuppliesFriend;
    case FreightActivity: return SuppliesFreightNeed;
    case StoreActivity: return SuppliesRetail;
    case HomeActivity: return SuppliesNull;
    case SleepActivity: return SuppliesNull;
    case DecisionActivity: return SuppliesNull;
    case WorkActivity: return SuppliesNull;
    case DoctorActivity: return SuppliesNull;
  }

  if (activity == InterviewActivity) {
    item edu = getEducationForPerson(personNdx);
    return (Supply)(SuppliesNoEduJob + edu);
  }

  if (activity == GovernmentActivity) {
    Person* person = getPerson(personNdx);
    bool isTourist = person->flags & _personIsTourist;
    if (isTourist) {
      return SuppliesTourism;
    } else {
      return SuppliesAmenity;
    }
  }

  return SuppliesNull; // Shouldn't happen
}

void handleActivity(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) return;
  if (person->flags & _personTraveling) return;
  if (person->flags & _personWaitingForRoute) return;

  if (person->location == 0) {
    assignActivity[HomeActivity](personNdx);
    sendPerson(personNdx);
    return;
  }

  Building* b = getBuilding(person->location);
  if (!(b->flags & _buildingExists) ||
    (b->flags & _buildingAbandoned)) {
    setDeciding(personNdx);
  }

  float bestScore = c(CMinActScore) + 1;
  int bestActivity = DecisionActivity;
  for (int i = SleepActivity; i < numActivities; i++) {
    if (i == person->lastActivity && person->activity == DecisionActivity) {
      continue;
    }
    float score = scoreActivity[i](personNdx) + randFloat(0, 8*oneHour);
    if (score > bestScore) {
      bestActivity = i;
      bestScore = score;
    }
  }

  item laneBlock = b->graphLoc.lane;
  Supply supply = supplyForActivity(personNdx, bestActivity);
  bool assigned = false;

  if (supply != SuppliesNull) {
    item brokered = routeBroker_p(laneBlock, supply);

    if (brokered != 0) {
      item biz = brokered;
      item building = 0;
      if (supply == SuppliesHome || supply == SuppliesDorm || supply ==
          SuppliesHotel || supply == SuppliesHomeless) {
        building = brokered;
        biz = 0;
      } else if (supply == SuppliesFriend) {
        Family* fam = getFamily(brokered);
        building = fam->home;
      } else {
        Business* bb = getBusiness(brokered);
        building = bb->building;
      }

      if (building != 0) {
        setActivity(personNdx, bestActivity, building, biz);
        adjustStat(ourCityEconNdx(), RoutesBrokered, 1);
        assigned = true;
      }
    } else {
      SPDLOG_INFO("broker failed: {} @ {}", supply, laneBlock);
    }
  }

  if (!assigned) assignActivity[bestActivity](personNdx);

  person->lastActivity = bestActivity;
  if (person->activity == DecisionActivity) {
    wakePerson(personNdx);
  } else {
    sendPerson(personNdx);
  }
}

void evaluatePerson(item personNdx, float dur);
void updatePerson(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) {
    sleepPersonForever(personNdx);
    return;
  }
  adjustStat(getPersonEcon(personNdx), PeopleProcessed, 1);

  item familyNdx = person->family;
  Family* family = getFamily(person->family);

  /*
  if (!(family->flags & _familyExists)) {
    SPDLOG_ERROR("Person:{} family:{} doesn't exist",
        personNdx, person->family);
    setSelection(SelectionPerson, personNdx);
    setGameSpeed(0);
    removePerson(personNdx, false);
    return;
  }

  //#ifdef LP_DEBUG
  //if (debugMode()) {
    if (!(person->flags & _personTraveling) && person->location != 0) {
      Building* b = getBuilding(person->location);
      bool hasPerson = false;
      for (int i = 0; i < b->peopleInside.size(); i++) {
        if (b->peopleInside[i] == personNdx) {
          hasPerson = true;
          break;
        }
      }
      if (!hasPerson) {
        SPDLOG_ERROR("Person not in building");
        setSelection(SelectionPerson, personNdx);
        setGameSpeed(0);
      }
    }

    if (family->home != 0) {
      Building* b = getBuilding(family->home);
      bool hasFam = false;
      for (int i = 0; i < b->families.size(); i++) {
        if (b->families[i] == person->family) {
          hasFam = true;
          break;
        }
      }
      if (!hasFam) {
        SPDLOG_ERROR("Family not in home");
        setSelection(SelectionPerson, personNdx);
        setGameSpeed(0);
        b->families.push_back(person->family);
      }
    }

    if (person->energy  < 0.5) {
      SPDLOG_ERROR("Person with energy %f", person->energy);
      setSelection(SelectionPerson, personNdx);
      setGameSpeed(0);
    }

    bool famIsTourist = (family->flags & _familyIsTourists);
    bool personIsTourist = (person->flags & _personIsTourist);
    if (famIsTourist != personIsTourist) {
      SPDLOG_ERROR("Person:{} is Tourist: {} Fam:{} is Tourist: {}",
          personNdx, personIsTourist, person->family, famIsTourist);
      setSelection(SelectionPerson, personNdx);
      setGameSpeed(0);
    }

    bool isInFam = false;
    for (int i = 0; i < family->members.size(); i++) {
      if (family->members[i] == personNdx) {
        if (isInFam) {
          SPDLOG_ERROR("Person:{} in family:{} twice",
            personNdx, person->family);
          setSelection(SelectionPerson, personNdx);
          setGameSpeed(0);
        }
        isInFam = true;
      }
    }

    if (!isInFam) {
      SPDLOG_ERROR("Person:{} not in family:{} with {} members",
        personNdx, person->family, family->members.size());
      setSelection(SelectionPerson, personNdx);
      setGameSpeed(0);
    }

    if ((person->flags & _personIsChild) &&
        (person->flags & _personIsWorker)) {
      SPDLOG_ERROR("Person:{} in family:{} is both a child and a worker",
        personNdx, person->family);
      setSelection(SelectionPerson, personNdx);
      setGameSpeed(0);
    }
  //}
  //#endif
  */

  float time = getCurrentDateTime();
  person->sleepTime = time;
  sleepPerson(personNdx);

  //handleJob(personNdx);
  //if (!(person->flags & _personExists)) return;

  handleHome(person->family);
  if (!(person->flags & _personExists)) return;

  if ((person->flags & _personTraveling) &&
      time - person->enterTime > 1) {
    putPersonInBuilding(personNdx, person->activityBuilding);
  }

  float eTime = c(CPersonEvaluateTime)*oneHour;
  evaluateTime.ensureSize(personNdx+1);
  float dur = time - evaluateTime[personNdx];
  if (dur > eTime) {
    handleAge(personNdx, dur);
    if (!(person->flags & _personExists)) return;
    if (person->family != familyNdx) return;

    evaluateTime.set(personNdx, time);
    evaluatePerson(familyNdx, eTime);
    if (!(person->flags & _personExists)) return;
  }

  if (person->flags & _personIsChild) return;
  handleActivity(personNdx);
}

float memorableExperiencesForPerson(item personNdx, float duration) {
  Person* person = getPerson(personNdx);
  bool isTourist = person->flags & _personIsTourist;
  if (!isTourist) return 0;

  float memorableExperiences = 0;
  float time = getCurrentDateTime();
  Family* family = getFamily(person->family);

  float timeSinceStore = time - family->lastStoreTime;
  if (timeSinceStore > c(CStoreDays)) {
    memorableExperiences += c(CHungryTouristMemories);
  }

  if (person->flags & _personSick) {
    memorableExperiences += c(CSickTouristMemories);
  }

  memorableExperiences += c(CPrestigeMemories) * getEffectValue(Prestige);

  vec3 loc = vec3(-1, -1, -1);

  if (family->home == 0) {
    memorableExperiences += c(CNoHotelMemories);
  } else {
    loc = getBuilding(family->home)->location;
  }

  if (!(person->flags & _personTraveling) && person->location != 0) {
    loc = getBuilding(person->location)->location;
  }

  if (loc.x > 0) {
    float pollution = heatMapGet(Pollution, loc);
    memorableExperiences += pollution * c(CTouristMemoriesPollution);

    float value = heatMapGet(Value, loc);
    memorableExperiences += value * c(CTouristMemoriesValue);

    float density = heatMapGet(Value, loc);
    memorableExperiences += density * c(CTouristMemoriesDensity);

    float crime = heatMapGet(Crime, loc);
    memorableExperiences += crime * c(CTouristMemoriesCrime);

    float prosp = heatMapGet(Prosperity, loc);
    memorableExperiences += prosp * c(CTouristMemoriesProsperity);

    float community = heatMapGet(CommunityHM, loc);
    memorableExperiences += community * c(CTouristMemoriesCommunity);

    float health = heatMapGet(HealthHM, loc);
    memorableExperiences += health * c(CTouristMemoriesHeath);
  }

  memorableExperiences *= duration;
  return memorableExperiences;
}

void haveMemories(item econNdx, item personNdx, float memories) {
  Person* person = getPerson(personNdx);
  bool isTourist = person->flags & _personIsTourist;
  if (!isTourist) return;
  item iMemories = randRound(memories);
  if (iMemories <= 0) return;
  adjustStat(econNdx, Memories, iMemories);
  makeRetailTransaction(getPersonEcon(personNdx),
      getEducationForPerson(personNdx), isTourist);
}

void evaluatePerson(item personNdx, float duration) {
  Person* person = getPerson(personNdx);
  Family* family = getFamily(person->family);
  item econ = getPersonEcon(personNdx);
  EducationLevel edu = getEducationForPerson(personNdx);
  bool isTourist = person->flags & _personIsTourist;
  float prosperityEffects = 0;
  float crimeEffects = 0;
  float educationEffects = 0;
  float communityEffects = 0.0f;
  float healthEffects = c(CPersonHealth);
  float time = getCurrentDateTime();
  float age = (time - person->birthday)/oneYear;
  float prosperity = 0;
  float durMult = duration * gameDayInRealSeconds;
  float timeSinceWork = time - family->lastWorkTime;
  item memoriesEcon = econ;

  if (isTourist) {
    prosperityEffects += c(CTouristProsperity);
    crimeEffects += mix(c(CTouristCrime0), c(CTouristCrime5),
        getStatistic(econ, TouristRating) / 5.f);

  } else {
    if (edu == Doctorate) {
      educationEffects += c(CPhdEducationEffect);
      prosperityEffects += c(CPhdEducationProsperity);
      crimeEffects += c(CPhdEducationCrime);
    } else if (edu == BachelorsDegree) {
      educationEffects += c(CBclEducationEffect);
      prosperityEffects += c(CBclEducationProsperity);
      crimeEffects += c(CBclEducationCrime);
    } else if (edu == HSDiploma) {
      educationEffects += c(CHSEducationEffect);
      prosperityEffects += c(CHSEducationProsperity);
      crimeEffects += c(CHSEducationCrime);
    } else {
      educationEffects += c(CNoEducationEffect);
      prosperityEffects += c(CNoEducationProsperity);
      crimeEffects += c(CNoEducationCrime);
    }

    if (person->employer != 0) {
      prosperityEffects += c(CEmployedProsperity);
    } else if (person->flags & _personIsWorker) {
      prosperityEffects += c(CJoblessProsperity);
      crimeEffects += c(CJoblessCrime);
      communityEffects += c(CJoblessCommunity);
      healthEffects += c(CJoblessHealth);
    }

    if (timeSinceWork > c(CWorkDays)) {
      prosperityEffects += c(CJoblessFamilyProsperity);
      crimeEffects += c(CJoblessFamilyCrime);
      healthEffects += c(CJoblessHealth);
    }
  }

  float timeSinceStore = time - family->lastStoreTime;
  if (timeSinceStore > c(CStoreDays)) {
    prosperityEffects += c(CHungryFamilyProsperity);
    crimeEffects += c(CHungryFamilyCrime);
    healthEffects += c(CHungryHealth);
  }

  vector<vec3> locs;
  if (family->home == 0) {
    prosperityEffects += c(CHomelessProsperity);
    healthEffects += c(CHomelessHealth);

  } else {
    Building* home = getBuilding(family->home);
    vec3 loc = home->location;
    locs.push_back(loc);
    memoriesEcon = home->econ;

    float eduVal = heatMapGet(Education, loc);
    if (educationEffects <= c(CHSEducationEffect) && eduVal > 0.25 &&
        !(person->flags & _personIsCollegeStudent)) {
      educationEffects = -c(CBclEducationEffect) * (eduVal-0.25);
    }
    heatMapAdd(Education, loc, educationEffects*duration);
  }

  if (!(person->flags & _personTraveling) && person->location != 0) {
    Building* building = getBuilding(person->location);
    vec3 loc = building->location;
    locs.push_back(loc);
    memoriesEcon = building->econ;

    // Potentially make person sick
    if (!(person->flags & _personSick)) {
      float health = heatMapGet(HealthHM, loc);
      if (family->home == 0) health *= 0.5f;
      float sickRate = mix(c(CSickRateMinHealth),
          c(CSickRateMaxHealth), health);
      if (sickRate * duration > randFloat()) {
        person->flags |= _personSick;
        adjustStat(econ, PeopleSick, 1);
      }

    // Potentially transfer sickness to another person
    } else if (c(CSickTransferRate) * duration > randFloat()) {
      item nearbyS = building->peopleInside.size();
      if (nearbyS > 1) {
        item otherNdx = building->peopleInside[randItem(nearbyS)];
        Person* other = getPerson(otherNdx);
        if (!(other->flags & _personSick)) {
          other->flags |= _personSick;
          adjustStat(econ, PeopleSick, 1);
          //if (getSelection() == 0) {
            //setSelection(SelectionPerson, otherNdx);
          //}
        }
      }
    }
  }

  if (person->flags & _personSick) {
    // Potentially heal person
    if (c(CSickHealRate) * duration > randFloat()) {
      person->flags &= ~_personSick;
      adjustStat(econ, PeopleSick, -1);

    } else {
      healthEffects += c(CSickHealthEffect);
      prosperityEffects += c(CSickProsperity);
      communityEffects += c(CCommunitySickEffect);
    }
  }

  float finesAndFeesEffect = 1 + c(CFinesAndFeesCrime)*
    getTaxRate(FinesAndFeesIncome);
  crimeEffects *= finesAndFeesEffect;

  for (int i = 0; i < locs.size(); i++) {
    vec3 loc = locs[i];
    //SPDLOG_INFO("preson {} {} {} {} {} {}", durMult,
        //crimeEffects, prosperityEffects, educationEffects,
        //person->employer, person->flags & _personIsWorker);
    float popFactor = clamp(numPeople(econ)/200000.f, 0.f, 10.f);
    float densityEffect = 1 + popFactor*c(CDensityCrime);
    heatMapAdd(Crime, loc, crimeEffects*duration*densityEffect);
    heatMapAdd(Prosperity, loc, prosperityEffects*durMult);
    heatMapAdd(CommunityHM, loc, communityEffects*duration);
    heatMapAdd(HealthHM, loc, healthEffects*duration);
  }

  // Have some memories
  if (isTourist) {
    float memories = memorableExperiencesForPerson(personNdx, duration);
    haveMemories(memoriesEcon, personNdx, memories);
  }

  if (c(CEnableMoveOuts) && !isTourist) {
    /*
    float unemploymentDiff = unemploymentRate() - targetUnemploymentRate();
    if (numPeople(econ) >= 2000 && randFloat() < duration*.01f &&
        randFloat() * (unemploymentDiff - 0.1) > prosperity*.1f) {
      if (family->home != 0 && randFloat() > .9f) {
        evictFamily(person->family);
      } else {
        removeFamily(person->family);
      }
      return;
    }
    */

    if (family->home != 0 && //getStatistic(NumHomeless) < maxHomeless()*.5f &&
        numPeople(econ) >= 8000 && family->members.size() > 0 &&
        randFloat() < duration) {
      vec3 loc = getBuilding(family->home)->location;
      float value = heatMapGet(Value, loc);
      float density = heatMapGet(Density, loc);
      float education = heatMapGet(Education, loc);
      float desire = value + density + education;
      float socialClass = 0;

      for (int i = 0; i < family->members.size(); i++) {
        item mNdx = family->members[i];
        Person* member = getPerson(mNdx);
        item mEdu = getEducationForPerson(mNdx);
        socialClass += mEdu*0.25;
        socialClass += member->energy*.25f;
        if (member->employer != 0) {
          socialClass ++;
        } else if (member->flags & _personIsWorker) {
          socialClass --;
        }
      }

      socialClass /= family->members.size();
      socialClass -= clamp(timeSinceStore-c(CStoreDays), 0.f, 1.f);
      socialClass -= clamp(timeSinceWork-c(CWorkDays), 0.f, 1.f);
      socialClass += 1;

      float unemploymentDiff = unemploymentRate(family->econ) -
        targetUnemploymentRate(family->econ);
      if (abs(socialClass - desire) + unemploymentDiff > 2) {
        //SPDLOG_INFO("socialClass {} value {} density {} edu {}\ndiff {}",
            //socialClass, value, density, education,
            //socialClass - desire);
        // Force a move to a more appropriate neighborhood, for better or worse
        if (randFloat() < 0.9) {
          evictFamily(person->family);
        } else {
          removeFamily(person->family);
        }
      }
    }
  }
}

static float famSpawnDurToGo = 0;
static float touristSpawnToGo = 0;
static float peopleToGo = 0;
static int currentPerson = 0;

void addFamily(double data) {
  item econNdx = data;
  addFamily(econNdx, false);
}

void addTouristFamily(double data) {
  item econNdx = data;
  addFamily(econNdx, true);
}

void updatePeople(double duration) {
  if (getGameMode() == ModeGame) {
    //Conditionally add new people
    famSpawnDurToGo += duration * sizeEcons();
    const float durStep = 1.0f / c(CFamilySpawnRate);

    for (; famSpawnDurToGo > durStep; famSpawnDurToGo -= durStep) {
      if (sizeEcons() == 0) continue;
      item econNdx = randItem(sizeEcons()) + 1;
      Econ* econ = getEcon(econNdx);
      if (!(econ->flags & _econExists)) continue;

      if (boardSize(econNdx, Homes) < 1 &&
        !allowNewCollegeStudents(econNdx)) continue;

      item pop = numPeople(econNdx);
      if (pop > 1000) {
        float unemploymentDiff = unemploymentRate(econNdx) -
          targetUnemploymentRate(econNdx);
        float score = -unemploymentDiff / c(CMaxUnemploymentDiff);
        score += getStatistic(econNdx, CommunityStat);
        score += getStatistic(econNdx, HealthStat);
        score += getStatistic(econNdx, ProsperityStat);
        score -= getStatistic(econNdx, CrimeStat);
        float rnd = randFloat();
        if (score < rnd*rnd) continue;
        //if (unemploymentDiff > rnd*rnd * c(CMaxUnemploymentDiff)) continue;
      }

      queueTask_g(TaskAddFamily, econNdx);
    }

    touristSpawnToGo += duration / gameDayInRealSeconds *
      getEffectMultiplier(Tourism) / oneYear;
    // Enforce a reasonable number of tourists
    float expectedTourists = getEffectMultiplier(Tourism) *
      getTouristTypicalStay(ourCityEconNdx()) / oneYear;
    float touristDiff = expectedTourists -
      getStatistic(ourCityEconNdx(), NumTouristsNow);
    if (touristDiff < touristSpawnToGo) touristSpawnToGo = touristDiff;
    if (touristSpawnToGo < 0) touristSpawnToGo = 0;

    //SPDLOG_INFO("touristSpawnToGo {}, +{} for {}",
        //touristSpawnToGo,
        //duration/gameDayInRealSeconds * getEffectMultiplier(Tourism) / oneYear,
        //tprintDurationString(duration/gameDayInRealSeconds));

    //Add tourists
    while (touristSpawnToGo >= 1 &&
      getStatistic(ourCityEconNdx(), NumEmptyHotelRooms) > 0) {
      queueTask_g(TaskAddTouristFamily, ourCityEconNdx());
      touristSpawnToGo--;
    }
  }

  //float peoplePerHour = numPeople()/60.f;
  //SPDLOG_INFO("wakeTimes:{} peopleToGo:{} numPeople():{}, currentPerson: {}",
      //wakeTimes.size(), peopleToGo, numPeople(ourCityEconNdx()),
      //currentPerson);
  if (wakeTimes.size() > 0) {
    float time = getCurrentDateTime();
    peopleToGo += duration/gameDayInRealSeconds*sizePeople()/oneHour;

    if (sizePeople() > 10000) {
      while (peopleToGo > 1000) {
        queueTask_g(TaskUpdate1000People, duration);
        peopleToGo -= 1000;
      }

    } else {
      for (int i = 0; i < sizePeople() && peopleToGo >= 1; i++) {
        currentPerson = currentPerson%(wakeTimes.size()-1) + 1;
        if (time > wakeTimes[currentPerson]) {
          peopleToGo--;
          updatePerson(currentPerson);
        }
      }
    }
  }
}

void update1000People(double duration) {
  float time = getCurrentDateTime();
  float num = 1000;
  if (num > sizePeople()) num = sizePeople();
  for (;num > 0; num--) {
    currentPerson = currentPerson%(wakeTimes.size()-1) + 1;
    if (time > wakeTimes[currentPerson]) {
      updatePerson(currentPerson);
    }
  }
}

void finishPersonRoute(item personNdx, Route* route) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personWaitingForRoute)) {
    clearRoute(route);
    return;
  }
  person->flags &= ~_personWaitingForRoute;
  item home = getFamily(person->family)->home;
  bool isTourist = person->flags & _personIsTourist;

  if (route->steps.size() == 0) {
    invalidateRouteCache(route->source, route->destination);

    if (person->location != 0) {
      heatMapAdd(Prosperity, getBuilding(person->location)->location,
        c(CRouteFailedProsperity));
    }

    if (person->activityBuilding != 0) {
      heatMapAdd(Prosperity, getBuilding(person->activityBuilding)->location,
        c(CRouteFailedProsperity));
    }

    if (person->activityBuilding == home) {
      putPersonInBuilding(personNdx, home);
    } else {
      setDeciding(personNdx);
    }

    return;
  }

  if (person->flags & _personTraveling) {
    clearRoute(route);
    return;

  } else if (person->location == 0 || isPresimulating()) {
    teleport(personNdx, person->activityBuilding);
    clearRoute(route);
    return;

  } else if (person->activity == FreightActivity) {
    if (countVehicles() > c(CMaxVehicles)) {
      clearRoute(route);
      return;
    } else {
      startFreightTrip_g(route, personNdx);
      return;
    }
  }

  bool isTransit = false;
  for (int i = 0; i < route->steps.size(); i++) {
    if (locationType(route->steps[i]) == LocTransitStop) {
      isTransit = true;
      break;
    }
  }

  RouteInfo info = computeRouteInfo_g(&route->steps, isTransit, false);
  if (!isTourist &&
      person->activityBuilding != getFamily(person->family)->home) {
    float timeEst = info.time;
    if (timeEst > c(CMaxCommute)*oneHour) {
      //SPDLOG_WARN("timeEst {} - {}",
          //printDurationString(timeEst),
          //routeString(route));
      if (person->activity == WorkActivity ||
          person->activity == InterviewActivity) {
        //SPDLOG_INFO("person quits job due to long estimated commute");
        quitJob(personNdx);
        if (randFloat() < c(CRejobRate)) {
          findAnyJob(personNdx);
        }
      }
      setDeciding(personNdx);
      adjustStat(getPersonEcon(personNdx), FailedTimeTests, 1);
      knownRouteErase_g(route->source, route->destination);
      clearRoute(route);
      return;
    }
  }

  // We've decided to take the trip
  knownRouteAdd_g(route->source, route->destination);

  if (isTourist && isTransit) {
    haveMemories(ourCityEconNdx(), personNdx, c(CTransitMemories));
  }

  int maxVehicles = c(CMaxVehicles);
  if ((isMonitored(SelectionPerson, personNdx) ||
      (randFloat() < getEffectiveTrafficRate() &&
       (maxVehicles > countVehicles() || isTransit))) &&
      !(person->flags & _personTraveling)) {
    startTrip_g(route, personNdx);
  } else {
    teleport(personNdx, person->activityBuilding);
    clearRoute(route);
  }
}

char* getPersonDescriptor(item ndx) {
  Person* p = getPerson(ndx);
  Family* fam = getFamily(p->family);
  char* activity = getActivityName(ndx);
  const char* comment = "";
  if (fam->home == 0) {
    comment = " (Homeless)";
  } else if ((p->flags & _personIsWorker) && p->employer == 0) {
    comment = " (Jobless)";
  } else if (getCurrentDateTime() - fam->lastStoreTime > 4) {
    comment = " (Hungry)";
  } else if (p->flags & _personSick) {
    comment = " (Sick)";
  }

  int age = (getCurrentDateTime() - p->birthday)/oneYear;
  char* result = sprintf_o("%s %s, %d - %s%s",
      p->name, fam->name, age, activity, comment);
  free(activity);
  return result;
}

IssueIcon getFamilyIssue(item ndx) {
  Family* fam = getFamily(ndx);
  bool isTourists = fam->flags & _familyIsTourists;
  float time = getCurrentDateTime();
  if (fam->home == 0) {
    return Homeless;
  } else if (time - fam->lastStoreTime > c(CStoreIssueDays)) {
    return Hungry;
  }

  IssueIcon result = NoIssue;
  bool lacksWork = !isTourists &&
    time - fam->lastWorkTime > c(CWorkIssueDays);
  //if (lacksWork) return Jobless;
  for (int i = 0; i < fam->members.size(); i++) {
    Person* p = getPerson(fam->members[i]);
    if (lacksWork && (p->flags & _personIsWorker) && p->employer == 0) {
      return Jobless;
    } else if (p->flags & _personSick) {
      result = SickIssue;
    }
  }

  return result;
}

char* getFamilyDescriptor(item ndx) {
  Family* fam = getFamily(ndx);

  IssueIcon ico = getFamilyIssue(ndx);
  const char* comment = "";
  if (ico == Homeless) {
    comment = " (Homeless)";
  } else if (ico == Hungry) {
    comment = " (Hungry)";
  } else if (ico == Jobless) {
    comment = " (Jobless)";
  } else if (ico == SickIssue) {
    comment = " (Sick)";
  }

  const char* title = (fam->flags & _familyIsTourists) ?
    "Family" : "Household";

  return sprintf_o("%s %s%s", fam->name, title, comment);
}

item numPeople(item econ) {
  return getStatistic(econ, Population);
}

item sizePeople() {
  return people->size();
}

item numFamilies(item econ) {
  return families->count();
}

float unemploymentRate(item econ, EducationLevel edu) {
  int numWorkers = getStatistic(econ, (Statistic)(NumNoEduWorkers+edu));
  if (numWorkers == 0) {
    return 0;
  } else {
    int numUnemp = getStatistic(econ, (Statistic)(NumNoEduUnemployed+edu));
    return float(numUnemp) / numWorkers;
  }
}

float unemploymentRate(item econ) {
  int numWorkers = getStatistic(econ, NumWorkers);
  if (numWorkers == 0) {
    return 0;
  } else {
    return getStatistic(econ, NumUnemployed) / numWorkers;
  }
}

float getTouristTypicalStay(item econ) {
  return mix(c(CTouristStay0), c(CTouristStay5),
      getStatistic(econ, TouristRating)/5.f);
}

void resetPeople() {
  for (int i = 1; i <= people->size(); i++) {
    Person* p = getPerson(i);
    //if (p->name) free(p->name);
  }
  for (int i = 1; i <= families->size(); i++) {
    Family* f = getFamily(i);
    //if (f->name) free(f->name);
    vector<item> swap;
    f->members.swap(swap);
  }
  people->clear();
  families->clear();
  wakeTimes.clear();
  evaluateTime.clear();
}

void rebuildPopulationStats() {
  resetStat(Population);
  resetStat(NumFamilies);
  resetStat(PeopleSick);
  resetStat(NumHomeless);
  resetStat(NumWorkers);
  resetStat(NumUnemployed);
  resetStat(NumEmployed);
  resetStat(NumCollegeStudents);
  resetStat(NumTouristsNow);

  for (int i = 0; i < numEducationLevels; i++) {
    resetStat((Statistic)(NumNoEdu+i));
    resetStat((Statistic)(NumNoEduWorkers+i));
    resetStat((Statistic)(NumNoEduUnemployed+i));
  }

  for (int i = 1; i <= families->size(); i++) {
    Family* f = getFamily(i);
    item econ = getFamilyEcon(i);

    if (!(f->flags & _familyIsTourists)) {
      adjustStat(econ, NumFamilies, 1);
      if ((f->flags & _familyExists) && f->home == 0) {
        adjustStat(econ, NumHomeless, f->members.size());
      }
    }

    if (f->flags & _familyIsDormRoom) {
      bool allCollege = true;
      for (int j = 0; j < f->members.size(); j++) {
        Person* p = getPerson(f->members[j]);
        if (!(p->flags & _personIsCollegeStudent)) {
          allCollege = false;
          break;
        }
      }

      if (!allCollege) {
        f->flags &= ~_familyIsDormRoom;
      }
    }
  }

  for (int i = 1; i <= people->size(); i++) {
    if (getPerson(i)->flags & _personExists) {
      adjustPersonStats(i, 1);
    }
  }
}

void writePerson(FileBuffer* file, item personNdx) {
  Person* person = getPerson(personNdx);

  fwrite_int(file, person->flags);
  fwrite_item(file, person->location);
  fwrite_item(file, person->family);
  fwrite_item(file, person->employer);
  fwrite_item(file, person->activity);
  fwrite_item(file, person->activityTarget);
  fwrite_item(file, person->activityBuilding);
  fwrite_float(file, person->energy);
  writeVehicleDescription(file, person->vehicleDescription);
  fwrite_float(file, person->enterTime);
  fwrite_float(file, wakeTimes[personNdx]);
  fwrite_float(file, person->sleepTime);
  fwrite_float(file, person->birthday);
  fwrite_string(file, person->name);
}

void readPerson(FileBuffer* file, int version, item personNdx) {
  Person* person = getPerson(personNdx);

  person->flags = fread_int(file);
  person->location = fread_item(file, version);
  person->family = fread_item(file, version);
  person->employer = fread_item(file, version);
  if (version < 40) {
    item role = fread_item(file, version);
    if (role == 1) {
      person->flags |= _personIsFemale;
    } else if (role == 2) {
      person->flags |= _personIsChild;
      if (randFloat() < .5f) {
        person->flags |= _personIsFemale;
      }
    }
  }
  person->activity = fread_item(file, version);
  person->lastActivity = person->activity;
  if (version < 45) {
    person->activityTarget = 0;
  } else {
    person->activityTarget = fread_item(file, version);
  }
  person->activityBuilding = fread_item(file, version);
  if (version < 45) {
    person->energy = randFloat(0.5, 2.0);
  } else {
    person->energy = fread_float(file);
  }
  person->vehicleDescription = readVehicleDescription(file, version);
  person->enterTime = fread_float(file);
  float time = getCurrentDateTime();
  if (version < 45) {
    fread_float(file); //old wake time
    setWakeTime(personNdx, 0);

    person->sleepTime = time;
    person->birthday = (person->flags & _personIsChild) ?
      time - 4*randFloat(0,16) : randomAge();

  } else {
    float wakeTime = fread_float(file);
    float maxWakeTime = time + c(CPersonUpdateTime)*oneHour;
    wakeTime = clamp(wakeTime, time, maxWakeTime);
    setWakeTime(personNdx, wakeTime);
    person->sleepTime = fread_float(file);
    person->birthday = fread_float(file);
  }
  person->name = intern(fread_string(file));
  person->flags &= ~_personWaitingForRoute;
}

void writeFamily(FileBuffer* file, item familyNdx) {
  Family* family = getFamily(familyNdx);

  fwrite_int(file, family->flags);
  fwrite_item(file, family->home);
  fwrite_float(file, family->lastStoreTime);
  fwrite_float(file, family->lastWorkTime);
  fwrite_item(file, family->econ);
  fwrite_string(file, family->name);
  fwrite_item_vector(file, &family->members);
}

void readFamily(FileBuffer* file, int version, item familyNdx) {
  Family* family = getFamily(familyNdx);

  family->flags = fread_int(file);
  family->home = fread_item(file, version);

  if (version < 43) {
    family->lastStoreTime = getCurrentDateTime();
  } else {
    family->lastStoreTime = fread_float(file);
  }

  if (version < 45) {
    family->lastWorkTime = getCurrentDateTime();
  } else {
    family->lastWorkTime = fread_float(file);
  }

  if (version >= 52) {
    family->econ = fread_item(file, version);
  } else if (family->home != 0) {
    family->econ = getBuilding(family->home)->econ;
  } else {
    family->econ = ourCityEconNdx();
  }

  family->name = intern(fread_string(file));
  fread_item_vector(file, &family->members, version);
}

void writePeople(FileBuffer* file) {
  people->write(file);
  for (int i=1; i <= people->size(); i++) {
    writePerson(file, i);
  }

  families->write(file);
  for (int i=1; i <= families->size(); i++) {
    writeFamily(file, i);
  }
}

void readPeople(FileBuffer* file, int version) {
  if (version < 52) {
    fread_item(file, version); // unemployed
    fread_item(file, version); // employed
    if (version >= 8) {
      fread_item(file, version); // homeless
    }
  }

  people->read(file, version);
  wakeTimes.ensureSize(people->size());
  for (int i=1; i <= people->size(); i++) {
    readPerson(file, version, i);
  }

  families->read(file, version);
  for (int i=1; i <= families->size(); i++) {
    readFamily(file, version, i);
  }
}

