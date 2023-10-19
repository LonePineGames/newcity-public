#include "constants.hpp"

#include "../platform/lua.hpp"

#include "spdlog/spdlog.h"

//#define C(NAME, DEFAULT_VALUE) setConstant(L, NAME, #NAME, (DEFAULT_VALUE))

const char* const boolConstantNames[numBoolConstants] = {
  #define B(N) #N,
  #define I(N)
  #define F(N)
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F
};


const char* const intConstantNames[numIntConstants] = {
  #define B(N)
  #define I(N) #N,
  #define F(N)
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F
};


const char* const floatConstantNames[numFloatConstants] = {
  #define B(N)
  #define I(N)
  #define F(N) #N,
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F
};

const int boolConstantsTableSize = (numBoolConstants+1)/8 + 1;
char boolConstants[boolConstantsTableSize];
int intConstants[numIntConstants];
float floatConstants[numFloatConstants];

bool c(BoolConstant c) {
  return boolConstants[c/8] & (1 << (c%8));
}

void setConstant(BoolConstant c, bool val) {
  if (val) {
    boolConstants[c/8] |= (1 << (c%8));
  } else {
    boolConstants[c/8] &= ~(1 << (c%8));
  }
}

int c(IntConstant c) {
  return intConstants[c];
}

void setConstant(IntConstant c, int val) {
  intConstants[c] = val;
}

float c(FloatConstant c) {
  return floatConstants[c];
}

void setConstant(FloatConstant c, float val) {
  floatConstants[c] = val;
}

void setConstantFromLua(BoolConstant c) {
  setConstant(c, getBoolFromLua(boolConstantNames[c]));
}

void setConstantFromLua(IntConstant c) {
  setConstant(c, getValueFromLua(intConstantNames[c]));
}

void setConstantFromLua(FloatConstant c) {
  setConstant(c, getValueFromLua(floatConstantNames[c]));
}

void resetConstants() {
  SPDLOG_INFO("resetConstants");

  for (int i = 0; i < numBoolConstants; i++) {
    setConstant((BoolConstant)i, 0);
  }

  for (int i = 0; i < numIntConstants; i++) {
    setConstant((IntConstant)i, 0);
  }

  for (int i = 0; i < numFloatConstants; i++) {
    setConstant((FloatConstant)i, 0);
  }
}

void loadConstantsFromLua() {
  SPDLOG_INFO("loadConstantsFromLua");
  for (int i = 0; i < boolConstantsTableSize; i++) {
    boolConstants[i] = 0;
  }

  for (int i = 0; i < numBoolConstants; i++) {
    setConstantFromLua((BoolConstant)i);
  }

  for (int i = 0; i < numIntConstants; i++) {
    setConstantFromLua((IntConstant)i);
  }

  for (int i = 0; i < numFloatConstants; i++) {
    setConstantFromLua((FloatConstant)i);
  }
}

