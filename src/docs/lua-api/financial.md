NewCity LUA API - Financial

### Financial Functions

##### inflate(number _uninflated_) => number

Takes the money value _uniflated_ and returns the inflated value.

Inflation is measured relative to 1950 dollars. In the real world, prices in US dollars in 2019 were approximately ten times higher than they were in 1950. So a bridge that would cost \$100M today would have cost \$10M in 1950. Inflation is random in NewCity, but generally prices double every two decades.

> inflate(150) => $inflate(150)$

---

##### formatMoney(number _money_) => string

Takes the money value _money_ and returns a formatted string representing that value.

> formatMoney(15000) => "$formatMoney(15000)$"
> formatMoney(-15000000) => "$formatMoney(-15000000)$"
> formatMoney(0.15) => "$formatMoney(0.15)$"
> formatMoney(inflate(150)) => "$formatMoney(inflate(150))$"

---

##### budgetLine(BudgetLine _line_, [number _budget_=0]) => number

Returns the value for BudgetLine _line_.

By default, returns the value for the current year-to-date budget. If _budget_ = 1, will return the value for the current year estimate budget. If _budget_ = 2, will return the value for next year's estimate budget. If _budget_ is negative, will return the value a past year's budget, with -1 being last year, -2 the year before, and so on. If no such budget exists, budgetLine() returns 0.

> budgetLine(BudgetCashflow) => $budgetLine(BudgetCashflow)$
> budgetLine(BudgetBalance, -1) => $budgetLine(BudgetBalance, -1)$
> budgetLine(BudgetPropertyTax, 1) => $budgetLine(BudgetPropertyTax, 1)$
> budgetLine(BudgetBalance, 2) => $budgetLine(BudgetBalance, 2)$

---

##### canBuy(number _amount_) => boolean

Returns true if the player can afford _amount_ as a discretionary expense.

Make sure to inflate _amount_ before passing it to canBuy().

> canBuy(1000\*1000) => $canBuy(1000*1000)$
> canBuy(1000\*1000\*1000) => $canBuy(1000*1000*1000)$
> canBuy(0) => $canBuy(0)$

---

##### transaction(BudgetLine _line_, number _amount_) => boolean

Performs a transaction against the current YTD budget, under BudgetLine _line_. If _amount_ is positive, the transaction is income. If _amount_ is negative, the transaction is an expense.

If _amount_ is negative, _line_ is discretionary, and the player cannot afford it, the transaction is not applied and transaction() returns false.

Make sure to inflate _amount_ before passing it transaction().

---

##### budgetControl(BudgetLine _line_) => number

Returns the current value of the numerical control associated with _line_. This means different things for different budget lines:
* For [[budget/budgettotalincome|Taxes]], it represents the tax rate.
* For [[budget/budgettotalmandatory|Mandatory Expenses]], it represents the funding percentage.
* For [[budget/budgetlineofcredit|Line of Credit]], it represents the loan term in years.
* For other budget lines, 0 is returned.

> formatPercent(budgetControl(BudgetPropertyTax)) => $formatPercent(budgetControl(BudgetPropertyTax))$ -- Proprety Tax Rate
> formatPercent(budgetControl(BudgetSalesTax)) => $formatPercent(budgetControl(BudgetSalesTax))$ -- Sales Tax Rate
> formatPercent(budgetControl(BudgetFinesAndFeesIncome)) => $formatPercent(budgetControl(BudgetFinesAndFeesIncome))$ -- Fines and Fees Rate
> formatPercent(budgetControl(BudgetEducationExpenses)) => $formatPercent(budgetControl(BudgetEducationExpenses))$ -- Percentage Funding for mandatory expenses
> formatPercent(budgetControl(BudgetRecreationExpenses)) => $formatPercent(budgetControl(BudgetRecreationExpenses))$
> formatPercent(budgetControl(BudgetServicesExpenses) => $formatPercent(budgetControl(BudgetServicesExpenses))$
> formatPercent(budgetControl(BudgetUniversityExpenses)) => $formatPercent(budgetControl(BudgetUniversityExpenses))$
> formatFloat(budgetControl(BudgetLineOfCredit)) => $formatFloat(budgetControl(BudgetLineOfCredit))$ -- Loan Term in Years

---

##### setBudgetControl(BudgetLine _line_, number _value_)

Sets the value of the numerical control associated with _line_.

---

### Budget Lines

* BudgetNull
* BudgetPropertyTax
* BudgetSalesTax
* BudgetFinesAndFeesIncome
* BudgetParkIncome
* BudgetAmenityIncome
* BudgetTransitIncome
* BudgetAssetSalesIncome
* BudgetRoadBuildExpenses
* BudgetExpwyBuildExpenses
* BudgetTransitBuildExpenses
* BudgetPillarBuildExpenses
* BudgetEminentDomainExpenses
* BudgetRepairExpenses
* BudgetBuildingBuildExpenses
* BudgetMiscDiscExpenses
* BudgetEducationExpenses
* BudgetRecreationExpenses
* BudgetServicesExpenses
* BudgetUniversityExpenses
* BudgetTransitExpenses
* BudgetTotalIncome
* BudgetTotalExpenses
* BudgetTotalDiscretionary
* BudgetTotalMandatory
* BudgetTotalEarnings
* BudgetAssets
* BudgetLineOfCredit
* BudgetLoanInterest
* BudgetCashflow
* BudgetBalance

### Financial Statistics

##### StatCityBondRate
> formatPercent(get(StatCityBondRate)) => $formatPercent(get(StatCityBondRate))$

##### StatPropertyTaxRateStat
> formatPercent(get(StatPropertyTaxRateStat)) => $formatPercent(get(StatPropertyTaxRateStat))$

##### StatSalesTaxRateStat
> formatPercent(get(StatSalesTaxRateStat)) => $formatPercent(get(StatSalesTaxRateStat))$

##### StatFinesAndFeesRateStat
> formatPercent(get(StatFinesAndFeesRateStat)) => $formatPercent(get(StatFinesAndFeesRateStat))$

##### StatPropertyTaxStat
> formatMoney(get(StatPropertyTaxStat)) => $formatMoney(get(StatPropertyTaxStat))$

##### StatSalesTaxStat
> formatMoney(get(StatSalesTaxStat)) => $formatMoney(get(StatSalesTaxStat))$

##### StatFinesAndFeesStat
> formatMoney(get(StatFinesAndFeesStat)) => $formatMoney(get(StatFinesAndFeesStat))$

##### StatParkIncomeStat
> formatMoney(get(StatParkIncomeStat)) => $formatMoney(get(StatParkIncomeStat))$

##### StatAmenityIncomeStat
> formatMoney(get(StatAmenityIncomeStat)) => $formatMoney(get(StatAmenityIncomeStat))$

##### StatTransitIncomeStat
> formatMoney(get(StatTransitIncomeStat)) => $formatMoney(get(StatTransitIncomeStat))$

##### StatAssetSalesIncomeStat
> formatMoney(get(StatAssetSalesIncomeStat)) => $formatMoney(get(StatAssetSalesIncomeStat))$

##### StatRoadBuildExpensesStat
> formatMoney(get(StatRoadBuildExpensesStat)) => $formatMoney(get(StatRoadBuildExpensesStat))$

##### StatExpwyBuildExpensesStat
> formatMoney(get(StatExpwyBuildExpensesStat)) => $formatMoney(get(StatExpwyBuildExpensesStat))$

##### StatRailBuildExpensesStat
> formatMoney(get(StatRailBuildExpensesStat)) => $formatMoney(get(StatRailBuildExpensesStat))$

##### StatPillarBuildExpensesStat
> formatMoney(get(StatPillarBuildExpensesStat)) => $formatMoney(get(StatPillarBuildExpensesStat))$

##### StatEminentDomainExpensesStat
> formatMoney(get(StatEminentDomainExpensesStat)) => $formatMoney(get(StatEminentDomainExpensesStat))$

##### StatRepairExpensesStat
> formatMoney(get(StatRepairExpensesStat)) => $formatMoney(get(StatRepairExpensesStat))$

##### StatBuildingBuildExpensesStat
> formatMoney(get(StatBuildingBuildExpensesStat)) => $formatMoney(get(StatBuildingBuildExpensesStat))$

##### StatMiscDiscExpensesStat
> formatMoney(get(StatMiscDiscExpensesStat)) => $formatMoney(get(StatMiscDiscExpensesStat))$

##### StatEducationExpensesStat
> formatMoney(get(StatEducationExpensesStat)) => $formatMoney(get(StatEducationExpensesStat))$

##### StatRecreationExpensesStat
> formatMoney(get(StatRecreationExpensesStat)) => $formatMoney(get(StatRecreationExpensesStat))$

##### StatServicesExpensesStat
> formatMoney(get(StatServicesExpensesStat)) => $formatMoney(get(StatServicesExpensesStat))$

##### StatUniversityExpensesStat
> formatMoney(get(StatUniversityExpensesStat)) => $formatMoney(get(StatUniversityExpensesStat))$

##### StatTransitExpensesStat
> formatMoney(get(StatTransitExpensesStat)) => $formatMoney(get(StatTransitExpensesStat))$

##### StatTotalIncomeStat
> formatMoney(get(StatTotalIncomeStat)) => $formatMoney(get(StatTotalIncomeStat))$

##### StatTotalExpensesStat
> formatMoney(get(StatTotalExpensesStat)) => $formatMoney(get(StatTotalExpensesStat))$

##### StatTotalDiscretionaryStat
> formatMoney(get(StatTotalDiscretionaryStat)) => $formatMoney(get(StatTotalDiscretionaryStat))$

##### StatTotalMandatoryStat
> formatMoney(get(StatTotalMandatoryStat)) => $formatMoney(get(StatTotalMandatoryStat))$

##### StatTotalEarningsStat
> formatMoney(get(StatTotalEarningsStat)) => $formatMoney(get(StatTotalEarningsStat))$

##### StatAssetsStat
> formatMoney(get(StatAssetsStat)) => $formatMoney(get(StatAssetsStat))$

##### StatLineOfCreditStat
> formatMoney(get(StatLineOfCreditStat)) => $formatMoney(get(StatLineOfCreditStat))$

##### StatLoanInterestStat
> formatMoney(get(StatLoanInterestStat)) => $formatMoney(get(StatLoanInterestStat))$

##### StatCashflowStat
> formatMoney(get(StatCashflowStat)) => $formatMoney(get(StatCashflowStat))$

##### StatBudgetBalanceStat
> formatMoney(get(StatBudgetBalanceStat)) => $formatMoney(get(StatBudgetBalanceStat))$

##### StatCashAvailableStat
> formatMoney(get(StatCashAvailableStat)) => $formatMoney(get(StatCashAvailableStat))$


