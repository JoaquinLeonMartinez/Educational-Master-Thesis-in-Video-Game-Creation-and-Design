#pragma once

class CDataProvider {
public:
  virtual ~CDataProvider() {}
  virtual bool isValid() = 0;
  virtual void read(void* dest, size_t num_bytes) = 0;
  template< class TPOD >
  void read(TPOD& pod) {
    read(&pod, sizeof(TPOD));
  }
};

class CFileDataProvider : public CDataProvider {
  FILE* f = nullptr;

public:
  CFileDataProvider(const char* filename) {
    f = fopen(filename, "rb");
  }

  ~CFileDataProvider() {
    if (f)
      fclose(f);
    f = nullptr;
  }
  bool isValid() override {
    return f != nullptr;
  }
  void read(void* dest, size_t num_bytes) {
    auto nbytes_read = fread(dest, 1, num_bytes, f);
    assert(nbytes_read == num_bytes);
  }

};

class CMemoryDataProvider : public CDataProvider, public std::vector< uint8_t > {
  size_t offset = 0;
public:
  CMemoryDataProvider(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f) {
      size_t sz2 = fseek(f, 0, SEEK_END);
      auto sz = ftell(f);
      resize(sz);
      size_t sz3 = fseek(f, 0, SEEK_SET);
      uint8_t* buf = data();
      auto bytes_read = fread(buf, 1, sz, f);
      assert(sz == bytes_read);
      fclose(f);
    }
  }
  virtual bool isValid() {
    return !empty();
  }
  void read(void* dest, size_t num_bytes) {
    // Do not read beyond the allocated buffer
    assert(offset + num_bytes <= size());
    memcpy(dest, data() + offset, num_bytes);
    offset += num_bytes;
  }
};