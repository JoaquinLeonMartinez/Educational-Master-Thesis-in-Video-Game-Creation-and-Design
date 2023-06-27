#include "mcv_platform.h"
#include "ai_platform.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "time.h"
#include "stdlib.h"
#include "modules/module_physics.h"
#include "ai_controller.h"
#include "engine.h"
#include "components/common/comp_id.h"

#include "input/input.h"


using namespace physx;


DECL_OBJ_MANAGER("ai_platform_mobile", CAIMobilePlatform);
////////////////////////PLATAFORMAS POINT TO POINT///////////////////////////////
void CAIMobilePlatform::InitPoinToPoint()
{
	
	AddState("SEEKPT", (statehandler)&CAIMobilePlatform::SeekwptState);
	AddState("NEXTPT", (statehandler)&CAIMobilePlatform::NextwptState);
	i = 0;
	nextPoint = positions[i];
	ChangeState("SEEKPT");

}



void CAIMobilePlatform::SeekwptState(float dt) {
	
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	//direccion a mover
	currentPosition = c_trans->getPosition();
	VEC3 direccion = nextPoint - currentPosition;
	direccion.Normalize();
	Vector3 nextPos = currentPosition + direccion * mTravelTime * dt;

	if (!controlledByTrigger) {
		float yaw, pitch, roll = 0.f; 
		c_trans->getAngles(&yaw, &pitch);
		if (c_trans->isInLeft(nextPoint)) {
		c_trans->setAngles(yaw + dt, pitch);
		}
		else {
		c_trans->setAngles(yaw - dt, pitch);
		}
	}

	QUAT quat = c_trans->getRotation();
	PxVec3 pos = VEC3_TO_PXVEC3(nextPos);
	PxQuat qua = QUAT_TO_PXQUAT(quat);
	const PxTransform tr(pos, qua);
	rigid_dynamic->setKinematicTarget(tr);
	float dist = Vector3::Distance(nextPoint, c_trans->getPosition());
	if (dist < 1.0f) {
		ChangeState("NEXTPT");
	}
}

void CAIMobilePlatform::NextwptState(float dt) {
	TCompID *c_id = get<TCompID>();
	
	if (!controlledByTrigger) { //esto solo sera cierto si lo esta controlando un trigger
		i = (i + 1) % positions.size();
	}
	else {
		dbg("la pared %i llega al wpt\n", c_id->getId());
		i++;
		if (i >= positions.size()) {
			setPaused(true);
			i = 0;
		}
	}
	
	nextPoint = positions[i];
	ChangeState("SEEKPT");

	//dbg("next point: x:%f,y:%f,z:%f \n", nextPoint.x, nextPoint.y, nextPoint.z);
}



/////////////////////////PLATAFORMAS ROTATORIAS ///////////////////////////////

void CAIMobilePlatform::InitRotationInfinity() {
	
	AddState("TURNINF", (statehandler)&CAIMobilePlatform::TurnInfinity);
	TCompTransform* c_trans = get<TCompTransform>();

	ChangeState("TURNINF");
}




void CAIMobilePlatform::TurnInfinity(float dt) {
	TCompTransform* c_trans = get<TCompTransform>();
	
	if (direction == 0) {
		//ralentiza con el cafe pero el giro del pitch al llegar a 90 grados se queda quieto 
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
		float yaw, pitch, roll;
		
		c_trans->getAngles(&yaw, &pitch, &roll);
		if (axis.x == 1 && axis.y == 0 && axis.z == 0) {//en el json el axis
			c_trans->setAngles(yaw + dt * rotationTime, pitch, roll); //ok funciona
		}
		else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
			//ko solo gira hasta llegar, a 90 grados
			//c_trans->setAngles(yaw,pitch + (dt * rotationTime), roll);
			
			
			QUAT angle = QUAT::CreateFromAxisAngle(VEC3(0, 1, 0), dt * rotationTime);
			c_trans->setRotation(angle * c_trans->getRotation());
			
		}
		else if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
			c_trans->setAngles(yaw, pitch, roll + dt * rotationTime); //OK funciona
		}
		
		PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
		PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
		PxTransform tr(pos, ori);
		rigid_dynamic->setKinematicTarget(tr);
		//dbg("Pitch:%f\n", rad2deg(pitch));
	}
	else {
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
		float yaw, pitch, roll;
		c_trans->getAngles(&yaw, &pitch, &roll);
		if (axis.x == 1 && axis.y == 0 && axis.z == 0) {//en el json el axis
			c_trans->setAngles(yaw - dt * rotationTime, pitch, roll); //OK funciona
		}
		else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
			//c_trans->setAngles(yaw, pitch - dt * rotationTime, roll); //KO solo gira hasta llegar, a 90 grados <-TODO->
			QUAT angle = QUAT::CreateFromAxisAngle(VEC3(0, 1, 0), -dt * rotationTime);
			c_trans->setRotation(angle * c_trans->getRotation());

		}
		else if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
			c_trans->setAngles(yaw, pitch, roll - dt * rotationTime); //OK funciona	
		}

		PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
		PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
		PxTransform tr(pos, ori);
		rigid_dynamic->setKinematicTarget(tr);
	}
	
}

//PLATAFORMAS ROTATORIAS HASTA UN DET GRADO
void CAIMobilePlatform::InitRotation() {

	AddState("TURN", (statehandler)&CAIMobilePlatform::Turn);
	AddState("WAIT", (statehandler)&CAIMobilePlatform::WaitState);

	TCompTransform* c_trans = get<TCompTransform>();
	ChangeState("TURN");
}

void CAIMobilePlatform::Turn(float dt) {
	
	TCompTransform* c_trans = get<TCompTransform>();
	
	
	if (firstTimeRotateDegree) {

		float yaw_init, pitch_init, roll_init;
		c_trans->getAngles(&yaw_init, &pitch_init, &roll_init);
		if (axis.x == 1 && axis.y == 0 && axis.z == 0) {
			i_ang = yaw_init;
		}
		else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
			i_ang = pitch_init;
		}
		if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
			i_ang = roll_init;
		}
		firstTimeRotateDegree = false;
	}
	
	if (active) {
		if (direction == 0) {
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			float yaw, pitch, roll;
			c_trans->getAngles(&yaw, &pitch, &roll);
			if (axis.x == 1 && axis.y == 0 && axis.z == 0) {
				if (i_ang <= deg2rad(angleTurn)) {
					c_trans->setAngles(yaw + dt * rotationTime, pitch, roll);
					i_ang = yaw + dt * rotationTime;
					dbg("%f\n", rad2deg(i_ang));
				}
			}
			else if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
				if (i_ang <= deg2rad(angleTurn)) {
					c_trans->setAngles(yaw , pitch,roll  + dt * rotationTime);
					i_ang = roll + dt * rotationTime;
					dbg("%f\n", rad2deg(roll));
				}
			}
			else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
				if (i_ang <= deg2rad(angleTurn)) {
					QUAT rotate = QUAT::CreateFromAxisAngle(VEC3(0, 1, 0), dt * rotationTime);
					c_trans->setRotation(rotate * c_trans->getRotation());
					i_ang = pitch + dt * rotationTime;
					dbg("%f\n", rad2deg(pitch));
				}
			}
			PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
			PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
			PxTransform tr(pos, ori);
			rigid_dynamic->setKinematicTarget(tr);
		}
		else {
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			float yaw, pitch, roll;
			c_trans->getAngles(&yaw, &pitch, &roll);
			if (axis.x == 1 && axis.y == 0 && axis.z == 0) {
				if (i_ang > deg2rad(angleTurn)) {
					c_trans->setAngles(yaw - dt * rotationTime, pitch, roll);
					i_ang = yaw - dt * rotationTime;
					dbg("%f\n", rad2deg(yaw));
				}
			}
			else if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
				if (i_ang > deg2rad(angleTurn)) {
					c_trans->setAngles(yaw, pitch, roll - dt * rotationTime);
					i_ang = roll - dt * rotationTime;
					dbg("%f\n", rad2deg(roll));
				}
			}
			else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
				if (i_ang > deg2rad(angleTurn)) {
					QUAT rotate = QUAT::CreateFromAxisAngle(VEC3(0, 1, 0), -dt * rotationTime);
					c_trans->setRotation(rotate * c_trans->getRotation());
					i_ang = pitch - dt * rotationTime;
					dbg("%f\n", rad2deg(pitch));
				}
			}
			dbg("%f\n", rad2deg(i_ang));
		}
	}
}



void CAIMobilePlatform::WaitState(float dt) {
}



//TRAVEL DE PLATAFORMAS//////////
void CAIMobilePlatform::InitTravel() {
	

		AddState("INITIALPOSITION", (statehandler)&CAIMobilePlatform::InitialPositionTravelState);
		AddState("SEEKPTTRAVEL", (statehandler)&CAIMobilePlatform::SeekwptTravelState);
		//AddState("NEXTPTTRAVEL", (statehandler)&CAIMobilePlatform::NextwptTravelState);
		i = 0;
		_knots = _curve->_knots;
		nextPoint = _knots[i];
		ChangeState("INITIALPOSITION");
	
}

void CAIMobilePlatform::InitialPositionTravelState(float dt) {
	//TCompTransform* c_trans = get<TCompTransform>();
	//c_trans->setPosition(nextPoint);
	mTravelTime = mTravelTime / 100;
	ChangeState("SEEKPTTRAVEL");
}


void CAIMobilePlatform::SeekwptTravelState(float dt) {
	if (active) {
		TCompTransform* c_trans = get<TCompTransform>();
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
		
		if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}
		ratio += dt * mTravelTime;
		VEC3 nextPoint = _curve->evaluate(ratio);
		float angleNecesariusforRotate = c_trans->getDeltaYawToAimTo(nextPoint);
		float yaw, pitch, roll = 0.f;
		c_trans->getAngles(&yaw, &pitch);
		if (angleNecesariusforRotate > 0.05f) {//este numero hay que tocarlo y quizas poner multiplicador en deltatime
			if (c_trans->isInLeft(nextPoint)) {
				c_trans->setAngles(yaw + dt, pitch);
			}
			else {
				c_trans->setAngles(yaw - dt, pitch);
			}
		}
		QUAT quat = c_trans->getRotation();
		PxVec3 pos = VEC3_TO_PXVEC3(nextPoint);
		PxQuat qua = QUAT_TO_PXQUAT(quat);
		const PxTransform tr(pos, qua);
		rigid_dynamic->setKinematicTarget(tr);
		float dist = Vector3::Distance(nextPoint, c_trans->getPosition());
	}
}


//POINT TO POINT CON ROTACION INFINITA
void CAIMobilePlatform::InitPoinToPointRotationInfinity() {
	AddState("SEEKPTROTATIONINF", (statehandler)&CAIMobilePlatform::SeekwPoinToPointRotationInfinity);
	AddState("NEXTPTROTATIONINF", (statehandler)&CAIMobilePlatform::NextwPoinToPointRotationInfinity);
	i = 0;
	nextPoint = positions[i];

	ChangeState("SEEKPTROTATIONINF");
}

void CAIMobilePlatform::SeekwPoinToPointRotationInfinity(float dt) {


	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	//direccion a mover
	currentPosition = c_trans->getPosition();
	VEC3 direccion = nextPoint - currentPosition;
	direccion.Normalize();
	Vector3 nextPos = currentPosition + direccion * mTravelTime * dt;

	PxQuat quat;
	if (direction == 0) {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			quat = QUAT_TO_PXQUAT(result);
			i_ang++;
			rotationTimeActual = rotationTime;
		}
	}
	else {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			quat = QUAT_TO_PXQUAT(result);
			i_ang--;
			rotationTimeActual = rotationTime;
		}
	}
	PxVec3 pos = VEC3_TO_PXVEC3(nextPos);
	PxQuat qua = QUAT_TO_PXQUAT(quat);
	const PxTransform tr(pos, qua);
	rigid_dynamic->setKinematicTarget(tr);
	float dist = Vector3::Distance(nextPoint, c_trans->getPosition());

	if (dist < 1.0f) {
		ChangeState("NEXTPTROTATIONINF");
	}
}

void CAIMobilePlatform::NextwPoinToPointRotationInfinity(float dt) {
	TCompID *c_id = get<TCompID>();
	
	if (!controlledByTrigger) { //esto solo sera cierto si lo esta controlando un trigger
		i = (i + 1) % positions.size();
	}
	else {
		dbg("la pared %i llega al wpt\n", c_id->getId());
		i++;
		if (i >= positions.size()) {
			setPaused(true);
			i = 0;
		}
	}
	nextPoint = positions[i];
	ChangeState("SEEKPTROTATIONINF");
}

//POINT TO POINT CON ROTACION INFINITA
void CAIMobilePlatform::InitTravelRotationInfinity() {
	AddState("INITIALPOSITIONROTATIONTRAVEL", (statehandler)&CAIMobilePlatform::InitialPositionRotationTravelState);
	AddState("SEEKTRAVELROTATIONINF", (statehandler)&CAIMobilePlatform::SeekwTravelRotationInfinity);
	
	i = 0;
	_knots = _curve->_knots;
	nextPoint = _knots[i];
	ChangeState("INITIALPOSITIONROTATIONTRAVEL");
}


void  CAIMobilePlatform::InitialPositionRotationTravelState(float dt) {
	//TCompTransform* c_trans = get<TCompTransform>();
	//c_trans->setPosition(nextPoint);
	mTravelTime = mTravelTime / 100;
	ChangeState("SEEKTRAVELROTATIONINF");
}

void CAIMobilePlatform::SeekwTravelRotationInfinity(float dt) {
	/*TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	//direccion a mover
	currentPosition = c_trans->getPosition();
	VEC3 direccion = nextPoint - currentPosition;
	direccion.Normalize();
	Vector3 nextPos = currentPosition + direccion * mTravelTime * dt;
	*/
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	
	if (ratio >= 1.0f || ratio < 0.0f) {
		mTravelTime = -mTravelTime;
	}
	ratio += dt * mTravelTime;
	VEC3 nextPoint = _curve->evaluate(ratio);

	PxQuat quat;
	if (direction == 0) {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			quat = QUAT_TO_PXQUAT(result);
			i_ang++;
			rotationTimeActual = rotationTime;
		}
	}
	else {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			quat = QUAT_TO_PXQUAT(angle);
			i_ang--;
			rotationTimeActual = rotationTime;
		}
	}
	PxVec3 pos = VEC3_TO_PXVEC3(nextPoint);
	PxQuat qua = QUAT_TO_PXQUAT(quat);
	const PxTransform tr(pos, qua);
	rigid_dynamic->setKinematicTarget(tr);
	
}


void CAIMobilePlatform::InitialPositionTravelStateNotRotation() {
	AddState("INITIALPOSITIONOTROTATION", (statehandler)&CAIMobilePlatform::InitialPositionTravelNotRotationState);
	AddState("SEEKPTTRAVELNOTROTATION", (statehandler)&CAIMobilePlatform::SeekwptTravelNotRotationState);
	i = 0;
	_knots = _curve->_knots;
	nextPoint = _knots[i];
	ChangeState("INITIALPOSITIONOTROTATION");
}


void CAIMobilePlatform::InitialPositionTravelNotRotationState(float dt) {
	//TCompTransform* c_trans = get<TCompTransform>();
	//c_trans->setPosition(nextPoint);
	mTravelTime = mTravelTime / 100;
	ChangeState("SEEKPTTRAVELNOTROTATION");
}

void CAIMobilePlatform::SeekwptTravelNotRotationState(float dt) {
	if (active) {
		TCompTransform* c_trans = get<TCompTransform>();
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

		/*if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}*/
		if (ratio >= 1.0f) {
			mTravelTime = -mTravelTime;
			ratio = 1.0f;
		}
		if (ratio < 0.0f) {
			mTravelTime = -mTravelTime;
			ratio = 0.0f;
		}
		float yaw, pitch, roll = 0.f;
		c_trans->getAngles(&yaw, &pitch);
		nextPoint = _curve->evaluate(ratio);
		ratio += dt * mTravelTime;
		QUAT quat = c_trans->getRotation();
		PxVec3 pos = VEC3_TO_PXVEC3(nextPoint);
		PxQuat qua = QUAT_TO_PXQUAT(quat);
		const PxTransform tr(pos, qua);
		rigid_dynamic->setKinematicTarget(tr);
		
	}
}

void CAIMobilePlatform::TurnInfinityNoDt(float dt) {
	TCompTransform* c_trans = get<TCompTransform>();

	if (direction == 0) {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			PxQuat ori = QUAT_TO_PXQUAT(result);
			PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
			i_ang++;
			PxTransform tr(pos, ori);
			rigid_dynamic->setKinematicTarget(tr);
			rotationTimeActual = rotationTime;
		}
	}
	else {
		if (rotationTimeActual > 0) {
			rotationTimeActual--;
		}
		else {
			QUAT result;
			QUAT actual = c_trans->getRotation();
			QUAT angle = QUAT::CreateFromAxisAngle(axis, deg2rad(i_ang));
			result = QUAT::Slerp(actual, angle, 1);
			TCompCollider* c_col = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			PxQuat ori = QUAT_TO_PXQUAT(result);
			PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
			i_ang--;
			PxTransform tr(pos, ori);
			rigid_dynamic->setKinematicTarget(tr);
			rotationTimeActual = rotationTime;
		}
	}
}


void CAIMobilePlatform::InitRotationInfinityNoDt() {
	AddState("TURNINFNODT", (statehandler)&CAIMobilePlatform::TurnInfinityNoDt);
	TCompTransform* c_trans = get<TCompTransform>();

	ChangeState("TURNINFNODT");
}



void CAIMobilePlatform::ToPosition(float dt) {
	if (active) {
		TCompTransform* c_trans = get<TCompTransform>();
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

		/*if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}*/
		if (ratio >= 1.0f) {
			//mTravelTime = -mTravelTime;
			//ratio = 1.0f;
			active = false;
		}
		if (ratio < 0.0f) {
			mTravelTime = -mTravelTime;
			ratio = 0.0f;
		}
		float yaw, pitch, roll = 0.f;
		c_trans->getAngles(&yaw, &pitch);
		nextPoint = _curve->evaluate(ratio);
		ratio += dt * mTravelTime;
		QUAT quat = c_trans->getRotation();
		PxVec3 pos = VEC3_TO_PXVEC3(nextPoint);
		PxQuat qua = QUAT_TO_PXQUAT(quat);
		const PxTransform tr(pos, qua);
		rigid_dynamic->setKinematicTarget(tr);

	}
}

void CAIMobilePlatform::InitToPositionWithCurveNotRotation() {
	mTravelTime = mTravelTime / 100;
	AddState("TOPOSITION", (statehandler)&CAIMobilePlatform::ToPosition);
	ChangeState("TOPOSITION");
}

void CAIMobilePlatform::ToSineMovement(float dt) {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 nextPos = c_trans->getPosition();
	nextPos.y = pivot.y + height + height * sin(((PI * 2) / timePeriod) * timeSinceStart);
	timeSinceStart += dt;
	c_trans->setPosition(nextPos);

	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
	PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
	const PxTransform tr(pos, ori);
	rigid_dynamic->setKinematicTarget(tr);
}

void CAIMobilePlatform::InitSinMov(float dt) {
	/*TCompTransform* c_trans = get<TCompTransform>();
	posLocalYLimiteMax = c_trans->getPosition().y + 5;
	posLocalYLimiteMin = c_trans->getPosition().y - 5;*/
	TCompTransform* c_trans = get<TCompTransform>();
	pivot = c_trans->getPosition();
	height /=  2;
	timeSinceStart = timeSinceStart = (3 * timePeriod) / 4;
	ChangeState("TOSINEMOVEMENT");
}


void CAIMobilePlatform::InitToSineMovement() {
	AddState("INITSINMOV", (statehandler)&CAIMobilePlatform::InitSinMov);
	AddState("TOSINEMOVEMENT", (statehandler)&CAIMobilePlatform::ToSineMovement);
	ChangeState("INITSINMOV");
	
	
}



void CAIMobilePlatform::load(const json& j, TEntityParseContext& ctx) {
	
	//trigger o no trigger
	controlledByTrigger = j.value("controlledByTrigger", false);
	//generico
	platformType = j.value("platformType_", platformType);
	if (j.count("curve") > 0) {
		_curve = Resources.get(j.value("curve", ""))->as<CCurve>();
	}
	controlledByTrigger = j.value("controlledByTrigger", controlledByTrigger);
	trigger_id = j.value("trigger_id",trigger_id);
	pair_object_id = j.value("pair_object_id", pair_object_id);
	setPaused(controlledByTrigger);
	//en caso de ser de tipo POINT_TO_POINT
	//const &json vector = j.value("positions");
	if (j.count("positions") > 0) {
		const json& jpositions = j["positions"];
		for (const json& i : j["positions"]) {
			VEC3 pos = loadVEC3(i);
			positions.push_back(pos);
		}
	}
	mTravelTime = j.value("mTravelTime_", mTravelTime);
	//mNbPts = positions.size;

	//en caso de ser de tipo ROTATION
	mRotationSpeed = j.value("mRotationSpeed_", mRotationSpeed);
	angleTurn = j.value("angleTurn_", angleTurn);
	rotationTime = j.value("mRotationTime_", rotationTime);
	direction = j.value("direction", direction);
	if (j.count("axis")) {
		axis = loadVEC3(j, "axis");
	}
	active = j.value("active", active);
	height = j.value("heightSinus", height);
	timePeriod = j.value("period", timePeriod);

	if (platformType == POINT_TO_POINT) {//0
		this->InitPoinToPoint();
	}
	else if (platformType == TRAVELLING) {//1

		this->InitTravel();
	}
	else if (platformType == ROTATION) {//2 ARREGLADO
		this->InitRotation();
	}
	else if (platformType == ROTATION_ALWAYS_WITH_TIME) {//3 ARREGLADO
		this->InitRotationInfinity();
	}
	else if (platformType == POINT_TO_POINT_ROTATION_ALWAYS) {//4
		this->InitPoinToPointRotationInfinity();
	}
	else if (platformType == TRAVELLING_ROTATION_ALWAYS) {//5
		this->InitTravelRotationInfinity();
	}
	else if (platformType == TRAVELLING_NOT_ROTATION) {//6
		this->InitialPositionTravelStateNotRotation();
	}
	else if (platformType == TRAVELLING_ALWAYS_WITHOUT_TIME) { //7
		this->InitRotationInfinityNoDt();
	}
	else if (platformType == TO_POSITION_WITH_CURVE_NOT_ROTATION) { //8
		this->InitToPositionWithCurveNotRotation();
	}
	else if (platformType == TO_MOVEMENT_SIN_MOVEMENT) {//9 
		this->InitToSineMovement();
	}


	
}

void CAIMobilePlatform::debugInMenu() {
	ImGui::Text("Curva: %s ", _curve->getName().c_str());
}

void CAIMobilePlatform::renderDebug() {
	
}



void CAIMobilePlatform::onActiveMsgReceived(const TMsgSetActive & msg) {

	setPaused(msg.active);

}

void CAIMobilePlatform::registerMsgs()
{
	DECL_MSG(CAIMobilePlatform, TMsgSetActive, onActiveMsgReceived);
	DECL_MSG(CAIMobilePlatform, TMsgEntityTriggerEnter, onTriggerEnter);
}

float CAIMobilePlatform::computeLength() const
{
	const PxU32 nbSegments = mNbPts - 1;
	float totalLength = 0.0f;
	for (PxU32 i = 0; i < nbSegments; i++)
	{
		const PxU32 a = i % mNbPts;
		const PxU32 b = (i + 1) % mNbPts;
		totalLength += VEC3::Distance(positions[b], positions[a]);
		//totalLength += (positions[b] - positions[a]).magnitude();
	}
	return totalLength;
}

void CAIMobilePlatform::onTriggerEnter(const TMsgEntityTriggerEnter& msg) {

	CEntity* e_wall = getEntityById(pair_object_id);
	if (e_wall == msg.h_entity && trigger_id != 0) { //si detecta la otra pared y tiene un trigger id diferente de 0, 0 es que no tiene
		TMsgSetActive msgToSend; 
		msgToSend.active = true; 
		CEntity* e_trigger = getEntityById(trigger_id); //tenemos el id de nuestro area de damage
		e_trigger->sendMsg(msgToSend); //activa el area de damage
	}

}



void CAIMobilePlatform::setCurve(const CCurve* curve) {

	this->_curve = curve;
	_knots = _curve->_knots;


	/*
	positions.clear();
	for (int i = 0; i < _knots.size(); i++) {
		positions.push_back(_knots[i]);
	}*/
}



