#include "testapp.hh"

#include <unistd.h>
#include <ois/OIS.h>


#include <vector>

#include "keycodes.h"
#include "Downloader.hh"
#include "Logging.hh"

class EventHandler : public OIS::KeyListener
{
public:
  OgreCardboardTestApp *app;
  EventHandler(OgreCardboardTestApp *app) : app(app) {}
  bool keyPressed( const OIS::KeyEvent &arg ) {
    if (arg.key == OIS::KC_W)
      app->handleKeyDown(AKEYCODE_W);
    if (arg.key == OIS::KC_S)
      app->handleKeyDown(AKEYCODE_S);
    if (arg.key == OIS::KC_A)
      app->handleKeyDown(AKEYCODE_A);
    if (arg.key == OIS::KC_D)
      app->handleKeyDown(AKEYCODE_D);
    if (arg.key == OIS::KC_R)
      app->handleKeyDown(AKEYCODE_R);
    if (arg.key == OIS::KC_P)
      app->partialUpdate("{\"Sofa\":{\"position\":{\"x\":-4.5,\"y\":3.4,\"z\":-1.0}}}");
    if (arg.key == OIS::KC_M)
      app->partialUpdate("{\"Sofa\":{\"materials\":{\"Sofa\": {\"color\": \"ff0000\"}}}}");

    return true;
  }
  bool keyReleased( const OIS::KeyEvent &arg ) {
    if (arg.key == OIS::KC_W)
      app->handleKeyUp(AKEYCODE_W);
    if (arg.key == OIS::KC_S)
      app->handleKeyUp(AKEYCODE_S);
    if (arg.key == OIS::KC_A)
      app->handleKeyUp(AKEYCODE_A);
    if (arg.key == OIS::KC_D)
      app->handleKeyUp(AKEYCODE_D);
    return true;
  }
};

int main(int argc, char *argv[])
{

  std::map<std::string, std::string> urlToFile = {
    {"/model", "../../backend/db/complex.mesh"},
    {"/model/meta", "../../backend/db/meta-complex.json"},
    {"/textures/parquet-dark-golden-oak-hardwood-floor-pine-tree-colorful-seamless-texture-256x256.jpg",
     "../../backend/static/textures/parquet-dark-golden-oak-hardwood-floor-pine-tree-colorful-seamless-texture-256x256.jpg"},
    {"/textures/DV_157x152_8352574_01_4c_SE_20170802162619.jpg",
     "../../backend/static/textures/DV_157x152_8352574_01_4c_SE_20170802162619.jpg"},
    {"/model/furniture", "../../backend/db/furniture.json"},
    {"/model/sofa_seat.000.mesh.mesh", "../../backend/db/sofa_seat.000.mesh.mesh"},
    {"/textures/sofa_base_diffuse_AOmap_alpha.png",
     "../../backend/static/textures/sofa_base_diffuse_AOmap_alpha.png"},
    {"/textures/sofa_base_diffuse_AOmap_alpha.png",
     "../../backend/static/textures/sofa_base_diffuse_AOmap_alpha.png"}
  };

  OgreCardboardTestApp *app = new OgreCardboardTestApp();
  app->initialize();


  OIS::ParamList pl;
  std::ostringstream windowHndStr;

  windowHndStr << app->getWindowHandle();
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
  pl.insert(std::make_pair(std::string("XAutoRepeatOn"), "true"));
  pl.insert(std::make_pair(std::string("x11_keyboard_grab"), "false"));
  pl.insert(std::make_pair(std::string("x11_mouse_grab"), "false"));
  pl.insert(std::make_pair(std::string("x11_mouse_hide"), "false"));
  OIS::InputManager *mInputManager = OIS::InputManager::createInputSystem(pl);

  OIS::Keyboard *k =
    static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));

  EventHandler *ehandler = new EventHandler(app);
  k->setEventCallback(ehandler);

  while (true) {
    k->capture();
    app->mainLoop();
    Downloader &dl = app->getDownloader();
    while (dl.hasQueuedDownloads())
      {
        LOGI("Serving fake download");
        auto uri = dl.dequeueDownload();
        LOGI("Downloading (fake) %s", uri.c_str());
        std::ifstream input(std::string(urlToFile[uri]), std::ios::binary);
        if (input.fail())
          throw std::runtime_error("File does not exist");
        std::vector<char> buffer((std::istreambuf_iterator<char>(input)),
                                 (std::istreambuf_iterator<char>()));
        dl.downloadFinished(uri, buffer.data(), buffer.size());
      }
    usleep(50000);
  }
}
