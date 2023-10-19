
float dayProximity(float to) {
  float time = 3 + to - getCurrentTime() - c(CMaxCommute);
  time = time - (int)time;
  if (1 - time < time) {
    time = 1 - time;
  }
  //SPDLOG_INFO("dayProximity {} - {} = {}",
    //(int)round(to*24), (int)round(getCurrentTime()*24), (int)round(time*24));
  return 1-time*2;
}

float dayProximity(FloatConstant fa, FloatConstant fb, FloatConstant fc,
    float energy) {
  return dayProximity((c(fa) + c(fb)/energy) * oneHour) * c(fc) * energy;
}

float activityInertia(Person* person, float timeTarget) {
  float time = getCurrentDateTime();
  float timeInActivity = time - person->enterTime;
  return timeTarget - timeInActivity;
}

float activityInertia(Person* person, FloatConstant fa,
    FloatConstant fb, FloatConstant fc, float energy) {
  return c(fa)*oneHour + c(fb)*activityInertia(person, c(fc)*oneHour*energy);
}

void setActivity(item ndx, item activity, item activityBuilding,
    item activityTarget) {
  Person* person = getPerson(ndx);
  if (!(person->flags & _personExists)) return;
  if (person->flags & _personTraveling) return;
  if (person->flags & _personWaitingForRoute) return;

  person->activity = activity;
  person->activityBuilding = activityBuilding;
  person->activityTarget = activityTarget;

  if (person->activity == InterviewActivity) {
    employeeHired(activityTarget, ndx);
  }
}

void setDeciding(item ndx) {
  setActivity(ndx, DecisionActivity, 0, 0);
  wakePerson(ndx);
}

float scoreDesicion(item ndx) {
  return c(CMinActScore);
}

float scoreSleep(item ndx) {
  Person* person = getPerson(ndx);
  float energy = 1/person->energy;
  float score = c(CSleepBaseScore);
  score += dayProximity(CSleepDayProxA,
      CSleepDayProxB, CSleepDayProxC, energy);
  score += c(CSleepLightFactor)*getLightLevel();
  if (person->location == getFamily(person->family)->home
      && person->activity == SleepActivity) {
    score += activityInertia(person, CSleepInertiaA,
        CSleepInertiaB, CSleepInertiaC, energy);
  }
  return score;
}

float scoreHome(item ndx) {
  Person* person = getPerson(ndx);
  Family* f = getFamily(person->family);
  item home = f->home;
  if (home == 0) return c(CMinActScore);
  bool isTourist = person->flags & _personIsTourist;

  float energy = 1/person->energy;
  float score = c(CHomeBaseScore);
  score += dayProximity(CHomeDayProxA,
      CHomeDayProxB, CHomeDayProxC, energy);
  if (!isTourist) {
    float timeSinceWork = getCurrentDateTime() - f->lastWorkTime;
    if (timeSinceWork > c(CWorkDays)) {
      score += c(CHomeUnemployed);
    }
  }
  score += getLightLevel() * c(CHomeLightFactor);
  if (person->location == home) {
    score += activityInertia(person, CHomeInertiaA,
        CHomeInertiaB, CHomeInertiaC, energy);
  }

  return score;
}

float scoreFriend(item ndx) {
  Person* person = getPerson(ndx);
  float energy = person->energy;
  float score = c(CFriendBaseScore);
  score += dayProximity(CFriendDayProxA,
      CFriendDayProxB, CFriendDayProxC, energy);
  score += getLightLevel() * c(CFriendLightFactor);
  score += c(CFriendEnergy) * energy;
  if (getFamily(person->family)->home == 0) score += c(CFriendHomeless);
  if (person->activity == FriendsActivity) {
    score += activityInertia(person, CFriendInertiaA,
        CFriendInertiaB, CFriendInertiaC, energy);
  }
  return score;
}

float scoreWork(item ndx) {
  Person* person = getPerson(ndx);
  if (person->employer == 0) return c(CMinActScore);
  Business* b = getBusiness(person->employer);
  if (!(b->flags & _businessExists)) return c(CMinActScore);
  Family* f = getFamily(person->family);

  float energy = person->energy;
  float score = c(CWorkBaseScore);
  score += dayProximity(CWorkDayProxA,
      CWorkDayProxB, CWorkDayProxC, energy);
  score += c(CWorkLightFactor)*getLightLevel();
  float timeSinceWork = getCurrentDateTime() - f->lastWorkTime;
  score += timeSinceWork * c(CWorkTimeSince);
  if (b->flags & _businessComeToWork) {
    score += c(CWorkOpenFactor);
  }
  if (person->activity == WorkActivity) {
    score += activityInertia(person, CWorkInertiaA,
        CWorkInertiaB, CWorkInertiaC, energy);
  }
  return score;
}

float scoreInterview(item ndx) {
  Person* person = getPerson(ndx);
  if (!(person->flags & _personIsWorker)) return c(CMinActScore);
  if (person->employer != 0) return c(CMinActScore);
  Family* f = getFamily(person->family);
  if (person->location != f->home && f->home != 0) return c(CMinActScore);

  float energy = person->energy;
  float score = c(CInterviewBaseScore);
  score += dayProximity(CInterviewDayProxA,
      CInterviewDayProxB, CInterviewDayProxC, energy);
  score += c(CInterviewLightFactor)*getLightLevel();
  float timeSinceWork = getCurrentDateTime() - f->lastWorkTime;
  score += timeSinceWork * c(CInterviewTimeSince);
  score += getEducationForPerson(ndx) * c(CInterviewEducation);
  if (person->activity == InterviewActivity) {
    score += activityInertia(person, CInterviewInertiaA,
        CInterviewInertiaB, CInterviewInertiaC, energy);
  } else if (isPresimulating()) score += 4;
  return score;
}

float scoreStore(item ndx) {
  Person* person = getPerson(ndx);
  Family* f = getFamily(person->family);
  item econ = getFamilyEcon(person->family);
  if (getStatistic(econ, NumRetailOpen) == 0) return c(CMinActScore);

  float energy = person->energy;
  float score = c(CStoreBaseScore);
  if (person->flags & _personIsTourist) score = c(CStoreBaseScoreTourist);
  score += dayProximity(CStoreDayProxA,
      CStoreDayProxB, CStoreDayProxC, energy);
  score += c(CStoreLightFactor)*getLightLevel();
  float prosperity = getStatistic(econ, EconomicDeterminant);
  score += c(CStoreProsperityFactor)*prosperity;
  float timeSinceStore = getCurrentDateTime() - f->lastStoreTime;
  score += timeSinceStore * c(CStoreTimeSince);
  if (person->activity == StoreActivity) {
    score += activityInertia(person, CStoreInertiaA,
        CStoreInertiaB, CStoreInertiaC, energy);
  }
  return score;
}

float scoreFreight(item ndx) {
  Person* person = getPerson(ndx);
  if (person->activity != WorkActivity) return c(CMinActScore);
  if (person->employer == 0) return c(CMinActScore);
  Business* b = getBusiness(person->employer);
  BusinessType type = getBusinessType(person->employer);
  if (type != Farm && type != Factory) return c(CMinActScore);
  if (!(b->flags & _businessExists)) return c(CMinActScore);

  /*
  for (int i=0; i < b->positions.size(); i++) {
    item eNdx = b->positions[i].employee;
    if (eNdx == ndx || eNdx == 0) continue;
    Person* e = getPerson(eNdx);
    if (e->activity == FreightActivity) return c(CMinActScore);
  }
  */

  float energy = person->energy;
  float score = c(CFreightBaseScore);
  float timeSinceFreightOut = getCurrentDateTime() - b->lastCustomerTime;
  score += energy * c(CFreightA) *
    clamp(timeSinceFreightOut-c(CFreightB), -c(CFreightB), c(CFreightC));
  score += dayProximity(CFreightDayProxA,
      CFreightDayProxB, CFreightDayProxC, energy);
  return score;
}

float scoreGovernment(item ndx) {
  item econ = getPersonEcon(ndx);
  int num = getStatistic(econ, NumGovBuildingsOpen);
  if (num <= 0) return c(CMinActScore);
  int pop = numPeople(econ);
  Person* person = getPerson(ndx);
  float energy = person->energy;
  float score = c(CAmenityBaseScore);
  if (person->flags & _personIsTourist) score = c(CAmenityBaseScoreTourist);
  //float openScore = clamp(num*c(CAmenityOpenPerPop)/pop,
      //0.f, c(CAmenityOpenMax));
  //float score = c(CAmenityBaseScore) + openScore;
  //if (getFamily(person->family)->home == 0) score += c(CAmenityHomeless);
  score += dayProximity(CAmenityDayProxA,
      CAmenityDayProxB, CAmenityDayProxC, energy);

  if (person->flags & _personIsCollegeStudent) {
    score += c(CAmenityStudentScore);
  }

  if (person->activity == GovernmentActivity &&
      person->activityBuilding != 0) {
    Building* b = getBuilding(person->activityBuilding);
    Design* d = getDesign(b->design);
    float typicalTime = d->category == ParksCategory ? c(CParkTime)
      : d->category == EducationCategory ? c(CEducationTime)
      : d->category == UniversityCategory ? c(CUniversityTime)
      : c(CServicesTime); // Community Services
    score += c(CAmenityInertiaA)*oneHour +
      c(CAmenityInertiaB)*activityInertia(person, typicalTime*oneHour*energy);
  }

  return score;
}

float scoreDoctor(item ndx) {
  item econ = getPersonEcon(ndx);
  Person* person = getPerson(ndx);
  float energy = person->energy;
  float score = c(CDoctorBaseScore);
  score += dayProximity(CDoctorDayProxA,
      CDoctorDayProxB, CDoctorDayProxC, energy);

  if (person->flags & _personSick) {
    score += c(CDoctorSick);
  }

  if (person->activity == DoctorActivity &&
      person->activityBuilding != 0) {
    score += c(CDoctorInertiaA)*oneHour +
      c(CDoctorInertiaB)*activityInertia(person,
          c(CDoctorInertiaC)*oneHour*energy);
  }

  return score;
}

void assignDesicion(item ndx) {
  setDeciding(ndx);
}

void assignSleep(item ndx) {
  Person* person = getPerson(ndx);
  item home = getFamily(person->family)->home;
  if (home == 0) home = person->location;
  setActivity(ndx, SleepActivity, home, person->family);
}

void assignHome(item ndx) {
  Person* person = getPerson(ndx);
  item home = getFamily(person->family)->home;
  if (home == 0) {
    setDeciding(ndx);
  } else {
    setActivity(ndx, HomeActivity, home, person->family);
  }
}

void assignWork(item ndx) {
  Person* person = getPerson(ndx);
  if (person->employer != 0) {
    Business* b = getBusiness(person->employer);
    if (b->flags & _businessExists) {
      setActivity(ndx, WorkActivity, b->building, person->employer);
    } else {
      setDeciding(ndx);
    }
  } else {
    setDeciding(ndx);
  }
}

void assignInterview(item ndx) {
  Person* person = getPerson(ndx);
  Family* fam = getFamily(person->family);
  EducationLevel edu = getEducationForPerson(ndx);
  while (edu > 0 && randFloat() < c(CUnderemploymentRate)) {
    edu = (EducationLevel)(edu-1);
  }

  // See if there are any jobs at home
  if (fam->home != 0) {
    Building* b = getBuilding(fam->home);
    for (int i = 0; i < b->businesses.size(); i++) {
      item businessNdx = b->businesses[i];
      Business* biz = getBusiness(businessNdx);
      for (int j = 0; j < biz->positions.size(); j++) {
        Position p = biz->positions[j];
        if (p.employee == 0 && p.minEducation <= edu) {
          setActivity(ndx, InterviewActivity, fam->home, businessNdx);
          return;
        }
      }
    }
  }

  item businessNdx = getRandomPosition(edu);
  if (businessNdx == 0) {
    setDeciding(ndx);
    return;
  }

  Business* b = getBusiness(businessNdx);
  if (!(b->flags & _businessExists)) {
    setDeciding(ndx);
    return;
  }

  setActivity(ndx, InterviewActivity, b->building, businessNdx);
}

void assignStore(item ndx) {
  Person* person = getPerson(ndx);
  item retail = 0;
  if (person->activity == StoreActivity && person->activityTarget != 0) {
    retail = person->activityTarget;
  } else {
    retail = getRandomRetail();
  }

  if (retail == 0) {
    setDeciding(ndx);
  } else {
    Business* business = getBusiness(retail);
    if (business->flags & _businessExists &&
      getBusinessType(retail) == Retail &&
      business->flags & _businessOpen
      //getCurrentDateTime() - business->lastCustomerTime > 0.1
    ) {
      setActivity(ndx, StoreActivity, business->building, retail);
    } else {
      setDeciding(ndx);
    }
  }
}

void assignFriend(item ndx) {
  Person* person = getPerson(ndx);
  item famNdx;
  if (person->activity == FriendsActivity && person->activityTarget != 0) {
    famNdx = person->activityTarget;
  } else {
    famNdx = getRandomFamily();
  }

  if (famNdx != 0) {
    Family* fam = getFamily(famNdx);
    if (!(fam->flags & _familyExists) || fam->home == 0) {
      setDeciding(ndx);
      return;
    }

    Building* b = getBuilding(fam->home);
    int designCapacity = b->families.size()*2;
    if (designCapacity > 200) designCapacity = 200;
    if (b->peopleInside.size() > designCapacity) {
      setDeciding(ndx);
      return;
    }

    for (int i = 0; i < fam->members.size(); i++) {
      Person* m = getPerson(fam->members[i]);
      if (!(m->flags & _personTraveling) && m->location == fam->home &&
          (m->activity == HomeActivity || m->activity == SleepActivity)) {
        setActivity(ndx, FriendsActivity, fam->home, famNdx);
        return;
      }
    }
  }

  setDeciding(ndx);
}

void assignFreight(item ndx) {
  Person* person = getPerson(ndx);
  if (person->activity != WorkActivity || person->employer == 0 ||
      person->location == 0) {
    setDeciding(ndx);
    return;
  }

  Business* b = getBusiness(person->employer);
  if (!(b->flags & _businessExists) || b->building != person->location) {
    setDeciding(ndx);
    return;
  }

  item businessNdx = getRandomBusiness();
  if (businessNdx != 0) {
    Business* business = getBusiness(businessNdx);
    float time = getCurrentDateTime();
    if ((business->flags & _businessExists) &&
        time - business->lastFreightTime > c(CFreightInDays)-1) {
      setActivity(ndx, FreightActivity, business->building, businessNdx);
      return;
    }
  }

  setActivity(ndx, WorkActivity, person->location, person->employer);
}

void assignGovernment(item ndx) {
  item service = getRandomGovernmentBuilding();
  if (service == 0) {
    setDeciding(ndx);
  } else {
    Building* b = getBuilding(service);
    Design* d = getDesign(b->design);
    int designCapacity = c(CAmenityCapacity) *
      d->cost * c(CAmenityCostMult) / 1000000;
    if (b->peopleInside.size() > designCapacity) {
      setDeciding(ndx);
      return;
    }

    Person* person = getPerson(ndx);
    bool isTourist = person->flags & _personIsTourist;
    if (isTourist && d->effect[Tourism] <= 0 && d->effect[Prestige] <= 0) {
      setDeciding(ndx);
      return;
    }

    item biz = 0;
    if (b->businesses.size() > 0) {
      biz = b->businesses[0];
      if (!(getBusiness(biz)->flags & _businessOpen)) {
        setDeciding(ndx);
        return;
      }
    }
    setActivity(ndx, GovernmentActivity, service, biz);
  }
}

void assignDoctor(item ndx) {
  item service = getRandomHealthcare();
  if (service == 0) {
    setDeciding(ndx);
  } else {
    Building* b = getBuilding(service);
    if (randFloat() < 0.75) {
      Design* d = getDesign(b->design);
      int designCapacity = c(CAmenityCapacity) *
        d->cost * c(CAmenityCostMult) / 100000;
      if (b->peopleInside.size() > designCapacity) {
        setDeciding(ndx);
        return;
      }
    }

    item biz = 0;
    if (b->businesses.size() > 0) {
      biz = b->businesses[0];
      if (!(getBusiness(biz)->flags & _businessOpen)) {
        setDeciding(ndx);
        return;
      }
    }
    setActivity(ndx, DoctorActivity, service, biz);
  }
}

float scoreTouristReturn(item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personIsTourist)) return c(CMinActScore);
  Family* f = getFamily(person->family);
  float time = getCurrentDateTime();
  float stayTime = time - f->lastWorkTime;
  stayTime += 0.5f; // fudge factor
  if (stayTime < getTouristTypicalStay(ourCityEconNdx())) {
    return c(CMinActScore);
  }

  // Check to see if they're already outside the city
  if (!(person->flags & _personTraveling) && person->location != 0) {
    Building* b = getBuilding(person->location);
    if (b->econ != ourCityEconNdx()) {
      return 100; // Stay here
    }
  }

  float score = 100;
  score *= getLightLevel();
  return score;
}

void assignTouristReturn(item personNdx) {
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
    setActivity(personNdx, TouristReturnActivity, buildingNdx, 0);
  } else {
    setDeciding(personNdx);
  }
}

typedef float (*ActivityScore)(item personNdx);
ActivityScore scoreActivity[] = {
  scoreDesicion, scoreSleep, scoreHome, scoreWork, scoreFriend,
  scoreStore, scoreFreight, scoreGovernment, scoreInterview, scoreDoctor,
  scoreTouristReturn,
};

typedef void (*ActivityAssign)(item personNdx);
ActivityAssign assignActivity[] = {
  assignDesicion, assignSleep, assignHome, assignWork, assignFriend,
  assignStore, assignFreight, assignGovernment, assignInterview, assignDoctor,
  assignTouristReturn,
};

