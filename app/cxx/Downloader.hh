#include <string>
#include <functional>
#include <vector>
#include <map>
#include <list>

class OgreCardboardTestApp;

class Downloader
{
public:
  typedef std::function<void(const std::string &, char *data, size_t len)> Callback;

private:
  struct QueueItem
  {
    std::string uri;
    std::list<Callback> callbacks;

    QueueItem(const std::string &uri, Callback f): uri(uri), callbacks{f} {}
  };

  struct ReadyItem
  {
    QueueItem qItem;
    char *data;
    size_t len;

    ReadyItem(QueueItem qi, char *data, size_t len): qItem(qi), data(data), len(len) {}
  };

  std::vector<QueueItem> queue;
  std::map<std::string, QueueItem> pending;
  std::map<std::string, ReadyItem> ready;
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
