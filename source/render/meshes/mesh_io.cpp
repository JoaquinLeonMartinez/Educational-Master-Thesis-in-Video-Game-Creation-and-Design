#include "mcv_platform.h"
#include "mesh_io.h"
#include "utils/data_saver.h"

bool TRawMesh::load(CDataProvider& dp) {

  if (!dp.isValid())
    return false;

  bool file_processed = false;
  while (!file_processed) {

    TChunk chunk;
    dp.read(chunk);
    switch(chunk.magic) {

    case magicHeader:
      assert(chunk.nbytes == sizeof(THeader));
      dp.read(header);
      break;

    case magicVtxs:
      vertices.resize(chunk.nbytes);
      assert(chunk.nbytes == vertices.size());
      dp.read(vertices.data(), chunk.nbytes);
      break;

    case magicIdxs:
      indices.resize(chunk.nbytes);
      dp.read(indices.data(), chunk.nbytes);
      break;

    case magicAABB:
      assert(chunk.nbytes == sizeof(AABB));
      dp.read(&aabb, chunk.nbytes);
      aabb_is_valid = true;
      break;

    case magicGroups:
      groups.resize(header.num_groups);
      assert(chunk.nbytes == sizeof(TMeshGroup) * header.num_groups);
      dp.read(groups.data(), chunk.nbytes);
      break;

    case magicEof:
      file_processed = true;
      break;

    default:
      fatal("Unexpected magic value %08x while reading mesh", chunk.magic);
    }

  }

  // To be valid, we need to have read the magicAABB and the half size must be non-zero
  if (aabb_is_valid && aabb.Extents.x == 0.f && aabb.Extents.y == 0.f && aabb.Extents.z == 0.f)
    aabb_is_valid = false;

  return true;
}

void saveChunk(CDataSaver& ds, uint32_t magic, uint32_t num_bytes, const void* data) {
  TRawMesh::TChunk s;
  s.magic = magic;
  s.nbytes = num_bytes;
  ds.write(s);
  if (data)
    ds.writeBytes(data, num_bytes);
}

bool TRawMesh::save(CDataSaver& ds) {
  assert(ds.isValid());
  saveChunk(ds, magicHeader, sizeof(THeader), &header);
  saveChunk(ds, magicVtxs, (uint32_t)vertices.size(), vertices.data());
  saveChunk(ds, magicIdxs, (uint32_t)indices.size(), indices.data());
  saveChunk(ds, magicGroups, (uint32_t)groups.size()*sizeof(TMeshGroup), groups.data());
  saveChunk(ds, magicEof, 0, nullptr);
  return true;
}


