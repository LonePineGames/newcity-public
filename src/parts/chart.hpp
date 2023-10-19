#pragma once

#include "part.hpp"

#include "../economy.hpp"

Part* chart(vec2 start, vec2 size, item econ, Statistic stat,
    item timePeriod, bool lbl, bool bar, float endDate);
Part* chart(vec2 start, vec2 size, item econ, Statistic stat, item timePeriod);
Part* chart(vec2 start, vec2 size, item econ, Statistic stat, item timePeriod,
    bool lbl, bool bar);

