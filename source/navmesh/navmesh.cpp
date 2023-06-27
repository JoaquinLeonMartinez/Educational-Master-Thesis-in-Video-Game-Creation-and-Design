#include <stdio.h>
#include <stdlib.h>
#include "mcv_platform.h"
#include "navmesh.h"
#include "navmesh/recast/DetourNavMeshBuilder.h"
#include "navmesh/recast/RecastDump.h"
#include "navmesh/recast/DetourDebugDraw.h"

#include "navmesh/recast/DetourTileCache.h"
#include "navmesh/recast/DetourCommon.h"
#include "navmesh/recast/fastlz/fastlz.h"
#include "navmesh/InputGeom.h"


struct MeshProcess : public dtTileCacheMeshProcess
{
	InputGeom* m_geom;

	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};


	inline MeshProcess() : m_geom(0)
	{
		

	}

	inline void init(InputGeom* geom)
	{
		m_geom = geom;
	}

	virtual void process(struct dtNavMeshCreateParams* params,
		unsigned char* polyAreas, unsigned short* polyFlags)
	{
		// Update poly flags from areas.
		for (int i = 0; i < params->polyCount; ++i)
		{
			if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
				polyAreas[i] = SAMPLE_POLYAREA_GROUND;

			if (polyAreas[i] == SAMPLE_POLYAREA_GROUND ||
				polyAreas[i] == SAMPLE_POLYAREA_GRASS ||
				polyAreas[i] == SAMPLE_POLYAREA_ROAD)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK;
			}
			else if (polyAreas[i] == SAMPLE_POLYAREA_WATER)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (polyAreas[i] == SAMPLE_POLYAREA_DOOR)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		// Pass in off-mesh connections.
		if (m_geom)
		{
			params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params->offMeshConRad = m_geom->getOffMeshConnectionRads();
			params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params->offMeshConUserID = m_geom->getOffMeshConnectionId();
			params->offMeshConCount = m_geom->getOffMeshConnectionCount();
			
		
		}
	}
};



struct LinearAllocator : public dtTileCacheAlloc
{
	unsigned char* buffer;
	size_t capacity;
	size_t top;
	size_t high;

	LinearAllocator(const size_t cap) : buffer(0), capacity(0), top(0), high(0)
	{
		resize(cap);
	}

	~LinearAllocator()
	{
		dtFree(buffer);
	}

	void resize(const size_t cap)
	{
		if (buffer) dtFree(buffer);
		buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
		capacity = cap;
	}

	virtual void reset()
	{
		high = dtMax(high, top);
		top = 0;
	}

	virtual void* alloc(const size_t size)
	{
		if (!buffer)
			return 0;
		if (top + size > capacity)
			return 0;
		unsigned char* mem = &buffer[top];
		top += size;
		return mem;
	}

	
};



struct FastLZCompressor : public dtTileCacheCompressor
{



	virtual int maxCompressedSize(const int bufferSize)
	{
		return (int)(bufferSize* 1.05f);
	}

	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
		unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
	{
		*compressedSize = fastlz_compress((const void *const)buffer, bufferSize, compressed);
		return DT_SUCCESS;
	}

	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
		unsigned char* buffer, const int maxBufferSize, int* bufferSize)
	{
		*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
		return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
	}
};



CNavmesh::CNavmesh()
	: m_navMesh(nullptr)
{
	m_navQuery = dtAllocNavMeshQuery();

}

void CNavmesh::prepareQueries() {
	dtStatus status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		printf("NAVMESH PREPARATION FAILED!\n");
}

void CNavmesh::destroy() {
	/* delete[ ] m_triareas;
	 m_triareas = 0;
	 rcFreeHeightField( m_solid );
	 m_solid = 0;
	 rcFreeCompactHeightfield( m_chf );
	 m_chf = 0;
	 rcFreeContourSet( m_cset );
	 m_cset = 0;
	 rcFreePolyMesh( m_pmesh );
	 m_pmesh = 0;
	 rcFreePolyMeshDetail( m_dmesh );
	 m_dmesh = 0;*/
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
}
//recastnavigation/RecastDemo/Source/Sample.cpp (carga archivos .bin)
int CNavmesh::loadAll(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return 0;

	// Read header.
	NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	m_navMesh = mesh;
	m_draw = new DebugDrawGL();
	m_tileCache = nullptr;
}
void CNavmesh::loadAllObstacle(const char* path) {
		FILE* fp = fopen(path, "rb");
	if (!fp) return;
	
	// Read header.
	TileCacheSetHeader header;
	size_t headerReadReturnCode = fread(&header, sizeof(TileCacheSetHeader), 1, fp);
	if( headerReadReturnCode != 1)
	{
		// Error or early EOF
		fclose(fp);
		return;
	}
	if (header.magic != TILECACHESET_MAGIC)
	{
		fclose(fp);
		return;
	}
	if (header.version != TILECACHESET_VERSION)
	{
		fclose(fp);
		return;
	}
	
	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return;
	}
	

	dtStatus status = mesh->init(&header.meshParams);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return;
	}

	dtTileCache::dtTileCache();
	dtTileCache* tileCache = dtAllocTileCache();
	if (!tileCache)
	{
		fclose(fp);
		return;
	}

	
	/*
	struct LinearAllocator* m_talloc;

	struct FastLZCompressor* m_tcomp;

	struct MeshProcess* m_tmproc;
 */
	
	
	m_talloc = new LinearAllocator(32000);
	m_tcomp = new FastLZCompressor();
	m_tmproc = new MeshProcess();
	
	m_tmproc->init(new InputGeom());
	
	//------PRUEBA DE MESH----- aqui se cargaran de un fichero de texto en el futuro
	//prueba  linlOfMesh
	/*
	VEC3 posIni(-5,0,0);
	VEC3 posFin(-5, 0, 30);
	float radAgent = 5.0;
	float m_bidir = true;
	const unsigned char area = m_tmproc->SAMPLE_POLYAREA_JUMP;
	const unsigned short flags = m_tmproc->SAMPLE_POLYFLAGS_JUMP;
	m_tmproc->m_geom->addOffMeshConnection(&posIni.x, &posFin.x, radAgent, m_bidir, area, flags);
	VEC3 posIni_(-2,0,0);
	VEC3 posFin_(-3, 0, 30);
	m_tmproc->m_geom->addOffMeshConnection(&posIni_.x, &posFin_.x, radAgent, m_bidir, area, flags);
	*/

		
	//-----------
	status = tileCache->init(&header.cacheParams, m_talloc, m_tcomp, m_tmproc);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return;
	}
		
	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		TileCacheTileHeader tileHeader;
		size_t tileHeaderReadReturnCode = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if( tileHeaderReadReturnCode != 1)
		{
			// Error or early EOF
			fclose(fp);
			return;
		}
		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		size_t tileDataReadReturnCode = fread(data, tileHeader.dataSize, 1, fp);
		if( tileDataReadReturnCode != 1)
		{
			// Error or early EOF
			dtFree(data);
			fclose(fp);
			return;
		}
		
		dtCompressedTileRef tile = 0;
		dtStatus addTileStatus = tileCache->addTile(data, tileHeader.dataSize, DT_COMPRESSEDTILE_FREE_DATA, &tile);
		if (dtStatusFailed(addTileStatus))
		{
			dtFree(data);
		}

		if (tile)
			tileCache->buildNavMeshTile(tile, mesh);
	}
	
	fclose(fp);

	

	m_navMesh = mesh;
	m_tileCache = tileCache;
	
	
	//prueba  linlOfMesh
	/*VEC3 posIni(10,0,10);
	VEC3 posFin(13, 14.184f, -10);
	float radAgent = 5.0;
	float m_bidir = true;
	const unsigned char area = m_tmproc->SAMPLE_POLYAREA_JUMP;
	const unsigned short flags = m_tmproc->SAMPLE_POLYFLAGS_JUMP;*/
	
	/*if (imguiCheck("One Way", !m_bidir))
		m_bidir = false;
	if (imguiCheck("Bidirectional", m_bidir))
		m_bidir = true;
		*/
	//m_tmproc->m_geom->addOffMeshConnection(&posIni.x, &posFin.x, radAgent, m_bidir, area, flags);
	
	//dtMeshTile* tile1 = 0;
	//int tileIndex = (int) 
	//const unsigned int* id = m_tmproc->m_geom->getOffMeshConnectionId();
	
	
	//prueba obstaculo
	//VEC3 pos(-20, 0, 48);
	//dtObstacleRef dtObstacle = addTempObstacle(pos,3.,3);
	//removeTempObstacle(dtObstacle);
	m_draw = new DebugDrawGL();
	
}



void CNavmesh::handleUpdate(const float dt)
{
	//Sample::handleUpdate(dt);

	if (!m_navMesh)
		return;
	if (!m_tileCache)
		return;
	
	m_tileCache->update(dt, m_navMesh);
	
}


dtObstacleRef CNavmesh::addTempObstacle(const VEC3 pos,const float radius, const float height)
{
	if (!m_tileCache)
		return -1;
	//float p[3];
	//dtVcopy(p, pos);
	VEC3 p;
	p = pos;
	p.y -= 0.5f;
	dtObstacleRef* res = new dtObstacleRef;
	m_tileCache->addObstacle(&p.x, radius, height, res);
	return *res  ;
	
}


bool sectSegAABB(const float* sp, const float* sq,
	const float* amin, const float* amax,
	float& tmin, float& tmax)
{
	static const float EPS = 1e-6f;

	float d[3];
	rcVsub(d, sq, sp);
	tmin = 0;  // set to -FLT_MAX to get first hit on line
	tmax = FLT_MAX;		// set to max distance ray can travel (for segment)

	// For all three slabs
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(d[i]) < EPS)
		{
			// Ray is parallel to slab. No hit if origin not within slab
			if (sp[i] < amin[i] || sp[i] > amax[i])
				return false;
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			const float ood = 1.0f / d[i];
			float t1 = (amin[i] - sp[i]) * ood;
			float t2 = (amax[i] - sp[i]) * ood;
			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2) rcSwap(t1, t2);
			// Compute the intersection of slab intersections intervals
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}
	}

	return true;
}


dtObstacleRef hitTestObstacle(const dtTileCache* tc, const float* sp, const float* sq)
{
	float tmin = FLT_MAX;
	const dtTileCacheObstacle* obmin = 0;
	for (int i = 0; i < tc->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = tc->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY)
			continue;

		float bmin[3], bmax[3], t0, t1;
		tc->getObstacleBounds(ob, bmin, bmax);

		if (sectSegAABB(sp, sq, bmin, bmax, t0, t1))
		{
			if (t0 < tmin)
			{
				tmin = t0;
				obmin = ob;
			}
		}
	}
	return tc->getObstacleRef(obmin);
}


void CNavmesh::removeTempObstacle(dtObstacleRef referencia) {
	m_tileCache->removeObstacle(referencia);
}


void CNavmesh::removeTempObstacle(const VEC3 sp, const VEC3 sq)
{
	if (!m_tileCache)
		return;
	dtObstacleRef ref = hitTestObstacle(m_tileCache, &sp.x, &sq.x);
	m_tileCache->removeObstacle(ref);
}

void CNavmesh::clearAllTempObstacles()
{
	if (!m_tileCache)
		return;
	for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY) continue;
		m_tileCache->removeObstacle(m_tileCache->getObstacleRef(ob));
	}
}

void CNavmesh::drawObstacles(duDebugDraw* dd, const dtTileCache* tc)
{
	// Draw obstacles
	for (int i = 0; i < tc->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = tc->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY) continue;
		float bmin[3], bmax[3];
		tc->getObstacleBounds(ob, bmin, bmax);

		unsigned int col = 0;
		if (ob->state == DT_OBSTACLE_PROCESSING)
			col = duRGBA(255, 255, 0, 128);
		else if (ob->state == DT_OBSTACLE_PROCESSED)
			col = duRGBA(255, 192, 0, 192);
		else if (ob->state == DT_OBSTACLE_REMOVING)
			col = duRGBA(220, 0, 0, 128);

		duDebugDrawCylinder(dd, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], col);
		duDebugDrawCylinderWire(dd, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duDarkenCol(col), 2);
	}
}


void CNavmesh::render() {
	const int SAMPLE_POLYFLAGS_DISABLED = 0xffff;
	if (m_navMesh != nullptr) {
		duDebugDrawNavMeshPolysWithFlags(m_draw, *m_navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0, 0, 255, 128));
		
	}
	/*if (m_tileCache != nullptr) {
			for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
			{
				const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
				if (ob->state == DT_OBSTACLE_EMPTY) continue;
				float bmin[3], bmax[3];
				m_tileCache->getObstacleBounds(ob, bmin, bmax);
			
				unsigned int col = 0;
				if (ob->state == DT_OBSTACLE_PROCESSING)
					col = duRGBA(255, 255, 0, 128);
				else if (ob->state == DT_OBSTACLE_PROCESSED)
					col = duRGBA(255, 192, 0, 192);
				else if (ob->state == DT_OBSTACLE_REMOVING)
					col = duRGBA(220, 0, 0, 128);

				duDebugDrawCylinder(m_draw, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], col);
				duDebugDrawCylinderWire(m_draw, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duDarkenCol(col), 2);
			}
		
	}*/
	//pintado de conexiones
	//m_tmproc->m_geom->drawOffMeshConnections(m_draw);
	
}



