#include "mcv_platform.h"
#include "transform.h"

MAT44 CTransform::asMatrix() const {
  return 
      MAT44::CreateScale(scale)
    * MAT44::CreateFromQuaternion(rotation)
    * MAT44::CreateTranslation(position)
    ;
}

// ---------------------------
CTransform CTransform::combineWith(const CTransform& delta_transform) const {
  CTransform new_t;
  new_t.rotation = delta_transform.rotation * rotation;

  VEC3 delta_pos_rotated = VEC3::Transform(delta_transform.position, rotation);
  new_t.position = position + (delta_pos_rotated * scale);

  new_t.scale = scale * delta_transform.scale;
  return new_t;
}


VEC3 CTransform::getFront() const {
  return -MAT44::CreateFromQuaternion(rotation).Forward();
}

VEC3 CTransform::getUp() const {
  return MAT44::CreateFromQuaternion(rotation).Up();
}

VEC3 CTransform::getLeft() const {
  return -MAT44::CreateFromQuaternion(rotation).Left();
}

VEC3 CTransform::getRight() const {
	return -MAT44::CreateFromQuaternion(rotation).Right();
}


void CTransform::fromMatrix(MAT44 mtx) {
  VEC3 scale3;
  bool is_valid = mtx.Decompose(scale3, rotation, position);
  scale = scale3.x;
}

void CTransform::lookAt(VEC3 eye, VEC3 target, VEC3 up_aux) {
  position = eye;
  VEC3 front = target - eye;
  float yaw, pitch, roll;
  getAngles(&yaw, &pitch, &roll);
  vectorToYawPitch(front, &yaw, &pitch);
  setAngles(yaw, pitch, roll);
}

void CTransform::getAngles(float* yaw, float* pitch, float* roll ) {
  VEC3 front = getFront();
  vectorToYawPitch(front, yaw, pitch);

  // If requested...
  if (roll) {
    VEC3 roll_zero_left = VEC3(0, 1, 0).Cross(getFront());
    VEC3 roll_zero_up = VEC3(getFront()).Cross(roll_zero_left);
    VEC3 my_real_left = getLeft();
    float rolled_left_on_up = my_real_left.Dot(roll_zero_up);
    float rolled_left_on_left = my_real_left.Dot(roll_zero_left);
    *roll = atan2f(rolled_left_on_up, rolled_left_on_left);
  }
    
}

void CTransform::setAngles(float yaw, float pitch, float roll) {
  _roll = roll;
  rotation = QUAT::CreateFromYawPitchRoll(yaw, pitch, roll);
}


bool CTransform::renderInMenu() {
  VEC3 front = getFront();
  VEC3 up = getUp();
  VEC3 left = getLeft();
  VEC3 right = getRight();

  bool changed_pos = ImGui::DragFloat3("Pos", &position.x, 0.025f, -350.f, 350.f);

  bool changed = false;
  float yaw, pitch, roll = 0.f;
  getAngles(&yaw, &pitch, &roll);
  float yaw_deg = rad2deg(yaw);
  if (ImGui::DragFloat("Yaw", &yaw_deg, 0.1f, -180.0f, 180.0f)) {
    changed = true;
    yaw = deg2rad(yaw_deg);
  }

  float pitch_deg = rad2deg(pitch);
  float max_pitch = 90.0f - 1e-3f;
  if (ImGui::DragFloat("Pitch", &pitch_deg, 0.1f, -max_pitch, max_pitch)) {
    changed = true;
    pitch = deg2rad(pitch_deg);
  }

  float roll_deg = rad2deg(roll);
  if (ImGui::DragFloat("Roll", &roll_deg, 0.1f, -180.0f, 180.0f)) {
    changed = true;
    roll = deg2rad(roll_deg);
  }

  if (changed) 
    setAngles(yaw, pitch, roll );

  bool changed_scaled = ImGui::DragFloat("Scale", &scale, 0.01f, 0.001f, 10.f);
  ImGui::LabelText("Front", "%f %f %f", front.x, front.y, front.z);
  ImGui::LabelText("Up", "%f %f %f", up.x, up.y, up.z);
  ImGui::LabelText("Left", "%f %f %f", left.x, left.y, left.z);
  ImGui::LabelText("Right", "%f %f %f", right.x, right.y, right.z);
  ImGui::Text("Quat: %f %f %f %f", getRotation().x, getRotation().y, getRotation().z, getRotation().w);
  return changed | changed_pos | changed_scaled;
}

float CTransform::getDeltaYawToAimTo(VEC3 target) const {
  VEC3 dir_to_target = target - position;
  float dot_left = getLeft().Dot(dir_to_target);
  float dot_front = getFront().Dot( dir_to_target );
  return atan2f(dot_left, dot_front);
}

float CTransform::getDeltaPitchToAimTo(VEC3 target) const {
  VEC3 dir_to_target = target - position;
  return -atan2f(dir_to_target.y, sqrt((dir_to_target.x * dir_to_target.x) + (dir_to_target.z * dir_to_target.z)));
}

bool CTransform::load(const json& j) {

  if (j.count("pos"))
    position = loadVEC3(j, "pos");

  if (j.count("rotation"))
    setRotation(loadQUAT(j, "rotation"));

  if (j.count("lookat"))
    lookAt(getPosition(), loadVEC3(j, "lookat"));

  if (j.count("axis")) {
    VEC3 axis = loadVEC3(j, "axis");
    float angle_deg = j.value("angle", 0.f);
    float angle_rad = deg2rad(angle_deg);
    setRotation(QUAT::CreateFromAxisAngle(axis, angle_rad));
  }

  if (j.count("yaw") || j.count("pitch")) {
    float pitch = deg2rad(j.value("pitch", 0.0f));
    float yaw = deg2rad(j.value("yaw", 0.0f));
    setAngles(yaw, pitch);
  }

  if (j.count("scale"))
    scale = j.value("scale", 1.0f);

  float yaw, pitch, roll = 0.f;
  getAngles(&yaw, &pitch, &roll);
  if (j.count("pitch")) {
    pitch = j.value("pitch", 0.0f);
    pitch = deg2rad(pitch);
  }
    
  setAngles(yaw, pitch, roll);

  return true;
}

/*
Rotate CTransform towards Vector3 over time
*/
void CTransform::rotateTowards(VEC3 target_point, float rotSpeed, float dt) {
	float rotSpeedRad = deg2rad(rotSpeed);
	float y, r, p;
	getAngles(&y, &p, &r);

	float deltaYaw = getDeltaYawToAimTo(target_point);
	if (fabsf(deltaYaw) <= rotSpeedRad * dt) {
		y += deltaYaw;
	}
	else {
		if (isInLeft(target_point))
		{
			y += rotSpeedRad * dt;
		}
		else {
			y -= rotSpeedRad * dt;
		}
	}
	setAngles(y, p, r);
}

/*
Rotate CTransform towards Vector3 instantly
*/
void CTransform::rotateTowards(VEC3 target_point) {
	float y, r, p;
	getAngles(&y, &p, &r);

	float deltaYaw = getDeltaYawToAimTo(target_point);
	y += deltaYaw;
	setAngles(y, p, r);
}

/*
Incline CTransform towards Vector3 instantly
*/
void CTransform::inclineTo(VEC3 target_point) {
  float y, r, p;
  getAngles(&y, &p, &r);

  float deltaPitch = getDeltaPitchToAimTo(target_point);
  p = deltaPitch;
  setAngles(y, p, r);
}

VEC3 CTransform::getTranslatePositionForAngle(VEC3 startPoint, float distance, float angle) {
	float yaw, pitch, roll = 0.f;
	getAngles(&yaw, &pitch);
	float x = distance * sin(deg2rad(angle) + yaw);
	float z = distance * cos(deg2rad(angle) + yaw);
	VEC3 newPosition = startPoint;
	newPosition.x += x;
	newPosition.z += z;
	return newPosition;
}

