#include <sys/prctl.h>

#include <android/asset_manager_jni.h>

#include <jni.h>
//#include "vr/gvr/capi/include/gvr.h"

#include "testapp.hh"
#include "OgreMemoryArchive.hh"
#include "Downloader.hh"
#include "Logging.hh"

#define JNI_METHOD(return_type, method_name)                    \
  JNIEXPORT return_type JNICALL                                 \
  Java_com_towersmatrix_rwav_Native_##method_name

static OgreCardboardTestApp *app;

extern "C" {

JNI_METHOD(jlong, InitOgre)(JNIEnv *env, jclass clazz,
                            jobject surface, jlong native_gvr_api, jobject assetManager)
{
  prctl(PR_SET_DUMPABLE, 1);
  AAssetManager* assetMgr = AAssetManager_fromJava(env, assetManager);
  app = new OgreCardboardTestApp(env, surface, reinterpret_cast<gvr_context *>(native_gvr_api), assetMgr);
  app->initialize();
  return 0;
}

JNI_METHOD(void, Render)(JNIEnv *env, jclass clazz)
{
  app->mainLoop();
}

JNI_METHOD(void, UpdateModel)(JNIEnv *env, jclass clazz, jbyteArray array)
{
  LOGI("UpdateModel");
  app->reload();
}

JNI_METHOD(void, UpdateMeta)(JNIEnv *env, jclass clazz, jbyteArray array)
{
}

JNI_METHOD(jboolean, HasQueuedDownloads)(JNIEnv *env, jclass clazz)
{
  Downloader &downloader = app->getDownloader();
  return downloader.hasQueuedDownloads() ?  1U : 0U;
}

JNI_METHOD(jstring, GetNextDownload)(JNIEnv *env, jclass clazz)
{
  Downloader &downloader = app->getDownloader();
  std::string uri(downloader.dequeueDownload());
  return env->NewStringUTF(uri.c_str());
}

JNI_METHOD(void, DownloadFinished)(JNIEnv *env, jclass clazz, jstring uri, jbyteArray array)
{
  jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);
  jsize lengthOfArray = env->GetArrayLength(array);

  char *buf = new char[lengthOfArray];
  memcpy(buf, bufferPtr, lengthOfArray);
  env->ReleaseByteArrayElements(array, bufferPtr, JNI_ABORT);

  const char *uriStr = env->GetStringUTFChars(uri, nullptr);

  Downloader &downloader = app->getDownloader();
  downloader.downloadFinished(std::string(uriStr), buf, lengthOfArray);

  env->ReleaseStringUTFChars(uri, uriStr);
}

JNI_METHOD(void, HandleKeyDown)(JNIEnv *env, jclass clazz, jint key)
{
  app->handleKeyDown(static_cast<int>(key));
}

JNI_METHOD(void, HandleKeyUp)(JNIEnv *env, jclass clazz, jint key)
{
  app->handleKeyUp(static_cast<int>(key));
}

} // extern "C"
