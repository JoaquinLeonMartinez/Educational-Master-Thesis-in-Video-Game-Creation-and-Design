#pragma once

class CCteBufferBase {
protected:

  ID3D11Buffer* cb = nullptr;
  std::string   name;
  uint32_t      total_bytes = 0;

  static const uint32_t invalid_slot_idx = ~0;
  uint32_t      slot_idx = invalid_slot_idx;

  bool createCB(uint32_t num_bytes, const char* new_name) {

    total_bytes = num_bytes;

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    // Create the constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = num_bytes;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = Render.device->CreateBuffer(&bd, nullptr, &cb);
    if (FAILED(hr))
      return false;

    name = new_name;
    setDXName(cb, new_name);
    return true;
  }

public:
  void destroy() {
    SAFE_RELEASE(cb);
  }

  void activate() const {
    assert(slot_idx != invalid_slot_idx);
    Render.ctx->VSSetConstantBuffers(slot_idx, 1, &cb);
    Render.ctx->PSSetConstantBuffers(slot_idx, 1, &cb);
  }

  void activateInCS(int slot_idx) const {
    assert(cb);
    Render.ctx->CSSetConstantBuffers(slot_idx, 1, &cb);
  }

  const std::string& getName() const { return name; }

  virtual void fromJson(const json & j) {}
  virtual void debugInMenu() {}
  virtual void updateGPU() = 0;

  void updateGPU(const void* cpu_data) const {
    assert(cb);
    Render.ctx->UpdateSubresource(cb, 0, NULL, cpu_data, 0, 0);
  }

  uint32_t size() const {
    return total_bytes;
  }

  uint32_t slotIndex() const {
    return slot_idx;
  }

};

template< typename TData >
void readCteJson(const json& j, TData& d) {}

template< typename TData >
bool debugCteInMenu(TData& d) { 
  ImGui::Text("(%ld bytes)", sizeof(d));
  return false; 
}

// Create an composed object, first the ctes data struct
// then, the pointer to the cb
// In memory the layout is:
//   struct         TData          
//   ID3D11Buffer*  cb; 
// But the cb uses the size of TData to allocate the exact
// amount of bytes in the GPU
template< typename TData >
class CCteBuffer : public TData, public CCteBufferBase {
public:
  CCteBuffer(int target_slot) {
    slot_idx = target_slot;
  }

  bool create(const char* name) {
    // Call the CCteBufferBase method to allocate the memory in GPU 
    // of sizeof the given struct
    return createCB(sizeof(TData), name);
  }
  void updateGPU() {
    CCteBufferBase::updateGPU((TData*)this);
  }
  void fromJson(const json& j) override {
    readCteJson<TData>(j, *this);
    updateGPU();
  }
  void debugInMenu() override {
    if (ImGui::TreeNode(getName().c_str())) {
      if (debugCteInMenu<TData>(*this))
        updateGPU();
      ImGui::TreePop();
    }
  }
};

// Anonymous buffer. No CPU data container
struct CCteRawBuffer : public CCteBufferBase {
  bool create(uint32_t new_num_bytes, const char* name, uint32_t new_slot_idx) {
    assert(new_num_bytes > 0);
    slot_idx = new_slot_idx;
    return createCB(new_num_bytes, name);
  }
  void updateGPU() override {
    fatal("Can't update Raw Buffer without a user pointer.\n");
  }

};



