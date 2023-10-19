
function birdUpdate(vNdx, duration)
  local v = getVehicle(vNdx);
  local model = getVehicleModel(v.model);

  local bnLoc = v3mult(v, 1/6400);
  local wave = sampleBlueNoise(bnLoc);
  local turnSpeed = model.speed * duration * 0.01;

  v.pitch = v.pitch + (wave.y*2-1) * duration*.25;
  v.pitch = mix(v.pitch, 0, duration);
  v.pitch = clamp(v.pitch, -.5, .5);
  v.yaw = normalizeAngle(v.yaw + (wave.x*2-1) * turnSpeed);

  local move = v3pitchYaw(v.pitch, v.yaw);
  move = v3mult(move, duration*model.speed);
  local loc = v3add(v, move);
  v.x = loc.x;
  v.y = loc.y;
  v.z = loc.z;

  local mapSize = getMapSize();
  local buffer = 1000;
  if (v.x > mapSize+buffer or v.x < -buffer or
      v.y > mapSize+buffer or v.y < -buffer) then
    removeVehicle(vNdx);
    return;

    -- Rotate 180 degrees
    --v.yaw = normalizeAngle(v.yaw + pi_o);
    --v.x = clamp(v.x, -buffer+2, mapSize+buffer-2);
    --v.y = clamp(v.y, -buffer+2, mapSize+buffer-2);
  end

  local floor = getTerrainElevation(v);
  if (floor < 0) then floor = 0; end
  local ceiling = CMaxHeight + 1000;
  if (floor > v.z or v.z > ceiling) then
    v.z = clamp(v.z, floor, ceiling);
  end

  if (v.z < floor + 400 and v.pitch < 0.05) then
    local pitchDiff = (floor+400 - v.z)/20000;
    pitchDiff = clamp(pitchDiff, 0, 0.01);
    v.pitch = v.pitch + pitchDiff;
  end

  setVehicle(v);
end

function VTOLUpdate(vNdx, duration, pitchMultiplier)
  local v = getVehicle(vNdx);
  local model = getVehicleModel(v.model);

  local bnLoc = v3mult(v, 1/6400);
  local wave = sampleBlueNoise(bnLoc);
  local turnSpeed = model.speed * duration * 0.005;

  v.yaw = normalizeAngle(v.yaw + (wave.x*2-1) * turnSpeed);

  local floor = getTerrainElevation(v);
  if (floor < 0) then floor = 0; end
  local ceiling = CMaxHeight + 1000;
  if (floor > v.z or v.z > ceiling) then
    v.z = clamp(v.z, floor, ceiling);
  end

  local vel = {x=0,y=0,z=0};
  local age = now() - v.createdAt;

  if v.z < floor + 400 or age < OneMinute then
    v.pitch = mix(v.pitch, 0, 0.1);
    vel.x = 0;
    vel.y = 0;
    vel.z = model.speed * 0.5;

  else
    v.pitch = mix(v.pitch/pitchMultiplier, wave.y*.5, 0.1)*pitchMultiplier;
    v.pitch = clamp(v.pitch, -.5, .5);
    vel = v3pitchYaw(v.pitch/pitchMultiplier-.25, v.yaw);
    vel = v3mult(vel, model.speed);
  end

  local oldVel = {x=v.vx, y=v.vy, z=v.vz};
  vel = v3mix(oldVel, vel, duration);
  local move = v3mult(vel, duration);

  local loc = v3add(v, move);
  v.x = loc.x;
  v.y = loc.y;
  v.z = loc.z;
  v.vx = vel.x;
  v.vy = vel.y;
  v.vz = vel.z;

  --log(v.x .. "," .. v.y .. "," .. v.z .. " age:" .. formatDuration(age));

  local mapSize = getMapSize();
  local buffer = 1000;
  if (v.x > mapSize+buffer or v.x < -buffer or
    v.y > mapSize+buffer or v.y < -buffer) then
    removeVehicle(vNdx);
    return;
  end

  setVehicle(v);
end

function helicopterUpdate(vNdx, duration)
  VTOLUpdate(vNdx, duration, -.5);
end

function balloonUpdate(vNdx, duration)
  VTOLUpdate(vNdx, duration, -1);
end

function blimpUpdate(vNdx, duration)
  VTOLUpdate(vNdx, duration, .1);
end

function boatUpdate(vNdx, duration)
  local v = getVehicle(vNdx);
  local model = getVehicleModel(v.model);

  local bnLoc = v3mult(v, 1/6400);
  local wave = sampleBlueNoise(bnLoc);
  local turnSpeed = model.speed * duration * 0.01;

  v.pitch = (wave.y-.5)*.25;

  local bestYaw = v.yaw;
  local bestLandZ = 100;
  local bestLoc = {x=0,y=0,z=0};
  local bestMove = {x=0,y=0,z=0};
  local vel = {x=v.vx,y=v.vy,z=v.vz};
  --local speed = v3length(vel);
  --if (not finite(speed)) then speed = model.speed*.5; end
  local speed = model.speed;

  v.yaw = normalizeAngle(v.yaw + (wave.x*2-1) * turnSpeed);

  for i=1,3 do
    local yaw = normalizeAngle(v.yaw + (i-2)*turnSpeed);
    local move = v3pitchYaw(0, yaw);
    move = v3mult(move, duration*model.speed);
    local loc = v3add(v, move);
    local landZ = getTerrainElevation(loc);

    if (landZ < bestLandZ and
        (bestLandZ > -5 or randomFloat() < 1/i)) then
      bestYaw = yaw;
      bestLandZ = landZ;
      bestLoc = loc;
      bestMove = move;
    end
  end

  if bestLandZ > 2 then
    removeVehicle(vNdx);
    return;
  --elseif bestLandZ > -20 then
    --speed = mix(speed, model.speed*.2, duration);
  --else
  end
  --local targetSpeed = model.speed * (.5 + wave.z*.5);
  --if speed < model.speed*.5 then speed = model.speed*.5 end
  --speed = mix(speed, targetSpeed, duration);

  v.yaw = bestYaw;
  --log("boat " .. vNdx .. " yaw:" .. v.yaw);
  local move = v3pitchYaw(0, v.yaw);
  move = v3mult(move, speed);
  v.vx = move.x;
  v.vy = move.x;
  v.vz = 0;

  move = v3mult(move, duration);
  local loc = v3add(v, move);
  v.x = loc.x;
  v.y = loc.y;
  v.z = v.pitch-1;

  local mapSize = getMapSize();
  local buffer = 0;
  if (v.x > mapSize+buffer or v.x < -buffer or
      v.y > mapSize+buffer or v.y < -buffer) then
    removeVehicle(vNdx);
    return;

    -- Rotate 180 degrees
    --v.yaw = normalizeAngle(v.yaw + pi_o);
    --v.x = clamp(v.x, -buffer+2, mapSize+buffer-2);
    --v.y = clamp(v.y, -buffer+2, mapSize+buffer-2);
  end

  setVehicle(v);
end

function fishUpdate(vNdx, duration)
  local v = getVehicle(vNdx);
  local model = getVehicleModel(v.model);

  local bnLoc = v3mult(v, 1/64000);
  local wave = sampleBlueNoise(bnLoc);
  local turnSpeed = model.speed * duration * 0.05;

  --v.yaw = normalizeAngle(v.yaw + (wave.x*2-1) * turnSpeed);

  local bestYaw = v.yaw;
  local bestLandZ = 100;
  local bestLoc = {x=0,y=0,z=0};
  local bestMove = {x=0,y=0,z=0};

  for i=1,3 do
    local yaw = normalizeAngle(v.yaw + (i-2)*turnSpeed);
    local move = v3pitchYaw(0, yaw);
    move = v3mult(move, duration*model.speed);
    local loc = v3add(v, move);
    local landZ = getTerrainElevation(loc);

    if (landZ < bestLandZ and
        (bestLandZ > -5 or randomFloat() < 1/i)) then
      bestYaw = yaw;
      bestLandZ = landZ;
      bestLoc = loc;
      bestMove = move;
    end
  end

  if bestLandZ > 0 then
    removeVehicle(vNdx);
    return;
  end

  bestLoc.z = -1-8*wave.y;
  v.pitch = bestLoc.z-v.z;
  v.yaw = bestYaw
  v.x = bestLoc.x;
  v.y = bestLoc.y;
  v.z = bestLoc.z;

  local mapSize = getMapSize();
  local buffer = 1000;
  if (v.x > mapSize+buffer or v.x < -buffer or
      v.y > mapSize+buffer or v.y < -buffer) then
    removeVehicle(vNdx);
    return;

    -- Rotate 180 degrees
    --v.yaw = normalizeAngle(v.yaw + pi_o);
    --v.x = clamp(v.x, -buffer+2, mapSize+buffer-2);
    --v.y = clamp(v.y, -buffer+2, mapSize+buffer-2);
  end

  setVehicle(v);
end

function nullUpdate(vNdx, duration)
end

