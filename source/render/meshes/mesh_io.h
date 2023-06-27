#pragma once

#include "mesh_group.h"

class CDataProvider;
class CDataSaver;

struct TRawMesh {

  struct TChunk {
    uint32_t magic;
    uint32_t nbytes;
  };

  static const uint32_t magicHeader = 0x44444444;
  static const uint32_t magicVtxs = 0x55554433;
  static const uint32_t magicIdxs = 0x55556677;
  static const uint32_t magicGroups = 0x55556688;
  static const uint32_t magicAABB = 0x55556611;
  static const uint32_t magicEof = 0x55009988;

  struct THeader {
    uint32_t num_faces;
    uint32_t num_vertex;
    uint32_t num_indices;
    uint32_t primitive_type;

    uint32_t bytes_per_index;
    uint32_t bytes_per_vertex;
    uint32_t num_groups;
    uint32_t padding2;

    char     vertex_type_name[32];
  };

  std::vector< uint8_t > vertices;
  std::vector< uint8_t > indices;
  THeader                header;
  VMeshGroups            groups;
  AABB                   aabb;
  bool                   aabb_is_valid = false;

  bool load(CDataProvider& dp);
  bool save(CDataSaver& ds);

  CMesh* createRenderMesh() const;
};
