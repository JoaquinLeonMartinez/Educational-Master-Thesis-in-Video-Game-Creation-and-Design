#include "mcv_platform.h"
#include "engine.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "module_navmesh.h""


bool CModuleNavmesh::start()
{
	navmesh = CNavmesh();
	return true;
}

void CModuleNavmesh::stop()
{
	navmesh.destroy();
}
void CModuleNavmesh::update(float dt)
{
	//actualizat tiles cuando se meten obstaculos
	//getNav().handleUpdate(dt);
}

void CModuleNavmesh::renderDebug() {
  //AQUI LINEAS
  //navmesh.render();
}

void CModuleNavmesh::createNavmesh(const std::string& path) {
	
	navmesh.loadAllObstacle(path.c_str());//cargar mesh para obstacles dtTileCache
	//navmesh.loadAll(path.c_str());//cargar solo mesh
	if (navmesh.m_navMesh) {
		navmesh.prepareQueries();
	}
	else {
		fatal("Error when creating navmesh\n");
	}
}

int CModuleNavmesh::addTempObstacle(const VEC3 pos, const float radius, const float height) {
	int res = navmesh.addTempObstacle(pos, radius, height);

	for (int i = 0; i < navmesh.m_tileCache->getObstacleCount();i++) {
		getNav().handleUpdate(1);
	}
	/*getNav().handleUpdate(1);
	getNav().handleUpdate(1);
	getNav().handleUpdate(1);
	getNav().handleUpdate(1);*/
	
	return res;
}

void CModuleNavmesh::removeTempObstacle(dtObstacleRef referencia) {
	navmesh.removeTempObstacle(referencia);
}

void CModuleNavmesh::removeTempObstacle(const VEC3 sp, const VEC3 sq) {
	navmesh.removeTempObstacle(sp, sq);
}



std::vector<VEC3> CModuleNavmesh::findPath(VEC3 start, VEC3 end) {
	return navmeshQuery.findPath(start, end);
}

float CModuleNavmesh::wallDistance(VEC3 pos) {
	return navmeshQuery.wallDistance(pos);
}

bool CModuleNavmesh::raycast(VEC3 start, VEC3 end, VEC3 &m_hitPos) {
	return navmeshQuery.raycast(start, end, m_hitPos);
}
void CModuleNavmesh::loadObstacles(VEC3 pos, float  radius, float  height) {
	
}

void CModuleNavmesh::destroyNavmesh() {
	navmesh.destroy();
}

CNavmesh CModuleNavmesh::getNav() {
	return navmesh;
}