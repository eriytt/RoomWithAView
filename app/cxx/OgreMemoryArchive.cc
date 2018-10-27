#include "OgreMemoryArchive.hh"

#include <stdexcept>

#include "Logging.hh"

namespace Ogre
{

static bool strmatch(const std::string &str, const std::string & pattern)
{
  size_t n = str.length();
  size_t m = pattern.length();

  // empty pattern can only match with
  // empty string
  if (m == 0)
    return (n == 0);

  // lookup table for storing results of
  // subproblems
  bool lookup[n + 1][m + 1];

  // initailze lookup table to false
  memset(lookup, false, sizeof(lookup));

  // empty pattern can match with empty string
  lookup[0][0] = true;

  // Only '*' can match with empty string
  for (size_t j = 1; j <= m; j++)
    if (pattern[j - 1] == '*')
      lookup[0][j] = lookup[0][j - 1];

  // fill the table in bottom-up fashion
  for (size_t i = 1; i <= n; i++)
    {
      for (size_t j = 1; j <= m; j++)
        {
          // Two cases if we see a '*'
          // a) We ignore ‘*’ character and move
          //    to next  character in the pattern,
          //     i.e., ‘*’ indicates an empty sequence.
          // b) '*' character matches with ith
          //     character in input
          if (pattern[j - 1] == '*')
            lookup[i][j] = lookup[i][j - 1] ||
              lookup[i - 1][j];

          // Current characters are considered as
          // matching in two cases
          // (a) current character of pattern is '?'
          // (b) characters actually match
          else if (pattern[j - 1] == '?' ||
                   str[i - 1] == pattern[j - 1])
            lookup[i][j] = lookup[i - 1][j - 1];

          // If characters don't match
          else lookup[i][j] = false;
        }
    }

  return lookup[n][m];
}

  bool MemoryArchive::exists(const String &filename)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);

    auto path = mName + "/" + filename;
    for (auto r : resourceMap)
      {
        LOGD("Checking if %s match %s", r.first.c_str(), path.c_str());
        if (strmatch(r.first, path))
          {
            LOGD("filename: %s, found as %s", filename.c_str(), r.first.c_str());
            return true;
        }
      }

    LOGD("filename: %s, not found", filename.c_str());
    return false;
  }

  StringVectorPtr MemoryArchive::find(const String &pattern, bool recursive, bool dirs)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    LOGD( "Finding %s, recursive: %s, dirs: %s",
          pattern.c_str(),
          recursive ? "true": "false",
          dirs ? "true": "false");

    StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

    for (auto r : resourceMap)
      if (strmatch(r.first, pattern))
        ret->push_back(r.first);
    LOGI("Found %d matching resources", ret->size());
    return ret;
  }

  FileInfoListPtr MemoryArchive::findFileInfo(const String &pattern, bool recursive, bool dirs) const
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    LOGI("Finding file info for %s", pattern.c_str());
    //std::cout << "Finding file info for " << pattern << std::endl;
    FileInfoListPtr p = FileInfoListPtr(OGRE_NEW FileInfoList);
    //throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not defined");
    return p;
  }

  time_t MemoryArchive::getModifiedTime(const String &filename)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not defined");
    return 0;
  }


  StringVectorPtr MemoryArchive::list(bool recursive, bool dirs)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not defined");
    StringVectorPtr p; p.setNull(); return p;
  }

  FileInfoListPtr MemoryArchive::listFileInfo(bool recursive, bool dirs)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not defined");
    FileInfoListPtr p; p.setNull(); return p;
  }

  void MemoryArchive::load()
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    LOGI("Loading Memory Archive");
  }

  DataStreamPtr MemoryArchive::open(const String &filename, bool readOnly) const
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    auto path = mName + "/" + filename;
    LOGD("Opening archive %s readOnly: %s", path.c_str(), readOnly ? "true" : "false");
    auto r = resourceMap.find(path);

    if (r == resourceMap.end())
      throw std::runtime_error("Resource does not exist");

    DataStreamPtr p = DataStreamPtr(OGRE_NEW MemoryDataStream((*r).second.data, (*r).second.size));
    return p;
  }

  void MemoryArchive::unload()
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not defined");
  }

  void MemoryArchive::setResource(const String &name, char *data, size_t size)
  {
    std::lock_guard<std::mutex> lock(resourceMutex);
    LOGI("Setting resource %s", name.c_str());
    resourceMap.insert(std::make_pair(name, MemoryResource(data, size)));
  }
}
