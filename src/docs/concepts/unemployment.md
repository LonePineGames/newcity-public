Concept: Unemployment

![](newspaper/images/downtown05.png)

Economies rarely provide a job for everyone who wants one. The national economy ebbs and flows, and this effects your city. High taxes and transportation problems can make unemployment worse. Some Amenities can reduce the unemployment rate via the [[effects/business|![](IconZoneMonoOffice)Business Score]].

![](Chart::StatUnemploymentRate)


##### Causes of Unemployment
* The National Economy
> Currently, the National Unemployment Rate is $formatPercent(get(StatUnemploymentRate, NationalEcon))$, and that contributes to your city's unemployment rate. The national economy is not within your control.
* High taxes
> Sales tax has the biggest effect on unemployment. Property Tax and Fines and Fees also make unemployment worse.
* Traffic jams
> Currently it takes an average of $formatDurationNice(get(StatAverageTripTime))$ for a citizen to get to their job. Citizens will not tolerate a commute longer than $formatInt(CMaxCommute)$ hours.

[[Tutorial::Transit00|![](docs/images/tutorial/transit/transit-poster.png)]]

##### Effects of Unemployment
* Increased [[heatmaps/hmCrime|Crime]]
* Reduced [[heatmaps/hmProsperity|Prosperity]]
* Slowed [[basics/basicsCitizens|Population Growth]]
* Increased [[concepts/homeless|Homelessness]]

##### Preventing Unemployment
* Lower [[budget/budgettotalincome|Taxes]]
* Build Amenities which have points in [[effects/business|![](IconZoneMonoOffice)Business]], such as Tennis Courts, Opportunity Schools or Social Services. These amenities make your city a more attractive location for businesses large and small, and this reduces unemployment.

> Your Business Score does not reduce the negative impact of taxes on unemployment.

![](newspaper/images/queue01.png)

##### Technical Details

> There is a _Target Unemployment Rate_. The real Unemployment Rate moves towards the target over time, as people move in and out and get hired or fired.

![](Chart::StatTargetUnemploymentRate)
![](Chart::StatUnemploymentRate)

> The _Target Unemployment Rate_ is calculed based on the national economy, taxes, and your ![](IconZoneMonoOffice)Business Score, as explained above.

[[index|Back to Index]]

