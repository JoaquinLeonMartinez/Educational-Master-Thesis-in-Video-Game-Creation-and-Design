#pragma once

struct CVertexDeclaration {
  std::string name;
  uint32_t    bytes_per_vertex = 0;
  D3D11_INPUT_ELEMENT_DESC* layout = nullptr;
  UINT        numElements = 0;
  bool        instancing = false;
  bool isCompatibleWith(const CVertexDeclaration* other) const;
};

const CVertexDeclaration* getVertexDeclByName(const std::string& name);


