// Copyright 2011 Rich Lane
#ifndef OORT_SIM_AI_LIB_H_
#define OORT_SIM_AI_LIB_H_

#include <memory>
#include "glm/glm.hpp"

namespace Oort {

class Ship;

namespace AILib {

float smallest_positive_root_of_quadratic_equation(float a, float b, float c);
int accelerated_goto(float p, float v, float a);
void turn_to(Ship &s, float angle);
void turn_towards(Ship &s, glm::vec2 tp);
void drive_towards(Ship &s, glm::vec2 tp, float speed);
float lead(glm::vec2 p1, glm::vec2 p2, glm::vec2 v1, glm::vec2 v2,
           float w, float t_max);
std::shared_ptr<Ship> find_target(Ship &s, float dist=1e9f);  // NOLINT
std::shared_ptr<Ship> find_missile_target(Ship &s, float dist=1e9f);  // NOLINT

class ProportionalNavigator {
 public:
  Ship &ship;
  float k;
  float a;
  float last_bearing;

  ProportionalNavigator(Ship &ship, float k, float a);
  void seek(glm::vec2 tp, glm::vec2 tv);
};

}
}

#endif
