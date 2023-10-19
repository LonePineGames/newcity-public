
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

function v3pitchYaw(pitch, yaw)
  local spitch = math.sin(pitch);
  local cpitch = math.cos(pitch);
  local syaw = math.sin(yaw) * cpitch;
  local cyaw = math.cos(yaw) * cpitch;
  return {x = syaw, y = cyaw, z = spitch};
end

function normalizeAngle(ang)
  local pi2 = math.pi*2;
  while ang < 0 do
    ang = ang + pi2;
  end
  while ang >= pi2 do
    ang = ang - pi2;
  end
  return ang;
end

function mix(v0, v1, a)
  return v0*(1-a) + v1*a;
end

function clamp(v, min, max)
  if (v > max) then v = max; end
  if (v < min) then v = min; end
  return v;
end

function v3mix(v0, v1, a)
  local a1 = 1-a;
  return {
    x = v0.x*a1 + v1.x*a,
    y = v0.y*a1 + v1.y*a,
    z = v0.z*a1 + v1.z*a,
  };
end


