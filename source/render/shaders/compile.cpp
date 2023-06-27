#include "mcv_platform.h"
#include "render/render.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler")

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const std::string& fileName, const std::string& szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;
  TFileContext fc(fileName);

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  //dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
  
  // To avoid requiring using the transpose when sending matrix to the ctes
  dwShaderFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

  WCHAR szFilename[MAX_PATH];
  mbstowcs(szFilename, fileName.c_str(), MAX_PATH);

  while (true) {

    ID3DBlob* pErrorBlob = NULL;
    hr = D3DCompileFromFile(
      szFilename,
      NULL,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      szEntryPoint.c_str(),
      szShaderModel,
      dwShaderFlags,
      0,
      ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
      if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
        fatal("Missing shader input file '%s'\n", fileName.c_str());
      }
      if (pErrorBlob != NULL) {
        const char* err_msg = (char*)pErrorBlob->GetBufferPointer();
        fatal(err_msg);
      }
      if (pErrorBlob)
        pErrorBlob->Release();
      continue;
    }
    if (pErrorBlob)
      pErrorBlob->Release();
    break;
  }
  return S_OK;
}


