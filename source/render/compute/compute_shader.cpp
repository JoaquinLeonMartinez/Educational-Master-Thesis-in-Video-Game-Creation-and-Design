#include "mcv_platform.h"
#include "compute_shader.h"
#include "render/shaders/compile.h"
#include <d3dcompiler.h>
#include <d3dcompiler.inl>    // D3D11Reflect

FORCEINLINE HRESULT
D3D11Reflect(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
  _In_ SIZE_T SrcDataSize,
  _Out_ ID3D11ShaderReflection** ppReflector)
{
  return D3DReflect(pSrcData, SrcDataSize,
    IID_ID3D11ShaderReflection, (void**)ppReflector);
}

// --------------------------------------------------------------------
void CComputeShader::onFileChanged(const std::string& filename) {
  if (filename == jdef["cs_fx"].get< std::string >()) {
    create(jdef);
  }
}

// --------------------------------------------------------------------
bool CComputeShader::create(const json& j) {
  jdef = j;
  std::string fx = j.value("cs_fx", "");
  assert(!fx.empty());
  std::string cs = j.value("cs", "");
  assert(!cs.empty());
  std::string profile = j.value("profile", "cs_5_0");
  return create(fx, cs, profile);
}

// --------------------------------------------------------------------
bool CComputeShader::create(
  const std::string& filename
, const std::string& fn_entry_name
, const std::string& profile
) {

  ID3DBlob* pBlob = nullptr;
  HRESULT hr = CompileShaderFromFile(filename.c_str(), fn_entry_name.c_str(), profile.c_str(), &pBlob);
  if (FAILED(hr))
    return false;

  // Create the compute shader
  hr = Render.device->CreateComputeShader(
    pBlob->GetBufferPointer()
    , pBlob->GetBufferSize()
    , nullptr
    , &cs);
  if (FAILED(hr)) {
    pBlob->Release();
    return false;
  }

  setDXName(cs, (filename + ":" + fn_entry_name).c_str());

  // Query bound resources
  ID3D11ShaderReflection* reflection = NULL;

  D3D11Reflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &reflection);
  D3D11_SHADER_DESC desc = {};
  reflection->GetDesc(&desc);

  bound_resources.clear();
  for (unsigned int i = 0; i < desc.BoundResources; ++i) {
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT hr = reflection->GetResourceBindingDesc(i, &desc);
    if (FAILED(hr))
      continue;
    bound_resources.push_back(desc);
  }

  reflection->GetThreadGroupSize(&thread_group_size[0], &thread_group_size[1], &thread_group_size[2]);

  pBlob->Release();

  return true;
}

void CComputeShader::destroy() {
  SAFE_RELEASE(cs);
}

void CComputeShader::activate() const {
  Render.ctx->CSSetShader(cs, nullptr, 0);
}

void CComputeShader::deactivate() {
  static const int max_slots = 4;
  ID3D11UnorderedAccessView *uavs_null[max_slots] = { nullptr, nullptr, nullptr, nullptr };
  ID3D11ShaderResourceView*  srvs_null[max_slots] = { nullptr, nullptr, nullptr, nullptr };
  // Null all cs params
  Render.ctx->CSSetUnorderedAccessViews(0, max_slots, uavs_null, nullptr);
  Render.ctx->CSSetShaderResources(0, max_slots, srvs_null);
  Render.ctx->CSSetShader(nullptr, nullptr, 0);
}

void CComputeShader::renderInMenu() {

  static const char* type_names[] = {
    "CBUFFER",
    "TBUFFER",
    "TEXTURE",
    "SAMPLER",
    "UAV_RWTYPED",
    "STRUCTURED",
    "UAV_RWSTRUCTURED",
    "BYTEADDRESS",
    "UAV_RWBYTEADDRESS",
    "UAV_APPEND_STRUCTURED",
    "UAV_CONSUME_STRUCTURED",
    "UAV_RWSTRUCTURED_WITH_COUNTER",
  };

  for (auto it : bound_resources) {
    if (ImGui::TreeNode(it.Name)) {
      const char* str = "unknown type";
      if (it.Type <= D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
        str = type_names[it.Type];
      ImGui::LabelText("Type", "%s (%d)", str, it.Type);
      ImGui::LabelText("Bind Point", "%d (x%d)", it.BindPoint, it.BindCount);
      ImGui::LabelText("NumSamples", "%d", it.NumSamples);
      ImGui::LabelText("Flags", "%08x", it.uFlags);
      ImGui::TreePop();
    }
  }
}

// --------------------------------------------------------
class CComputeShaderResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "compute"; }
  const char* getName() const override {
    return "ComputeShaders";
  }
  IResource* create(const std::string& name) const override {
    fatal("ComputeShaders from source not implemented.");
    return nullptr;
  }
};

template<>
const CResourceType* getResourceTypeFor<CComputeShader>() {
  static CComputeShaderResourceType resource_type;
  return &resource_type;
}