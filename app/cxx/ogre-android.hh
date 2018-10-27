#ifndef OGREANDROID
#define OGREANDROID

#include "vr/gvr/capi/include/gvr.h"

#include <android/asset_manager.h>

#include "ogre.hh"

class OgreCardboardApp: public OgreApp
{
private:
  static Ogre::Matrix4 PerspectiveMatrixFromView(const gvr::Rectf& fov, float z_near, float z_far);

private:
  bool initialized = false;
  const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;
  std::unique_ptr<gvr::GvrApi> gvr_api;
  std::unique_ptr<gvr::BufferViewportList> viewport_list;
  std::unique_ptr<gvr::SwapChain> swapchain;

protected:
  JNIEnv *env;
  jobject androidSurface;
  gvr_context *gvr;
  AAssetManager* amgr;

  Ogre::Root* root = nullptr;
  Ogre::SceneManager* sceneManager = nullptr;
  Ogre::Camera* lcam = nullptr;
  Ogre::Camera* rcam = nullptr;
  Ogre::Viewport* lviewport = nullptr;
  Ogre::Viewport* rviewport = nullptr;

private:
  Ogre::RenderWindow* renderWindow = nullptr;

public:
  OgreCardboardApp(JNIEnv *env, jobject androidSurface, gvr_context *gvr, AAssetManager* amgr);
  virtual void initialize();
  virtual void renderFrame();
  virtual void setupResources(Ogre::ResourceGroupManager &rgm) = 0;
  virtual void setupCamera() = 0; // Should set clip distances

  template <typename R>
  std::pair<R, R> forBothCameras(std::function<R (Ogre::Camera *camera)> f)
  { return std::pair<R, R>(f(lcam), f(rcam));}
  void forBothCameras(std::function<void (Ogre::Camera *camera)> f)
  { f(lcam); f(rcam);}


   template <typename R>
   std::pair<R, R> forBothCamerasAndViewports(std::function<R (Ogre::Camera *camera,
                                                               Ogre::Viewport *viewport)> f)
   { return std::pair<R, R>(f(lcam, lviewport), f(rcam, rviewport)); }

  void forBothCamerasAndViewports(std::function<void (Ogre::Camera *camera,
                                                      Ogre::Viewport *viewport)> f)
  { f(lcam, lviewport); f(rcam, rviewport); }

};

#endif // OGREANDROID
