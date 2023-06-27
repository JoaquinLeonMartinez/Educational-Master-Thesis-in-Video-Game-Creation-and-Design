
#pragma once
#include "modules/module.h"
#include "navmesh/navmesh.h"
#include "navmesh/navmesh_query.h"

class CModuleNavmesh : public IModule
{
public:
	CModuleNavmesh(const std::string& name) : IModule(name) { }
	bool start() override;
	void stop() override;
	void update(float dt) override;
	void renderDebug() override;
	void createNavmesh(const std::string& path);
	CModuleNavmesh* getNavmesh() { return this; }
	std::vector<VEC3> findPath(VEC3 start, VEC3 end);
	float wallDistance(VEC3 pos);
	bool raycast(VEC3 start,VEC3 end,VEC3 &m_hitPos);
	CNavmesh getNav();
	int addTempObstacle(const VEC3 pos, const float radius, const float height);
	void removeTempObstacle(const VEC3 sp, const VEC3 sq);
	void removeTempObstacle(dtObstacleRef referencia);
	void loadObstacles(VEC3 pos,float  radius,float  height);
	void destroyNavmesh();
private:
	CNavmesh navmesh;
	CNavmeshQuery navmeshQuery = CNavmeshQuery(&navmesh);
};