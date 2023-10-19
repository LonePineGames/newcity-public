addNewspaperArticleGroup({ code = "economic" });
addNewspaperArticleGroup({ code = "crime" });
addNewspaperArticleGroup({ code = "education" });
addNewspaperArticleGroup({ code = "budget" });
addNewspaperArticleGroup({ code = "construction" });
addNewspaperArticleGroup({ code = "population" });
addNewspaperArticleGroup({ code = "community" });
addNewspaperArticleGroup({ code = "pollution" });
addNewspaperArticleGroup({ code = "traffic" });
addNewspaperArticleGroup({ code = "taxes" });
addNewspaperArticleGroup({ code = "health" });
addNewspaperArticleGroup({ code = "sports" });
addNewspaperArticleGroup({ code = "fluff" });

function num(value)
  return value and 1 or 0
end

function clamp(value, min, max)
  if value < min then
    return min
  elseif value > max then
    return max
  else
    return value
  end
end

OneMillion = 1000000
OneBillion = 1000000000

addNewspaperArticle({
  code = "StocksDown01",
  group = "economic",
  file = "newspaper/stocks-down01.md",
  score = function()
    return growth(StatNationalStockIndex, OneDay, NationalEcon) * -30 - 0.5
  end
});

addNewspaperArticle({
  code = "StocksDown02",
  group = "economic",
  file = "newspaper/stocks-down02.md",
  score = function()
    return clamp(
      growth(StatNationalStockIndex, OneYear, NationalEcon) * -40 - 1.5,
      0, 2)
  end
});

addNewspaperArticle({
  code = "StocksDown03",
  group = "economic",
  file = "newspaper/industryFinancial.md",
  score = function()
    return growth(StatNationalStockIndex, OneDay, NationalEcon) * -30 - 0.5
  end
});

addNewspaperArticle({
  code = "StocksDown04",
  group = "economic",
  file = "newspaper/economy02.md",
  score = function()
    return growth(StatNationalStockIndex, OneDay, NationalEcon) * -40 - 1.5
  end
});

addNewspaperArticle({
  code = "StocksUp01",
  group = "economic",
  file = "newspaper/stocks-up01.md",
  score = function()
    return growth(StatNationalStockIndex, OneDay, NationalEcon) * 30 - 0.5
  end
});

addNewspaperArticle({
  code = "StocksUp02",
  group = "economic",
  file = "newspaper/stocks-up02.md",
  score = function()
    return (growth(StatNationalStockIndex, OneYear, NationalEcon) * 50 - 1) *
      num(growth(StatNationalStockIndex, OneDay, NationalEcon) > 0.02)
  end
});

addNewspaperArticle({
  code = "StocksUp03",
  group = "economic",
  file = "newspaper/steadyGrowthColum.md",
  score = function()
    return (growth(StatNationalStockIndex, OneYear, NationalEcon) * 50 - 1) *
      num(growth(StatNationalStockIndex, OneDay, NationalEcon) > 0.02)
  end
});

addNewspaperArticle({
  code = "InflationUp01",
  group = "economic",
  file = "newspaper/inflationary.md",
  score = function()
    return growth(StatInflationIndex, OneYear, NationalEcon) * 10 - 1
  end
});

addNewspaperArticle({
  code = "UnemploymentNational01",
  group = "economic",
  file = "newspaper/unemployment-national01.md",
  score = function()
    return get(StatNationalUnemploymentRate, NationalEcon) * 5 - 2
  end
});

addNewspaperArticle({
  code = "FullEmploymentNational01",
  group = "economic",
  file = "newspaper/full-employment-national01.md",
  score = function()
    return num(get(StatNationalUnemploymentRate, NationalEcon) < 0.04) * 1.5
  end
});

addNewspaperArticle({
  code = "EconDetDown01",
  group = "economic",
  file = "newspaper/econdet-down01.md",
  score = function()
    return (growth(StatNationalStockIndex, OneDay, NationalEcon) * -40 - 1)
      * num(get(StatEconomicDeterminant, NationalEcon) < 0.8)
  end
});

addNewspaperArticle({
  code = "EconDetDown02",
  group = "economic",
  file = "newspaper/industryAgriculture.md",
  score = function()
    return num(get(StatEconomicDeterminant, NationalEcon) < 0.8)
  end
});

addNewspaperArticle({
  code = "Farms-Empty01",
  group = "economic",
  file = "newspaper/farms-empty01.md",
  score = function()
    return num(get(StatEmptyFarms) >= 6)
  end
})

addNewspaperArticle({
  code = "Offices-Empty01",
  group = "economic",
  file = "newspaper/industryOffice.md",
  score = function()
    return num(get(StatEmptyOffices) >= 6)
  end
})

addNewspaperArticle({
  code = "EconDetUp01",
  group = "economic",
  file = "newspaper/econdet-up01.md",
  score = function()
    return (0.8-getPast(StatEconomicDeterminant, OneYear, NationalEcon)) * 8
      * num(get(StatEconomicDeterminant, NationalEcon) > 0.8)
  end
});

addNewspaperArticle({
  code = "Pollution01",
  group = "pollution",
  file = "newspaper/pollution01.md",
  score = function()
    return num(featureEnabled(FZonePark)) * get(StatPollutionStat) * 4
  end
});

addNewspaperArticle({
  code = "Pollution02",
  group = "pollution",
  file = "newspaper/pollution02.md",
  score = function()
    return get(StatPollutionStat) * 4 + 0.5
  end
});

addNewspaperArticle({
  code = "BeautifulCity01",
  group = "pollution",
  file = "newspaper/beautiful-city01.md",
  score = function()
    local parks = (get(StatParkLots) * .1 + get(StatNumParks))
    local value = parks * 500 / get(StatPopulation)
      - get(StatPollutionStat)*5
    return num(get(StatPollutionStat) < 0.05) *
      num(get(StatPopulation) > 50000) * value --clamp(value, 0, 2)
  end
});

addNewspaperArticle({
  code = "FriendlyCity01",
  group = "community",
  file = "newspaper/friendly-city01.md",
  score = function()
    local parks = (get(StatParkLots) * .1 + get(StatNumParks))
    local services = (get(StatNumCommServices))
    local value = (parks + services) * 100 / get(StatPopulation)
      + get(StatCommunityStat)*5
    return num(get(StatPopulation) > 50000) * value --clamp(value, 0, 2)
  end
});

addNewspaperArticle({
  code = "Restaurants01",
  group = "community",
  file = "newspaper/trendy-restaurants.md",
  score = function()
    return num(amenityEffectScore(PrestigeScore) > 200) * 2
  end
});

addNewspaperArticle({
  code = "RestaurantsBad01",
  group = "community",
  file = "newspaper/chef-snubs.md",
  score = function()
    return num(amenityEffectScore(PrestigeScore) < 100 and get(StatPopulation) > 80000) * 2
  end
});

addNewspaperArticle({
  code = "Crime01",
  group = "crime",
  file = "newspaper/crime01.md",
  score = function()
    return get(StatCrimeStat) * 10
  end
});

addNewspaperArticle({
  code = "Crime02",
  group = "crime",
  file = "newspaper/crime02.md",
  score = function()
    return get(StatCrimeStat) * 10 + 0.5
  end
});

addNewspaperArticle({
  code = "Crime03",
  group = "crime",
  file = "newspaper/crime03.md",
  score = function()
    return get(StatCrimeStat) * 8 + 0.75
  end
});

addNewspaperArticle({
  code = "Crime04",
  group = "crime",
  file = "newspaper/crime04.md",
  score = function()
    return get(StatCrimeStat) * 6 + 0.75
  end
});

addNewspaperArticle({
  code = "Crime05",
  group = "crime",
  file = "newspaper/boss_joe_again.md",
  score = function()
    return get(StatCrimeStat) * 6 + 0.75
  end
});

addNewspaperArticle({
  code = "Traffic01",
  group = "traffic",
  file = "newspaper/traffic01.md",
  score = function()
    return get(StatAverageTripTime) * 24 * 4
      * num(get(StatAverageTripTime) > 2*OneHour)
      * num(get(StatUnemploymentRate) > 0.08)
  end
});

addNewspaperArticle({
  code = "Traffic02",
  group = "traffic",
  file = "newspaper/traffic02.md",
  score = function()
    return num(get(StatAverageTripTime) <= OneHour*.75 and
      getPast(StatAverageTripTime, OneYear) > OneHour*1.5) * 1.5
  end
})

addNewspaperArticle({
  code = "Traffic03",
  group = "traffic",
  file = "newspaper/traffic03.md",
  score = function()
    return get(StatAverageTripTime) * 24 * 4
      * num(get(StatAverageTripTime) > OneHour)
  end
})

addNewspaperArticle({
  code = "Traffic04",
  group = "traffic",
  file = "newspaper/traffic04.md",
  score = function()
    return get(StatAverageTripTime) * 24 * 4
      * num(get(StatAverageTripTime) > OneHour)
  end
})

addNewspaperArticle({
  code = "TrafficBetter01",
  group = "traffic",
  file = "newspaper/traffic-better.md",
  score = function()
    return -3 * growth(StatAverageTripTime, OneYear)
  end
})

addNewspaperArticle({
  code = "Homeless01",
  group = "population",
  file = "newspaper/homeless01.md",
  score = function()
    return (get(StatNumHomeless) / 100) * num(amenityEnabled("public_housing"))
  end
});

addNewspaperArticle({
  code = "HealthNoClinic01",
  group = "health",
  file = "newspaper/health-no-clinic01.md",
  score = function()
    return num(amenitiesBuilt("_clinic") == 0)
      * num(amenityEnabled("_clinic"))
      * (1 - get(StatHealthStat)) * 4
  end
});

addNewspaperArticle({
  code = "HealthWithClinic01",
  group = "health",
  file = "newspaper/health-with-clinic01.md",
  score = function()
    return num(amenitiesBuilt("_clinic") > 0) * (1 - get(StatHealthStat)) * 6
  end
});

addNewspaperArticle({
  code = "Health02",
  group = "health",
  file = "newspaper/astronauts-snubbed.md",
  score = function()
    return (1 - get(StatHealthStat)) * 6 * num(get(StatPopulation)>80000)
  end
});

addNewspaperArticle({
  code = "HealthGood01",
  group = "health",
  file = "newspaper/astronauts-selected.md",
  score = function()
    return (4 - get(StatPeopleSick) / 10) * num(get(StatPopulation)>80000)
  end
});

addNewspaperArticle({
  code = "Parks01",
  group = "construction",
  file = "newspaper/parks01.md",
  score = function()
    return delta(StatParkLots, OneDay) * .05 + delta(StatNumParks, OneDay)
  end
});

addNewspaperArticle({
  code = "Parks02",
  group = "construction",
  file = "newspaper/parks02.md",
  score = function()
    return delta(StatParkLots, OneDay) * 0.075 + delta(StatNumParks, OneDay)
  end
})

addNewspaperArticle({
  code = "University01",
  group = "construction",
  file = "newspaper/university01.md",
  score = function()
    return num(getPast(StatNumUniBuildings, OneDay*2) == 0) *
      delta(StatNumUniBuildings, OneDay)
  end
});

addNewspaperArticle({
  code = "University02",
  group = "construction",
  file = "newspaper/university02.md",
  score = function()
    return num(getPast(StatNumUniBuildings, OneDay*2) > 0) *
      delta(StatNumUniBuildings, OneDay)
  end
});

addNewspaperArticle({
  code = "Highway01",
  group = "construction",
  file = "newspaper/highway01.md",
  score = function()
    return sum(StatExpwyBuildExpensesStat, OneDay) / inflate(OneMillion)
  end
});

addNewspaperArticle({
  code = "Education01",
  group = "construction",
  file = "newspaper/education01.md",
  score = function()
    return clamp(delta(StatNumSchools, OneDay), 0, 2)
  end
});

addNewspaperArticle({
  code = "Education02",
  group = "construction",
  file = "newspaper/education02.md",
  score = function()
    return clamp(delta(StatNumSchools, OneDay), 0, 2)
  end
});

addNewspaperArticle({
  code = "Growth01",
  group = "population",
  file = "newspaper/growth01.md",
  score = function()
    return num(now() > OneYear*4) * growth(StatPopulation, OneYear) * 5
  end
});

addNewspaperArticle({
  code = "Growth02",
  group = "population",
  file = "newspaper/growth02.md",
  score = function()
    return num(now() > OneYear*4) * growth(StatPopulation, OneYear) * 4
  end
})

addNewspaperArticle({
  code = "NoEducation01",
  group = "education",
  file = "newspaper/no-education01.md",
  score = function()
    return (1 - get(StatNoEduPerecent)) * 4 * num(get(StatPopulation) > 20000)
  end
});

addNewspaperArticle({
  code = "TooMuchEducation01",
  group = "education",
  file = "newspaper/too-much-education01.md",
  score = function()
    return get(StatPhdEduPercent) * 50 * num(get(StatPopulation) > 100000)
  end
});

addNewspaperArticle({
  code = "TooMuchEducation02",
  group = "education",
  file = "newspaper/too-much-education02.md",
  score = function()
    return (get(StatHSEduPercent) * 10 - 6) * num(get(StatPopulation) > 100000)
  end
});

addNewspaperArticle({
  code = "Taxes01",
  group = "taxes",
  file = "newspaper/taxes01.md",
  score = function()
    return num(now() > OneYear) * num(growth(StatPopulation, OneYear) < 0.01) *
      get(StatPropertyTaxRateStat) * 20
  end
});

addNewspaperArticle({
  code = "Taxes02",
  group = "taxes",
  file = "newspaper/taxes02.md",
  score = function()
    return num(now() > OneYear) *
      num(growth(StatNumRetailBiz, OneYear) < 0.01) *
      get(StatSalesTaxRateStat) * 20
  end
});

addNewspaperArticle({
  code = "Taxes03",
  group = "taxes",
  file = "newspaper/taxes03.md",
  score = function()
    return num(now() > OneYear) *
      num(growth(StatNumHomes, OneYear) < 0.05) *
      get(StatPropertyTaxRateStat) * 20
  end
})

addNewspaperArticle({
  code = "Corruption01",
  group = "taxes",
  file = "newspaper/corruption01.md",
  score = function()
    return num(now() > OneYear) *
      get(StatFinesAndFeesRateStat) * 10
  end
});

addNewspaperArticle({
  code = "Corruption02",
  group = "taxes",
  file = "newspaper/corruption02.md",
  score = function()
    return num(now() > OneYear) *
      get(StatFinesAndFeesRateStat) * 10
  end
});

addNewspaperArticle({
  code = "Corruption03",
  group = "taxes",
  file = "newspaper/corruption03.md",
  score = function()
    return num(now() > OneYear) *
      get(StatFinesAndFeesRateStat) * 10
  end
});

addNewspaperArticle({
  code = "Unemployment01",
  group = "economic",
  file = "newspaper/unemployment01.md",
  score = function()
    return get(StatUnemploymentRate) * 10 - 1
  end
});

addNewspaperArticle({
  code = "FullEmployment01",
  group = "economic",
  file = "newspaper/full-employment01.md",
  score = function()
    return num(get(StatUnemploymentRate) < 0.02) * 2
  end
});

addNewspaperArticle({
  code = "Baseball01",
  group = "sports",
  file = "newspaper/baseball01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Baseball02",
  group = "sports",
  file = "newspaper/baseball02.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Baseball03",
  group = "sports",
  file = "newspaper/baseball03.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Baseball04",
  group = "sports",
  file = "newspaper/baseball04.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Soccer01",
  group = "sports",
  file = "newspaper/soccer01.md",
  score = function()
    return 1;
  end;
})

addNewspaperArticle({
  code = "Soccer02",
  group = "sports",
  file = "newspaper/soccer02.md",
  score = function()
    return 1;
  end;
})

addNewspaperArticle({
  code = "Soccer03",
  group = "sports",
  file = "newspaper/soccer03.md",
  score = function()
    return num(amenitiesBuilt("stadium") > 0)
  end;
})

addNewspaperArticle({
  code = "Soccer04",
  group = "sports",
  file = "newspaper/soccer04.md",
  score = function()
    return num(amenitiesBuilt("stadium") > 0)
  end;
})

addNewspaperArticle({
  code = "Sandwich1",
  group = "fluff",
  file = "newspaper/sandwich1.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Balloons1",
  group = "fluff",
  file = "newspaper/balloons.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Cooler1",
  group = "fluff",
  file = "newspaper/cooler1.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Derail01",
  group = "fluff",
  file = "newspaper/derail01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Volcano01",
  group = "fluff",
  file = "newspaper/volcano01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Riots01",
  group = "fluff",
  file = "newspaper/riots01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "FactoryFire01",
  group = "fluff",
  file = "newspaper/factory-fire01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "ForestFire01",
  group = "fluff",
  file = "newspaper/forest-fire01.md",
  score = function()
    return 1;
  end
});

addNewspaperArticle({
  code = "Pundits01",
  group = "fluff",
  file = "newspaper/pundits01.md",
  score = function()
    return 1;
  end
});

-- Should be a late-game fluff article
addNewspaperArticle({
  code = "Scientist01",
  group = "fluff",
  file = "newspaper/scientist01.md",
  score = function()
    return num(get(StatNumGovBuildingsEnabled) > 50);
  end
})

-- Should be a late-game fluff article
addNewspaperArticle({
  code = "ForeignFood01",
  group = "fluff",
  file = "newspaper/foreign-food01.md",
  score = function()
    return num(get(StatNumGovBuildingsEnabled) > 35);
  end
})

addNewspaperArticle({
  code = "BigNews01",
  group = "fluff",
  file = "newspaper/big-news-day.md",
  score = function()
    return num(get(StatPopulation) > 20000);
  end
});

addNewspaperArticle({
  code = "Nothing01",
  group = "fluff",
  file = "newspaper/nothing.md",
  score = function()
    return 1;
  end
});

