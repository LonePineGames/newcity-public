#include "chart.hpp"

#include "../color.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"

#include "button.hpp"
#include "block.hpp"
#include "label.hpp"
#include "panel.hpp"

#include "spdlog/spdlog.h"

static const float scl = 0.8;

static const float trillion = 1000000000000.f;
static const float billion  = 1000000000.f;
static const float million  = 1000000.f;
static const float thousand = 1000.f;

char* formatValue(Statistic stat, float val, bool asPercent) {
  if (isStatDuration(stat)) {
    return printNiceDurationString(val);
  }

  const char* negSign = " ";
  if (val < -0.00001f) {
    negSign = "-";
    val = -val;
  }

  if (asPercent) {
    if (val >= 0.1) {
      return sprintf_o("%s%3d%%", negSign, (int)(val*100));
    } else {
      return sprintf_o("%s%3.2f%%", negSign, val*100);
    }
  } else if (val >= trillion*10) {
    return sprintf_o("%s%3dT", negSign, (int)(val/trillion));
  } else if (val >= trillion) {
    return sprintf_o("%s%1.1fT", negSign, val/trillion);
  } else if (val >= billion*10) {
    return sprintf_o("%s%3dB", negSign, (int)(val/billion));
  } else if (val >= billion) {
    return sprintf_o("%s%1.1fB", negSign, val/billion);
  } else if (val >= million*10) {
    return sprintf_o("%s%3dM", negSign, (int)(val/million));
  } else if (val >= million) {
    return sprintf_o("%s%1.1fM", negSign, val/million);
  } else if (val >= thousand*10) {
    return sprintf_o("%s%3dK", negSign, (int)(val/thousand));
  } else if (val >= thousand) {
    return sprintf_o("%s%1.1fK", negSign, val/thousand);
  } else if (val >= 100) {
    return sprintf_o("%s%3d", negSign, (int)val);
  } else if (val >= 10) {
    return sprintf_o("%s%2.1f", negSign, val);
  } else {
    return sprintf_o("%s%1.2f", negSign, val);
  }
}

void makeNotchline(Part* result, vec2 start, vec3 notch, float size,
    Statistic stat, float val, bool asPercent, const char* pre) {

  notch.y -= 0.025;
  Part* lineBlk = brightBox(vec2(notch), vec2(notch.z, 0.05));
  lineBlk->dim.start.z += 0.2;
  r(result, lineBlk);

  /*
  notch.y -= 0.05;
  lineBlk = darkBlock(vec2(notch), vec2(notch.z, 0.2));
  lineBlk->dim.start.z += 0.2;
  r(result, lineBlk);
  */

  char* temp = formatValue(stat, val, asPercent);
  char* text = sprintf_o("%s%s", pre, temp);
  free(temp);
  r(result, labelCenter(start, vec2(1, size), text));
}

Part* chart(vec2 start, vec2 size, item econ, Statistic stat,
    item timePeriod, bool lbl, bool bar, float endDate) {
  Part* result = panel(start, size);
  result->flags |= _partLowered | _partClip;

  float inX = 0; //.1f;
  float inY = 1;
  float inW = size.x;// - 0.2f;
  float inH = size.y - 1.f;
  float labScl = 0.65;

  if (stat >= numStatistics || stat < 0 || !timeSeriesHasData(econ, stat) ||
      timePeriod < 0 || timePeriod >= numTimePeriods) {
    if (bar) {
      float ySize;
      char* lblText = sprintf_o("%s NO DATA", statName(stat));
      r(result, multiline(vec2(.1f,.1f), vec2(size.x-1.5, labScl),
          lblText, &ySize));
    } else {
      r(result, labelCenter(vec2(inX, inY + (size.y-scl-inY)*.5f),
            vec2(size.x-.2f, scl), strdup_s("NO DATA")));
    }
    return result;
  }

  TimeSeries series = getTimeSeries(econ, stat);
  int numValues = series.values.size();
  int endT = (endDate - series.startDate) / series.timeStep;
  if (endT > numValues) endT = numValues;
  if (endT < 0) endT = 0;
  int startT = timePeriod == 0 ? 0 :
    (endT - timePeriodLength[timePeriod] / series.timeStep);
  if (startT < 0) startT = 0;

  int stride = (endT-startT) / (25 * size.x);
  if (stride < 1) stride = 1;
  if (stride > 1) startT = stride*(startT/stride);
  if (startT < 0) startT = 0;

  const float lineWidth = 0.05;
  float lastTime = series.startDate + series.timeStep * endT;
  float timeShift = 0; //(getCurrentDateTime() - lastTime) / series.timeStep;
  float colWidth;
  float scrollX;
  if (startT <= 0 || startT - timeShift <= 0) {
    colWidth = inW / endT;
    scrollX = 0;
  } else {
    colWidth = inW / (endT - startT);
    scrollX = colWidth * timeShift;
  }

  float min = FLT_MAX;
  float max = -FLT_MAX;
  float minPre = FLT_MAX;
  float maxPre = -FLT_MAX;
  float minPost = FLT_MAX;
  float maxPost = -FLT_MAX;
  for (int i = startT-stride*2; i < endT; i+=stride) {
    if (i < 0) i = 0;
    float val = 0;
    int j = 0;
    for (; j < stride && i+j < endT; j++) {
      val += convertStatistic(stat, series.values[i + j]);
    }
    val /= j;
    if (i+stride <= endT) {
      if (val < minPre) minPre = val;
      if (val > maxPre) maxPre = val;
    }
    //if (i >= startT) {
      if (val < minPost) minPost = val;
      if (val > maxPost) maxPost = val;
    //}
  }

  min = mix(minPre, minPost, timeShift);
  max = mix(maxPre, maxPost, timeShift);
  float currentVal = isStatRateCounter(stat) || endDate < getCurrentDateTime()-0.001 ? series.values[endT-1] : getStatistic(econ, stat);
  float current = convertStatistic(stat, currentVal);
  min = std::min(min, current);
  max = std::max(max, current);
  float realMin = min;

  if (min > max || abs(max-min) < 0.01) {
    min = max * .5f;
  } else if (min > 0) {
    if (abs(max-min) < 0.2*max) {
      min = max*.8f;
    } else {
      min -= (max-min)*0.1;
      min = clamp(min, 0.f, max);
    }
  }

  float currentFraction = 1 - (current - min) / (max - min);
  float labelLocInner = inX + (inW-1)*.5f;
  float labelLocOuter = inX + inW - 2;
  bool asPercent = (max <= 2 && min >= -1) && stat != FuelPrice &&
    stat != TouristRating;

  if (lbl) {
    float ySize;
    char* lblText;
    if (bar) {
      char* valTxt = formatValue(stat, current, asPercent);
      lblText = sprintf_o("%s  %s", statName(stat), valTxt);
      free(valTxt);
    } else {
      lblText = strdup_s(statName(stat));
    }

    r(result, multiline(vec2(.1f,.1f), vec2(size.x-1.5, labScl),
        lblText, &ySize));
  }

  if (!bar && min > -0.02 && max < 0.002) {
    r(result, labelCenter(vec2(inX, inY + (size.y-scl-inY)*.5f),
          vec2(size.x-.2f, scl), strdup_s("NO DATA")));
    return result;
  }

  if (bar) {
    if (current >= 0 && min < 0) min = 0;
    if (current < 0 && max > 0) max = 0;
    double frac = (current-min)/(max-min);
    float xLoc = size.x * frac;
    Part* blk;
    if (current >= 0) {
      blk = r(result, gradientBlock(vec2(0, 0), vec2(xLoc, size.y),
        colorGoldGrad1, colorGoldGrad0));
    } else {
      blk = r(result, gradientBlock(vec2(xLoc, 0), vec2(size.x-xLoc, size.y),
        colorRedGrad0, colorRedGrad1));
    }
    blk->dim.start.z -= 0.5;
    return result;
  }

  makeNotchline(result, vec2(1, inY),
      vec3(0, inY, 3.f),
      labScl, stat, max, asPercent, "Max ");
  float minY = inY + (1-(realMin-min)/(max-min))*inH;
  makeNotchline(result, vec2(1, minY-labScl),
      vec3(0, minY, 3.f),
      labScl, stat, realMin, asPercent, "Min ");
  float nowY = inY+inH*currentFraction;
  makeNotchline(result, vec2(labelLocOuter, inY+currentFraction*(inH-labScl)),
      vec3(labelLocOuter-1, nowY, 3.f),
      labScl, stat, current, asPercent, "Now ");

  float yrLblY = size.y - .5f;
  while (abs(minY-yrLblY) < .75f || abs(nowY-yrLblY) < .75f) {
    yrLblY -= .25f;
  }

      /*
        Part* lineBlk = block(vec2(xLoc, 1.f), vec2(lineWidth, size.y));
        lineBlk->dim.start.z += 0.25;
        r(result, lineBlk);
  r(result, label(vec2(labelLocInner, inY), labScl,
    formatValue(stat, max, asPercent)));
  r(result, label(vec2(labelLocInner, inY+inH-labScl), labScl,
    formatValue(stat, min, asPercent)));

  r(result, label(vec2(labelLocOuter, inY+currentFraction*(inH-labScl)), labScl,
    formatValue(stat, current, asPercent)));
  */

  float time = series.startDate + startT * series.timeStep;
  float lastYearX = -20;
  float prevFraction = 1 - (series.values[startT] - min) / (max - min);
  float xYDiff = colorGoldGrad0.y - colorGoldGrad1.y;
  float zeroFrac = 1 + min / (max - min);
  float clampedZeroFrac = clamp(zeroFrac, 0.001f, 1.f);
  float zeroY = inH * zeroFrac;
  int year = series.startDate/oneYear-2;

  for (int i = startT-stride*2-1; i < endT; i+=stride) {
    if (i < 0) i = 0;
    float xLoc = inX + (i-startT)*colWidth - scrollX;
    float val = 0;

    int j = 0;
    for (; j < stride && i+j < endT; j++) {
      val += convertStatistic(stat, series.values[i + j]);
    }
    val /= j;

    float fraction = 1 - (val - min) / (max - min);
    if ((prevFraction < zeroFrac) != (fraction < zeroFrac) &&
        prevFraction != zeroFrac && fraction != zeroFrac) {
      prevFraction = zeroFrac;
    }

    float fracDiff = prevFraction - fraction;
    float yStart, yEnd;
    vec3 colr, endColr, vecData;

    if (fraction < zeroFrac) {
      yStart = inH*fraction;
      yEnd = zeroY - yStart;
    } else  {
      yStart = zeroY;
      yEnd = inH*fraction - zeroY;
    }

    if (yStart < -inY) yStart = -inY;
    if (yStart + yEnd > size.y) yEnd = size.y - yStart;

    if (fraction < zeroFrac ||
        (fraction == zeroFrac && prevFraction < zeroFrac)) {
      endColr = colorGoldGrad0;
      float colorFrac = 1-(clampedZeroFrac - fraction) / clampedZeroFrac;
      float prevColorFrac = 1-
        (clampedZeroFrac - prevFraction) / clampedZeroFrac;
      colr = mix(colorGoldGrad1, colorGoldGrad0, colorFrac);
      vecData = vec3((prevColorFrac-colorFrac)*xYDiff, fracDiff*inH, 1);

    } else  {
      colr = colorRedGrad0;
      float colorFrac = (fraction - clampedZeroFrac) / (1-clampedZeroFrac);
      float prevColorFrac = (prevFraction - clampedZeroFrac)
        / (1-clampedZeroFrac);
      endColr = mix(colorRedGrad0, colorRedGrad1, colorFrac);
      vecData = vec3((colorFrac-prevColorFrac)*xYDiff, fracDiff*inH, -1);
    }

    float yLoc = inY + yStart;
    Part* blk = gradientBlock(vec2(xLoc, yLoc), vec2(colWidth*j, yEnd),
          colr, endColr);
    blk->renderMode = RenderGradientAdjusted;
    if (i == 0) {
      blk->dim.start.x -= inX;
      blk->dim.end.x += inX;
    }
    blk->vecData = vecData;
    r(result, blk);

    /*
    for (int i = 0; i < 3; i++) {
      float frac = i / 2.f;
      if (abs(frac - fraction) > 0.1 && abs(frac - prevFraction) > 0.1)
        continue;
      Part* lineBlk = block(vec2(xLoc-0.1, inY + inH*frac - lineWidth*.5f),
          vec2(.2, lineWidth));
      lineBlk->dim.start.z += 0.25;
      r(result, lineBlk);
    }
    */

    if (timePeriodLength[timePeriod] < 5) {
      float timeOfDay = time - (int)time;
      if (timeOfDay < 6*oneHour || timeOfDay > 18*oneHour) {
        Part* shade = darkBlock(vec2(xLoc, 0), vec2(colWidth*stride, size.y));
        shade->dim.start.z -= 0.5;
        r(result, shade);
      }
    }

    time += series.timeStep * stride;
    int newYear = time/oneYear;
    if (newYear > year) {
      year = newYear;
      if (xLoc < 0) xLoc = 0;
      if (xLoc >= lastYearX+1.1) {
        Part* lineBlk = brightBox(vec2(xLoc, 1.f), vec2(lineWidth, size.y));
        lineBlk->dim.start.z += 0.2;
        r(result, lineBlk);
        vec2 yrLblLoc(xLoc + lineWidth*2, yrLblY-.5f);
        if (yrLblLoc.x > inW-1) {
          yrLblLoc = vec2(xLoc-1.1f, yrLblY);
        }
        Part* yearLabel = label(yrLblLoc, .5f,
              sprintf_o("%d", int(year + c(CStartYear))));
        yearLabel->padding = 0.05;
        r(result, yearLabel);
        lastYearX = xLoc;
      }
    }

    prevFraction = fraction;
  }

  return result;
}

Part* chart(vec2 start, vec2 size, item econ, Statistic stat,
    item timePeriod, bool lbl, bool bar) {
  return chart(start, size, econ, stat, timePeriod, lbl, bar, getCurrentDateTime());
}

Part* chart(vec2 start, vec2 size, item econ, Statistic stat, item timePeriod) {
  return chart(start, size, econ, stat, timePeriod, false, false);
}

