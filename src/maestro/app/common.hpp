#pragma once

#include <cassert>
#include <ctime>
#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include "../../common/communication_const.hpp"

inline bool isKeyPressed() {
  struct termios oldSettings, newSettings;
  tcgetattr(STDIN_FILENO, &oldSettings);
  newSettings = oldSettings;
  newSettings.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

  bool keyPressed = false;
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(STDIN_FILENO, &readSet);
  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;

  if(select(STDIN_FILENO + 1, &readSet, NULL, NULL, &timeout) > 0) {
    char c;
    read(STDIN_FILENO, &c, 1);
    keyPressed = true;
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
  return keyPressed;
}

template <typename T> void send_n_to16bit(const T arg_value, int16_t* ptr, int index) {
  assert(sizeof(T) % sizeof(int16_t) == 0);
  assert(index < MODBUS_WRITE_IN_CNT);
  const void* p = static_cast<const void*>(&arg_value);
  for(size_t i = 0; i < sizeof(T) / sizeof(int16_t); i++) ptr[index + i] = ((int16_t*)p)[i];
};

template <typename T> T read_n_from16bit(const int16_t* ptr, int index) {
  assert(sizeof(T) % sizeof(int16_t) == 0);
  assert(index < MODBUS_READ_CNT);
  T ret;
  void* p = static_cast<void*>(&ret);
  for(size_t i = 0; i < sizeof(T) / sizeof(int16_t); i++) ((int16_t*)p)[i] = ptr[index + i];
  return ret;
};
