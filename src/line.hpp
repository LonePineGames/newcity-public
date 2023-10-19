#ifndef LINE_H
#define LINE_H

#include "main.hpp"

struct Line {
  vec3 start, end;
};

Line line(vec3 start, vec3 end);
float length(Line line);

#endif
