#ifndef LOGGING_HH
#define LOGGING_HH

#ifdef ANDROID

#include <android/log.h>

#define LOG_TAG ((std::string("RoomWithAView(C++@") +  __PRETTY_FUNCTION__  + ")").c_str())
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#else

inline void NATIVE_LOG(std::ostream &stream, const std::string &level, const char *format, ...)
{
  char *s;
  va_list ap;
  va_start(ap, format);

  if (vasprintf(&s, format, ap) == -1)
    std::cerr << "ERROR: Could not format log message for format string \"" << format << std::endl;
  else
    {
      stream << level << ": " << s << std::endl;
      free(s);
    }

  va_end(ap);
}

#define LOGD(format, ...) NATIVE_LOG(std::cout, "DEBUG", (format), ##__VA_ARGS__);
#define LOGW(format, ...) NATIVE_LOG(std::cout, "WARNING", (format), ##__VA_ARGS__);
#define LOGE(format, ...) NATIVE_LOG(std::cerr, "ERROR", format, ##__VA_ARGS__)
#define LOGI(format, ...) NATIVE_LOG(std::cout, "INFO", format, ##__VA_ARGS__)

#endif // ANDROID

#endif // LOGGING_HH
