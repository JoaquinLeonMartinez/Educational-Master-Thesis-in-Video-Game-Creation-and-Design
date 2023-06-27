#pragma once

class IResource;

class CResourceType {
public:
  virtual const char* getExtension(int idx = 0) const = 0;
  virtual int getNumExtensions() const { return 1; }
  virtual const char* getName() const = 0;
  virtual IResource* create(const std::string& name) const = 0;
};

// Forward declaration for a function that given a type
// will return a CResourceType
template< typename T >
const CResourceType* getResourceTypeFor();

// -----------------------------------------------------
class IResource {

protected:
  std::string          name;
  const CResourceType* resource_type = nullptr;

public:
  const std::string& getName() const { return name; }
  const CResourceType* getResourceType() const { return resource_type; }
  void setNameAndType(const std::string& new_name, const CResourceType* new_resource_type);

  virtual ~IResource() { }
  virtual void renderInMenu() { }
  virtual void onFileChanged( const std::string& filename ) { }

  template< typename T>
  const T* as() const {
    assert(resource_type);
    assert(getResourceTypeFor<T>());
    assert(getResourceTypeFor<T>() == resource_type
      || fatal("You are trying to convert the resource '%s' of type '%s' to resource type '%s'"
        , getName().c_str()
        , resource_type->getName()
        , getResourceTypeFor<T>()->getName()
      ));
    return static_cast<const T*>(this);
  }

};

// ----------------------------------------
class CResourcesManager {

  std::unordered_map< std::string, IResource * > all_resources;

  std::vector< const CResourceType* > resource_types;

  // name = data/meshes/car.mesh
  const CResourceType* findResourceType(const std::string& name) {
    assert(!name.empty());

    // Find extension
    auto idx = name.find_last_of(".");
    assert(idx != std::string::npos || fatal( "Can't find extension to identify resource in %s", name.c_str()));

    // ext = mesh
    std::string ext = name.substr(idx+1);

    return findResourceTypeByExtension(ext);
  }

  // -----------------------------------------------
  const CResourceType* findResourceTypeByExtension(const std::string& ext) {

    for (auto it : resource_types) {
      int n = it->getNumExtensions();
      for( int i = 0; i<n; ++i ) {
        if (it->getExtension(i) == ext)
          return it;
      }
    }
    return nullptr;
  }

public:

  void registerResourceType(const CResourceType* res_type) {
    // assert is new!! and the handled extension is not handled by any other res type
    for (int i = 0; i < res_type->getNumExtensions(); i++) {
      const char* ext = res_type->getExtension(i);
      assert(findResourceTypeByExtension(ext) == nullptr);
    }
    resource_types.push_back(res_type);
  }

  bool exists(const std::string& name) const {
    return all_resources.find(name) != all_resources.end();
  }

  void registerResource(IResource* new_resource) {
    // Must exists and have a valid name
    assert(new_resource);
    assert(!new_resource->getName().empty());
    std::string name = new_resource->getName();
    // The resource MUST be unique by name (or they are registering exactly the same resource)
    assert(!exists(name) || all_resources[name] == new_resource );
    all_resources[name] = new_resource;
  }

  void deleteResources() {
	  for (auto it = all_resources.begin(); it != all_resources.end(); )
	  {
		 all_resources.erase(it++); 
	  }
  }

  const IResource* get(const std::string& name) {
    auto it = all_resources.find(name);
    if (it != all_resources.end())
      return it->second;

    // Identify the type
    auto resource_type = findResourceType(name);
    assert(resource_type || fatal("Can't identify resource type %s\n", name.c_str()));

    // Create a context entry
    TFileContext fc(name);

    // Someone to load
    PROFILE_FUNCTION_COPY_TEXT(name.c_str());
    IResource* new_resource = resource_type->create(name);
    assert(new_resource || fatal( "Failed to create resource %s of type %s\n", name.c_str(), resource_type->getName()));

    // Register
    registerResource(new_resource);

    return new_resource;
  }

  void onFileChanged(const std::string& strfilename);
  void renderInMenu();

  void destroyAll();

};

extern CResourcesManager Resources;



