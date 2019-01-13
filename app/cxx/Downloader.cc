#include "Downloader.hh"
#include "testapp.hh"
#include "Logging.hh"

void Downloader::download(const std::string &uri, Callback f)
{
  auto dq = std::find_if(queue.begin(), queue.end(),
                         [&](auto item){return item.uri == uri;});
  if (dq != queue.end())
    {
      LOGD("Adding download callback for queued download %s", uri.c_str());
      (*dq).callbacks.push_back(f);
      return;
    }

  auto pq = pending.find(uri);
  if (pq != pending.end())
    {
      LOGD("Adding download callback for pending download %s", uri.c_str());
      (*pq).second.callbacks.push_back(f);
      return;
    }

  auto rq = ready.find(uri);
  if (rq != ready.end())
    {
      LOGD("Adding download callback for ready download %s", uri.c_str());
      (*rq).second.qItem.callbacks.push_back(f);
      return;
    }

  LOGD("Queuing %s for download", uri.c_str());
  queue.push_back(QueueItem(uri, f));

  app->runOnApplicationThread([&, uri, f](){
      LOGD("Running download callback for uri %s", uri.c_str());
      auto itemIter = ready.find(uri);
      auto item = (*itemIter).second;
      for (auto f : item.qItem.callbacks)
        f(uri, item.data, item.len);
      delete[] item.data;
      ready.erase(itemIter);
    },
    [&, uri](){return ready.find(uri) != ready.end();});
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

  auto item = std::move(queue[0]);
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
    {
      LOGW("Download of %s dropped", uri.c_str());
      return;
    }

  auto item = (*itemIter).second;
  pending.erase(itemIter);


  char *dataCopy = new char[len];
  memcpy(dataCopy, data, len);

  LOGD("Download of %s inserted into ready queue", uri.c_str());
  ready.insert(std::make_pair(uri, ReadyItem(std::move(item), dataCopy, len)));

  //app->runOnApplicationThread([=](){item.f(item.uri, dataCopy, len); delete[] dataCopy;});

  return;
}
