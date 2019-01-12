#include "Downloader.hh"
#include "testapp.hh"
#include "Logging.hh"

void Downloader::download(const std::string &uri, Callback f)
{
  queue.push_back(QueueItem(uri,f));
}

bool Downloader::hasQueuedDownloads() const
{
  // TODO: thread safety
  return not queue.empty();
}

std::string Downloader::dequeueDownload()
{
  // TODO: thread safety

  if (queue.empty())
    throw std::runtime_error("Download queue empty");

  QueueItem item = std::move(queue[0]);
  queue.erase(queue.begin());
  LOGD("Download of %s pending", item.uri.c_str());
  std::string uri(item.uri);
  pending.insert(std::make_pair(item.uri, std::move(item)));

  return uri;
}

void Downloader::downloadFinished(const std::string &uri, char *data, size_t len)
{
  // TODO: thread safety

  LOGD("Download of %s finished", uri.c_str());
  auto itemIter = pending.find(uri);
  if (itemIter == pending.end())
    return;

  auto item = (*itemIter).second;
  pending.erase(itemIter);

  char *dataCopy = new char[len];
  memcpy(dataCopy, data, len);

  app->runOnApplicationThread([=](){item.f(item.uri, dataCopy, len); delete[] dataCopy;});

  return;
}
