#include "mcv_platform.h"
#include "engine.h"
#include "module_scripting.h"
#include "input/devices/device_mouse.h"
#include "components/common/comp_tags.h"
#include "components/controllers/character/comp_character_controller.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "windows/app.h"
#include "components/objects/checkpoints/checkpoint.h"
#include "components/common/comp_tags.h"
#include "input/module_input.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"
#include "module_blackBoard.h"


bool CModuleBlackBoard::start() {
	
	doBingings();
	
	try {//TODO: esta ruta hay que cambiarla
		//s->doFile("C:\\Users\\joaki\\Desktop\\VisualStudioProjects\\Proyecto_37\\mcv_sm\\bin\\data\\scripts\\script1.lua");
		loadScriptsInFolder("data/scripts"); //en un futuro hacerlo asi directamente, el problema es que no admite el parametro
	}
	catch (exception e) {
    fatal("error starting lua\n");
	}

    return true;
}

void CModuleScripting::update(float delta) {

}

void CModuleScripting::stop()
{

}

