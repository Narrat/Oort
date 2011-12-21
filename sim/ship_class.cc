// Copyright 2011 Rich Lane
#include "sim/ship_class.h"

#include <iostream>
#include <boost/foreach.hpp>
#include "sim/math_util.h"
#include "sim/model.h"

namespace Oort {

std::unique_ptr<ShipClass> fighter,
                           ion_cannon_frigate,
                           assault_frigate;

void ShipClass::initialize() {
	{
		ShipClassDef def;
		def.name = "fighter";
		def.mass = 10e3;
		def.hull = 4.5e6;
		def.max_main_acc = 100;
		def.max_lateral_acc = 10;
		def.max_angular_acc = 1;
		def.scale = 10;
		def.model = Model::load("fighter");
		GunDef gun;
		gun.mass = 0.03f;
		gun.radius = 0.01f;
		gun.velocity = 3000.0f;
		gun.ttl = 1.0f;
		gun.reload_time = 0.125f;
		def.guns.push_back(gun);
		fighter = std::unique_ptr<ShipClass>(new ShipClass(def));
	}

	{
		ShipClassDef def;
		def.name = "ion cannon frigate";
		def.mass = 160e3;
		def.hull = 90e6;
		def.max_main_acc = 20;
		def.max_lateral_acc = 2;
		def.max_angular_acc = 0.5;
		def.scale = 40;
		def.model = Model::load("ion_cannon_frigate");
		GunDef gun;
		gun.mass = 0.1f;
		gun.radius = 0.01f;
		gun.velocity = 2000.0f;
		gun.ttl = 1.3f;
		gun.reload_time = 0.125f;
		def.guns.push_back(gun);
		ion_cannon_frigate = std::unique_ptr<ShipClass>(new ShipClass(def));
	}

	{
		ShipClassDef def;
		def.name = "assault frigate";
		def.mass = 160e3;
		def.hull = 135e6;
		def.max_main_acc = 20;
		def.max_lateral_acc = 4;
		def.max_angular_acc = 0.7;
		def.scale = 40;
		def.model = Model::load("assault_frigate");
		GunDef gun;
		gun.mass = 10.0f;
		gun.radius = 0.01f;
		gun.velocity = 600.0f;
		gun.ttl = 10.0f;
		gun.reload_time = 0.6f;
		def.guns.push_back(gun);
		assault_frigate = std::unique_ptr<ShipClass>(new ShipClass(def));
	}
}

ShipClass::ShipClass(const ShipClassDef &def)
  : ShipClassDef(def)
{
	auto physics_vertices = model->collision_shape.vertices;
	BOOST_FOREACH(glm::vec2 &v, physics_vertices) {
		v *= (scale/Oort::SCALE);
	}
	shape.Set((b2Vec2*) &physics_vertices[0], physics_vertices.size());

	// calculate density for desired mass
	b2MassData md;
	shape.ComputeMass(&md, 1);
	density = mass/md.mass;
}

}
