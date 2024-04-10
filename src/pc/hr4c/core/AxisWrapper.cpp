#include <hr4c/core/AxisWrapper.hpp>
#include <hr4c/core/h4rc.hpp>
#include <hr4c/core/propdict.hpp>
#include <imgui.h>

namespace hr4c {
void AxisWrapper::update_sensor() {
  interface_.update_sensor();
  auto pos = interface_.get_pos();
  auto vel = interface_.get_vel();
  auto cur = interface_.get_cur();
  position = map(pos);
  velocity = map(vel);
  torque   = map(cur);
  buf.add(position, velocity, torque);
}

void AxisWrapper::load_config(const PropertyDict& prop) {
  name              = prop.get("name");
  joint_name        = prop.get("joint_name");
  min_angle         = prop["min_angle"];
  max_angle         = prop["max_angle"];
  min_encoder_value = prop["min_encoder_value"];
  max_encoder_value = prop["max_encoder_value"];
}

PropertyDict AxisWrapper::save_config() const {
  PropertyDict prop;
  prop["name"]              = name;
  prop["joint_name"]        = joint_name;
  prop["min_angle"]         = min_angle;
  prop["max_angle"]         = max_angle;
  prop["min_encoder_value"] = min_encoder_value;
  prop["max_encoder_value"] = max_encoder_value;
  return prop;
}

void AxisWrapper::plot_axis() {
  if(buf.sd_pos.Data.Size == 0) return;
  auto w = ImGui::GetWindowWidth() / 3 - 20;
  ImVec2 size(w, 50);
  ImGui::PushID(name.c_str());
  ImGui::PlotHistogram("## pos", &buf.sd_pos.Data[0], buf.sd_pos.Data.size(), buf.sd_pos.Offset, "pos", min_angle, max_angle, size);
  ImGui::SameLine();
  ImGui::PlotHistogram("## vel", &buf.sd_vel.Data[0], buf.sd_vel.Data.size(), buf.sd_vel.Offset, "vel", min_angle, max_angle, size);
  ImGui::SameLine();
  ImGui::PlotHistogram("## cur", &buf.sd_tor.Data[0], buf.sd_tor.Data.size(), buf.sd_tor.Offset, "torque", min_angle, max_angle, size);
  ImGui::PopID();
}

void AxisWrapper::update_minmax() {
  min_encoder_value = std::min<float>(min_encoder_value, interface_.get_pos());
  max_encoder_value = std::max<float>(max_encoder_value, interface_.get_pos());
}

AxisWrapper::AxisWrapper(int id) : interface_(hr4c::eAx1 + id * hr4c::EachAxisSize) {}

} // namespace hr4c
