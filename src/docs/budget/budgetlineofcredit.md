Budget Line: Line of Credit

![](docs/images/lineofcredit.png)

The amount that the bank is willing to lend the city.

![](Chart::StatLineOfCreditStat)

The Line of Credit, or LoC, is calculated based on the following factors:

* [[budget/budgettotalearnings|Total Earnings]]
> Accountants estimate your Total Earnings for the next year (year+1). You are estimated to $selectIf(budgetLine(BudgetTotalEarnings, 2) < 0, "lose", "make")$ $formatMoney(math.abs(budgetLine(BudgetTotalEarnings, 2)))$ in $formatYear(now()+OneYear)$.

* [[budget/budgetassets|Assets]]
> The value of all your amenities, were you to sell them, is taken as collateral for the loan. Your assets are currently valued at $formatMoney(budgetLine(BudgetAssets, 0)*CAssetLOC)$.

* Loan Term
> If you take out a longer-term loan, there is more time to earn money to pay it off, so you can take a larger loan. However, you will be charged a higher interest rate.
> ![](Chart::StatNationalInterestRate:National)

* National Economy
> The liquidity of the nation's financial system can play a role in your Line of Credit.

You must [[budget/budgetloaninterest|pay interest]] on your loan every year.

See also [[budget/budgetcashtospend|Cash to Spend]] and [[budget/budgetbalance|Budget Balance]].

