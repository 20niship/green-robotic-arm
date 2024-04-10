#pragma once
#include <iostream>
#include <vector>

#include <hr4c/core/AxisWrapper.hpp>
#include <hr4c/core/h4rc.hpp>
#include <hr4c/core/propdict.hpp>
#include <hr4c/fsensor/Fsensor.hpp>

namespace hr4c {

class Hr4cRobot {
public:
  std::vector<AxisWrapper> axis_;

public:
  Hr4cRobot();
  ~Hr4cRobot();
  // no copyable
  Hr4cRobot(Hr4cRobot& other)            = delete;
  Hr4cRobot(Hr4cRobot&& other)           = delete;
  Hr4cRobot& operator=(const Hr4cRobot&) = delete;
  Hr4cRobot& operator=(Hr4cRobot&&)      = delete;


  bool start();
  bool disconnect();

  // update joint angles of urdf model
  void update_gui();

  //! tomlファイルを読み込んでロボットデータを初期化
  void load_config(const std::string& filepath);
  void save_config(const std::string& filepath) const;

  void update();
  void poweron_all();
  void poweroff_all();

  std::shared_ptr<fsensor> fsensor_ = nullptr;

private:
  bool read(uint16_t* tab_rp_registers, int nb);
  bool write(uint16_t* tab_rp_registers, int nb);

  std::string ip_;
  int port_ = 0;
  std::string config_filepath_;
  std::string devname_    = "/dev/ttyACM0";
  bool is_connected_      = false;
  bool show_plot_         = true;
  bool show_axis_control_ = true;
};

} // namespace hr4c
