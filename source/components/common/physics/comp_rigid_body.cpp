#include "mcv_platform.h"
#include "comp_rigid_body.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "modules/module_physics.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("rigidbody", TCompRigidBody);

using namespace physx;

void TCompRigidBody::load(const json& j, TEntityParseContext& ctx) {
	is_enabled = true;
}

// --------------------------------------------------------------------
TCompRigidBody::~TCompRigidBody() {

}

void TCompRigidBody::setUsingUnscaledTime(bool usingUnscaledTime) {
	_usingUnscaledTime = usingUnscaledTime;
}

void TCompRigidBody::update(float delta) {
	float dt;
	if (_usingUnscaledTime) {
		dt = Time.delta_unscaled;
	}
	else {
		dt = Time.delta;
	}
	if (CHandle(this).getOwner().isValid() && is_enabled) {

		TCompCollider* c_collider = get<TCompCollider>();
		if (!c_collider)
			return;

    time_on_air += dt;

		//APPLY GRAVITY
		if (is_grounded) {
			impulse = VEC3();
			getGroundNormal(ground_normal);
      time_on_air = 0.f;
		}

		if (impulse.y <= 0) {
			impulse.y += _descentGravity * mass * dt;
		}
		else if(impulse.y > 0) {
			impulse.y += _ascensionGravity * mass * dt;
		}
		manageImpulseXZ(dt);

    impulse.x = clamp(impulse.x, -10.f, 10.f);
    impulse.y = clamp(impulse.y, -20.f, 10.f);
    impulse.z = clamp(impulse.z, -10.f, 10.f);

		PxVec3 velocity = physx::PxVec3(impulse.x, impulse.y, impulse.z);

		physx::PxControllerCollisionFlags col = c_collider->controller->move(velocity * dt, 0.f, dt, physx::PxControllerFilters());
		bool new_is_grounded = col.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

    if (!is_grounded && new_is_grounded) {//just landed
			if (smokeTimer <= 0) {
				TCompTransform* c_trans = get<TCompTransform>();
				TEntityParseContext ctx;
				ctx.root_transform.setPosition(c_trans->getPosition() - 0.2f* c_trans->getFront());
				parseScene("data/prefabs/vfx/smoke.json", ctx);
				smokeTimer = smokeTimerMax;
			}
    }
		smokeTimer -= dt;
    is_grounded = new_is_grounded;


	}
}

void TCompRigidBody::manageImpulseXZ(float delta) {
	if (impulse.x > 0.0f) {
		impulse.x -= lossRatio * delta;
	}
	else if (impulse.x < 0.0f) {
		impulse.x += lossRatio * delta;
	}

	if (impulse.z > 0.0f) {
		impulse.z -= lossRatio * delta;
	}
	else if (impulse.z < 0.0f) {
		impulse.z += lossRatio * delta;
	}

	if (abs(impulse.x) < 0.1) {
		impulse.x = 0.f;
	}
	if (abs(impulse.z) < 0.1) {
		impulse.z = 0.f;
	}
}

void TCompRigidBody::getGroundNormal(VEC3 &value) {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 char_pos = c_trans->getPosition();
	char_pos.y += 0.0001f; //So we can detect collision with planes
	PxVec3 origin = VEC3_TO_PXVEC3(char_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(VEC3(0, -1, 0));
	PxReal maxDistance = 1.0f;
	PxRaycastBuffer hit;

	bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit);
	if (hit.hasBlock) {
		PxF32 dist = hit.block.distance;
		if (dist <= 0.5f) {
			value = PXVEC3_TO_VEC3(hit.block.normal);
			value.Normalize();
			return;
		}
	}
	else if (hit.hasAnyHits()) {
		PxF32 dist = hit.touches->distance;
		if (dist <= 0.5f) {
			value = PXVEC3_TO_VEC3(hit.touches->normal);
			value.Normalize();
			return;
		}
	}
	value = VEC3();
}

void TCompRigidBody::jump(const VEC3 dir) {
	if (is_grounded || time_on_air <= jump_time_offset) {
		impulse = dir;
		is_grounded = false;
	}
}

bool TCompRigidBody::isGrounded() { //only for the player, not the enemies

	return is_grounded || time_on_air <= jump_time_offset;
}

void TCompRigidBody::doubleJump(const VEC3 dir) {
	impulse = dir;
	is_grounded = false;
}

void TCompRigidBody::addForce(const VEC3 dir) {
	//dbg("Adding force\n");
	impulse += dir;
	if (impulse.y > 0.0f)
		is_grounded = false;

}

void TCompRigidBody::renderDebug() {
}

void TCompRigidBody::debugInMenu() {
  ImGui::Checkbox("Grounded", &is_grounded);
	ImGui::DragFloat3("Normal", &ground_normal.x);
	ImGui::DragFloat3("Impulse", &impulse.x);
	ImGui::DragFloat("Mass", &mass);

}

void TCompRigidBody::enableGravity(bool gravity) {
	is_enabled = gravity;
}

const VEC3 TCompRigidBody::getImpulse() {
	return impulse;
}

// --------------------------------------------------------------------
void TCompRigidBody::registerMsgs() {
}

void TCompRigidBody::onEntityCreated(const TMsgEntityCreated& msg) {
}