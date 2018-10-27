#include "Downloader.hh"
#include "testapp.hh"

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

  QueueItem item = queue[0];
  queue.erase(queue.begin());

  pending.insert(std::make_pair(item.uri, item));

  return item.uri;
}

void Downloader::downloadFinished(const std::string &uri, char *data, size_t len)
{
  // TODO: thread safety

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
