#include "ui/nacl/oort_instance.h"
#include "ppapi/cpp/var.h"
#include "json_spirit_reader_template.h"
#include "json_spirit_reader.h"
#include "ui/gui.h"
#include "sim/game.h"
#include "sim/scenario.h"
#include "sim/ai.h"
#include "sim/team.h"
#include "sim/ship.h"
#include "sim/ship_class.h"
#include "sim/builtin_ai.h"
#include "common/log.h"

using namespace Oort;
namespace js = json_spirit; 

void OortInstance::HandleMessage(const pp::Var& message) {
	log("HandleMessage");

	auto data = message.AsString();
	js::mValue value;
	js::read_string(data, value);
	js::mObject &obj = value.get_obj();
	auto &key = obj.find("key")->second.get_str();
	auto &scenario = obj.find("scenario")->second.get_str();

	if (gui) {
		log("stopping old gui");
		gui->stop();
		log("destroying old gui");
		delete gui;
		log("destroyed gui");
	}

	log("creating game");
	Scenario scn = Scenario::load(scenario);
	std::vector<std::shared_ptr<AIFactory>> ai_factories = { builtin_ai_factory, builtin_ai_factory, builtin_ai_factory };
	game = std::make_shared<Game>(scn, ai_factories);
	log("game created");

	log("creating gui");
	gui = new GUI(game, NULL);

	log("resizing gui");
	gui->handle_resize(size.width(), size.height());

	log("starting gui");
	gui->start();
	log("started gui");
}
