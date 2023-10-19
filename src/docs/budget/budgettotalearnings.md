Budget Line: Total Earnings

![](docs/images/earnings.png)

[[budget/budgettotalincome|Total Income]] minus [[budget/budgettotalmandatory|Total Mandatory Expenses]]. Does not include [[budget/budgettotaldiscretionary|discretionary spending]] or [[budget/budgetloaninterest|Loan Interest]]. If Total Earnings is negative, you are losing money.

Accountants estimate your Total Earnings for the next year (year+1). You are estimated to $selectIf(budgetLine(BudgetTotalEarnings, 2) < 0, "lose", "make")$ $formatMoney(math.abs(budgetLine(BudgetTotalEarnings, 2)))$ in $formatYear(now()+OneYear)$.

![](Chart::StatTotalEarningsStat)

