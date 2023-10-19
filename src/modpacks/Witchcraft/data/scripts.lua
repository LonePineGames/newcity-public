
yawSpeed = 1
pitchSpeed = 1
pitchVerticalMovementFudge = 0.1

rollSpeed = 0.5
rollFriction = 0.25

moveSpeed = 80
moveFriction = 0.2
brakeFriction = 1
walkSpeed = 3
walkFriction = 2

jumpSpeed = 10

playerLoc = {x=0, y=0, z=0}
playerVel = {x=0, y=0, z=0}
playerYaw = pi_o
playerPitch = pi_o*.5
playerRoll = 0
playerLanded = true

on(Load, function()
  setCameraZoom(.5)
  playerLoc = getCameraTarget();
  playerLoc.z = getTerrainElevation(playerLoc)
end)

function v3add(v1, v2)
  return {
    x = v1.x + v2.x,
    y = v1.y + v2.y,
    z = v1.z + v2.z,
  }
end

function v3mult(v1, f)
  return {
    x = v1.x * f,
    y = v1.y * f,
    z = v1.z * f
  }
end

function v3length(v1)
  return math.sqrt(v1.x*v1.x + v1.y+v1.y + v1.z*v1.z)
end

function v3normalize(v1)
  local length = v3length(v1)
  return {
    x = v1.x / length,
    y = v1.y / length,
    z = v1.z / length
  }
end

on(Frame, function()
  local duration = getFrameDuration();
  local accel = 0
  local friction = moveFriction

  if isKeyDown(65) then -- A
    playerRoll = playerRoll + rollSpeed * duration
  end

  if isKeyDown(68) then -- D
    playerRoll = playerRoll - rollSpeed * duration
  end

  if isKeyDown(87) then -- W
    accel = -moveSpeed * duration
  end

  if isKeyDown(83) then -- S
    friction = brakeFriction
    --accel = moveSpeed * duration
  end

  local mouseLoc = getMouseLocation_screenspace();

  local rollCos = math.cos(playerRoll)
  local rollSin = math.sin(playerRoll)
  playerYaw = playerYaw + yawSpeed * duration * (
    mouseLoc.x * rollCos - mouseLoc.y * rollSin)
  playerPitch = playerPitch + pitchSpeed * duration * (
    mouseLoc.y * rollCos + mouseLoc.x * rollSin)

  --playerYaw = playerYaw + yawSpeed * duration * mouseLoc.x
  --playerPitch = playerPitch + pitchSpeed * duration * mouseLoc.y

  --[[
  if playerYaw < 0 then
    playerYaw = playerYaw + pi_o*2
  elseif playerYaw > pi_o*2 then
    playerYaw = playerYaw - pi_o*2
  end
  ]]--

  if playerPitch < 0 then
    playerPitch = 0
  elseif playerPitch > pi_o then
    playerPitch = pi_o
  end

  playerRoll = playerRoll * (1-duration*rollFriction)

  if playerLanded then friction = walkFriction end
  playerVel = v3mult(playerVel, 1-duration*friction)

  local horzAccel = math.sin(playerPitch) * accel
  playerVel = v3add(playerVel, {
    x = math.cos(playerYaw) * horzAccel,
    y = math.sin(playerYaw) * horzAccel,
    z = math.cos(playerPitch + pitchVerticalMovementFudge) * accel
  })

  if playerLanded then
    playerVel.z = 0
  end

  local elevation = getTerrainElevation(playerLoc)
  if elevation < 1 then elevation = 1 end

  if playerLoc.z < elevation+2 then playerLanded = true end

  if playerLanded then
    playerRoll = 0

    local velMag = v3length(playerVel)
    if velMag > 0 then
      playerVel = v3mult(playerVel, walkSpeed/velMag)
    end
  end

  if isKeyDown(32) then -- Spacebar
    playerLoc.z = playerLoc.z + jumpSpeed*duration
    if playerLoc.z >= elevation+2 then playerLanded = false end
  end

  playerLoc = v3add(playerLoc, v3mult(playerVel, duration))

  if playerLoc.z < elevation+1 then
    playerLoc.z = elevation+1
  end

  setCameraTarget(playerLoc)
  setCameraYaw(playerYaw)
  setCameraPitch(playerPitch)
  setCameraRoll(playerRoll)
  setGameSpeed(1)
end)

-- addInputHandler(function(event)
  -- return true
-- end)

