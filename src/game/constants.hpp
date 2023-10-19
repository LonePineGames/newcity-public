#pragma once

const float tileSize = 25;
const float pi_o = 3.141592;

enum BoolConstant {
  #define B(N) N,
  #define I(N)
  #define F(N)
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F

  numBoolConstants
};

enum IntConstant {
  #define B(N)
  #define I(N) N,
  #define F(N)
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F

  numIntConstants
};

enum FloatConstant {
  #define B(N)
  #define I(N)
  #define F(N) N,
  #include "constantsList.hpp"
  #undef B
  #undef I
  #undef F

  numFloatConstants
};

bool c(BoolConstant c);
int c(IntConstant c);
float c(FloatConstant c);
void resetConstants();
void loadConstantsFromLua();

