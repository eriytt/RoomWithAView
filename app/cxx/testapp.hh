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
  Ogre::SceneNode *mNode = nullptr;
  Ogre::Entity *mEnt = nullptr;
  bool forward = false, backward = false, left = false, right = false;

  Ogre::MemoryArchive *modelsArchive = nullptr;
  Ogre::MemoryArchive *dynmaterialsArchive = nullptr;
  Downloader *downloader = nullptr;

  std::vector<Callback> callbacks;
  std::mutex callbackMutex;

protected:
  void setupCamera();
  void setupResources(Ogre::ResourceGroupManager &rgm);

public:
#if defined(ANDROID)
  OgreCardboardTestApp(JNIEnv *env, jobject androidSurface, gvr_context *gvr, AAssetManager* amgr);
#else
  OgreCardboardTestApp();
#endif
  void initialize();
  void reload();
  void reloadMeta(char *json, size_t length);
  void mainLoop();
  void handleKeyDown(int key);
  void handleKeyUp(int key);
  Ogre::MemoryArchive *getModelsResourceArchive() {return modelsArchive;}
  Downloader &getDownloader() {if (downloader == nullptr) throw std::runtime_error("Downloader not allocated"); return *downloader;}
  void runOnApplicationThread(Callback f);
};

#endif // TESTAPP
