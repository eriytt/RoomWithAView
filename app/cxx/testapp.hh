#ifndef TESTAPP
#define TESTAPP

#include <mutex>

#if defined(ANDROID)
#  include "ogre-android.hh"
#else
#  include "ogre-linux.hh"
#endif

namespace Ogre {
class MemoryArchive;
}
class Downloader;

class OgreCardboardTestApp: public OgreCardboardApp
{
public:
  typedef std::function<void()> Callback;

private:
  struct Promise {
    Callback function;
    std::function<bool()> validator;

    Promise(Callback function, std::function<bool()> validator): function(function), validator(validator) {}
    bool ready() {return validator();}
  };

  struct Object {
    Ogre::Entity *entity;

    Object(): entity(nullptr) {}
    Object(Ogre::Entity *e): entity(e) {}
  };

private:
  Ogre::Timer * timer;
  Ogre::SceneNode *mNode = nullptr;
  Ogre::Entity *mEnt = nullptr;
  bool forward = false, backward = false, left = false, right = false;

  Ogre::MemoryArchive *modelsArchive = nullptr;
  Ogre::MemoryArchive *dynmaterialsArchive = nullptr;
  Downloader *downloader = nullptr;

  std::list<Promise> callbacks;
  std::recursive_mutex callbackMutex; // needs to

  unsigned long lastFrameTime_us;

  std::map<std::string, Object> objects;

protected:
  void setupCamera();
  void setupResources(Ogre::ResourceGroupManager &rgm);
  void reloadFurniture();

public:
#if defined(ANDROID)
  OgreCardboardTestApp(JNIEnv *env, jobject androidSurface, gvr_context *gvr, AAssetManager* amgr);
#else
  OgreCardboardTestApp();
#endif
  void initialize();
  void reload();
  void reloadModel(char *modelData, size_t len);
  void reloadMeta(char *json, size_t length);
  void partialUpdate(const std::string &json);
  void mainLoop();
  bool handleKeyDown(int key);
  bool handleKeyUp(int key);
  Ogre::MemoryArchive *getModelsResourceArchive() {return modelsArchive;}
  Downloader &getDownloader() {if (downloader == nullptr) throw std::runtime_error("Downloader not allocated"); return *downloader;}

  void runOnApplicationThread(Callback f);
  void runOnApplicationThread(Callback f, std::function<bool()> r);
};

#endif // TESTAPP
