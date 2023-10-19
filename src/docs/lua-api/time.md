NewCity LUA API - Time

### Time-Related Functions

##### now() => number

Returns a time value representing the present moment.

Time is represented by the number of days since $formatDateTime(0)$. Hours and minutes are represented as a fraction within the day.

> formatDateTime(now()) => "$formatDateTime(now())$"
> now() => $now()$
> now() - math.floor(now()) => $now() - math.floor(now())$

---

##### timeNow() => number

Returns a time value representing the present moment, but only the time component and not the date component. Returns a number representing a fraction of a day.

> formatTime(timeNow()) => "$formatTime(timeNow())$"
> now() => $timeNow()$

---

##### year([number _time_]) => number

Returns the in-game year for optional _time_. If _time_ is ommitted, returns the current year.

---

##### setGameSpeed(number _speed_)

Sets the game speed. _speed_ must be a number between 0 and 7 inclusive. If _speed_ is zero, the game will be paused.

---

##### formatDuration(number _duration_) => string

Takes the time value _duration_ and returns a formatted string representing that value.

> formatDuration(OneYear) => "$formatDuration(OneYear)$"
> formatDuration(OneDay) => "$formatDuration(OneDay)$"
> formatDuration(OneDay\*.5) => "$formatDuration(OneDay*.5)$"
> formatDuration(OneHour\*2) => "$formatDuration(OneHour*2)$"
> formatDuration(OneHour\*.57) => "$formatDuration(OneHour*.57)$"

---

##### formatDurationNice(number _duration_) => string

Takes the time value _duration_ and returns a formatted string representing that value.

> formatDurationNice(OneYear) => "$formatDurationNice(OneYear)$"
> formatDurationNice(OneDay) => "$formatDurationNice(OneDay)$"
> formatDurationNice(OneDay\*.5) => "$formatDurationNice(OneDay*.5)$"
> formatDurationNice(OneHour\*2) => "$formatDurationNice(OneHour*2)$"
> formatDurationNice(OneHour\*.57) => "$formatDurationNice(OneHour*.57)$"

---

##### formatDateTime(number _dateTime_) => string

Takes the time value _dateTime_ and returns a formatted string representing that value.

> formatDateTime(now()) => "$formatDateTime(now())$"
> formatDateTime(0) => "$formatDateTime(0)$"
> formatDateTime(now()-OneYear) => "$formatDateTime(now()-OneYear)$"
> formatDateTime(now()-OneDay) => "$formatDateTime(now()-OneDay)$"
> formatDateTime(now()+OneYear\*10) => "$formatDateTime(now()+OneYear*10)$"

---

##### formatDate(number _date_) => string

Takes the time value _date_ and returns a formatted string representing that value.

> formatDate(now()) => "$formatDate(now())$"
> formatDate(0) => "$formatDate(0)$"
> formatDate(now()-OneYear) => "$formatDate(now()-OneYear)$"
> formatDate(now()-OneDay) => "$formatDate(now()-OneDay)$"

---

##### formatYear(number _date_) => string

> formatYear(now()) => "$formatYear(now())$"

---

##### formatSeason(number _date_) => string

Takes the time value _date_ and returns a formatted string representing that value.

> formatSeason(now()) => "$formatSeason(now())$"
> formatSeason(0) => "$formatSeason(0)$"
> formatSeason(now()-OneDay) => "$formatSeason(now()-OneDay)$"

---

##### formatTime(number _time_) => string

Takes the time value _time_ and returns a formatted string representing that value.

> formatTime(now()) => "$formatTime(now())$"
> formatTime(0) => "$formatTime(0)$"
> formatTime(OneHour\*9) => "$formatTime(OneHour*9)$"

---

### Time-Related Global Values

##### OneMinute

A time value representing one minute.

> formatDuration(OneMinute) => "$formatDuration(OneMinute)$"
> OneMinute => $OneMinute$

---

##### OneHour

A time value representing one hour.

> formatDuration(OneHour) => "$formatDuration(OneHour)$"
> OneHour => $OneHour$

---

##### OneDay

A time value representing one day.

> formatDuration(OneDay) => "$formatDuration(OneDay)$"
> OneDay => $OneDay$

---

##### OneYear

A time value representing one year.

> formatDuration(OneYear) => "$formatDuration(OneYear)$"
> OneYear => $OneYear$

---

