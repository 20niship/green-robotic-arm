#include "common.hpp"
#include <hr4c.hpp>
#include <MMC_definitions.h>
#include <iostream>
#include <semaphore.h>

#include <array>
#include <assert.h>
#include <iostream>
#include <semaphore.h>
#include <spdlog/spdlog.h>
#include <string>

constexpr int MOTOR_COUNT = 3;

const std::array<const char*, MOTOR_COUNT> motor_names = {"a01", "a02", "a03"};
std::array<TorControls, MOTOR_COUNT> controlers;
std::array<double, MOTOR_COUNT> default_pos = {0, 0, 0};

float kp = 0.0003;
float kd = 0.006;

int main(int argc, char* argv[]) {
  init_logger();
  try {
    MainInit();
    for(size_t i = 0; i < MOTOR_COUNT; i++) {
      controlers[i] = TorControls(16, 0.01 * std::pow(10, -5), 1.5, 2000);
      spdlog::info("initializing {}...", motor_names[i]);
      /* if(!controlers[i].init(motor_names[i], gConnHndl)) { */
      if(!controlers[i].init(motor_names[i], gConnHndl)) {
        spdlog::error("torque control init failed");
        goto terminate;
      }
      spdlog::info("torque control poweron ... {}", motor_names[i]);
      if(!controlers[i].poweron()) {
        spdlog::error("torque control 1 poweron failed");
        goto terminate;
      }
      spdlog::info("poweron done!{}", motor_names[i]);
      controlers[i].set_KP(kp);
      controlers[i].set_KD(kd);
      controlers[i].set_limit(500);
    }

    StartMain();
    return 0;
  } catch(CMMCException excp) {
    spdlog::error("CMMCException: {}", excp.what());
    spdlog::error("   : axisref = {}", excp.axisRef());
    spdlog::error("   : error = {}", excp.error());
    // Maestro Administrative and Motion API.pdf の 66ページ参照
    spdlog::error("   : status = {}", excp.status());

    const auto msg = get_cmmc_exception_error_message(excp);
    spdlog::error(msg);

    DISP(excp.what());
    DISP(excp.axisRef());
    DISP(excp.error());
    DISP(excp.status());

    goto terminate;
  } catch(std::exception& e) {
    spdlog::error("std Exception: {}", e.what());
    goto terminate;
  } catch(...) {
    spdlog::error("unknown exception");
    goto terminate;
  }

terminate:
  MainClose();
  giTerminate = true;
  spdlog::info("MainClose end");
  return 1;
}

void terminateApp() {
  for(auto& control : controlers) {
    // control.poweroff();
    control.abort();
  }
}

static float nframes = 0;

void update() {
  if(giTerminate) return; //	Avoid reentrance of this time function

  for(size_t i = 0; i < MOTOR_COUNT; i++) {
    controlers[i].update();
    if(default_pos[i] == 0) {
      spdlog::warn("pos is zero!!, name = {}", controlers[i].get_axis_name());
      auto pos       = controlers[i].get_pos();
      default_pos[i] = pos;
    }
  }

  for(size_t i = 0; i < MOTOR_COUNT; i++) {
    auto d      = std::sin(nframes / 80.0f) * 3 * 1e6;
    float pos   = controlers[i].get_pos();
    auto target = default_pos[i] + d;
    controlers[i].set_target(target);
    controlers[i].p_pi_controlAxis();
    std::cout << " [target, now, torque]: " << int(nframes / 1000) << ": " << controlers[i].get_axis_name() << " " << //
      pos << ", " << d << ", " << controlers[i].get_tor_order() << std::endl;
  }
  nframes++;

  if(isKeyPressed()) {
    spdlog::info("Terminate app by key press");
    TerminateApplication(0);
  }
}

void ModbusWrite_Received() {}
