Park Lots

![](docs/images/nature.png)

Park Lots allow you to designate large areas of green space in your city. Use the [[tools/toolZone|Zone Tool]] to place Park Lots. Park Lots do not spawn buildings.

Uncovered Park Lots will increase [[heatmaps/hmValue|![](IconLandValue)Value]] and reduce [[heatmaps/hmPollution|![](IconPollution)Pollution]]. Park Lots which are underneath buildings have no effect. 

##### Cost

* Each active Park Lot costs $formatMoney(inflate(CParkLotMaint*OneYear))$/year.
* You have $formatInt(get(StatParkLots))$ active Park Lots.
* Your Recreation budget control is set at $formatPercent(budgetControl(BudgetRecreationExpenses))$
* $formatMoney(inflate(CParkLotMaint*OneYear))$/year X $formatInt(get(StatParkLots))$ X $formatPercent(budgetControl(BudgetRecreationExpenses))$ = (drum-roll)
* #### $formatMoney(inflate(CParkLotMaint*OneYear)*get(StatParkLots)*budgetControl(BudgetRecreationExpenses))$/year (Estimated)

