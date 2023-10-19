NewCity LUA API - City

##### cityName() => string

Returns the player-configured city name as a string.

> cityName() => "$cityName()$"

---

##### randomName() => string

Returns a random citizen's name as a string.

> randomName() => "$randomName()$"

---

##### randomMansName() => string

Returns a random male citizen's name as a string.

> randomMansName() => "$randomMansName()$"

---

##### randomWomansName() => string

Returns a random female citizen's name as a string.

> randomWomansName() => "$randomWomansName()$"

---

##### randomBusinessName() => string

Returns a random business's name as a string.

> randomBusinessName() => "$randomBusinessName()$"

---

##### randomTeam() => string

Returns a random sports team name as a string.

> randomTeam() => "$randomTeam()$"
> (function() team1 = randomTeam(); return team1 end)() => "$(function() team1 = randomTeam(); return team1 end)()$"
> team1 => "$team1$"

---

