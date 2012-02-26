#include "renderer/batches/particle.h"

#include <memory>
#include <array>
#include <list>
#include <stdint.h>
#include <boost/foreach.hpp>
#include <boost/random/uniform_real.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "sim/game.h"
#include "sim/ship.h"
#include "sim/bullet.h"
#include "sim/team.h"
#include "gl/check.h"
#include "common/resources.h"
#include "renderer/bunch.h"

using glm::vec2;
using glm::vec4;
using std::make_shared;
using std::shared_ptr;

namespace Oort {

namespace RendererBatches {

struct Particle {
	glm::vec2 initial_position;
	glm::vec2 velocity;
	float lifetime;
	float type;
};

template<> std::vector<GLuint> Bunch<Particle>::buffer_freelist = std::vector<GLuint>();

struct ParticlePriv {
	GL::Program prog;
	std::vector<Particle> tmp_particles;
	std::list<Bunch<Particle>> bunches;
	GL::Texture tex;
	boost::random::mt19937 prng;
	float time;

	ParticlePriv()
		: prog(GL::Program::from_resources("particle")),
		  prng(42)
	{
		tex.bind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		const int n = 256;
		std::array<uint8_t, 3*n*n> data;
		for (int y = 0; y < n; y++) {
			for (int x = 0; x < n; x++) {
				int i = n*y+x;
				vec2 point = vec2(float(x)/n, float(y)/n);
				float dist = glm::length(vec2(0.5f,0.5f) - point);
				float alpha = powf(1-glm::clamp(2*dist, 0.0f, 1.0f), 2);
				data[i] = (uint8_t) (alpha*255);
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, n, n, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

ParticleBatch::ParticleBatch(Renderer &renderer)
	: Batch(renderer),
	  priv(make_shared<ParticlePriv>())
{
}

void ParticleBatch::render(float time_delta) {
	auto &prog = priv->prog;
	glBlendFunc(GL_ONE, GL_ONE);
	prog.use();
	prog.uniform("p_matrix", renderer.p_matrix);
	prog.uniform("current_time", priv->time + time_delta);
	prog.uniform("view_scale", renderer.view_scale);
	prog.uniform("tex", 0);

	auto stride = sizeof(Particle);
	Particle *p = (Particle*)NULL;

	prog.enable_attrib_array("initial_position");
	prog.enable_attrib_array("velocity");
	prog.enable_attrib_array("lifetime");
	prog.enable_attrib_array("type");
	priv->tex.bind();

	BOOST_FOREACH(auto &bunch, priv->bunches) {
		if (bunch.size == 0) {
			continue;
		}
		bunch.bind();
		prog.attrib("initial_time", bunch.initial_time);
		prog.attrib_ptr("initial_position", &p->initial_position, stride);
		prog.attrib_ptr("velocity", &p->velocity, stride);
		prog.attrib_ptr("lifetime", &p->lifetime, stride);
		prog.attrib_ptr("type", &p->type, stride);
		glDrawArrays(GL_POINTS, 0, bunch.size);
	}
	Bunch<Particle>::unbind();

	glBindTexture(GL_TEXTURE_2D, 0);
	prog.disable_attrib_array("initial_position");
	prog.disable_attrib_array("velocity");
	prog.disable_attrib_array("lifetime");
	prog.disable_attrib_array("type");

	glUseProgram(0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleBatch::snapshot(const Game &game) {
	priv->time = game.time;

	if (priv->bunches.size() >= 128) {
		priv->bunches.pop_back();
	}

	BOOST_FOREACH(auto bullet, game.bullets) {
		if (bullet->def.type == GunType::PLASMA) {
			shower(ParticleType::PLASMA,
			       bullet->get_position(game.time),
			       vec2(0,0), bullet->velocity * 0.707f,
			       20.0f, 0.2f, 0.4f, 4);
		}
	}

	BOOST_FOREACH(auto &hit, game.hits) {
		auto n = short(glm::max(hit.e/10000,1.0f));
		shower(ParticleType::HIT, hit.cp,
		       hit.ship->get_velocity(), vec2(0,0),
		       256, 0.03f, 0.63f, n);
	}

	BOOST_FOREACH(auto &exp, game.explosions) {
		auto n = 50;
		shower(ParticleType::EXPLOSION, exp.p,
		       vec2(0,0), vec2(0,0),
		       256, 0.03f, 0.63f, n);
	}

	priv->bunches.push_front(Bunch<Particle>(game.time, std::move(priv->tmp_particles)));
}

void ParticleBatch::shower(
	ParticleType type,
	vec2 p0, vec2 v0, vec2 v,
	float s_max, float life_min, float life_max,
	int count)
{
	boost::uniform_real<float> a_dist(0.0f, M_PI*2.0f);
	boost::uniform_real<float> s_dist(0.0f, s_max);
	boost::uniform_real<float> fdp_dist(0.0f, 1.0f);
	boost::uniform_real<float> life_dist(life_min, life_max);
	for (int i = 0; i < count; i++) {
		float a = a_dist(priv->prng);
		float s = s_dist(priv->prng);
		float fdp = fdp_dist(priv->prng);
		float lifetime = life_dist(priv->prng);
		vec2 dp = v * fdp * Game::tick_length;
		vec2 dv = vec2(cosf(a)*s, sinf(a)*s);
		priv->tmp_particles.emplace_back(Particle{
			/* initial_position */ p0 + dp + dv*Game::tick_length,
		  /* velocity */ v0 + v + dv,
			/* lifetime */ lifetime,
			/* type */ (float)type
		});
	}
}

}
}
