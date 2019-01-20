#include "testapp.hh"

#if defined(ANDROID)
#  include <android/keycodes.h>
#else
#  include "keycodes.h"
#endif

#include "Logging.hh"
#include "OgreMemoryArchive.hh"
#include "Downloader.hh"

#if defined(ANDROID)
OgreCardboardTestApp::OgreCardboardTestApp(JNIEnv *env, jobject androidSurface, gvr_context *gvr, AAssetManager* amgr)
  : OgreCardboardApp(env, androidSurface, gvr, amgr), timer(0), lastFrameTime_us(0)
{
  downloader = new Downloader(this);
}
#else
OgreCardboardTestApp::OgreCardboardTestApp() : OgreCardboardApp(), timer(0), lastFrameTime_us(0)
{
  downloader = new Downloader(this);
}
#endif

void OgreCardboardTestApp::setupCamera()
{
  forBothCameras([](Ogre::Camera *cam){
      cam->setNearClipDistance(0.2f);
      cam->setFarClipDistance(50.0f);
    });
}

#if defined(ANDROID)
void OgreCardboardTestApp::setupResources(Ogre::ResourceGroupManager &rgm)
{
  Ogre::ArchiveManager::getSingleton().addArchiveFactory(OGRE_NEW Ogre::MemoryArchiveFactory);
  rgm.createResourceGroup("StaticMaterials", false);
  rgm.createResourceGroup("DynamicMaterials", false);
  rgm.createResourceGroup("Models", false);

  rgm.addResourceLocation("/materials", "APKFileSystem", "StaticMaterials");
  rgm.addResourceLocation("/dynmaterials", "Memory", "DynamicMaterials");
  rgm.addResourceLocation("/model",    "Memory", "Models");

  for (auto i : Ogre::ArchiveManager::getSingleton().getArchiveIterator())
    {
      if (i.second->getName() == "/model")
        modelsArchive = static_cast<Ogre::MemoryArchive*>(i.second);
      else if (i.second->getName() == "/dynmaterials")
        dynmaterialsArchive = static_cast<Ogre::MemoryArchive*>(i.second);
    }
}

#else
void OgreCardboardTestApp::setupResources(Ogre::ResourceGroupManager &rgm)
{
  Ogre::ArchiveManager::getSingleton().addArchiveFactory(OGRE_NEW Ogre::MemoryArchiveFactory);
  rgm.createResourceGroup("StaticMaterials", false);
  rgm.createResourceGroup("DynamicMaterials", false);
  rgm.createResourceGroup("Models", false);

  rgm.addResourceLocation("/dynmaterials", "Memory", "DynamicMaterials");
  rgm.addResourceLocation("/model", "Memory", "Models");
  rgm.addResourceLocation("test-resources", "FileSystem");

  for (auto i : Ogre::ArchiveManager::getSingleton().getArchiveIterator())
    {
      if (i.second->getName() == "/model")
        modelsArchive = static_cast<Ogre::MemoryArchive*>(i.second);
      else if (i.second->getName() == "/dynmaterials")
        dynmaterialsArchive = static_cast<Ogre::MemoryArchive*>(i.second);
    }
}

#endif // ANDROID

void OgreCardboardTestApp::initialize()
{
  OgreCardboardApp::initialize();

  forBothCamerasAndViewports([](Ogre::Camera *c, Ogre::Viewport *vp){
    c->setPosition(Ogre::Vector3(6.0f, 1.87f, 6.0f));
    c->lookAt(Ogre::Vector3(0.0f, 1.87f, 0.0f));
    vp->setBackgroundColour(Ogre::ColourValue::Black);
    });

  Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
  lightdir.normalise();
  Ogre::Light *l = sceneManager->createLight("tstLight");
  l->setType(Ogre::Light::LT_DIRECTIONAL);
  l->setDirection(lightdir);
  l->setDiffuseColour(Ogre::ColourValue::White);
  l->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));


  sceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));

  // mEnt = sceneManager->createEntity("head", "ogrehead.mesh", "Room");
  // mEnt->setMaterialName("wall-color");
  // mNode = sceneManager->getRootSceneNode()->createChildSceneNode();
  // mNode->attachObject(mEnt);

  timer = new Ogre::Timer();
  timer->reset();
  lastFrameTime_us = timer->getMicroseconds();
}


static Ogre::SubEntity *FindSubEntityByName(const Ogre::Entity *ent, const std::string &name)
{
  if (ent == nullptr)
    return nullptr;

  auto mesh = ent->getMesh();

  try
    {
      auto idx = mesh->_getSubMeshIndex(name);
      return ent->getSubEntity(idx);
    }
  catch (Ogre::Exception e)
    {
      return nullptr;
    }
}

static std::string GenTextureMaterialScript(const std::string &materialName,
                                            const std::string &textureName,
                                            const std::string &fProgramName,
                                            const std::vector<std::string> &params)
{
  std::stringstream ss;

  ss << "material " << materialName << "\n"
     << "{\n"
     << "    // first, preferred technique\n"
     << "    technique\n"
     << "    {\n"
     << "        // first pass\n"
     << "        pass\n"
     << "        {\n"
     << "            // Texture unit 0\n"
     << "            texture_unit\n"
     << "            {\n"
     << "                texture " << textureName << "\n"
     << "            }\n"
     << "            vertex_program_ref vUnit\n"
     << "            {\n"
     << "                param_named_auto uWorldViewProj WORLDVIEWPROJ_MATRIX\n"
     << "            }\n"
     << "            fragment_program_ref " << fProgramName << "\n"
     << "            {\n"
     << "                param_named tex int 0\n";

  for (size_t i = 0; i < params.size(); ++i)
    ss << "param_named_auto " << params[i] << " custom " << (i + 1) << "\n";

  ss << "            }\n"
     << "        }\n"
     << "    }\n"
     << "}\n";

  return ss.str();
}

static void MakeTextureMaterial(const std::string &materialName,
                                const std::string &textureName,
                                const std::string &programName,
                                const std::vector<std::string> &params)
{
  std::string ShaderMaterialScript(GenTextureMaterialScript(materialName, textureName, programName, params));
  auto scriptStream = OGRE_NEW Ogre::MemoryDataStream("materialStream",
                                                      // TODO: this is not safe
                                                      (void*)(ShaderMaterialScript.c_str()),
                                                      ShaderMaterialScript.length());
  Ogre::SharedPtr<Ogre::DataStream> ptr =
    Ogre::SharedPtr<Ogre::DataStream>(scriptStream, Ogre::SPFM_DELETE);
  Ogre::MaterialManager::getSingleton().parseScript(ptr, "General");
}

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

void OgreCardboardTestApp::reloadMeta(char *json, size_t length)
{
  LOGI(__PRETTY_FUNCTION__);
  LOGD("json:\n%s", json);
  rapidjson::Document doc;
  json[length] = 0;
  doc.ParseInsitu(json);
  if (doc.HasParseError())
    {
      LOGD("Parsing has error");
      LOGE("Parsing JSON meta failed at offset %d: %s",
           doc.GetErrorOffset(), GetParseError_En(doc.GetParseError()));
      return;
    }

  for (auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
    {

      std::string name(itr->name.GetString());
      const rapidjson::Value &v = doc[itr->name.GetString()];

      LOGI("Reloading materials for subentity %s", name.c_str());

      Ogre::SubEntity *subEnt = FindSubEntityByName(mEnt, name);
      if (subEnt == nullptr)
        {
          LOGW("Subentity %s not found", name.c_str());
          continue;
        }

      std::string type(v["type"].GetString());

      if (type == "SolidColor")
        {
          std::stringstream ss;
          unsigned int color;
          std::string colorString(v["color"].GetString());

          ss << std::hex << colorString;
          ss >> color;

          LOGI("Setting material type %s, parameter %s (0x%x)", type.c_str(), colorString.c_str(), color);
          subEnt->setMaterialName("wall-color");
          //Ogre::MaterialPtr m = subEnt->getMaterial();
          // m->getTechnique(0)->getPass(0)->getFragmentProgramParameters()
          //   ->setNamedConstant("pColor",
          //                      Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
          //                                    ((color & 0xff00) >> 8)     / 256.0f,
          //                                    ((color & 0xff))            / 256.0f,
          //                                    1.0));
          subEnt->setCustomParameter(1, Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
                                                      ((color & 0xff00) >> 8)     / 256.0f,
                                                      ((color & 0xff))            / 256.0f,
                                                      1.0));
        }
      else if (type == "Texture")
        {
          std::string textureURI(v["uri"].GetString());
          LOGI("Material type 'Texture' with uri: %s", textureURI.c_str());
          if (dynmaterialsArchive->find(textureURI)->empty())
            {
              LOGI("GETing texture %s", textureURI.c_str());
              Ogre::MemoryArchive *arch = dynmaterialsArchive;
              // TODO: check if it's pending already (or just queue the callback)
              downloader->download(textureURI, [textureURI, subEnt, arch](const std::string &uri, char *data, size_t len) {
                  const std::string textureName(textureURI.substr(textureURI.rfind('/') + 1, std::string::npos));
                  const std::string matName(textureName);
                  LOGI("Updating resource %s", textureName.c_str());
                  arch->setResource(std::string("/dynmaterials/") + textureName, data, len);
                  LOGI("Creating material %s", matName.c_str());
                  MakeTextureMaterial(matName, textureName, "fTexture", std::vector<std::string>());
                  LOGI("Setting material name %s for subentity", matName.c_str());
                  // TODO: Dangerous, how do we now the subentity is still alive,
                  //       should probably look it up again...
                  subEnt->setMaterialName(matName);
                  return 0;
                });
            }
          else
            subEnt->setMaterialName(textureURI);
          //subEnt->setCustomParameter(1, Ogre::Vector4(1.0, 0.0, 0.0, 1.0));
        }
      else
        {
          // TODO: set default material
        }
    }
}

void OgreCardboardTestApp::reloadFurniture() {
  downloader->download("/model/furniture", [this](const std::string &uri, char *data, size_t len) {
      rapidjson::Document doc;

      data[len] = 0;

      doc.ParseInsitu(data);

      if (doc.HasParseError())
        {
          LOGD("Parsing has error");
          LOGE("Parsing JSON meta failed at offset %d: %s",
               doc.GetErrorOffset(), GetParseError_En(doc.GetParseError()));
          return;
        }


      for (auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
        {

          std::string name(itr->name.GetString());
          const rapidjson::Value &v = doc[itr->name.GetString()];

          std::string mesh(v["mesh"].GetString());
          std::string meshUri = std::string("/model/") + mesh + ".mesh";

          Ogre::Vector3 position(v["position"]["x"].GetDouble(),
                                 v["position"]["y"].GetDouble(),
                                 v["position"]["z"].GetDouble());
          Ogre::Vector3 rotation(v["rotation"]["x"].GetDouble(),
                                 v["rotation"]["y"].GetDouble(),
                                 v["rotation"]["z"].GetDouble());

          downloader->download(meshUri,
                               [&, this, mesh, position, rotation, name]
                               (const std::string &uri, char *data, size_t len) {
              char *buf = new char[len];
              memcpy(buf, data, len);
              auto archive = this->getModelsResourceArchive();
              archive->setResource(uri, buf, len);

              LOGD("Creating entity for %s", mesh.c_str());
              Ogre::Entity *e = this->sceneManager->createEntity(mesh, mesh + ".mesh", "Models");
              objects.insert(std::make_pair(name, Object(e)));
              Ogre::SceneNode *n = sceneManager->getRootSceneNode()->createChildSceneNode();
              n->attachObject(e);
              n->translate(position);
              Ogre::Matrix3 r;
              r.FromEulerAnglesXYZ(Ogre::Radian(rotation.x),
                                   Ogre::Radian(rotation.y),
                                   Ogre::Radian(rotation.z));
              n->setOrientation(Ogre::Quaternion(r));
            });

          auto matNode = v["materials"].GetObject();
          for (auto sm_itr = matNode.MemberBegin(); sm_itr != matNode.MemberEnd(); ++sm_itr)
            {
              std::string submeshName(sm_itr->name.GetString());
              const rapidjson::Value &mv = matNode[sm_itr->name.GetString()];
              std::string type(mv["type"].GetString());

              if (type == "SolidColor")
                {
                  std::stringstream ss;
                  unsigned int color;
                  std::string colorString(mv["color"].GetString());

                  ss << std::hex << colorString;
                  ss >> color;

                  runOnApplicationThread([&, name, submeshName, color]() {
                      LOGD("Setting subentity material for %s.%s", name.c_str(), submeshName.c_str());
                      Ogre::Entity *e = objects[name].entity;

                      Ogre::SubEntity *subEnt = FindSubEntityByName(e, submeshName);
                      if (subEnt == nullptr)
                        {
                          LOGW("Subentity %s not found", name.c_str());
                          return;
                        }

                      LOGI("Setting solid color 0x%x material on %s.%s", color, name.c_str(), submeshName.c_str());
                      subEnt->setMaterialName("wall-color");
                      subEnt->setCustomParameter(1, Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
                                                                  ((color & 0xff00) >> 8)     / 256.0f,
                                                                  ((color & 0xff))            / 256.0f,
                                                                  1.0));
                    });
                }
              else if (type == "TextureColor")
                {
                  std::stringstream ss;
                  unsigned int color;
                  std::string colorString(mv["color"].GetString());

                  ss << std::hex << colorString;
                  ss >> color;

                  std::string textureURI(mv["uri"].GetString());
                  LOGI("Material type 'Texture' with uri: %s", textureURI.c_str());
                  if (dynmaterialsArchive->find(textureURI)->empty())
                    {
                      LOGI("GETing texture %s", textureURI.c_str());
                      downloader->download(textureURI, [&, textureURI, name, submeshName, color](const std::string &uri, char *data, size_t len) {
                          Ogre::Entity *e = objects[name].entity;

                          Ogre::SubEntity *subEnt = FindSubEntityByName(e, submeshName);
                          if (subEnt == nullptr)
                            {
                              LOGW("Subentity %s not found", name.c_str());
                              return;
                            }

                          Ogre::MemoryArchive *arch = dynmaterialsArchive;
                          const std::string textureName(textureURI.substr(textureURI.rfind('/') + 1, std::string::npos));
                          const std::string matName(textureName);
                          LOGI("Updating resource %s", textureName.c_str());
                          arch->setResource(std::string("/dynmaterials/") + textureName, data, len);
                          LOGI("Creating material %s", matName.c_str());
                          MakeTextureMaterial(matName, textureName, "fTextureColor", std::vector<std::string>{"pColor"});
                          LOGI("Setting material name %s for subentity", matName.c_str());
                          subEnt->setMaterialName(matName);
                          subEnt->setCustomParameter(1, Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
                                                                      ((color & 0xff00) >> 8)     / 256.0f,
                                                                      ((color & 0xff))            / 256.0f,
                                                                      1.0));

                        });
                    }
                  else {
                    Ogre::Entity *e = objects[name].entity;

                    Ogre::SubEntity *subEnt = FindSubEntityByName(e, submeshName);
                      if (subEnt == nullptr)
                        {
                          LOGW("Subentity %s not found", name.c_str());
                          return;
                        }

                    subEnt->setMaterialName(textureURI);
                    subEnt->setCustomParameter(1, Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
                                                                ((color & 0xff00) >> 8)     / 256.0f,
                                                                ((color & 0xff))            / 256.0f,
                                                                1.0));

                  }
                }
            }
        }
    });
}

void OgreCardboardTestApp::reloadModel(char *modelData, size_t len)
{
  auto archive = getModelsResourceArchive();
  LOGI("Setting resource");
  archive->setResource("/model/model.mesh", modelData, len);
  if (mEnt != nullptr)
    {
      Ogre::SceneNode *parent = mEnt->getParentSceneNode();
      parent->detachObject(mEnt);
      sceneManager->destroyEntity(mEnt);
      sceneManager->destroySceneNode(mNode);
      Ogre::ResourceGroupManager &rgm = Ogre::ResourceGroupManager::getSingleton();
      rgm.destroyResourceGroup("Models");
      rgm.createResourceGroup("Models", false);
      rgm.addResourceLocation("/model", "Memory", "Models");
    }
  mEnt = sceneManager->createEntity("model", "model.mesh", "Models");

  bool hasUV = false;
  for (unsigned i = 0; i < mEnt->getNumSubEntities(); ++i)
    {
      std::cout << "SubEntity: " << i << std::endl;
      auto decl = mEnt->getSubEntity(i)->getSubMesh()->vertexData->vertexDeclaration;
      for (unsigned e = 0; e < decl->getElementCount(); ++e)
        {
          auto element = decl->getElement(e);
          auto semantic = element->getSemantic();
          std::cout << "  Element has semantic: " << semantic << std::endl;
          if (semantic == Ogre::VES_TEXTURE_COORDINATES)
            hasUV = true;
        }
      if (hasUV)
        std::cout << "It has texture coordinates: " << std::endl;
    }
  std::cout << "Checking for uvs done" << std::endl;
  //mEnt->setMaterialName("floor-texture");
  //Ogre::SubEntity *subEnt = mEnt->getSubEntity(0);
  //subEnt->setCustomParameter(1, Ogre::Vector4(1.0, 0.0, 0.0, 1.0));
  //Ogre::MaterialPtr m = subEnt->getMaterial();
  //m->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("pColor", Ogre::Vector4(1.0, 0.0, 0.0, 1.0));
  //subEnt = mEnt->getSubEntity(1);
  //subEnt->setCustomParameter(1, Ogre::Vector4(0.0, 0.0, 1.0, 1.0));
  mNode = sceneManager->getRootSceneNode()->createChildSceneNode();
  mNode->attachObject(mEnt);
  //return mNode;
  //mNode->pitch(Ogre::Radian(-1.57 - (1.57 / 2.0)));

}

void OgreCardboardTestApp::reload()
{
  LOGI("Reloading");

  downloader->download("/model", [&](const std::string &uri, char *data, size_t len){
      reloadModel(data, len);
      downloader->download("/model/meta", [&](const std::string &uri, char *data, size_t len){
          reloadMeta(data, len);
        });
      reloadFurniture();
    });
  LOGD("reload exit");
}

void OgreCardboardTestApp::partialUpdate(const std::string &json)
{
  LOGI("Partial update: %s", json.c_str());

  rapidjson::Document doc;
  doc.Parse(json.c_str());

  if (doc.HasParseError())
    {
      LOGE("Parsing JSON partial update failed at offset %d: %s",
           doc.GetErrorOffset(), GetParseError_En(doc.GetParseError()));
      return;
    }


  for (auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
    {
      std::string name(itr->name.GetString());
      auto o = objects.find(name);
      if (o == objects.end())
        {
          LOGW("Partial update for missing object: %s", name.c_str());
          return;
        }

      LOGD("Partial update for object: %s", name.c_str());

      const rapidjson::Value &v = doc[itr->name.GetString()];
      for (auto uitr = v.MemberBegin(); uitr != v.MemberEnd(); ++uitr)
        {
          std::string updateType(uitr->name.GetString());
          LOGD("Partial update type: %s", updateType.c_str());
          if (updateType == "position")
            {
              LOGD("Partial position update");
              const rapidjson::Value &pv = v[uitr->name.GetString()];
              Ogre::Real x(pv["x"].GetFloat());
              Ogre::Real y(pv["y"].GetFloat());
              Ogre::Real z(pv["z"].GetFloat());
              (*o).second.entity->getParentSceneNode()->setPosition(x, y, z);
            }
          if (updateType == "materials")
            {
              const rapidjson::Value &mv = v[uitr->name.GetString()];
              for (auto seitr =  mv.MemberBegin(); seitr != mv.MemberEnd(); ++seitr)
                {
                  std::stringstream ss;
                  unsigned int color;
                  std::string colorString(mv[seitr->name.GetString()]["color"].GetString());

                  ss << std::hex << colorString;
                  ss >> color;

                  Ogre::SubEntity *subEnt = FindSubEntityByName((*o).second.entity, seitr->name.GetString());
                  subEnt->setCustomParameter(1, Ogre::Vector4(((color & 0xff0000) >> 16)  / 256.0f,
                                                              ((color & 0xff00) >> 8)     / 256.0f,
                                                              ((color & 0xff))            / 256.0f,
                                                              1.0));
                }
            }
          else
            {
              LOGE("Partial update with unhandled type: %s", updateType.c_str());
            }
        }
    }
}

void OgreCardboardTestApp::mainLoop()
{

  unsigned long frame_time = timer->getMicroseconds();
  Ogre::Real tdelta = (frame_time - lastFrameTime_us) / Ogre::Real(1000000);
  lastFrameTime_us = frame_time;

  if (forward)
    {
      Ogre::Vector3 d(lcam->getDerivedDirection());
      forBothCameras([&](Ogre::Camera *cam){cam->move(d *  1.5 * tdelta);});
    }
  else if (backward)
    {
      Ogre::Vector3 d(lcam->getDerivedDirection());
      forBothCameras([&](Ogre::Camera *cam){cam->move(d * -1.5 * tdelta);});
    }

  if (left)
    forBothCameras([&](Ogre::Camera *cam){cam->yaw(Ogre::Radian( 1.5 * tdelta));});
  else if (right)
    forBothCameras([&](Ogre::Camera *cam){cam->yaw(Ogre::Radian(-1.5 * tdelta));});

  //mNode->translate(Ogre::Vector3(0.1, 0.1, 0.1));
  //mNode->yaw(Ogre::Radian(0.05));
  //mNode->pitch(Ogre::Radian(0.05));
  // if (mNode->getPosition().z > 100.0)
  //   mNode->setPosition(0.0f, 0.0f, 0.0f);

  // Run main thread events
  callbackMutex.lock();
  while(not callbacks.empty() and callbacks.front().ready())
    {
      callbacks.front().function();
      callbacks.pop_front();
    }
  callbackMutex.unlock();

  renderFrame();
}

bool OgreCardboardTestApp::handleKeyDown(int key)
{
  switch(key)
    {
    case AKEYCODE_A:
      left = true;
      break;
    case AKEYCODE_D:
      right = true;
      break;
    case AKEYCODE_W:
      forward = true;
      break;
    case AKEYCODE_S:
      backward = true;
      break;
    case AKEYCODE_R:
      {
        reload();
      }
      break;
    default:
      return false;
    }
  return true;
}

bool OgreCardboardTestApp::handleKeyUp(int key)
{
  switch(key)
    {
    case AKEYCODE_A:
      left = false;
      break;
    case AKEYCODE_D:
      right = false;
      break;
    case AKEYCODE_W:
      forward = false;
      break;
    case AKEYCODE_S:
      backward = false;
      break;
    default:
      return false;
    }
  return true;
}

void OgreCardboardTestApp::runOnApplicationThread(Callback f)
{
  std::lock_guard<std::recursive_mutex> lock(callbackMutex);
  callbacks.push_back(Promise(f, [](){return true;}));
}

void OgreCardboardTestApp::runOnApplicationThread(Callback f, std::function<bool()> r)
{
  std::lock_guard<std::recursive_mutex> lock(callbackMutex);
  callbacks.push_back(Promise(f, r));
}
