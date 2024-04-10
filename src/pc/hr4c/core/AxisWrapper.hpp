#pragma once
#include <iostream>
#include <vector>

#include <hr4c/core/AxisInterface.hpp>
#include <hr4c/core/ModbusClient.hpp>
#include <hr4c/core/h4rc.hpp>
#include <hr4c/core/propdict.hpp>
#include <hr4c/core/vector.hpp>
#include <hr4c/graphics/gui.hpp>

namespace hr4c {

struct AxisWrapper {
  AxisWrapper() = delete;
  AxisWrapper(int index);

  void send_cmd(float rad_edata, int Torlim);
  void update_sensor();

  bool poweron() {
    is_poweron = interface_.poweron();
    return is_poweron;
  }
  bool poweroff() {
    is_poweron = !interface_.poweroff();
    return !is_poweron;
  }

  void set_gain(double kp, double kd, double ki);
  void set_target_pos(double pos) { interface_.set_target_pos(map_rev(pos)); }
  double get_motorOtptAxis_rad();

  void update_gui();

  std::string name;       // a01, a.02....
  std::string joint_name; // urdf joint name
  float position = 0;     // ジョイントの位置 [rad]
  float velocity = 0;     // ジョイントの速度 [rad/s]
  float torque   = 0;     // ジョイントにかかるトルク [Nm]

  float min_angle         = 0;
  float max_angle         = 0;
  float min_encoder_value = 0;
  float max_encoder_value = 0;

  void load_config(const PropertyDict& prop);
  PropertyDict save_config() const;
  bool is_poweron;

  void plot_axis();
  void update_minmax();

  AxisInterface interface_;

private:
  ModbusClient* client_;

  inline float map(const float v) const {
    HR4C_ASSERT(min_encoder_value < max_encoder_value);
    return (v - min_encoder_value) / (max_encoder_value - min_encoder_value) * (max_angle - min_angle) + min_angle;
  }
  inline float map_rev(const float v) const {
    HR4C_ASSERT(min_encoder_value < max_encoder_value);
    return (v - min_angle) / (max_angle - min_angle) * (max_encoder_value - min_encoder_value) + min_encoder_value;
  }

  AxisBuffer buf;
  bool command      = false;
  float command_pos = 0;
};



} // namespace hr4c
