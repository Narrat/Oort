// Copyright 2011 Rich Lane
#ifndef OORT_SIM_SHIP_CLASS_H_
#define OORT_SIM_SHIP_CLASS_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <Box2D/Box2D.h>
#include "glm/glm.hpp"
#include "sim/bullet.h"
#include "sim/beam.h"

namespace Oort {

struct Model;

struct ShipClassDef {
  std::string name;
  float mass;
  float hull;
  float max_main_acc;
  float max_lateral_acc;
  float max_angular_acc;
  float scale;
  float tail_alpha;
  std::shared_ptr<Model> model;
  std::vector<GunDef> guns;
  std::vector<BeamDef> beams;
};

class ShipClass : public ShipClassDef {
public:
  float density;
  b2PolygonShape shape;

  static std::map<std::string, const ShipClass&> klasses;
  static void initialize();
  static const ShipClass &lookup(const std::string &name);

  ShipClass(const ShipClassDef &def);
};

extern std::unique_ptr<ShipClass> fighter,
                                  ion_cannon_frigate,
                                  assault_frigate,
                                  missile,
                                  target;
}

#endif
