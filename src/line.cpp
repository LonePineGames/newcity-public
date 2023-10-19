#include "line.hpp"
#include "util.hpp"

Line line(vec3 start, vec3 end) {
  Line result;
  result.start = start;
  result.end = end;
  return result;
}

float length(Line line) {
  return vecDistance(line.start, line.end);
}
