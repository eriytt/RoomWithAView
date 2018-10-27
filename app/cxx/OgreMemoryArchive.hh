#ifndef OGREMEMORYARCHIVE_H
#define OGREMEMORYARCHIVE_H

#include <mutex>

#include <Ogre.h>
#include <OgreArchiveFactory.h>


namespace Ogre {
  class MemoryArchive : public Archive
  {
  private:
    struct MemoryResource
    {
      char *data;
      size_t size;

      MemoryResource(char *data, size_t size): size(size)
      {
        this->data = new char[size];
        memcpy(this->data, data, size);
      }
      MemoryResource(const MemoryResource &r): size(r.size)
      {
        data = new char[size];
        memcpy(this->data, r.data, size);
      }
      ~MemoryResource() {delete[] data;}
    };

    std::map<std::string, MemoryResource> resourceMap;
    mutable std::mutex resourceMutex;

  public:
    MemoryArchive(const String &name): Archive(name, "Memory") {}
    virtual bool exists(const String &filename);
    virtual StringVectorPtr find(const String &pattern, bool recursive=true, bool dirs=false);
    virtual FileInfoListPtr findFileInfo(const String &pattern, bool recursive=true, bool dirs=false) const;
    virtual time_t getModifiedTime(const String &filename);
    virtual bool isCaseSensitive(void) const {return true;}
    virtual StringVectorPtr list(bool recursive = true, bool dirs = false);
    virtual FileInfoListPtr listFileInfo(bool recursive=true, bool dirs=false);
    virtual void load();
    virtual DataStreamPtr open(const String &filename, bool readOnly=true) const;
    virtual void unload();

    void setResource(const String &name, char *data, size_t size);
  };


  class MemoryArchiveFactory: public ArchiveFactory
  {
  private:
    String type;

  public:
    MemoryArchiveFactory() : type("Memory") {}
    virtual ~MemoryArchiveFactory() {}
    virtual Archive *createInstance(const String &name, bool readOnly)
    {
      std::cout << "Creating memory archive " << name << std::endl;
      return OGRE_NEW MemoryArchive(name);
    }

    virtual void destroyInstance(Archive *ptr)
    {
      OGRE_DELETE static_cast<MemoryArchive*>(ptr);
    }

    virtual const String &getType() const
    {
      return type;
    }
  };
}
#endif // OGREMEMORYARCHIVE_H
