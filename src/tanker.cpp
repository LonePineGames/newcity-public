#include "tanker.hpp"

#include "util.hpp"

item randomInTanker(Tanker* tanker) {
  item tS = tanker->elements.size();
  for (int i = 0; i < 10; i++) {
    item rnd = randItem(tS);
    item result = tanker->elements[rnd];
    if (result != 0) return result;
  }
  return 0;
}

void resetTanker(Tanker* tanker) {
  tanker->num = 0;
  tanker->elements.clear();
  tanker->gaps.clear();
  tanker->ndx.clear();
  tanker->elements.push_back(0); // Put something in the zero element
}

bool insertIntoTanker(Tanker* tanker, item element) {
  item ndx = 0;
  if (tanker->ndx.size() > element) {
    ndx = tanker->ndx[element];
    if (ndx != 0) {
      //#ifdef LP_DEBUG
        item val = tanker->elements[ndx];
        if (val != element) {
          handleError("Tanker corrupt: inserting %d at ndx %d, %d found",
              element, ndx, val);
        }
      //#endif
      return false;
    }
  }

  if (tanker->gaps.size() > 0) {
    item ndx = tanker->gaps.back();
    tanker->gaps.pop_back();
    tanker->elements.set(ndx, element);

  } else {
    ndx = tanker->elements.size();
    tanker->elements.push_back(element);
  }

  tanker->ndx.ensureSize(element+1);
  tanker->ndx.set(element, ndx);
  tanker->num ++;
  return true;
}

bool removeFromTanker(Tanker* tanker, item element) {
  if (tanker->ndx.size() > element) {
    item ndx = tanker->ndx[element];
    if (ndx != 0) {
      //#ifdef LP_DEBUG
        item val = tanker->elements[ndx];
        if (val != element) {
          handleError("Tanker corrupt: removing %d at ndx %d, %d found",
              element, ndx, val);
        }
      //#endif
      tanker->elements.set(ndx, 0);
      tanker->ndx.set(element, 0);
      tanker->gaps.push_back(ndx);
      tanker->num --;
      return true;
    }
  }

  return false;
}

