#include "testapp.hh"

#include "Logging.hh"
#include "OgreMemoryArchive.hh"
#include "Downloader.hh"

#if defined(ANDROID)
OgreCardboardTestApp::OgreCardboardTestApp(JNIEnv *env, jobject androidSurface, gvr_context *gvr, AAssetManager* amgr)
  : OgreCardboardApp(env, androidSurface, gvr, amgr)
{
  downloader = new Downloader(this);
}
#else
OgreCardboardTestApp::OgreCardboardTestApp() : OgreCardboardApp()
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
                                            const std::string &textureName)
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
     << "            fragment_program_ref fTexture\n"
     << "            {\n"
     << "                param_named tex int 0\n"
     << "            }\n"
     << "        }\n"
     << "    }\n"
     << "}\n";

  return ss.str();
}

static void MakeTextureMaterial(const std::string &materialName,
                                const std::string &textureName)
{
  std::string ShaderMaterialScript(GenTextureMaterialScript(materialName, textureName));
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
                  MakeTextureMaterial(matName, textureName);
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

          downloader->download(meshUri, [&, this, mesh, position, rotation](const std::string &uri, char *data, size_t len) {
              char *buf = new char[len];
              memcpy(buf, data, len);
              auto archive = this->getModelsResourceArchive();
              archive->setResource(uri, buf, len);

              LOGD("Creating entity for %s", mesh.c_str());
              Ogre::Entity *e = this->sceneManager->createEntity(mesh, mesh + ".mesh", "Models");
              Ogre::SceneNode *n = sceneManager->getRootSceneNode()->createChildSceneNode();
              n->attachObject(e);
              n->translate(position);
              Ogre::Matrix3 r;
              r.FromEulerAnglesXYZ(Ogre::Radian(rotation.x),
                                   Ogre::Radian(rotation.y),
                                   Ogre::Radian(rotation.z));
              n->setOrientation(Ogre::Quaternion(r));
            });
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

void OgreCardboardTestApp::mainLoop()
{
  if (forward)
    {
      Ogre::Vector3 d(lcam->getDerivedDirection());
      forBothCameras([&d](Ogre::Camera *cam){cam->move(d *  0.5);});
    }
  else if (backward)
    {
      Ogre::Vector3 d(lcam->getDerivedDirection());
      forBothCameras([&d](Ogre::Camera *cam){cam->move(d * -0.5);});
    }

  if (left)
    forBothCameras([](Ogre::Camera *cam){cam->yaw(Ogre::Radian( 0.05));});
  else if (right)
    forBothCameras([](Ogre::Camera *cam){cam->yaw(Ogre::Radian(-0.05));});

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

void OgreCardboardTestApp::handleKeyDown(int key)
{
  switch(key)
    {
    case 1:
      left = true;
      break;
    case 2:
      right = true;
      break;
    case 3:
      forward = true;
      break;
    case 4:
      {
        std::ifstream input("meta.json", std::ios::binary);
        std::vector<char> buffer((std::istreambuf_iterator<char>(input)),
                               (std::istreambuf_iterator<char>()));
        buffer.push_back(0);
        reloadMeta(buffer.data(), buffer.size());
      }
      break;
    case 5:
      {
        reload();
      }
      break;
    default:
      break;
    }
}

void OgreCardboardTestApp::handleKeyUp(int key)
{
  switch(key)
    {
    case 1:
      left = false;
      break;
    case 2:
      right = false;
      break;
    case 3:
      forward = false;
      break;
    case 4:
      backward = false;
      break;
    default:
      break;
    }
}

void OgreCardboardTestApp::runOnApplicationThread(Callback f)
{
  std::lock_guard<std::mutex> lock(callbackMutex);
  callbacks.push_back(Promise(f, [](){return true;}));
}
