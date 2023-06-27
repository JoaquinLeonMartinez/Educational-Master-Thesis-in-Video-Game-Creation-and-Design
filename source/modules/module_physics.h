#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "components/common/physics/comp_collider.h"
//#include "physics/comp_ragdoll.h"
#include "components/common/physics/comp_collider.h"
#include "components/john/comp_character_collider.h"


#define VEC3_TO_PXVEC3(vec3) physx::PxVec3(vec3.x, vec3.y, vec3.z)
#define VEC3_TO_PXEXVEC3(vec3) physx::PxExtendedVec3(vec3.x, vec3.y, vec3.z)
#define PXVEC3_TO_VEC3(pxvec3) VEC3(pxvec3.x, pxvec3.y, pxvec3.z)
#define PXEXVEC3_TO_VEC3(pxexvec3) VEC3(pxexvec3.x, pxexvec3.y, pxexvec3.z)

#define QUAT_TO_PXQUAT(quat) physx::PxQuat(quat.x, quat.y, quat.z, quat.w)
#define PXQUAT_TO_QUAT(pxquat) QUAT(pxquat.x, pxquat.y, pxquat.z,pxquat.w)

extern CTransform toTransform(physx::PxTransform pxtrans);
extern physx::PxTransform toPxTransform(CTransform mcvtrans);

class CModulePhysics : public IModule
{

	physx::PxRigidActor* createController(TCompCollider& comp_collider);
	bool readShape(physx::PxRigidActor* actor, const json& jcfg);

public:

  physx::PxScene*				gScene = NULL;
  static const VEC3 gravity;
  float threshold = 0.01f;

  enum FilterGroup {
    Wall = 1 << 0,
    Floor = 1 << 1,
    Player = 1 << 2,
    TeleportableObj = 1 << 3,
    EnemyProjectile = 1 << 4,
    Grenade = 1 << 5,
    Cupcake = 1 << 6,
    Sushi = 1 << 7,
    Golem = 1 << 8,
    Trigger = 1 << 9,
    Spawner = 1 << 10,
    Battery = 1 << 11,
    Puddle = 1 << 12,
    Carrito = 1 << 13,
    DestroyableWall = 1 << 14,
    ExplosiveObj = 1 << 15,
    Platform = 1 << 16,
    Column = 1 << 17,
	  Panel = 1 << 18,
    Product = 1 << 19,
    Weapon = 1 << 20,

    CCD_FLAG = 1 << 29,
    NotWeapon = Wall | Floor | TeleportableObj | EnemyProjectile | Grenade | Cupcake | Sushi | Golem | Trigger | Spawner | Battery | Puddle | Carrito | DestroyableWall | ExplosiveObj | Platform | Column | Panel | Product,
    Obstacle = Wall | Column /*| Spawner*/ | TeleportableObj | ExplosiveObj | DestroyableWall,
	  Scenario = Wall | Floor,
    CameraJump = Floor | Platform, //discuss if you can jump onto more things like explosive things
	  PlayerAndScenario = Player | Scenario,
		ProyectileEnemy = PlayerAndScenario | Panel | DestroyableWall,
    Enemy = Cupcake | Sushi | Golem | Spawner,
    Characters = Player | Enemy,
    TeleportableEnemy = Cupcake | Sushi,
    NotPlayer = Scenario | Grenade | Enemy,
    TpRay = TeleportableEnemy | Scenario | TeleportableObj | Grenade,
    CameraCollision = Scenario | TeleportableObj | ExplosiveObj | DestroyableWall | Spawner | Panel,
    VulnerableToMelee = Enemy | Puddle | DestroyableWall | Panel,
	  NotSpawnerObjects = TeleportableObj | Enemy | Player,
    VulnerableToFire = ExplosiveObj | Enemy | DestroyableWall | Product,
    VulnerableToExplosions = ExplosiveObj | Characters,
	  ExplosiveAndTeleportable = ExplosiveObj | TeleportableObj,
		All = -1
	};

  FilterGroup getFilterByName(const std::string& name, bool isGroup = false);
  void setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask);
  void setupFilteringOnAllShapesOfActor(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);
  void createActor(TCompCollider& compCollider);
  //void createRagdoll(TCompRagdoll& compRagdoll);
  //void createRagdollJoints(TCompRagdoll& comp_ragdoll, int bone_id);
	static physx::PxQueryFilterData defaultFilter;

	CModulePhysics(const std::string& aname) : IModule(aname) { }
	bool start() override;
	void stop() override;
	void update(float delta) override;
	void renderDebug() override;
	void renderInMenu() override;

	bool isActive() const;

	physx::PxScene* getScene() const;

	/* Ray casting & related methods*/
	bool Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlags flag = physx::PxQueryFlag::eSTATIC, physx::PxQueryFilterData filterdata = defaultFilter);
	bool Sweep(physx::PxGeometry& geometry, const VEC3& position, const QUAT& rotation, const VEC3& direction, float distance, std::vector<physx::PxSweepHit>& hits, physx::PxQueryFlags flag = physx::PxQueryFlag::eSTATIC, physx::PxQueryFilterData filterdata = defaultFilter);
	bool Overlap(physx::PxGeometry& geometry, VEC3 pos, std::vector<physx::PxOverlapHit> & hits, physx::PxQueryFilterData filterdata);

	class CustomSimulationEventCallback : public physx::PxSimulationEventCallback


	{
		// Implements PxSimulationEventCallback
		virtual void							onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
		virtual void							onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
		virtual void							onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) {}
		virtual void							onWake(physx::PxActor**, physx::PxU32) {}
		virtual void							onSleep(physx::PxActor**, physx::PxU32) {}
		virtual void							onAdvance(const physx::PxRigidBody*const*, const physx::PxTransform*, const physx::PxU32) {}

	};
	 CustomSimulationEventCallback customSimulationEventCallback;
 
  class CustomControllerBehaviorCallback : public physx::PxControllerBehaviorCallback
  {
  public:
	  virtual  physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor);
	  virtual  physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxController& controller);
	  virtual  physx::PxControllerBehaviorFlags	getBehaviorFlags(const physx::PxObstacle& obstacle);
  };
  CustomControllerBehaviorCallback customControllerBehaviorCallback;
  

	class CustomUserControllerHitReport : public physx::PxUserControllerHitReport
	{
	public:

		virtual void onShapeHit(const physx::PxControllerShapeHit& hit);
		virtual void onControllerHit(const physx::PxControllersHit& hit);
		virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit);
	};

	CustomUserControllerHitReport customUserControllerHitReport;



	struct TJoint {
		struct TObj {
			std::string          name;
			CHandle              h_collider;
			CTransform           transform;
			physx::PxRigidActor* actor = nullptr;
		};
		std::string joint_type;
		TObj obj0;
		TObj obj1;
		void create();
		physx::PxJoint*         px_joint = nullptr;
	};

	typedef std::vector< TJoint > VJoints;
	VJoints joints;
	void debugActor(const char* title, physx::PxRigidActor* actor);
	void debugInMenuJoints();
	void debugInMenuJoint(TJoint& c);
	void debugInMenuJointObj(TJoint& j, physx::PxJointActorIndex::Enum idx);
};
