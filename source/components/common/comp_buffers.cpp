#include "mcv_platform.h"
#include "comp_buffers.h"
#include "render/compute/gpu_buffer.h"
#include "components/common/comp_render.h"
#include "utils/data_saver.h"

DECL_OBJ_MANAGER("buffers", TCompBuffers);

void from_json(const json& j, TCtesParticles& p);
template<> bool debugCteInMenu<TCtesParticles>(TCtesParticles& d);

TCompBuffers::~TCompBuffers() {
  for (auto& b : gpu_buffers)
    b->destroy();
  gpu_buffers.clear();

  for (auto& b : cte_buffers)
    b->destroy();
  cte_buffers.clear();
}

void TCompBuffers::debugInMenu() {
  for (auto b : gpu_buffers)
    ImGui::Text("GPU Buffer %s (%ld bytes)", b->name.c_str(), b->cpu_data.size());
  for (auto b : cte_buffers)
    b->debugInMenu();
}

CGPUBuffer* TCompBuffers::getBufferByName(const char* name) {
  for (auto b : gpu_buffers) {
    if (strcmp(b->name.c_str(), name) == 0)
      return b;
  }
  return nullptr;
}

CCteBufferBase* TCompBuffers::getCteByName(const char* name) {
  for (auto b : cte_buffers) {
    if (strcmp(b->getName().c_str(), name) == 0)
      return b;
  }
  return nullptr;
}

void TCompBuffers::load(const json& j, TEntityParseContext& ctx) {

  // Owned buffers by me
  for (auto jit : j.items()) {
    const std::string& key = jit.key();
    json jval = jit.value();
    jval["name"] = key;

    if (jval.count("total_bytes")) {
      CCteRawBuffer* cte_buffer = new CCteRawBuffer();
      uint32_t slot_idx = jval.value("slot", -1);
      uint32_t total_bytes = jval.value("total_bytes", 0);
      cte_buffer->create(total_bytes, key.c_str(), slot_idx);
      assert(cte_buffer->size() == total_bytes);
      cte_buffers.push_back(cte_buffer);
      continue;
    }

    if (key == "TCtesParticles") {
      auto cte_buffer = new CCteBuffer< TCtesParticles >( CTE_BUFFER_SLOT_PARTICLES );
      bool is_ok = cte_buffer->create(key.c_str());
      assert(is_ok);
      from_json(jval, *cte_buffer);
      cte_buffer->updateGPU();
      cte_buffers.push_back(cte_buffer);
      continue;
    }

    {
      CGPUBuffer* gpu_buffer = new CGPUBuffer();
      bool is_ok = gpu_buffer->create(jval);
      assert(is_ok || fatal("Failed to create compute shader buffer %s\n", key.c_str()));
      gpu_buffers.push_back(gpu_buffer);

      if (gpu_buffer->is_indirect && jval.count( "init_indirect_from_mesh" )) {
        std::string mesh_name = jval["init_indirect_from_mesh"];
        const CMesh* mesh = Resources.get(mesh_name)->as<CMesh>();
        auto& g = mesh->getGroups();
        assert(g.size() > 0);
        uint32_t args[5] = { g[0].num_indices, 0, g[0].first_idx, 0, 0 };
        gpu_buffer->copyCPUtoGPUFrom(args);
      }

      if (jval.value("init_as_sequencial_ints", false)) {
        std::vector<uint32_t> ints;
        for (uint32_t i = 0; i < gpu_buffer->num_elems; ++i)
          ints.push_back(i);
        gpu_buffer->copyCPUtoGPUFrom(ints.data());
      }

      if (jval.value("init_with_values", false)) {
        CMemoryDataSaver ds;
        for (auto& j : jval.items()) {
          auto& jv = j.value();
          if (jv.is_number_float()) {
            float f = jv;
            ds.write(f);
          }
          else if (jv.is_number_integer()) {
            uint32_t i = jv;
            ds.write(i);
          }
          else {
            fatal("Don't know how to init gpu buffer %s with value '%s'\n", gpu_buffer->name.c_str(), jv.dump().c_str() );
          }
        }
        assert(ds.buffer.size() <= gpu_buffer->bytes_per_elem * gpu_buffer->num_elems);
        gpu_buffer->copyCPUtoGPUFrom(ds.buffer.data());
      }

    }
  }

}

