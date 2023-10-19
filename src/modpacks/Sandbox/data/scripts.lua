
on(Load, function()
  for feature=0,NumFeatures do
    setFeatureEnabled(feature, true)
  end
end)

on(EconomyUpdate, function()
  set(StatResidentialZoneDemand, 1)
  set(StatRetailZoneDemand, 1)
  set(StatFarmZoneDemand, 1)
  set(StatOfficeZoneDemand, 1)
  set(StatFactoryZoneDemand, 1)
  set(StatMixedUseZoneDemand, 1)

  local balance = budgetLine(BudgetBalance)
  local oneBillion = 1000*1000*1000
  if balance < 2*oneBillion then
    transaction(BudgetAssetSalesIncome, 2*oneBillion - balance)
  end
end)

