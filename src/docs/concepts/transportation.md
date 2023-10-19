Concept: Transportation

![](docs/images/transit01.png)

People live in cities so they can be closer to the things in the city, such as work or amenities. But if they can't get around, the benefits of living here are lost.

The most important metric for transportation is Average Travel Time.

![](Chart::StatAverageTripTime)

In NewCity, people are always engaged in an activity, or travelling to that activity. People have need to do many things:
* They go to work.
* They visit friends.
* They go to the store.
* They visit amenities such as parks.
* They go to the doctor when they are sick.
* And at the end of the day, they go home.

Moving around hundreds of thousands of people is no easy task! For your city to be successful, you have to have a transportation system which can move all of them, every day. In addition to roads, you can build expressways and train systems to increase $cityName()$'s transportation capacity.

[[Tutorial::Expressways00|![](docs/images/tutorial/expressways/expressways-poster.png)]]
[[Tutorial::Transit00|![](docs/images/tutorial/transit/transit-poster.png)]]

Before a person makes a trip, they estimate how long it will take to get there, and also how much it will cost. If mass transit is available, they compare the time and cost of going by bus and train vs car.

For cars, travellers consider time driving, including traffic, the cost of fuel, and the cost of the wear on their car. For transit, travellers consider time walking to the station or bus stop, time waiting for the train or bus, time riding, and the ticket cost.

> $CDifficultyLevelName$ Difficulty:
> $selectIf(CTransitBias > 1, "Because it's nice to not have to drive, people are willing to spend " .. formatPercent(CTransitBias) .. " as much time walking to, waiting for and riding transit, compared to time spent in cars.", selectIf(CTransitBias == 1, "People are equally willing to spend an hour driving as they are to spend an hour on transit.", "Because transit is less convenient than driving, people are only willing to spend " .. formatPercent(CTransitBias) .. " as much time walking to, waiting for and riding transit as they are willing to spend in traffic."))$

In $cityName()$ circa $formatYear(now())$, citizens value their time at $formatMoney(inflate(CValueOfTime))$ per hour, so they'll wait $formatDuration(1/(24*inflate(CValueOfTime)))$ to save one dollar.

Based on these factors, people choose to go by car or transit. They choose the faster and/or cheaper option, in their weighted estimate. You can discover how people are travelling around your city using the [[tools/toolQuery|Route Inspector]].

Bad traffic can strangle a city. If a trip will take more than $formatInt(CMaxCommute)$ hours, citizens will just stay home. If the trip was to work, they are fired and must look for another job. This can raise unemployment and stifle growth.

Transportation effects many heatmaps:
* Successful trips to and from boost the [[heatmaps/hmProsperity|![](IconTrade)Prosperity Heatmap]].
* Trains, buses, trucks, and cars travelling by boost local [[heatmaps/hmDensity|![](IconDensity)Density]] and ![](IconTrade)Prosperity.
  * Trains and buses boost ![](IconDensity)Density more than cars.
  * Vehicles on Expressways do not boost ![](IconDensity)Density or ![](IconTrade)Prosperity.
* Trains and buses passing by boost [[heatmaps/hmCommunity|![](IconFamily)Community]]
* All vehicles produce [[heatmaps/hmPollution|![](IconPollution)Pollution]].
* All vehicles reduce [[heatmaps/hmValue|![](IconLandValue)Value]].
  * Cars have a bigger effect on ![](IconLandValue)Value because there are more of them.

