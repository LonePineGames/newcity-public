The player will get an algorithmically selected headline every game day (4 per year).
The headlines will appear on the right sidebar, along with the achievments and tracked items
The newspaper will appear in grey, to contrast with the green of the achivements and tracked items
The player can click the X to remove the headline. The game might automatically remove headlines as well.
There might be an interface to see the entire newspaper archives, going back to 1950
I estimate there needs to be at least 200 headlines, the more the better
We could go further and have entire 2-3 paragraph articles and multiple headlines per day
- Player would see one headline on the right, then click on it to see a dedicated newspaper interface with multiple articles
- But for now, let's just keep it to the headlines - minimum viable product

The goal with headline writing:
- fit a lot of information in a small space
- keep it under 10 words
- give players gameplay hints without breaking immersion
- should be realistic "newspaper speak" and not seem to be speaking directly to the player

It might be possible to make the headlines algorithmic:
  "{MoveOuts} families left {CityName} this year"
  - Use curly braces {} and CamelCase, make reasonable guesses at what values can be inserted, and I'll clean it up.

Questions:
  - Case: Title Case, ALL CAPS, or Sentence case?
  - How to choose which headline -- is economy more important than crime?

# Topics

## Economy
Sudden Stock Market drop
Sudden Stock Market gains (possible bubble talk)
Stock Market is flat, investors are anxious
Stock Market continues steady gains
- note: Stock Market is a bit of a red herring, unemployment is the real number
National Economy is about to turn bad
National Economy is bad, also local econ
National Economy is bad, but not local econ
National Economy is recovering, local econ is leading recovery
National Economy is recovering, local econ lags
National Economy is gaining steam
Local Economy is heating up
National Economy is amazing/unstoppable (bubble talk)
- In real life, these headlines would make frequent reference to "Wall Street" or "NYSE/Dow"
- We want the game to be vague about it's location (obviously it's America, but still keep the pretense)
- Therefore, refer to NSMI (National Stock Market Index) and "Capitol Street" or w/e
Unemployment woes ("Experienced College Grads still can't find work")
"There's a job for everyone who wants one!"
Companies raise wages, still can't find workers (connected to inflation)
Frustration with rising gas prices. Remember this is 19XX: "A dollar a gallon, are you serious?"
- This communicates inflation, which might help the player understand why roads cost twice as much in 1970 as they do in 1950
"Loaf of bread, 1950: 10 cents; 1970: 50 cents"
Talk of interest rates up/down/"inverted yeild curve", etc (connected with city debt burden)

## Budget & Amenities
Citizens moan about property tax
Businesses moan about sales tax
City is celebrated for low taxes (property/sales)
Citizens moan about fines and fees / "Police force/city beauracracy is very corrupt."
City is celebrated for lack of corruption (Need some way to connect this to fines and fees slider)
Fears about city debt burden
- I've been thinking about introducing a bankruptcy mechanic to give the player more choices, so maybe talk about this possibility
City celebrated for lack of debt burden
Citizens moan about schools (etc) being shut down
Citizens moan about disproportionate spending between parks, schools, and services (police/social services)
Celebrate new
  park (small, early game)
  grand park
  ballpark (little league/minor league team forms)
  big park
  public pool
  zoo
  pavilion (downtown park)
  stadium
  school (first school, possibly new schools)
  library
  science museum
  history museum
  nature museum
  art museum
  community college
  technical college
  convention center
  mayoral mansion
  city hall
  police station
  public housing
  social services
  jail
  courthouse
  clinic
  hospital

## City Growth / Degrowth
New Residential development (player built a lot of roads and zones)
Significant population growth
Population decline (people are moving out -- why?)
  No jobs (this is usually the real reason)
  Traffic, Pollution, Crime
  Taxes/Fees too high
  No place to live / rent too high
People want to move here, but no place to live / rent too high
New Commercial Development (Supermarket, Mall, Corporate Campus, Office Tower)
New Factory / Industrial Development

## City Challenges
Some pollution (city officials say it's fine)
Serious pollution / health hazard
Traffic ("Local deskjockey endures 90 minute commute")
Crime
  - This is a fun one, because we can tell stories
  - There should be one or more thematic crime stories that might develop over time.
  - Example: Local mobster "Big Joe" still on the loose / finally arrested / evades conviction and released / returns to crime
  - To simplify implementation, stories should be episodic. Therefore, Big Joe should never actually face justice.
  - I'd like at least one story about corporate/industrial corruption, which might tie into pollution, fines and fees, and economic issues.
  - Episodic story about a serial killer? (as distinct from a mobster)
  - One off stories about heinous crimes
  - Early game crime stories, petty theft
  - Crime statistics (X murders, Y carjackings) for later game

## Weather / Fluff
Serious rainstorm (some property damage -- maybe hint that there isn't a gameplay aspect to this)
Blizzard/"X feet of snow"
Heatwave
Plesant and mild spring/summer/fall weather
Seasonal events (Easter festival, Citizens have huge picnic, harvest festival, halloween, Christmas lights, etc)
Sports
  - Reporting on little league/minor league sports starts when player places ballpark (unlocked at 50,0000)
  - Reporting on major league sports starts at stadium, which I currently have at 1,000,000, therefore very late game
  - Reporting on major league sports in other cities ("The Capital")
National politics? (So and so wins election, talks of wars and foriegn policy, etc)
  - I've thought about introducing an election system to the game. I probably won't, but in case I do (future proofing):
  - There are four parties (maybe it's a parlimentary system)
  - Social party (social services, education, health)
  - Family party (social conservatives/crime focus, foil to the Social party)
  - Green party (anti-pollution, anti-industry/business, pro-parks)
  - Freedom party (anti-taxes, anti-debt, pro-business)
  - I'm okay with ommitting this entirely
Other fluff pieces
  - "Local artist creates stir"
  - "Schoolchild wins national competition"
  - appropriate to city size (small town => dog bites man, big city => man bites dog)


