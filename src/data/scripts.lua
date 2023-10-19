
function countDesignData(design, data, key)
  if design[key] > 0 then

    local parcels = design.sx * design.sy / CTileSize / CTileSize
    if parcels == 0 then parcels = 1 end
    local perParcel = design[key] / parcels

    data[key].count = data[key].count + 1
    data[key].val = data[key].val + perParcel
  end
end

function logDesignData(data, zone, key)
  local line = ""
  for i=0,10 do
    if data[i][key].count > 0 then
      local avg = data[i][key].val / data[i][key].count
      --local avg = data[i][key].count
      line = string.format("%s%5.2f ", line, avg)
    else
      line = line .. "      "
    end
  end
  line = line .. key .. "\n"
  return line
end

function logDesignStats()
  local designStats = {}

  for i=0,numZones() do
    local zoneData = {}
    for j=0,10 do
      zoneData[j] = {
        numFamilies =  {val=0,count=0},
        numRetail =    {val=0,count=0},
        numOffices =   {val=0,count=0},
        numFarms =     {val=0,count=0},
        numFactories = {val=0,count=0},
      };
    end
    designStats[i] = zoneData;
  end

  for i=1,maxDesignNdx() do
    local design = getDesign(i);
    local density = math.floor(design.minDensity*10+0.1)
    local zoneData = designStats[design.zone][density]

    countDesignData(design, zoneData, "numFamilies")
    countDesignData(design, zoneData, "numRetail")
    countDesignData(design, zoneData, "numOffices")
    countDesignData(design, zoneData, "numFarms")
    countDesignData(design, zoneData, "numFactories")
  end

  for i=0,numZones()-1 do
    local line = "----------- " .. zoneName(i) .. "\n"
    line = line .. "  0     1     2     3     4     5     6     7     8     9    10\n"

    local data = designStats[i]
    line = line .. logDesignData(data, i, "numFamilies")
    line = line .. logDesignData(data, i, "numRetail")
    line = line .. logDesignData(data, i, "numOffices")
    line = line .. logDesignData(data, i, "numFarms")
    line = line .. logDesignData(data, i, "numFactories")

    log(line)
  end

end

-- on(Load, logDesignStats)

