#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>
#include <string>

#if 0
#include <filesystem>
#else
// for create direcroy  c
// TODO: disable this
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <loguru.hpp>

inline std::string get_logger_filename() {
  auto now       = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");
  return ss.str();
}

inline void init_logger() {
  const auto fname = get_logger_filename();
#if 0
  std::filesystem::create_directory("logs");
#else
  struct stat st = {0};
  if(stat("logs", &st) == -1) {
    mkdir("logs", 0700);
  }
  const auto path = "./logs/" + fname + ".log";
#endif

  // Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
  loguru::add_file(path.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
  loguru::g_stderr_verbosity = 1;
  loguru::g_preamble_time = false;
  loguru::g_preamble_date = false;
  loguru::g_preamble_uptime = false;
  loguru::g_preamble_thread = false;
}

#define DISP(X) std::cout << #X << " = " << X << std::endl;
