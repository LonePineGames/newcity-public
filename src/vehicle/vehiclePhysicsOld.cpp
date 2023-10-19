
//Vehicle Physics Constants
const float staticFriction = 0.85; // Rubber on dry asphalt
const float massBase = 1143; //kilograms
const float g = 9.8;
const float enginePowerBase = 104000; //kilowatts
const float coefficientOfRollingResistance = 0.015;
const float pilotDistance = 5; //meters
const float turningRadius = 5.48; //meters
const float timeBetweenCars = 1.75; // seconds
const float minSpeed = 0.01;
const float bumperDistance = 2;
const float trailerDistance = 0.5;
const float routeMergeDistance = 60;
const float routeMergeFuzz = 200;
const float mergeDistance = 10;
const float mergeSafetyDistance = 40;
const float maxMergeSpeedDiff = 1;
const float mergeProbability = 0.01;
const float routeMergeProbability = 0.01;
const float maxVehicleLife = oneHour * 24;
const float vehiclePollution = 2.0/enginePowerBase;
const bool simpleVehiclePhysics = false;
const float simplifyDuration = 0.1;

const static float massMult[] = {
  //Sedan, SUV, Pickup, Sport, Hatchback, BoxTruck, SemiTractor, SemiTrailer,
  1.0,     2.0, 2.0,    0.8,   1.5,       3.0,      5.0,         4.0
};

const static float powerMult[] = {
  //Sedan, SUV, Pickup, Sport, Hatchback, BoxTruck, SemiTractor, SemiTrailer,
  1.0,     1.5, 1.5,    1.5,   1.0,       2.0,      2.0,         0.0
};

const float vehicleLengthForStyle[] = {
  4, 5, 5, 4, 4, 3, 3, 8
};

void enterLaneInitial(item vehicleNdx) {
  Vehicle* vehicle = getVehicle(vehicleNdx);
  float currentTime = getCurrentDateTime();
  vehicle->laneBlockEnterTime = getCurrentDateTime();
  vehicle->flags &= ~_vehicleIsMerging;

  addVehicleToLane(vehicleNdx, vehicle->pilot.lane);
}

void enterLane(Vehicle* vehicle, item vehicleNdx, item laneNdx) {
  Lane* pilotLane = getLane(vehicle->pilot.lane);
  float maxAdvance = pilotLane->length - vehicle->pilot.dap;
  float speed = length(vehicle->velocity);
  bool fast = (speed*speed / (2*staticFriction*g)) > maxAdvance;
  if (spaceInLane(laneNdx, fast) > vehicleLength(vehicle)) {
    float currentTime = getCurrentDateTime();
    recordLaneBlockTime(vehicle->laneLoc.lane,
        currentTime - vehicle->laneBlockEnterTime);
    vehicle->laneBlockEnterTime = currentTime;

    if (vehicle->pilot.lane != vehicle->laneLoc.lane) {
      removeVehicleFromLane(vehicleNdx, vehicle->pilot.lane);
    }
    vehicle->pilot.lane = laneNdx;
    vehicle->pilot.dap = 0;
    vehicle->flags &= ~_vehicleIsMerging;
    addVehicleToLane(vehicleNdx, vehicle->pilot.lane);

  } else if (length(vehicle->velocity) < 0.25) {
    requestBlock(laneNdx);
  }
}

void enterLaneByMerge(item vehicleNdx, item laneNdx) {
  Vehicle* vehicle = getVehicle(vehicleNdx);
  if (vehicle->pilot.lane != vehicle->laneLoc.lane) {
    removeVehicleFromLane(vehicleNdx, vehicle->pilot.lane);
  }
  vehicle->pilot.lane = laneNdx;
  vehicle->flags |= _vehicleIsMerging;
  vehicle->distanceSinceMerge = 0;
  addVehicleToLane(vehicleNdx, vehicle->pilot.lane);
}

item getNextLane(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  GraphLocation pilot = vehicle->pilot;
  Lane* pilotLane = getLane(pilot);

  if (vehicle->route.size() == 0 || pilotLane->drains.size() == 0) return 0;

  item nextBlockNdx = vehicle->route.back();
  for (int i = 0; i < pilotLane->drains.size(); i++) {
    item nextLane = pilotLane->drains[i];
    if (getLaneBlockIndex(nextLane) == nextBlockNdx) {
      return nextLane;
    }
  }

  return pilotLane->drains[0];

  /* Random Walk
    int numDrains = pilotLane->drains.size();
    if (numDrains == 0) {
      return 0;
    } else {
      int drainNum;
      LaneBlock* nextBlock;
      int iter = 0;
      do {
        if (iter > numDrains*2) {
          return 0;
        }
        drainNum = randItem(numDrains);
        nextBlock = getLaneBlock(pilotLane->drains[drainNum]);
        iter ++;
      } while(!(nextBlock->flags & _laneOpen));
      return pilotLane->drains[drainNum];
    }
    */
}

float considerMerge(item vehicleNdx, float duration) {
  Vehicle* vehicle = getVehicle(vehicleNdx);
  if (vehicle->flags & _vehicleIsMerging) {
    return -1;
  }

  if (vehicle->vehicleAhead &&
    getVehicle(vehicle->vehicleAhead)->flags & _vehicleIsMerging) {
    return -1;
  }

  item pilotLaneNdx = vehicle->pilot.lane;
  item pilotBlockNdx = getLaneBlockIndex(pilotLaneNdx);
  LaneBlock* block = getLaneBlock(pilotBlockNdx);
  if (block->graphElements[1] < 0) {
    return -1;
  }

  item pilotLaneNBlock = laneIndexInBlock(pilotLaneNdx);
  item nextBlockNdx = vehicle->route.size() > 0 ? vehicle->route.back() : 0;
  Lane* pilotLane = getLane(pilotLaneNdx);
  float distanceToEnd = pilotLane->length - vehicle->pilot.dap;
  bool mergeValidity[3];
  item laneNdx[3];
  float aggressiveness = vehicle->aggressiveness;
  item targetLane = 0;

  if (length(vehicle->velocity) < 1 && distanceToEnd > pilotDistance) {
    return -1;
  }

  for (int i = 0; i < 3; i++) {
    mergeValidity[i] = false;
    laneNdx[i] = pilotLaneNdx + i - 1;
  }
  if (pilotLaneNBlock == 0) {
    mergeValidity[0] = false;
  }
  if (pilotLaneNBlock == block->numLanes-1) {
    mergeValidity[2] = false;
  }

  for (int i = 0; i < block->numLanes; i++) {
    int k = i < pilotLaneNBlock ? 0 : i == pilotLaneNBlock ? 1 : 2;
    if (nextBlockNdx != 0) {
      Lane* lane = &block->lanes[i];
      for (int i = 0; i < lane->drains.size(); i++) {
        item nextLane = lane->drains[i];
        if (getLaneBlockIndex(nextLane) == nextBlockNdx) {
          mergeValidity[k] = true;
        }
      }
    } else if (getGameMode() == ModeTest) {
      mergeValidity[k] = true;
    } else {
      mergeValidity[k] =
        getLaneIndex(pilotBlockNdx, i) == vehicle->destination.lane;
    }
  }

  bool doRouteMerge = !mergeValidity[1] && distanceToEnd <
    (routeMergeDistance + pow(randFloat(),2) * routeMergeFuzz) * aggressiveness
    || randFloat() < routeMergeProbability * duration * aggressiveness;
  bool doExpedienceMerge = distanceToEnd > routeMergeDistance &&
    randFloat() < mergeProbability * duration * aggressiveness;
  for (int i = 0; i < 3; i += 2) {
    if (mergeValidity[i] && (doRouteMerge || (doExpedienceMerge &&
      numVehiclesInLane(laneNdx[i])+1 < numVehiclesInLane(pilotLaneNdx)))) {
      targetLane = laneNdx[i];
      break;
    }
  }

  if (targetLane == 0) {
    return -1;
  }

  if (numVehiclesInLane(targetLane) >= maxVehiclesInLane-1) {
    return 0;
  }

  // Check to see if there is a competitor in the lane
  if (distanceToEnd > mergeDistance) {
    item compNdx = getNearestVehicleInLane(targetLane, vehicle->pilot.dap);
    if (compNdx != 0) {
      Vehicle* comp = getVehicle(compNdx);
      float cdap = comp->pilot.dap;

      // Is the competitor close?
      if (abs(cdap - vehicle->pilot.dap) < mergeSafetyDistance) {
        float compSpeed = length(comp->velocity);
        float ourSpeed = length(vehicle->velocity);

        // Is the competitor moving too fast relative to us?
        if (abs(compSpeed - ourSpeed) > maxMergeSpeedDiff) {
          // Then don't merge, but tell vehicle physics to slow down.
          return compSpeed;
        }
      }
    }
  }

  enterLaneByMerge(vehicleNdx, targetLane);
  return -1;
}

Vehicle* getVehicleAhead(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  if (!(vehicle->flags & _vehicleExists)) {
    return 0;
  }

  if (vehicle->vehicleAhead == 0) return 0;
  Vehicle* result = getVehicle(vehicle->vehicleAhead);
  while (result->trailer != 0) {
    result = getVehicle(result->trailer);
  }
  return result;
}

void updateVehicleTrailer(item ndx, float duration) {
  Vehicle* vehicle = getVehicle(ndx);
  Vehicle* trailing = getVehicle(vehicle->trailing);
  vec3 pilotVector = trailing->location - vehicle->location;

  if (length(pilotVector) < 1) {
    vehicle->velocity = vec3(0,0,0);
    return;
  }

  vec3 nextLoc = trailing->location - normalize(pilotVector) *
    (.5f * (vehicleLength(vehicle) + vehicleLength(trailing)) +
     trailerDistance);
  vec3 advance = (nextLoc - vehicle->location);
  vec3 velocity = advance / duration;

  vehicle->velocity = velocity;
  vehicle->location = nextLoc;
  vehicle->yaw = atan2(pilotVector.x, pilotVector.y);
  vehicle->pitch = trailing->pitch;

  if (vehicle->pilot.lane != trailing->laneLoc.lane) {
    if (vehicle->pilot.lane != vehicle->laneLoc.lane) {
      removeVehicleFromLane(ndx, vehicle->pilot.lane);
    }
    vehicle->pilot = trailing->laneLoc;
    vehicle->pilot.dap -= 0.01;
    if (vehicle->laneLoc.lane != vehicle->pilot.lane) {
      addVehicleToLane(ndx, vehicle->pilot.lane);
      vehicle->flags |= _vehicleIsMerging;
      vehicle->distanceSinceMerge = 0;
    }

  } else {
    vehicle->pilot.dap = trailing->laneLoc.dap - 0.01;
  }

  if (vehicle->flags & _vehicleIsMerging) {
    vehicle->distanceSinceMerge += length(advance);
    if (vehicle->distanceSinceMerge > mergeDistance) {
      if (vehicle->laneLoc.lane != vehicle->pilot.lane) {
        removeVehicleFromLane(ndx, vehicle->laneLoc.lane);
        vehicle->laneLoc.lane = vehicle->pilot.lane;
      }
      vehicle->flags &= ~_vehicleIsMerging;
    }
  }

  vehicle->laneLoc = graphLocation(vehicle->laneLoc.lane, vehicle->location);
}

void updateVehiclePhysicsSimple(item ndx,
    float duration, float mergeSpeed, item nextLane) {
  Vehicle* vehicle = getVehicle(ndx);
  vec3 pilotVector = getLocation(vehicle->pilot) - vehicle->location;
  GraphLocation laneLoc = vehicle->laneLoc;
  GraphLocation pilot = vehicle->pilot;
  Lane* pilotLane = getLane(pilot.lane);
  vec3 laneOffset = getLocation(laneLoc) - vehicle->location;
  LaneBlock* block = getLaneBlock(laneLoc);
  float pilotVectorLength = length(pilotVector);
  vec3 pilotNorm = pilotVector / pilotVectorLength;
  float aggro = vehicle->aggressiveness;
  vec3 location = vehicle->location;
  float speed = length(vehicle->velocity);
  float oldSpeed = speed;

  if (pilotVectorLength < 0.1) {
    vehicle->velocity = vec3(0,0,0);
    return;
  }

  float maxAdvance = 0;
  float advanceSpeed = 0;
  Vehicle* vehicleAhead = getVehicleAhead(ndx);
  if (vehicleAhead != 0 && vehicleAhead->vehicleAhead != ndx) {
    advanceSpeed = length(vehicleAhead->velocity);
    maxAdvance = vehicleAhead->laneLoc.dap - pilot.dap -
      timeBetweenCars * speed - bumperDistance;
  } else if (getLaneBlockIndex(pilot.lane) ==
      getLaneBlockIndex(vehicle->destination.lane) &&
      getGameMode() == ModeTest) {
    maxAdvance = 100000;

  } else {
    advanceSpeed = 0;
    maxAdvance = pilotLane->length - pilot.dap;
    if (nextLane != 0) {
      bool fast =
        (duration*speed*.5f + speed*speed / (2*staticFriction*g)) > maxAdvance;
      float space = spaceInLane(nextLane, fast);
      if (space > 0) {
        maxAdvance += space;
        if (vehicleAhead == 0) {
          item nextVehicleAhead = lastVehicleInLane(nextLane);
          if (nextVehicleAhead != 0) {
            vehicleAhead = getVehicle(nextVehicleAhead);
            advanceSpeed = length(vehicleAhead->velocity);
          }
        }
      }
    }
  }

  maxAdvance += pilotVectorLength - vehicleLength(vehicle)*.5f -
    bumperDistance;

  float vehicleAheadDist = 0;
  if (vehicleAhead != 0) {
    vehicleAheadDist = length(vehicleAhead->location - location);
    maxAdvance = std::min(maxAdvance, vehicleAheadDist - bumperDistance
        - .5f*(vehicleLength(vehicleAhead) + vehicleLength(vehicle)));
  }

  float targetSpeed = mergeSpeed >= 0 ? mergeSpeed :
    block->speedLimit * c(CVehicleSpeed) * aggro;

  targetSpeed = std::min(targetSpeed, advanceSpeed +
    sqrt(2*staticFriction*g*maxAdvance) / aggro);
  speed = mix(speed, targetSpeed, duration*aggro);
  speed = std::min(speed, (pilotVectorLength - 0.1f)/duration);
  vec3 velocity = pilotNorm * speed;
  vec3 advance = velocity * duration;

  vehicle->velocity = velocity;
  vec3 oldLoc = vehicle->location;
  vec3 physicalLoc = oldLoc + advance;
  laneLoc = graphLocation(laneLoc.lane, physicalLoc);
  vehicle->laneLoc = laneLoc;
  vec3 alignedLoc = getLocation(laneLoc) + laneOffset;
  vec3 newLoc = mix(physicalLoc, alignedLoc, clamp(duration, 0.f, .5f));
  vehicle->location = newLoc;
  //vec3 laneVec = normalize(-getLocation(laneLoc) +
      //getLocation(graphLocation(laneLoc.lane, laneLoc.dap+5)));
  //vec3 movementVec = normalize(pilotNorm + newLoc - oldLoc);
  //vec3 targetAngleVec = laneVec + movementVec*.1f;
  vec3 oldAngleVec = vec3(sin(vehicle->yaw), cos(vehicle->yaw), 0);
  vec3 angleVec = oldAngleVec + velocity;
  vehicle->yaw = atan2(angleVec.x, angleVec.y);
  if (vehicle->flags & _vehicleIsMerging) {
    vehicle->distanceSinceMerge += length(advance);
  }

  int model = vehicleStyle(vehicle)/numVehicleColors;
  float mass = massBase * massMult[model];
  float enginePower = enginePowerBase * powerMult[model]; //kilowatts
  float idlePower = enginePower * 0.2; //kilowatts
  float wearEnergy = .5f*mass*abs(speed*speed - oldSpeed*oldSpeed)/1000;
  //kilowatts again

  wearEdge(block->graphElements[1], wearEnergy);
  heatMapAdd(Pollution, newLoc, c(CVehiclePollution) *
      (duration*idlePower + wearEnergy));
  if (speed > 10.f && speed < 30.f) {
    float mult = model >= BoxTruck ? c(CTruckDensity) : c(CVehicleDensity);
    heatMapAdd(Density, newLoc, duration * mult);
  }
  heatMapAdd(Value, newLoc, duration * c(CTrafficValue));
}

void updateVehiclePhysics(item ndx, float duration, float mergeSpeed,
    item nextLane) {
  Vehicle* vehicle = getVehicle(ndx);
  GraphLocation laneLoc = vehicle->laneLoc;
  GraphLocation pilot = vehicle->pilot;
  LaneBlock* block = getLaneBlock(laneLoc);
  Lane* pilotLane = getLane(pilot);
  Lane* lane = getLane(laneLoc);
  Vehicle* vehicleAhead = getVehicleAhead(ndx);
  vec3 velocity = vehicle->velocity;
  vec3 location = vehicle->location;

  vec3 pilotLoc = getLocation(pilot);
  GraphLocation dest = vehicle->destination;
  vec3 laneVec = normalize(
      getLocation(graphLocation(pilot.lane, pilot.dap+5)) -
      getLocation(pilot));
  if (pilot.lane == dest.lane && pilot.dap > dest.dap - 10) {
    pilotLoc -= zNormal(laneVec) * c(CLaneWidth);
  }
  vec3 pilotVector = pilotLoc - vehicle->location;
  vec3 upilot = normalize(pilotVector);

  float speed = length(velocity);
  float oldSpeed = speed;
  float aggressiveness = vehicle->aggressiveness;
  float maxAdvance = 0;
  float advanceSpeed = 0;
  float targetPedal = 0;
  float maxSpeed = block->speedLimit * aggressiveness;
  int model = vehicleStyle(vehicle)/numVehicleColors;
  float mass = massBase * massMult[model];
  float normalForce = mass * g;
  float brakingForce = staticFriction * normalForce; //Newtons
  float rollingResistanceForce = coefficientOfRollingResistance * normalForce;
  float energy = speed * speed * mass * .5f  -
    rollingResistanceForce * speed * duration -
    duration * velocity.z * normalForce;
  float wearEnergy = 0;
  float engineEnergy = 0;
  float brakeEnergy = 0;
  float yaw = vehicle->yaw;
  float brakeLight = vehicle->acceleration.x;
  float pedal = vehicle->acceleration.y;
  float enginePower = enginePowerBase * powerMult[model]; //kilowatts
  float idlePower = enginePower * 0.2; //kilowatts

  // Can't get too close to or too far from the pilot
  if (length(pilotVector) > pilotDistance * 4) {
    location = pilotLoc;
    vehicle->laneLoc.dap = laneLoc.dap =
      graphLocation(laneLoc.lane, location).dap - pilotDistance;
    location = getLocation(laneLoc);

  } else if (length(pilotVector) < 0.01 ||
      (laneLoc.dap > pilot.dap && laneLoc.lane == pilot.lane)) {
    vehicle->velocity = vec3(0,0,0);
    float slope = upilot.z;
    float slopeTheta = atan2(slope, length(vec2(upilot)));
    float pitch = vehicle->pitch*.9f;
    pitch += slopeTheta * .1f;
    pitch += (-oldSpeed)/duration*0.0002f;
    pitch = clamp(pitch, -.25f, .25f);
    vehicle->pitch = pitch;
    return;
  }

  //Steering
  if (speed > 0.01) {
    vec3 unitVelocity = normalize(velocity);
    vec3 steerTargetVector = upilot + laneVec;
    float turnAmount = 0;

    /*
    if (duration > 0.1) {
      float targetYaw = atan2(steerTargetVector.x, steerTargetVector.y);
      turnAmount = yaw - targetYaw;
      yaw = targetYaw;

    } else {
    */
      float turnDet = cross(steerTargetVector, unitVelocity).z;
      if (speed > 10) {
        turnDet = turnDet*pow(10,1.5)/pow(speed,1.5);
      }
      if (abs(turnDet) > .5f/speed) {
        turnAmount = turnDet * speed * duration / turningRadius;
        if (block->graphElements[1] < 0) {
          turnAmount *= randFloat(0.9, 2.0);
        } else if (vehicle->flags & _vehicleIsMerging) {
          turnAmount *= randFloat(0.3, 0.4);
        } else {
          turnAmount *= randFloat(0.2, 0.8);
        }
        turnAmount = clamp(turnAmount, -.5f, .5f);
        yaw += turnAmount;
        if (yaw > pi_o*2) {
          yaw -= pi_o*2;
        } else if (yaw < 0) {
          yaw += pi_o*2;
        }
      }
    //}

    float speedLost = speed*(1-cos(turnAmount));
    speed -= speedLost;
    float energyLost = speedLost*speedLost * mass * .5f;
    energy -= energyLost;
    energy = std::max(0.f, energy);
    wearEnergy += energyLost;
  }

  // How far can we go? And how fast should we be going when we get there?
  if (vehicleAhead != 0 && vehicleAhead->vehicleAhead != ndx) {
    advanceSpeed = length(vehicleAhead->velocity);
    maxAdvance = vehicleAhead->laneLoc.dap - pilot.dap -
      timeBetweenCars * speed - bumperDistance;
  } else if (getLaneBlockIndex(pilot.lane) ==
      getLaneBlockIndex(vehicle->destination.lane) &&
      getGameMode() == ModeTest) {
    maxAdvance = 100000;

  } else {
    advanceSpeed = 0;
    maxAdvance = pilotLane->length - pilot.dap;
    if (nextLane != 0) {
      bool fast =
        (duration*speed*.5f + speed*speed / (2*staticFriction*g)) > maxAdvance;
      float space = spaceInLane(nextLane, fast);
      if (space > 0) {
        maxAdvance += space;
        if (vehicleAhead == 0) {
          item nextVehicleAhead = lastVehicleInLane(nextLane);
          if (nextVehicleAhead != 0) {
            vehicleAhead = getVehicle(nextVehicleAhead);
            advanceSpeed = length(vehicleAhead->velocity);
          }
        }
      }
    }
  }

  maxAdvance += length(pilotVector) - vehicleLength(vehicle)*.5f -
    bumperDistance;

  float vehicleAheadDist = 0;
  if (vehicleAhead != 0) {
    vehicleAheadDist = length(vehicleAhead->location - location);
    maxAdvance = std::min(maxAdvance, vehicleAheadDist - bumperDistance
        - .5f*(vehicleLength(vehicleAhead) + vehicleLength(vehicle)));
  }

  //maxAdvance -= speed * duration *.5f;
  maxAdvance = maxAdvance < 0 ? 0 : maxAdvance;

  // Target speed
  float speedDiff = advanceSpeed - speed;
  float targetSpeed = std::min(maxSpeed, advanceSpeed +
    sqrt(2*staticFriction*g*maxAdvance) / aggressiveness);
  if (mergeSpeed >= 0) {
    targetSpeed = std::min(mergeSpeed, targetSpeed);
  }

  // Driver gas/brake pedal
  float brakingDistance = speedDiff < 0 ?
    speedDiff*speedDiff / (2*staticFriction*g) : 0;
  if (maxAdvance < 0.01) {
    targetPedal = -1;
  } else if (brakingDistance*2 > maxAdvance) {
    targetPedal = -2*brakingDistance / maxAdvance;
  } else {
    targetPedal = clamp((targetSpeed - speed) / targetSpeed, -.5f, 1.f);
  }

  // Apply engine or brake energy
  pedal = mix(pedal, targetPedal, pedal > targetPedal ? 0.2 : 0.05);
  if (pedal > 0.1f) {
    engineEnergy = enginePower * duration * aggressiveness * pedal;
    energy += engineEnergy;
    wearEnergy += engineEnergy;

  } else if (pedal < -0.1f) {
    brakeEnergy = std::min(energy,
        brakingForce * speed * duration * aggressiveness * -pedal);
    energy -= brakeEnergy;
    wearEnergy += brakeEnergy;
  }

  // Set brake lights
  brakeLight -= duration*(pedal + speed*0.02 - 0.2);
  if (brakeLight > 0) {
    vehicle->flags |= _vehicleIsBraking;
  } else {
    vehicle->flags &= ~_vehicleIsBraking;
  }

  // Get the new speed and velocity
  energy = std::max(energy, 0.f);
  speed = sqrt(2* energy / mass);
  speed = std::min(speed, (length(pilotVector)-0.02f) / duration);
  //yaw = atan2(pilotVector.x, pilotVector.y);
  float syaw = sin(yaw);
  float cyaw = cos(yaw);
  float slope = upilot.z;
  float slopeTheta = atan2(slope, length(vec2(upilot)));
  float sslope = sin(slopeTheta);
  float cslope = cos(slopeTheta);
  velocity = vec3(cslope*syaw*speed, cslope*cyaw*speed, sslope*speed);
  vec3 advance = velocity * duration;
  location += advance;

  // Pitch the vehicle
  float pitch = vehicle->pitch*.9f;
  pitch += slopeTheta * .1f;
  pitch += (speed-oldSpeed)/duration*0.0002f;
  pitch = clamp(pitch, -.25f, .25f);

  // Write everything back to the vehicle
  vehicle->yaw = yaw;
  vehicle->pitch = pitch;
  vehicle->velocity = velocity;
  vehicle->acceleration = vec2(brakeLight, pedal);
  vehicle->location = location;
  vehicle->laneLoc = graphLocation(laneLoc.lane, location);
  /* if (vehicle->laneLoc.lane == vehicle->pilot.lane &&
      vehicle->laneLoc.dap > vehicle->pilot.dap) {
    vehicle->laneLoc.dap = vehicle->pilot.dap - pilotDistance*.5f;
    vehicle->location = getLocation(vehicle->laneLoc);
  } */
  if (vehicle->flags & _vehicleIsMerging) {
    vehicle->distanceSinceMerge += length(advance);
  }

  // Apply wear, pollution, and density effects
  wearEdge(block->graphElements[1], wearEnergy);
  heatMapAdd(Pollution, location, c(CVehiclePollution) *
      (wearEnergy + idlePower * duration));
  if (vehicleAhead && vehicleAheadDist < 10) {
    // Traffic
    heatMapAdd(Value, location, duration * c(CTrafficValue) *
        (1.f - speed/10.f));
  } else if (speed > 10.f && speed < 30.f) {
    float mult = model >= BoxTruck ? c(CTruckDensity) : c(CVehicleDensity);
    heatMapAdd(Density, location, duration * mult);
  }
}

void updateVehicleLaneLoc(item ndx, float duration) {
  Vehicle* vehicle = getVehicle(ndx);
  GraphLocation laneLoc = vehicle->laneLoc;
  GraphLocation pilot = vehicle->pilot;
  Lane* lane = getLane(laneLoc);

  if (laneLoc.dap + pilotDistance >= lane->length) {
    if (laneLoc.lane == pilot.lane) {
      //laneLoc.dap = lane->length;
    } else {
      if (laneLoc.lane != pilot.lane) {
        removeVehicleFromLane(ndx, laneLoc.lane);
      }
      laneLoc = graphLocation(pilot.lane, vehicle->location);
      vehicle->flags &= ~_vehicleIsMerging;
    }
  }

  if (vehicle->flags & _vehicleIsMerging &&
      vehicle->distanceSinceMerge > mergeDistance) {
    if (laneLoc.lane != pilot.lane) {
      removeVehicleFromLane(ndx, laneLoc.lane);
    }
    laneLoc.lane = pilot.lane;
    vehicle->flags &= ~_vehicleIsMerging;
  }

  vehicle->laneLoc = laneLoc;
}

void selectVehicleAndPause(item ndx, const char* message) {
  /*
  setSelection(SelectionVehicle, ndx);
  setGameSpeed(0);
  Vehicle* v = getVehicle(ndx);
  SPDLOG_INFO("{}({}): f:{:#b} vA:{} e:{} pl:{} ll:{}",
      message, ndx, v->flags, v->vehicleAhead, v->entity,
      v->pilot.lane, v->laneLoc.lane);
  item vAndx = v->vehicleAhead;
  if (vAndx != 0) {
    v = getVehicle(vAndx);
    SPDLOG_INFO("vA:({}): f:{:#b} vA:{} e:{} pl:{}, ll:{}",
        vAndx, v->flags, v->vehicleAhead, v->entity,
        v->pilot.lane, v->laneLoc.lane);
  }
  */
}

void repairVehicleDestinationLane(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  item laneNdx = vehicle->destination.lane;
  LaneBlock* destBlock = getLaneBlock(laneNdx);
  vehicle->destination.lane = getLaneBlockIndex(laneNdx) +
    destBlock->numLanes-1;
}

void updateVehiclePilot(item ndx, float duration, item nextLane) {
  Vehicle* vehicle = getVehicle(ndx);
  GraphLocation pilot = vehicle->pilot;
  Lane* pilotLane = getLane(pilot);
  GraphLocation laneLoc = vehicle->laneLoc;
  float adjPilotDistance = pilotDistance;
  //float adjPilotDistance = duration > .1 ?
      //10 * duration * pilotDistance : pilotDistance;
  float backset = vehicleLength(vehicle)*.5f
    + bumperDistance / vehicle->aggressiveness;

  float lastPilotDap = pilot.dap;
  if (pilot.lane == laneLoc.lane) {
    pilot.dap = laneLoc.dap + adjPilotDistance;
  } else {
    float diff = adjPilotDistance -
      vecDistance(getLocation(pilot), vehicle->location);
    pilot.dap += std::max(diff, 0.f);
  }

  /*
  if (debugMode() && vehicle->vehicleAhead != 0) {
    Vehicle* vehicleAhead = getVehicle(vehicle->vehicleAhead);
    if (vehicle->trailer == vehicle->vehicleAhead) {
      selectVehicleAndPause(ndx, "vehicle behind its own trailer");
    }
    if (vehicle->vehicleAhead == ndx) {
      //handleError("vehicleAhead assigned to itself\n");
    } else {
      bool isBad = !(vehicleAhead->flags & _vehicleExists);
      item vApl = vehicleAhead->pilot.lane;
      item vAll = vehicleAhead->laneLoc.lane;
      item pl = pilot.lane;
      item ll = laneLoc.lane;
      isBad = isBad || (vApl != ll && vApl != pl && vAll != ll && vAll != pl);
      if (isBad) {
        selectVehicleAndPause(ndx, "Bad vehicleAhead");
      }
    }
  }
  */

  Vehicle* vehicleAhead = getVehicleAhead(ndx);

  /*
  if (vehicleAhead != 0 &&
      getLaneBlockIndex(vehicleAhead->laneLoc.lane) ==
      getLaneBlockIndex(laneLoc.lane) &&
      vehicleAhead->laneLoc.dap < laneLoc.dap) {
    vehicleAhead = 0;
  }

  if (vehicleAhead != 0 &&
      getLaneBlockIndex(vehicleAhead->pilot.lane) ==
      getLaneBlockIndex(pilot.lane) &&
      vehicleAhead->pilot.dap < pilot.dap) {
    vehicleAhead = 0;
  }
  */

  // WTF the second check? There could be cycles longer than 2.
  if (vehicleAhead != 0 && vehicleAhead->vehicleAhead != ndx) {
    if (vehicleAhead->pilot.lane == pilot.lane) {
      pilot.dap = std::min(pilot.dap, vehicleAhead->pilot.dap
        -.5f*vehicleLength(vehicleAhead)-backset);
    }
    if ((getLaneBlockIndex(vehicleAhead->laneLoc.lane) !=
        getLaneBlockIndex(laneLoc.lane) ||
        vehicleAhead->laneLoc.dap >= laneLoc.dap) &&
        vehicleAhead->laneLoc.lane == pilot.lane) {
      pilot.dap = std::min(pilot.dap, vehicleAhead->laneLoc.dap
        -.5f*vehicleLength(vehicleAhead)-backset);
    }
  }
  pilot.dap = std::min(pilot.dap, pilotLane->length - backset);
  pilot.dap = std::max(pilot.dap, lastPilotDap);
  if (pilot.lane == laneLoc.lane) {
    pilot.dap = std::max(pilot.dap, laneLoc.dap);
  }
  vehicle->pilot = pilot;

  //Split?

  if (vehicle->flags & _vehicleHasRoute) {
    if (vehicle->route.size() == 0) {
      if (getLaneBlockIndex(pilot.lane) !=
        getLaneBlockIndex(vehicle->destination.lane) ||
          pilot.dap >= vehicle->destination.dap) {
        vehicleRate ++;
        removeVehicle(ndx);
        return;
      }
      repairVehicleDestinationLane(ndx);

    } else {
      item nextBlockNdx = vehicle->route.back();
      if (nextBlockNdx == getLaneBlockIndex(pilot.lane)) {
        vehicle->route.pop_back();

      } else {
        bool goodRoute = false;
        LaneBlock* pilotBlk = getLaneBlock(pilot.lane);
        for (int i = 0; i < pilotBlk->drains.size(); i++) {
          item nextBlk = pilotBlk->drains[i];
          if (nextBlk == nextBlockNdx) {
            goodRoute = true;
          }
        }
        if (!goodRoute) {
          rerouteVehicle(ndx);
        }
      }
    }
  }

  if (pilot.dap + backset + 1 >= pilotLane->length) {
    //if (pilot.lane != laneLoc.lane) {
     // vehicle->pilot.dap = pilotLane->length;
    //} else {
      if (nextLane == 0) {
        vehicle->pilot.dap = pilotLane->length - backset;
        if (vehicle->route.size() == 0) {
          if (getGameMode() != ModeTest) {
            rerouteVehicle(ndx);
          } else {
            vehicle->pilot.dap = pilotLane->length + 100;
          }
        } else {
          // See if there is anywhere we can go at all
          item nextBlockNdx = vehicle->route.back();
          LaneBlock* pilotBlock = getLaneBlock(vehicle->pilot.lane);
          bool anyMatch = false;
          for (int i = 0; i < pilotBlock->drains.size(); i++) {
            if (pilotBlock->drains[i] == nextBlockNdx) {
              anyMatch = true;
              break;
            }
          }
          if (!anyMatch) {
            rerouteVehicle(ndx);
          }
        }

      } else {
        enterLane(vehicle, ndx, nextLane);
      }
//    }
  }
}

void updateVehicle(item ndx, float duration) {
  Vehicle* vehicle = getVehicle(ndx);
  if (!(vehicle->flags & _vehicleExists)) {
    return;
  }

  if (getGameMode() == ModeGame &&
      getCurrentDateTime() - vehicle->creationTime > maxVehicleLife) {
    removeVehicle(ndx);
    return;
  }

  if (vehicle->trailing == 0 && !(vehicle->flags & _vehicleHasRoute)) {
    return;
  }

  if (!(vehicle->flags & _vehiclePlaced)) {
    if (vehicle->trailing &&
      !(getVehicle(vehicle->trailing)->flags & _vehiclePlaced)) return;
    if (numVehiclesInLane(vehicle->pilot.lane) >= randFloat(10, 30)) return;
    enterLaneInitial(ndx);
    renderVehicle(ndx);
    vehicle->flags |= _vehiclePlaced;
    setEntityVisible(vehicle->entity, true);
    if (vehicle->lightEntity) {
      setEntityVisible(vehicle->lightEntity, true);
    }
    //setEntityVisible(vehicle->pilotEntity, true);
  }

  if (!(getLaneBlock(vehicle->laneLoc)->flags & _laneExists)) {
    removeVehicle(ndx);
    return;
  }

  if (vehicle->trailing != 0) {
    updateVehicleTrailer(ndx, duration);

  } else {
    float mergeSpeed = considerMerge(ndx, duration);
    float durationToGo = duration;
    while (durationToGo > 0.0001f) {
      float dur = std::min(0.1f, durationToGo);
      item nextLane = getNextLane(ndx);
      durationToGo -= dur;
      updateVehiclePhysicsSimple(ndx, dur, mergeSpeed, nextLane);
      updateVehicleLaneLoc(ndx, dur);
      updateVehiclePilot(ndx, dur, nextLane);
    }
  }

  updateVehicleHeadlights(ndx, duration);
  placeVehicleEntity(vehicle);
}

void simulateVehicles(float duration) {
  vehicleRate *= (60*6 - duration)/60/6;
  for(int i = 1; i <= vehicles->size(); i++) {
    updateVehicle(i, duration);
  }
}

