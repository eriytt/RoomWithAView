#include <string>
#include <functional>
#include <vector>
#include <map>

class OgreCardboardTestApp;

class Downloader
{
public:
  typedef std::function<void(const std::string &, char *data, size_t len)> Callback;

private:
  struct QueueItem
  {
    std::string uri;
    Callback f;

    QueueItem(const std::string &uri, Callback f): uri(uri), f(f) {}
  };

  std::vector<QueueItem> queue;
  std::map<std::string, QueueItem> pending;
  OgreCardboardTestApp *app;

public:
  Downloader(OgreCardboardTestApp *app): app(app) {}

  // Application interface
  void download(const std::string &uri, Callback f);

  // JNI interface
  bool hasQueuedDownloads() const;
  std::string dequeueDownload();
  void downloadFinished(const std::string &uri, char *data, size_t len);
};
