#pragma once

extern HRESULT CompileShaderFromFile(const std::string& fileName, const std::string& szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
