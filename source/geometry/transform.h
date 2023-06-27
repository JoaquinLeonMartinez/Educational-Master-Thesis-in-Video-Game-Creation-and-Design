#pragma once

class CTransform
{
  QUAT  rotation = QUAT::Identity;
  VEC3  position = VEC3::Zero;
  float scale = 1.0f;
  float _roll = 0.0f;

public:

  CTransform() = default;
  CTransform(VEC3 new_pos, QUAT new_rot, float new_scale = 1.0f)
    : rotation(new_rot)
    , position(new_pos)
    , scale(new_scale)
  {}

  const VEC3& getPosition() const { return position; }
  const QUAT& getRotation() const { return rotation; }
  void setPosition(VEC3 new_position) { position = new_position; }
  void setRotation(QUAT new_rotation) { rotation = new_rotation; }
  void setScale(float _scale) { scale = _scale; }

  MAT44 asMatrix() const;
  void fromMatrix(MAT44 mtx);

  float getScale() const { return scale; }
  

  VEC3 getFront() const;
  VEC3 getUp() const;
  VEC3 getLeft() const;
  VEC3 getRight() const;

  void getAngles(float* yaw, float* pitch, float* roll = nullptr );
  void setAngles(float yaw, float pitch, float roll = 0.0f);


  bool isInFront(VEC3 p) const {
	  VEC3 delta = p - position;
	  return delta.Dot(getFront()) > 0.f;
  }

  bool isInLeft(VEC3 p) const {
	  VEC3 delta = p - position;
	  return delta.Dot(getLeft()) > 0.f;
  }

  void lookAt(VEC3 eye, VEC3 target, VEC3 up_aux = VEC3(0, 1, 0));
  void rotateTowards(VEC3 target_point, float rotSpeed, float dt);
  void rotateTowards(VEC3 target_point);
  void inclineTo(VEC3 target_point);

  CTransform combineWith(const CTransform& delta_transform) const;

  float getDeltaYawToAimTo(VEC3 target) const;
  float getDeltaPitchToAimTo(VEC3 target) const;

  bool load(const json& j);

  bool renderInMenu();

  VEC3 getTranslatePositionForAngle(VEC3 startPoint, float distance, float angle);
};
