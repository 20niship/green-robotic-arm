#include "common.hpp"
#include "hr4c.hpp"
#include <MMC_definitions.h>
#include <TorqueControl.hpp>
#include <array>
#include <iostream>
#include <semaphore.h>
#include <vector>

std::array<TorControls, 20> controls;
int control_count = 0;

inline std::string int_to_str(int i) {
  std::stringstream ss;
  ss << i;
  return ss.str();
}

/**
 * @brief 自動で接続されているモータを検索し、controlsに代入する
 */
void search_motors() {
  int i = 0;
  while(i < 20) {
    TorControls c(166, 0.095 * std::pow(10.0f, -5), 1.5, 2000);
    auto name = "a0" + int_to_str(i + 1);
    LOG_F(INFO, "searching %s", name.c_str());
    try {
      if(!c.init(name, gConnHndl)) break;
    } catch(CMMCException excp) {
      if(excp.error() == -9) break;

      LOG_F(ERROR, "CMMCException: %s", excp.what());
      LOG_F(ERROR, "   : axisref = %d", excp.axisRef());
      LOG_F(ERROR, "   : error = %d", excp.error());
      // Maestro Administrative and Motion API.pdf の 66ページ参照
      LOG_F(ERROR, "   : status = %d", excp.status());

      const auto msg = get_cmmc_exception_error_message(excp);
      LOG_F(ERROR, "   : error message = %s", msg.c_str());
      break;
    } catch(std::exception& e) {
      LOG_F(ERROR, "std Exception: %s", e.what());
      break;
    } catch(...) {
      LOG_F(ERROR, "unknown exception");
      break;
    }
    controls[i] = c;
    i++;
  }
  control_count = i;
}

int main(int argc, char* argv[]) {
  init_logger();
  try {
    MainInit();

    search_motors();
    LOG_F(INFO, "found %d motors", controls.size());

    StartMain();
    return 0;
  } catch(CMMCException excp) {
    LOG_F(ERROR, "CMMCException: %s", excp.what());
    LOG_F(ERROR, "   : axisref = %d", excp.axisRef());
    LOG_F(ERROR, "   : error = %d", excp.error());
    // Maestro Administrative and Motion API.pdf の 66ページ参照
    LOG_F(ERROR, "   : status = %d", excp.status());

    const auto msg = get_cmmc_exception_error_message(excp);
    LOG_F(ERROR, "   : error message = %s", msg.c_str());
    goto terminate;
  } catch(std::exception& e) {
    LOG_F(ERROR, "std Exception: %s", e.what());
    goto terminate;
  } catch(...) {
    LOG_F(ERROR, "unknown exception");
    goto terminate;
  }

terminate:
  MainClose();
  giTerminate = true;
  LOG_F(ERROR, "main function terminated");
  return 1;
}

/**
 * Ctrl+Cとかで終了したときに呼ばれる関数
 */
void terminateApp() {
  for(int i = 0; i < control_count; i++) controls[i].abort();
}

/**
 * @brief StartMain()内で一定周期で呼び出される関数
 * 実行周期は hr4c.cppのTIMER_CYCLEとSLEEP_TIMEに依存する（デフォルトで1kHz)
 */
void update() {
  if(giTerminate) return;

  for(int i = 0; i < control_count; ++i) {
    controls[i].update();
    auto p = controls[i].get_pos();
    auto v = controls[i].get_vel();
    auto t = controls[i].get_tor_order();
    // std::cout << "A" << i << " : [" << p << ", " << v << ", " << t << "]" << std::endl;

    std::cout << p << "\t";
    send_n_to16bit<int32_t>(p, mbus_write_in.regArr, hr4c::eAx1 + i * hr4c::EachAxisSize + hr4c::eActualPos);
    send_n_to16bit<int32_t>(v, mbus_write_in.regArr, hr4c::eAx1 + i * hr4c::EachAxisSize + hr4c::eActualVel);
    send_n_to16bit<int32_t>(10, mbus_write_in.regArr, hr4c::eAx1 + i * hr4c::EachAxisSize + hr4c::eActualTor);
  }
  std::cout << std::endl;
}

void ModbusWrite_Received() {
  LOG_F(INFO, "ModbusWrite_Received");
}
