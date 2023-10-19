Lua API - Utilities

##### formatInt(number _value_) => string

Takes _value_ and returns a formatted string representing that value as a whole integer. Any fractional value is truncated. Negative values are truncated towards zero.

> formatInt(15000) => "$formatInt(15000)$"
> formatInt(1.15) => "$formatInt(1.15)$"
> formatInt(-1.15) => "$formatInt(-1.15)$"

---

##### formatFloat(number _value_) => string

Takes _value_ and returns a formatted string representing that value. This is the default way that numbers are formatted.

> formatFloat(15000) => "$formatFloat(15000)$"
> formatFloat(0.15) => "$formatFloat(0.15)$"
> formatFloat(-0.15) => "$formatFloat(-0.15)$"

---

##### formatPercent(number _value_) => string

Takes _value_ and returns a formatted string representing that value as a percentage or multiplier.

> formatPercent(0.15) => "$formatPercent(0.15)$"
> formatPercent(-0.15) => "$formatPercent(-0.15)$"
> formatPercent(15) => "$formatPercent(15)$"

---

##### randomInt(number _min_, number _max_) => string

Takes _min_ and _max_ and returns a random integer number between _min_ and _max_. The results are inclusive of _min_ and exclusive of _max_. In other words, _min_ may be returned but _max_ never will be, unless _min_ == _max_.

> randomInt(0,10) => $randomInt(0,10)$
> randomInt(1,10) => $randomInt(1,10)$
> randomInt(0,11) => $randomInt(0,11)$

---

##### randomFloat([number _min_=0, number _max_=1]) => string

Takes _min_ and _max_ and returns a random number between _min_ and _max_. The results are inclusive of _min_ and exclusive of _max_. In other words, _min_ may be returned but _max_ never will be, unless _min_ == _max_.

If called with no parameters, randomFloat() returns a number between 0 and 1.

> randomFloat(0,10) => $randomFloat(0,10)$
> randomFloat() => $randomFloat()$

---

##### selectIf(boolean|number _condition_, any _option1_, any _option2_) => any

Returns _option1_ if _condition_ is true or nonzero. Returns _option2_ otherwise.

> selectIf(true, "A", "B") => $selectIf(true, "A", "B")$
> selectIf(0, "A", "B") => $selectIf(0, "A", "B")$
> You have \$selectIf(get(StatAbandonedRetailBuildings) > 20, "more", "less")\$ than 20 abandoned retail buildings. =>
> You have $selectIf(get(StatAbandonedRetailBuildings) > 20, "more", "less")$ than 20 abandoned retail buildings.

---

##### selectRandom(any _option1_, any _option2_, ...) => any

Randomly returns one of the parameters. Can take any number of parameters.

> selectRandom("A", "B") => "$selectRandom("A", "B")$"
> selectRandom("A", "B", "C") => "$selectRandom("A", "B", "C")$"
> I really love \$selectRandom("swimming", "riding my bike")\$! => I really love $selectRandom("swimming", "riding my bike")$!

---

##### sampleBlueNoise(table _sample_)

Takes _sample_, which should have values _x_ and _y_. Returns a random 3d vector (as a table), with elements _x_, _y_, _z_. Each element is between 0 and 1, based on the color values of textures/blue-noise.png.

sampleBlueNoise({x=0.5,y=0.5}).y => $sampleBlueNoise({x=0.5,y=0.5}).y$

---

