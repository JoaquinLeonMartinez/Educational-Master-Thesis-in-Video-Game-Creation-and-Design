#include "mcv_platform.h"
#include "module_physics.h"
#include "entity/entity.h"
#include "handle/handle.h"
#include "components/common/comp_transform.h"
#include "skeleton/comp_skeleton.h"
#include "skeleton/game_core_skeleton.h"
#include "cal3d/cal3d.h"
#include "components/common/comp_name.h"
#include "components/controllers/character/comp_character_controller.h"
#include "render/meshes/collision_mesh.h"
#include "skeleton/cal3d2engine.h"

#include "PxPhysicsAPI.h"

#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PxFoundation_x64.lib")
#pragma comment(lib, "PxPvdSDK_x64.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")

using namespace physx;
physx::PxQueryFilterData CModulePhysics::defaultFilter;

// -----------------------------------------------------
CTransform toTransform(PxTransform pxtrans) {
  CTransform trans;
  trans.setPosition(PXVEC3_TO_VEC3(pxtrans.p));
  trans.setRotation(PXQUAT_TO_QUAT(pxtrans.q));
  return trans;
}

// -----------------------------------------------------
PxTransform toPxTransform(CTransform mcvtrans) {
  PxTransform trans;
  trans.p = VEC3_TO_PXVEC3(mcvtrans.getPosition());
  trans.q = QUAT_TO_PXQUAT(mcvtrans.getRotation());
  return trans;
}

// -----------------------------------------------------
const VEC3 CModulePhysics::gravity = VEC3(0.0f, -9.81, 0.0f);

PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;
PxFoundation*           gFoundation = nullptr;
PxPhysics*              gPhysics = nullptr;
PxControllerManager*    gControllerManager = nullptr;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene*                gScene = nullptr;
PxMaterial*             gMaterial = nullptr;
PxPvd*                  gPvd = nullptr;

// -----------------------------------------------------
static PxGeometryType::Enum readGeometryType(const json& j) {
  PxGeometryType::Enum geometryType = PxGeometryType::eINVALID;
  if (!j.count("shape"))
    return geometryType;

  std::string jgeometryType = j["shape"];
  if (jgeometryType == "sphere") {
    geometryType = PxGeometryType::eSPHERE;
  }
  else if (jgeometryType == "box") {
    geometryType = PxGeometryType::eBOX;
  }
  else if (jgeometryType == "plane") {
    geometryType = PxGeometryType::ePLANE;
  }
  else if (jgeometryType == "convex") {
    geometryType = PxGeometryType::eCONVEXMESH;
  }
  else if (jgeometryType == "capsule") {
    geometryType = PxGeometryType::eCAPSULE;
  }
  else if (jgeometryType == "trimesh") {
    geometryType = PxGeometryType::eTRIANGLEMESH;
  }
  else {
    dbg("Unsupported shape type in collider component %s", jgeometryType.c_str());
  }
  return geometryType;
}

bool CModulePhysics::isActive() const {
  return gScene != nullptr;
}

PxScene* CModulePhysics::getScene() const {
  return gScene;
}

// -----------------------------------------------------------------------------
PxRigidActor* CModulePhysics::createController(TCompCollider& comp_collider) {
  const json& jconfig = comp_collider.jconfig;

  PX_ASSERT(desc.mType == PxControllerShapeType::eCAPSULE);

  PxCapsuleControllerDesc capsuleDesc;
  capsuleDesc.height = jconfig.value("height", 1.f);
  capsuleDesc.radius = jconfig.value("radius", 1.f);
  capsuleDesc.climbingMode = PxCapsuleClimbingMode::eEASY;
  capsuleDesc.material = gMaterial;
  capsuleDesc.stepOffset = 0.01;
  capsuleDesc.contactOffset = 0.01f;
  capsuleDesc.reportCallback = &customUserControllerHitReport;
  capsuleDesc.behaviorCallback = &customControllerBehaviorCallback;
  PxCapsuleController* ctrl = static_cast<PxCapsuleController*>(gControllerManager->createController(capsuleDesc));
  PX_ASSERT(ctrl);
  TCompTransform* c = comp_collider.get<TCompTransform>();
  VEC3 pos = c->getPosition();
  ctrl->setFootPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
  PxRigidActor* actor = ctrl->getActor();
  comp_collider.controller = ctrl;

  setupFilteringOnAllShapesOfActor(actor,
    getFilterByName(jconfig.value("group", "all"), true),
    getFilterByName(jconfig.value("mask", "all"))
  );

  return actor;
}

// ---------------------------------------------------------------------
// Reads a single shape for jcfg and adds it to actor
bool CModulePhysics::readShape(PxRigidActor* actor, const json& jcfg) {

  // Shapes....
  PxGeometryType::Enum geometryType = readGeometryType(jcfg);
  if (geometryType == PxGeometryType::eINVALID)
    return false;

  PxShape* shape = nullptr;
  if (geometryType == PxGeometryType::eBOX)
  {
    VEC3 jhalfExtent = loadVEC3(jcfg, "half_size");
    PxVec3 halfExtent = VEC3_TO_PXVEC3(jhalfExtent);
    shape = gPhysics->createShape(PxBoxGeometry(halfExtent), *gMaterial);
  }
  else if (geometryType == PxGeometryType::ePLANE)
  {
    VEC3 jplaneNormal = loadVEC3(jcfg, "normal");
    float jplaneDistance = jcfg.value("distance", 0.f);
    PxVec3 planeNormal = VEC3_TO_PXVEC3(jplaneNormal);
    PxReal planeDistance = jplaneDistance;
    PxPlane plane(planeNormal, planeDistance);
    PxTransform offset = PxTransformFromPlaneEquation(plane);
    shape = gPhysics->createShape(PxPlaneGeometry(), *gMaterial);
    shape->setLocalPose(offset);
  }
  else if (geometryType == PxGeometryType::eSPHERE)
  {
    PxReal radius = jcfg.value("radius", 1.f);;
    shape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);
  }
  else if (geometryType == PxGeometryType::eCAPSULE)
  {
    if (jcfg.count("controller") == 0)
      fatal("Capsules not implemented yet");
  }
  else if (geometryType == PxGeometryType::eCONVEXMESH)
  {
   // fatal("Convex mesh not implemented yet");
	std::string col_mesh_name = jcfg.value("collision_mesh", "");
	const CCollisionMesh* mesh = Resources.get(col_mesh_name)->as<CCollisionMesh>();
	dbg("Collision mesh has %d vtxs\n", mesh->header.num_vertex);

	assert(strcmp(mesh->header.vertex_type_name, "Pos") == 0);

	PxConvexMeshDesc meshDesc;
	meshDesc.points.count = mesh->header.num_vertex;
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.points.data = mesh->vertices.data();

	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	PxTolerancesScale scale;
	PxCooking *cooking = PxCreateCooking(PX_PHYSICS_VERSION, gPhysics->getFoundation(), PxCookingParams(scale));
	physx::PxConvexMeshCookingType::Enum convextype = physx::PxConvexMeshCookingType::eQUICKHULL;

	physx::PxCookingParams params = cooking->getParams();
	params.convexMeshCookingType = convextype;
	params.gaussMapLimit = 256;
	cooking->setParams(params);

	PxDefaultMemoryOutputStream writeBuffer;
	PxConvexMeshCookingResult::Enum result;
	bool status = cooking->cookConvexMesh(meshDesc, writeBuffer, &result);
	assert(status);

	// writeBuffer could be saved to avoid the cooking in the next execution.
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxConvexMesh* convexMesh = gPhysics->createConvexMesh(readBuffer);
	shape = gPhysics->createShape(PxConvexMeshGeometry(convexMesh), *gMaterial);

	CMesh* render_mesh = mesh->createRenderMesh();
	char res_name[64];
	sprintf(res_name, "Physics_%p", render_mesh);
	render_mesh->setNameAndType(res_name, getResourceTypeFor<CMesh>());
	Resources.registerResource(render_mesh);


	//shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	// Bind the render mesh as userData of the SHAPE, not the ACTOR.
	shape->userData = render_mesh;


  }
  else if (geometryType == PxGeometryType::eTRIANGLEMESH)
  {
    std::string col_mesh_name = jcfg.value("collision_mesh", "");
    const CCollisionMesh* mesh = Resources.get(col_mesh_name)->as<CCollisionMesh>();
    dbg("Collision mesh has %d vtxs\n", mesh->header.num_vertex);

    assert(strcmp(mesh->header.vertex_type_name, "Pos") == 0);

    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = mesh->header.num_vertex;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = mesh->vertices.data();

    // Swap faces for physics
    meshDesc.flags |= PxMeshFlag::eFLIPNORMALS;

    // Indices
    meshDesc.triangles.count = mesh->header.num_indices / 3;
    meshDesc.triangles.stride = 3 * mesh->header.bytes_per_index;
    meshDesc.triangles.data = mesh->indices.data();
    if (mesh->header.bytes_per_index == 2)
      meshDesc.flags |= PxMeshFlag::e16_BIT_INDICES;

    // We could save this cooking process
    PxTolerancesScale scale;
    PxCooking *cooking = PxCreateCooking(PX_PHYSICS_VERSION, gPhysics->getFoundation(), PxCookingParams(scale));

    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    bool status = cooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
    assert(status);

    // writeBuffer could be saved to avoid the cooking in the next execution.
    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* tri_mesh = gPhysics->createTriangleMesh(readBuffer);
    shape = gPhysics->createShape(PxTriangleMeshGeometry(tri_mesh), *gMaterial);

    // We could create CMesh and save it in shape->userData
    CMesh* render_mesh = mesh->createRenderMesh();
    char res_name[64];
    sprintf(res_name, "Physics_%p", render_mesh);
    render_mesh->setNameAndType(res_name, getResourceTypeFor<CMesh>());
    Resources.registerResource(render_mesh);

	
	//shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    // Bind the render mesh as userData of the SHAPE, not the ACTOR.
    shape->userData = render_mesh;
	
  }

  if (jcfg.value("trigger", false))
  {
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
  }

  // Shape offset
  if (jcfg.count("offset")) {
    CTransform trans;
    trans.load(jcfg["offset"]);
    shape->setLocalPose(toPxTransform(trans));
  }

  assert(shape);

  setupFiltering(
    shape,
    getFilterByName(jcfg.value("group", "all"), true),
    getFilterByName(jcfg.value("mask", "all"))
  );

  actor->attachShape(*shape);
  shape->release();
  return true;
}

// --------------------------------------------------------------
void CModulePhysics::createActor(TCompCollider& comp_collider)
{
  const json& jconfig = comp_collider.jconfig;

  // Entity start transform
  TCompTransform* c = comp_collider.get<TCompTransform>();
  PxTransform transform = toPxTransform(*c);

  PxRigidActor* actor = nullptr;

  bool is_controller = jconfig.count("controller") > 0;

  // Controller or physics based?
  if (is_controller) {
    actor = createController(comp_collider);

  }
  else {

    // Dynamic or static actor?
    bool isDynamic = jconfig.value("dynamic", false);
    if (isDynamic)
    {
      PxRigidDynamic* dynamicActor = gPhysics->createRigidDynamic(transform);
      PxReal density = jconfig.value("density", 1.f);
      PxRigidBodyExt::updateMassAndInertia(*dynamicActor, density);
      actor = dynamicActor;

      if( jconfig.value("kinematic", false)) {
        dynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	  }
    }
    else
    {
      PxRigidStatic* static_actor = gPhysics->createRigidStatic(transform);
      actor = static_actor;
    }
  }

  // The capsule was already loaded as part of the controller
  if (!is_controller) {
    // if shapes exists, try to read as an array of shapes
    if (jconfig.count("shapes")) {
      const json& jshapes = jconfig["shapes"];
      for (const json& jshape : jshapes)
        readShape(actor, jshape);
    }
  // Try to read shape directly (general case...)
  else {
      readShape(actor, jconfig);
    }
  }

  // PhysX is complaining 'adding the actor twice...'
  if (!is_controller)
    gScene->addActor(*actor);

  comp_collider.actor = actor;
  CHandle h_collider(&comp_collider);
  comp_collider.actor->userData = h_collider.asVoidPtr();

  // Joints
  if (jconfig.count("joints")) {
    for (const json& j : jconfig["joints"]) {
      TJoint joint;
      joint.joint_type = j.value("type", "spherical");
      joint.obj0.name = j["obj0"].value("name", "");
      joint.obj0.transform.load(j["obj0"]["transform"]);
      joint.obj1.name = j["obj1"].value("name", "");
      joint.obj1.transform.load(j["obj1"]["transform"]);
      joint.create();
      joints.push_back(joint);
    }
  }
}
/*
void CModulePhysics::createRagdoll(TCompRagdoll& comp_ragdoll) {
  if (comp_ragdoll.ragdoll.created)
    return;
  CHandle h_comp_ragdoll(&comp_ragdoll);
  CEntity* e = h_comp_ragdoll.getOwner();

  TCompSkeleton* skel = e->get<TCompSkeleton>();
  TCompTransform* comp_transform = e->get<TCompTransform>();
  CTransform* entity_trans = (CTransform*)comp_transform;

  auto core_skel = (CGameCoreSkeleton*)skel->model->getCoreModel();

  core_skel->ragdoll_core;

  for (auto& ragdoll_bone_core : core_skel->ragdoll_core.ragdoll_bone_cores) {
    auto cal_core_bone = core_skel->getCoreSkeleton()->getCoreBone(ragdoll_bone_core.bone);
    assert(cal_core_bone);
    CTransform trans;
    trans.setPosition(Cal2DX(cal_core_bone->getTranslationAbsolute()));
    trans.setRotation(Cal2DX(cal_core_bone->getRotationAbsolute()));
    trans.setPosition(trans.getPosition() + trans.getLeft()*ragdoll_bone_core.height*0.5f);

    PxTransform px_entity_transform = toPxTransform(*entity_trans);
    PxTransform px_transform = toPxTransform(trans);
    px_transform = px_entity_transform * px_transform;

    PxRigidActor* actor = nullptr;
    PxRigidDynamic* dynamicActor = gPhysics->createRigidDynamic(px_transform);
    PxRigidBodyExt::updateMassAndInertia(*dynamicActor, 1.f);
    actor = dynamicActor;

    PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(ragdoll_bone_core.radius, ragdoll_bone_core.height), *gMaterial);
    setupFiltering(shape, CModulePhysics::FilterGroup::All, CModulePhysics::FilterGroup::All);
    CTransform offset;
    offset.setPosition(VEC3(0.f, ragdoll_bone_core.height, 0.f));
    shape->setLocalPose(toPxTransform(offset));

    actor->attachShape(*shape);
    shape->release();

    PxRigidDynamic* body = (PxRigidDynamic*)actor;
    assert(body);

    TRagdoll::TRagdollBone& ragdoll_bone = comp_ragdoll.ragdoll.bones[comp_ragdoll.ragdoll.num_bones];
    ragdoll_bone.actor = body;
    ragdoll_bone.core = &ragdoll_bone_core;

    ragdoll_bone.idx = core_skel->getCoreSkeleton()->getCoreBoneId(ragdoll_bone_core.bone);
    ragdoll_bone.core->instance_idx = comp_ragdoll.ragdoll.num_bones;
    ++comp_ragdoll.ragdoll.num_bones;
  }

  for (int i = 0; i < comp_ragdoll.ragdoll.num_bones; ++i) {
    auto& ragdoll_bone = comp_ragdoll.ragdoll.bones[i];
    if (ragdoll_bone.core->parent_core) {
      ragdoll_bone.parent_idx = ragdoll_bone.core->parent_core->instance_idx;
      auto& parent_ragdoll_bone = comp_ragdoll.ragdoll.bones[ragdoll_bone.parent_idx];

      parent_ragdoll_bone.children_idxs[parent_ragdoll_bone.num_children] = i;
      ++parent_ragdoll_bone.num_children;
    }
  }

  createRagdollJoints(comp_ragdoll, 0);

  comp_ragdoll.ragdoll.created = true;

}
*/
/*
void CModulePhysics::createRagdollJoints(TCompRagdoll& comp_ragdoll, int bone_id) {
  TRagdoll::TRagdollBone& ragdoll_bone = comp_ragdoll.ragdoll.bones[bone_id];

  for (int i = 0; i < ragdoll_bone.num_children; ++i) {
    auto child_id = ragdoll_bone.children_idxs[i];
    TRagdoll::TRagdollBone& child_ragdoll_bone = comp_ragdoll.ragdoll.bones[child_id];

    PxJoint* joint = nullptr;

    PxVec3 offset(0.1f, 0.f, 0.f);

    assert(child_ragdoll_bone.actor);
    assert(ragdoll_bone.actor);
    auto d1 = child_ragdoll_bone.actor->getGlobalPose().q.rotate(PxVec3(1, 0, 0));
    auto d2 = ragdoll_bone.actor->getGlobalPose().q.rotate(PxVec3(1, 0, 0));
    auto axis = d1.cross(d2).getNormalized();
    auto pos = ragdoll_bone.actor->getGlobalPose().p;
    auto diff = (pos - child_ragdoll_bone.actor->getGlobalPose().p).getNormalized();
    if (diff.dot(d2) < 0) d2 = -d2;

    PxShape* shape;
    if (ragdoll_bone.actor->getShapes(&shape, 1) == 1)
    {
      PxCapsuleGeometry capsule;
      if (shape->getCapsuleGeometry(capsule))
      {
        pos -= (capsule.halfHeight + capsule.radius) * d2;
      }
    }

    PxTransform tr0 = PxTransform(PxVec3(0.f, 0.f, 0.f));
    PxMat44 mat(d1, axis, d1.cross(axis).getNormalized(), pos);
    PxTransform pxTrans(mat);
    if (pxTrans.isSane())
    {
      PxTransform ragdoll_bone_actor_trans = ragdoll_bone.actor->getGlobalPose();
      if (ragdoll_bone_actor_trans.isSane())
      {
        PxTransform ragdoll_bone_actor_trans_inv = ragdoll_bone_actor_trans.getInverse();
        if (ragdoll_bone_actor_trans_inv.isSane())
        {
          tr0 = ragdoll_bone_actor_trans_inv * pxTrans;
        }
      }
    }

    PxTransform tr1 = child_ragdoll_bone.actor->getGlobalPose().getInverse() * ragdoll_bone.actor->getGlobalPose() * tr0;
    if (!tr1.isValid()) {
      tr1 = PxTransform(PxVec3(0.f, 0.f, 0.f));
    }
    //-----
    //PxSphericalJoint joint
    joint = PxSphericalJointCreate(gScene->getPhysics(), ragdoll_bone.actor, tr0, child_ragdoll_bone.actor, tr1);
    assert(joint);
    if (joint)
    {
      auto* spherical = static_cast<PxSphericalJoint*>(joint);
      spherical->setProjectionLinearTolerance(0.1f);
      spherical->setLimitCone(physx::PxJointLimitCone(0.01f, 0.f, 0.01f));
      spherical->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);
    }
    /*

    //PxRevoluteJoint joint
    joint =
    PxRevoluteJointCreate(mScene->getPhysics(), ragdoll_bone.actor, tr0, child_ragdoll_bone.actor, tr1);
    if (joint)
    {
    auto* hinge = static_cast<PxRevoluteJoint*>(joint);
    hinge->setProjectionLinearTolerance(0.1f);
    hinge->setProjectionAngularTolerance(0.01f);
    hinge->setLimit(physx::PxJointAngularLimitPair(-PxPi / 4, PxPi / 4, 0.01f));
    hinge->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
    }

    //fixed joint
    joint = PxFixedJointCreate(mScene->getPhysics(), ragdoll_bone.actor, tr0, child_ragdoll_bone.actor, tr1);
    assert(joint);
    if (joint)
    {
    auto* fixed_joint = static_cast<PxFixedJoint*>(joint);
    fixed_joint->setProjectionLinearTolerance(0.1f);
    fixed_joint->setProjectionAngularTolerance(0.01f);
    }
    
    if (joint)
    {
      joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
      joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
      joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
    }
    child_ragdoll_bone.parent_joint = joint;

    createRagdollJoints(comp_ragdoll, child_id);
  }

}*/
//-------------------------------------------------------------------


PxFilterFlags filter(PxFilterObjectAttributes	attributes0,
	PxFilterData				filterData0,
	PxFilterObjectAttributes	attributes1,
	PxFilterData				filterData1,
	PxPairFlags&				pairFlags,
	const void*					constantBlock,
	PxU32						constantBlockSize)
{
	pairFlags = PxPairFlag::eSOLVE_CONTACT;
	pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
	pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
	return PxFilterFlags();
}


// ------------------------------------------------------------------
PxFilterFlags CustomFilterShader(
  PxFilterObjectAttributes attributes0, PxFilterData filterData0,
  PxFilterObjectAttributes attributes1, PxFilterData filterData1,
  PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize
)
{
  if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
  {
    if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
    {
      pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
    }
    else {
      pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
	  pairFlags |= PxPairFlag::eSOLVE_CONTACT;
	  pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
	  pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
	  return PxFilterFlags();
    }
    return PxFilterFlag::eDEFAULT;
  }
  return PxFilterFlag::eSUPPRESS;
}

// ------------------------------------------------------------------
bool CModulePhysics::start()
{
  gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

  gPvd = PxCreatePvd(*gFoundation);
  PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
  gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

  gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
  PxInitExtensions(*gPhysics, gPvd);

  PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
  sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
  gDispatcher = PxDefaultCpuDispatcherCreate(2);
  sceneDesc.cpuDispatcher = gDispatcher;
  //sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
  sceneDesc.filterShader = CustomFilterShader;  

  gScene = gPhysics->createScene(sceneDesc);
  gScene->setFlag(PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
  gScene->setFlag(PxSceneFlag::eENABLE_CCD, true);
  gScene->setFlag(PxSceneFlag::eENABLE_KINEMATIC_PAIRS, true);
  gScene->setFlag(PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS, true);

  PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
  if (pvdClient)
  {
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
  }
  gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

  gControllerManager = PxCreateControllerManager(*gScene);

  gScene->setSimulationEventCallback(&customSimulationEventCallback);


  // Set a default filter to do query checks
  physx::PxFilterData pxFilterData;
  pxFilterData.word0 = FilterGroup::Scenario;
  defaultFilter.data = pxFilterData;
  return true;
}

// ------------------------------------------------------------------
void CModulePhysics::update(float delta) {
  delta = Time.delta_unscaled;
	if (delta >= 0.03333f) { //less than 30 frames per second it will be loading
    delta = 0.03333f; //update it as if it was moving normally
	}


  {
    PROFILE_FUNCTION("Simulate");
    gScene->simulate(delta);
  }
  {
    PROFILE_FUNCTION("fetch");
    gScene->fetchResults(true);
  }
  
  PxU32 nbActorsOut = 0;
  PxActor** activeActors = gScene->getActiveActors(nbActorsOut);
  for (unsigned int i = 0; i < nbActorsOut; ++i)
  {
    PROFILE_FUNCTION("Actor");
    PxRigidActor* rigidActor = ((PxRigidActor*)activeActors[i]);
    assert(rigidActor);
    CHandle h_collider;
    h_collider.fromVoidPtr(rigidActor->userData);
    TCompCollider* c_collider = h_collider;
    if (c_collider != nullptr)
    {
      TCompTransform* c = c_collider->get<TCompTransform>();
	  if (c) {
		  if (c_collider->controller) {
			PxExtendedVec3 pxpos_ext = c_collider->controller->getFootPosition();
			c->setPosition(VEC3((float)pxpos_ext.x, (float)pxpos_ext.y, (float)pxpos_ext.z));
		  }
		  else {
			PxTransform PxTrans = rigidActor->getGlobalPose();
			c->setPosition(VEC3((float)PxTrans.p.x, (float)PxTrans.p.y, (float)PxTrans.p.z));
			c->setRotation(PXQUAT_TO_QUAT(PxTrans.q));
			//c->set(toTransform(PxTrans));
		  }
	  }
	}
  }
}

void CModulePhysics::stop() {
  gScene->release(); gScene = nullptr;
  gDispatcher->release();
  gPhysics->release();
  PxPvdTransport* transport = gPvd->getTransport();
  gPvd->release();
  transport->release();
  gFoundation->release();
}

void CModulePhysics::renderInMenu()
{
  debugInMenuJoints();
}

void CModulePhysics::renderDebug() {

  // Show all the joints
  /*for (auto& c : joints) {
    PxRigidActor* actors[2];
    c.px_joint->getActors(actors[0], actors[1]);
    for (int i = 0; i < 2; ++i) {
      if (!actors[i])
        continue;
      CTransform local_t = toTransform(c.px_joint->getLocalPose( i == 0 ? PxJointActorIndex::eACTOR0 : PxJointActorIndex::eACTOR1));
      CTransform actor_t = toTransform(actors[i]->getGlobalPose());
      MAT44 world = local_t.asMatrix() * actor_t.asMatrix();
      drawAxis(world);
    }
  }*/
}

CModulePhysics::FilterGroup CModulePhysics::getFilterByName(const std::string& name, bool isGroup)
{
  if (strcmp("player", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Player;
  }
  else if (strcmp("sushi", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Sushi;
  }
  else if (strcmp("cupcake", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Cupcake;
  }
  else if (strcmp("golem", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Golem;
  }
  else if (strcmp("characters", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Characters;
  }
  else if (strcmp("wall", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Wall;
  }
  else if (strcmp("floor", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Floor;
  }
  else if (strcmp("scenario", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Scenario;
  }
  else if (strcmp("player_scenario", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::PlayerAndScenario;
  }
	else if (strcmp("proyectile_enemy", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::ProyectileEnemy;
	}
  else if (strcmp("trigger", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Trigger;
  }
  else if (strcmp("teleportable_obj", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::TeleportableObj;
  }
  else if (strcmp("spawner", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Spawner;
  }
  else if (strcmp("enemy", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::Enemy;
  }
  else if (strcmp("puddle", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Puddle;
  }
  else if (strcmp("grenade", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Grenade;
  }
  else if (strcmp("battery", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::Battery;
  }
  else if (strcmp("destroyable_wall", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::DestroyableWall;
  }
  else if (strcmp("panel", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::Panel;
  }
  else if (strcmp("not_spawner_obj", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::NotSpawnerObjects;
  }
  else if (strcmp("carrito", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Carrito;
  }
  else if (strcmp("ExplosiveObj", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::ExplosiveObj;
  }
  else if (strcmp("VulnerableToExplosions", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::VulnerableToExplosions;
  }
  else if (strcmp("VulnerableToFire", name.c_str()) == 0) {
      return CModulePhysics::FilterGroup::VulnerableToFire;
  }
  else if (strcmp("ExplosiveAndTeleportable", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::ExplosiveAndTeleportable;
  }
  else if (strcmp("enemy_projectile", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::EnemyProjectile;
  }
  else if (strcmp("platform", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Platform;
  }
  else if (strcmp("column", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Column;
  }
  else if (strcmp("obstacle", name.c_str()) == 0) {
	  return CModulePhysics::FilterGroup::Obstacle;
  }
  else if (strcmp("VulnerableToMelee", name.c_str()) == 0) {
      return CModulePhysics::FilterGroup::VulnerableToMelee;
  }
  else if (strcmp("notplayer", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::NotPlayer;
  }
  else if (strcmp("product", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Product;
  }
  else if (strcmp("weapon", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::Weapon;
  }
  else if (strcmp("not_weapon", name.c_str()) == 0) {
    return CModulePhysics::FilterGroup::NotWeapon;
  }
  
  
  //good solution, if from max is not specified its group, it will be scenario
  if (isGroup) {
    return CModulePhysics::FilterGroup::Scenario;
  }

  return CModulePhysics::FilterGroup::All;
  
}

void CModulePhysics::setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask)
{
  PxFilterData filterData;
  filterData.word0 = filterGroup; // word0 = own ID
  filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
  shape->setSimulationFilterData(filterData);
  shape->setQueryFilterData(filterData);
}

void CModulePhysics::setupFilteringOnAllShapesOfActor(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask)
{
  const PxU32 numShapes = actor->getNbShapes();
  std::vector<PxShape*> shapes;
  shapes.resize(numShapes);
  actor->getShapes(&shapes[0], numShapes);
  for (PxU32 i = 0; i < numShapes; i++)
    setupFiltering(shapes[i], filterGroup, filterMask);
}

void CModulePhysics::CustomSimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
  PROFILE_FUNCTION("onTrigger");
  for (PxU32 i = 0; i < count; i++)
  {
    // ignore pairs when shapes have been deleted
    if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
      continue;

    CHandle h_comp_trigger;
    h_comp_trigger.fromVoidPtr(pairs[i].triggerActor->userData);
    CHandle h_comp_other;
    h_comp_other.fromVoidPtr(pairs[i].otherActor->userData);

    CEntity* e_trigger = h_comp_trigger.getOwner();
    CEntity* e_other = h_comp_other.getOwner();

    if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
    {
      TMsgEntityTriggerEnter msg;
	  
      // Notify the trigger someone entered
      msg.h_entity = h_comp_other.getOwner();
	  if (e_trigger != nullptr)
		e_trigger->sendMsg(msg);
	  
      // Notify that someone he entered in a trigger
      msg.h_entity = h_comp_trigger.getOwner();
	  if(e_other != nullptr)
		e_other->sendMsg(msg);
	  
    }
    else if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_LOST)
    {
      TMsgEntityTriggerExit msg;

      // Notify the trigger someone exit
      msg.h_entity = h_comp_other.getOwner();
      e_trigger->sendMsg(msg);

      // Notify that someone he exit a trigger
      msg.h_entity = h_comp_trigger.getOwner();
      e_other->sendMsg(msg);
    }
    else if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
      TMsgEntityTriggerStay msg;

      // Notify the trigger someone exit
      msg.h_entity = h_comp_other.getOwner();
      e_trigger->sendMsg(msg);

      // Notify that someone he exit a trigger
      msg.h_entity = h_comp_trigger.getOwner();
      e_other->sendMsg(msg);
    }
	
  }
}

void CModulePhysics::CustomSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
	for (PxU32 i = 0; i < nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];
		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

		  CHandle h_comp_physics;
		  h_comp_physics.fromVoidPtr(pairHeader.actors[0]->userData);
		  CEntity* entityContact1 = h_comp_physics.getOwner();
		  CHandle h_comp_physics2;
		  h_comp_physics2.fromVoidPtr(pairHeader.actors[1]->userData);
		  CEntity* entityContact2 = h_comp_physics2.getOwner();

		  TMsgOnContact msg;

		  if (entityContact1 != nullptr) {
			msg.source = h_comp_physics2;
			entityContact1->sendMsg(msg);
		  }
		  if (entityContact2 != nullptr) {
			msg.source = h_comp_physics;
			entityContact2->sendMsg(msg);
		  }
		}
	}
}

void CModulePhysics::CustomUserControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& hit) {
  CHandle h_comp_physics;
  h_comp_physics.fromVoidPtr(hit.actor->userData);
  CEntity* entityContact = h_comp_physics.getOwner();

  CHandle h_comp_character;
  h_comp_character.fromVoidPtr(hit.controller->getActor()->userData);
  CEntity* entityCharacter = h_comp_character.getOwner();

  TMsgOnContact msg;

  VEC3 a = PXVEC3_TO_VEC3(hit.worldPos);
  msg.pos = a;

  if (entityContact != nullptr) {
    msg.source = h_comp_character;
    entityContact->sendMsg(msg);
  }

  if (entityCharacter != nullptr) {
    msg.source = h_comp_physics;
    entityCharacter->sendMsg(msg);
  }
}

void CModulePhysics::CustomUserControllerHitReport::onControllerHit(const physx::PxControllersHit& hit) {
  CHandle h_comp_physics;
  h_comp_physics.fromVoidPtr(hit.other->getActor()->userData);
  CEntity* entityContact = h_comp_physics.getOwner();

  CHandle h_comp_character;
  h_comp_character.fromVoidPtr(hit.controller->getActor()->userData);
  CEntity* entityCharacter = h_comp_character.getOwner();

  TMsgOnContact msg;
  VEC3 a = PXVEC3_TO_VEC3(hit.worldPos);
  msg.pos = a;

  if (entityContact != nullptr) {
    msg.source = h_comp_character;
    entityContact->sendMsg(msg);
  }

  if (entityCharacter != nullptr) {
    msg.source = h_comp_physics;
    entityCharacter->sendMsg(msg);
  }
}

void CModulePhysics::CustomUserControllerHitReport::onObstacleHit(const physx::PxControllerObstacleHit& hit) {
  dbg("obstacle hit");
}


PxControllerBehaviorFlags CModulePhysics::CustomControllerBehaviorCallback::getBehaviorFlags(const PxShape& shape, const PxActor& actor){
	/*const char* actorName = actor.getName();
	CHandle h_comp_physics;
	h_comp_physics.fromVoidPtr(actor.userData);
	CEntity* entityContact = h_comp_physics.getOwner();
	//
	if (strcmp("BoxKinematic", entityContact->getName()) == 0) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | PxControllerBehaviorFlag::eCCT_SLIDE;
	}	*/
	/*if (actorName == gPlankName)OTRO CASO
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;*/

	CHandle h_comp_physics;
	h_comp_physics.fromVoidPtr(actor.userData);
	if (!h_comp_physics.isValid())
		return PxControllerBehaviorFlags(0);
	CEntity* entityContact = h_comp_physics.getOwner();
	if(!entityContact)
		return PxControllerBehaviorFlags(0);
	TCompCollider* col_ = entityContact->get<TCompCollider>();
	const json& jconfig = col_->jconfig;
	bool isPlatform = jconfig.value("platform", false);
	if (isPlatform) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
  }
		
	return PxControllerBehaviorFlags(0);
}
PxControllerBehaviorFlags CModulePhysics::CustomControllerBehaviorCallback::getBehaviorFlags(const PxController& controller) {
	/*CHandle h_comp_physics;
	h_comp_physics.fromVoidPtr(controller.getActor());
	if (!h_comp_physics.isValid())
		return PxControllerBehaviorFlags(0);
	CEntity* entityContact = h_comp_physics.getOwner();
	if (!entityContact)
		return PxControllerBehaviorFlags(0);
	TCompCollider* col_ = entityContact->get<TCompCollider>();
	const json& jconfig = col_->jconfig;
	bool isPlatform = jconfig.value("platform", false);
	if (isPlatform) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}*/
	return PxControllerBehaviorFlags(0);
}
PxControllerBehaviorFlags CModulePhysics::CustomControllerBehaviorCallback::getBehaviorFlags(const PxObstacle& obstacle) {
	return PxControllerBehaviorFlags(0);
}


void CModulePhysics::debugInMenuJoints() {
  if (ImGui::TreeNode("Joints")) {
    for (auto& c : joints)
      debugInMenuJoint(c);
    ImGui::TreePop();
  }
}

void CModulePhysics::debugActor(const char* title, PxRigidActor* actor) {
  if (actor == nullptr) {
    ImGui::Text("%s NULL", title);
    return;
  }
  ImGui::Text("%s", title); ImGui::SameLine();
  CHandle h_collider;
  h_collider.fromVoidPtr(actor->userData);
  CEntity* e = h_collider.getOwner();
  if (e)
    e->debugInMenu();
  else
    ImGui::Text("NULL");
}


void CModulePhysics::TJoint::create() {
  PxRigidActor* actor0 = nullptr;
  PxRigidActor* actor1 = nullptr;
  
  TCompCollider* c0 = obj0.h_collider;
  if (!c0) {
    CEntity* e = getEntityByName(obj0.name);
    if (e)
      obj0.h_collider = e->get<TCompCollider>();
    c0 = obj0.h_collider;
  }
  if (c0)
    actor0 = c0->actor;

  TCompCollider* c1 = obj1.h_collider;
  if (!c1) {
    CEntity* e = getEntityByName(obj1.name);
    if (e)
      obj1.h_collider = e->get<TCompCollider>();
    c1 = obj1.h_collider;
  } 
  if (c1) {
    actor1 = c1->actor;
  }
  
  PxTransform frame0 = toPxTransform(obj0.transform);
  PxTransform frame1 = toPxTransform(obj1.transform);
  if(joint_type == "spherical"){
    px_joint = PxSphericalJointCreate(*gPhysics, actor0, frame0, actor1, frame1);
  }
  else if (joint_type == "fixed"){
    px_joint = PxFixedJointCreate(*gPhysics, actor0, frame0, actor1, frame1);
  }
  else if (joint_type == "distance"){
    px_joint = PxDistanceJointCreate(*gPhysics, actor0, frame0, actor1, frame1);
  }
  else if (joint_type == "revolute"){
    px_joint = PxRevoluteJointCreate(*gPhysics, actor0, frame0, actor1, frame1);
    PxRevoluteJoint* custom = (PxRevoluteJoint*)px_joint;
	
    custom->setLimit(PxJointAngularLimitPair(-PxPi / 1.5f, PxPi / 1.5f, 0.1f));
    custom->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);		
	
  }
  else{
    fatal("Invalid joint type %s\n", joint_type.c_str());
  }


}

void CModulePhysics::debugInMenuJointObj(TJoint& j, physx::PxJointActorIndex::Enum idx) {
  std::string title = (idx == PxJointActorIndex::eACTOR0) ? "Obj0 " : "Obj1 ";
  TJoint::TObj& obj = (idx == PxJointActorIndex::eACTOR0) ? j.obj0 : j.obj1;
  title += obj.name;
  if (ImGui::TreeNode(title.c_str())) {
    // Ask physx
    PxTransform pxtrans = j.px_joint->getLocalPose(idx);
    // Save it locally
    obj.transform = toTransform(pxtrans);

    // Update phyxs trans if changed from imgui
    if (obj.transform.renderInMenu()) {
      pxtrans = toPxTransform(obj.transform);
      j.px_joint->setLocalPose(idx, pxtrans);
    }
    ImGui::TreePop();
  }
}

void CModulePhysics::debugInMenuJoint(TJoint& j) {
  if (!j.px_joint) {
    ImGui::LabelText("Joint Type", "Invalid Joint");
    return;
  }

  auto flags = j.px_joint->getConstraintFlags();
  if (flags.isSet(PxConstraintFlag::eBROKEN))
    ImGui::Text("Is Broken!");

  ImGui::LabelText("Joint Type", "%s", j.px_joint->getConcreteTypeName());
  debugInMenuJointObj(j, PxJointActorIndex::eACTOR0);
  debugInMenuJointObj(j, PxJointActorIndex::eACTOR1);
  if (j.joint_type == "spherical") {
    PxSphericalJoint* custom = (PxSphericalJoint*)j.px_joint;
    //spherical->setLimitCone();
  }
  else if (j.joint_type == "fixed") {
    PxFixedJoint* custom = (PxFixedJoint*)j.px_joint;
  }
  else if (j.joint_type == "distance")
  {
    PxDistanceJoint* custom = (PxDistanceJoint*)j.px_joint;
    float distance = custom->getDistance();
    ImGui::LabelText("Distance", "%f", distance);
    // ..
  }
  else if (j.joint_type == "revolute"){
	  PxRevoluteJoint* custom = (PxRevoluteJoint*)j.px_joint;
	  float driveVelocity = custom->getDriveVelocity();
    float driveGearRatio = custom->getDriveGearRatio();
    VEC3 a = PXVEC3_TO_VEC3(custom->getRelativeAngularVelocity());
    float velocity = custom->getVelocity();
	  ImGui::LabelText("driveVelocity", "%f", driveVelocity);
    ImGui::LabelText("driveGearRatio", "%f", driveGearRatio);
    ImGui::LabelText("velocity", "%f", velocity);
    ImGui::LabelText("Angular velocity", "%f &f %f", a.x, a.y, a.z);
  }
  if( ImGui::SmallButton( "Break"))
    j.px_joint->setBreakForce(0.0f, 0.0f);
  ImGui::Separator();
}

bool CModulePhysics::Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlags flag, physx::PxQueryFilterData filterdata)
{
	PxVec3 px_origin = PxVec3(origin.x, origin.y, origin.z);
	PxVec3 px_dir = PxVec3(dir.x, dir.y, dir.z); // [in] Normalized ray direction
	PxReal px_distance = (PxReal)(distance); // [in] Raycast max distance

	PxRaycastBuffer px_hit; // [out] Raycast results
	filterdata.flags = flag;

	bool status = gScene->raycast(px_origin, px_dir, px_distance, px_hit, PxHitFlags(PxHitFlag::eDEFAULT), filterdata); // Closest hit
	hit = px_hit.block;

	return status;
}

/* Returns true if there was some hit with the sweep cast. Hit will contain all hits */
bool CModulePhysics::Sweep(physx::PxGeometry& geometry, const VEC3 & position, const QUAT & rotation, const VEC3 & direction, float distance, std::vector<physx::PxSweepHit>& hits, physx::PxQueryFlags flag, physx::PxQueryFilterData filterdata)
{
	PxSweepHit sweepHit[256];     //With 256 it is supossed to be enough
	PxSweepBuffer px_hit(sweepHit, 256);
	filterdata.flags = flag;
	PxVec3 px_dir = PxVec3(direction.x, direction.y, direction.z); // [in] Normalized sweep direction


	physx::PxTransform transform(PxVec3(position.x, position.y, position.z), PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));
	bool status = gScene->sweep(geometry, transform, px_dir, distance, px_hit, PxHitFlags(PxHitFlag::eDEFAULT), filterdata);

	if (status) {
		for (PxU32 i = 0; i < px_hit.nbTouches; i++) {
			hits.push_back(px_hit.touches[i]);
		}
	}

	return status;
}

/* Returns true if there was some hit with the sphere cast. Hit will contain all hits */
bool CModulePhysics::Overlap(physx::PxGeometry& geometry, VEC3 pos, std::vector<physx::PxOverlapHit> & hits, physx::PxQueryFilterData filterdata)
{
	PxOverlapHit overlapHit[256];     //With 256 it is supossed to be enough
	PxOverlapBuffer px_hit(overlapHit, 256);

	physx::PxTransform transform(PxVec3(pos.x, pos.y, pos.z));

	bool status = gScene->overlap(geometry, transform, px_hit, filterdata);

	if (status) {
		for (PxU32 i = 0; i < px_hit.nbTouches; i++) {
			hits.push_back(px_hit.touches[i]);
		}
	}

	return status;
}


