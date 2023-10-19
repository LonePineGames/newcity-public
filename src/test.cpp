#include "draw/entity.hpp"
#include "land.hpp"
#include "name.hpp"
#include "game/game.hpp"
#include "test.hpp"
#include "testGraph.cpp"
#include "testRouting.cpp"

void test() {
  setRenderEnabled(false);
  initNames();
  resetAll();
  generateFlatLand(2);
  testGraph();
  testRouting();
  resetNames();
}
