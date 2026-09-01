#include "aws_session.h"

#include <cctype>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>

#include "utils.h"

namespace awssdk {

const std::map<std::string, Aws::Utils::Logging::LogLevel> log_level_mapping = {
  {"OFF", Aws::Utils::Logging::LogLevel::Off},
  {"FATAL", Aws::Utils::Logging::LogLevel::Fatal},
  {"ERROR", Aws::Utils::Logging::LogLevel::Error},
  {"WARN", Aws::Utils::Logging::LogLevel::Warn},
  {"INFO", Aws::Utils::Logging::LogLevel::Info},
  {"DEBUG", Aws::Utils::Logging::LogLevel::Debug},
  {"TRACE", Aws::Utils::Logging::LogLevel::Trace}
};

static bool parse_log_level(K options, Aws::Utils::Logging::LogLevel & log_level) {
  std::string level;
  switch (dict_find_str(options, "loglevel", level)) {
    case DictLookup::Absent:
      return true;
    case DictLookup::WrongType:
      return false;
    case DictLookup::Found:
      break;
  }

  for (char & c : level) { c = (char)std::toupper((unsigned char)c); }
  const auto level_it = log_level_mapping.find(level);
  if (level_it == log_level_mapping.end()) {
    return false;
  }
  log_level = level_it->second;
  return true;
}

static void shut_down_at_exit() {
  AwsSession::getInstance().shutDown();
}

bool AwsSession::initialize(K options_k) {
  if (initialized) return false;
  Aws::Utils::Logging::LogLevel log_level = Aws::Utils::Logging::LogLevel::Info;
  if (!parse_log_level(options_k, log_level)) {
    throw std::invalid_argument("loglevel");
  }
  options.loggingOptions.logLevel = log_level;
  Aws::InitAPI(options);
  initialized = true;
  if (!atexit_registered) {
    std::atexit(&shut_down_at_exit);
    atexit_registered = true;
  }
  return true;
}

bool AwsSession::shutDown() {
  if (!initialized) return false;
  Aws::ShutdownAPI(options);
  initialized = false;
  return true;
}

bool AwsSession::isInitialized() const {
  return initialized;
}

AwsSession& AwsSession::getInstance() {
  static AwsSession instance;
  return instance;
}

K initialize(K options) {
  if (options->t != XD && options->t != 101) {
    return krr("type");
  }

  try {
    return kb(AwsSession::getInstance().initialize(options));
  } catch (const std::exception& exc) {
    return krr(ss((S)exc.what()));
  }
}

K shutDown(K) {
  return kb(AwsSession::getInstance().shutDown());
}

}
