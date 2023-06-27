#pragma once

struct CVertexDeclaration;

class CVertexShader {
  ID3D11VertexShader* vs = nullptr;
  ID3D11InputLayout*  vertex_layout = nullptr;
  const CVertexDeclaration* vertex_decl = nullptr;
public:
  bool create( const std::string& source, const std::string& entry_fn, const std::string& vertex_decl);
  void destroy();
  void activate() const;
  const CVertexDeclaration* getVertexDecl() const { return vertex_decl; }

  static void deactivateResources();
};

class CPixelShader {
  ID3D11PixelShader* ps = nullptr;
public:
  bool create(const std::string& source, const std::string& entry_fn);
  void destroy();
  void activate() const;
  bool isValid() const { return ps != nullptr; }
};


