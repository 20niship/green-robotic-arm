#include <TorqueControl.hpp>
#include <MMC_definitions.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "common.hpp"
#include "hr4c.hpp"
#include "logger.h"
#include <semaphore.h>

#include <iostream>

TorControls control_a1, control_a2;

int main(int argc, char* argv[]) {
  init_logger();
  try {
    MainInit();
    control_a1 = TorControls(166, 0.095, 1.5, 2000);
    control_a2 = TorControls(166, 0.095, 1.5, 2000);

    if(!control_a1.init("a01", gConnHndl)) {
      spdlog::error("torque control init failed");
      goto terminate;
    }

    if(!control_a2.init("a02", gConnHndl)) {
      spdlog::error("torque control init failed");
      goto terminate;
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
  control_a1.abort();
  control_a2.abort();
}

void ModbusWrite_Received() { spdlog::info("Modbus Write Received"); }

void update() {
  if(giTerminate) return; //	Avoid reentrance of this time function
  static unsigned long nFrames = 0;
  nFrames++;

  control_a2.update();
  control_a1.update();

  {
    int32_t tmp_pos = control_a1.get_pos();
    int32_t tmp_vel = control_a1.get_vel();
    int32_t tmp_tor = control_a1.get_tor_order();
    int start_ref   = hr4c::eAx1;
    send_n_to16bit<int32_t>(tmp_pos, mbus_write_in.regArr, start_ref + hr4c::eActualPos);
    send_n_to16bit<int32_t>(tmp_vel, mbus_write_in.regArr, start_ref + hr4c::eActualVel);
    send_n_to16bit<int32_t>(tmp_tor, mbus_write_in.regArr, start_ref + hr4c::eActualTor);
    spdlog::info("pos: {}, vel: {}, tor: {}", tmp_pos, tmp_vel, tmp_tor);

    start_ref = hr4c::eAx2;
    tmp_pos   = control_a2.get_pos();
    tmp_vel   = control_a2.get_vel();
    tmp_tor   = control_a2.get_tor_order();
    send_n_to16bit<int32_t>(tmp_pos, mbus_write_in.regArr, start_ref + hr4c::eActualPos);
    send_n_to16bit<int32_t>(tmp_vel, mbus_write_in.regArr, start_ref + hr4c::eActualVel);
    send_n_to16bit<int32_t>(tmp_tor, mbus_write_in.regArr, start_ref + hr4c::eActualTor);
    spdlog::info("pos: {}, vel: {}, tor: {}", tmp_pos, tmp_vel, tmp_tor);
  }

  {
    auto hoge  = read_n_from16bit<int32_t>(mbus_read_out.regArr, hr4c::eCommand1);
    auto hoge2 = read_n_from16bit<int32_t>(mbus_read_out.regArr, hr4c::eCommand2);
    auto hoge3 = read_n_from16bit<int32_t>(mbus_read_out.regArr, hr4c::eCommand3);
    if(hoge != 0 && hoge2 != 0 && hoge3 != 0) {
      spdlog::info("received!");
      spdlog::info("hoge: {}, hoge2: {}, hoge3: {}", hoge, hoge2, hoge3);
    }
    for(int i = 0; i < MODBUS_READ_CNT; i++) mbus_read_out.regArr[i] = 0;
  }

  if(isKeyPressed()) {
    spdlog::info("Terminate app by key press");
    TerminateApplication(0);
  }
}
