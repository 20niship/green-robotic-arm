#include <IconsFontAwesome6.h>
#include <complex>
#include <fstream>
#include <hr4c/core/robot.hpp>
#include <imgui.h>
#include <loguru/loguru.hpp>

namespace hr4c {

void Hr4cRobot::update() {
  if(!is_connected_) {
    LOG_F(WARNING, "マエストロのサーバーに接続されていません");
    return;
  }
  for(auto& axis : axis_) axis.update_sensor();
  if(fsensor_ != nullptr) fsensor_->GetForceInfo();
}

void Hr4cRobot::update_gui() {
  // ImGui::Begin("Plot", &show_plot_);
  ImGui::Begin("Plot");
  ImGui::Checkbox("Plot", &show_plot_);
  static bool v_ = true;
  if(!show_plot_) {
    ImGui::End();
    return;
  }
  for(auto& axis : axis_) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    auto str = axis.name + " @ link=" + axis.joint_name;
    if(ImGui::TreeNode(str.c_str())) {
      ImGui::Text("Pos, vel, cur  %.2f, %.2f, %.2f", axis.position, axis.velocity, axis.torque);
      ImGui::SameLine();
      ImGui::TextDisabled("Encoder %d %d %d", axis.interface_.get_pos(), axis.interface_.get_vel(), axis.interface_.get_cur());
      axis.plot_axis();
      ImGui::TreePop();
    }
  }
  ImGui::SeparatorText("controls");
  if(ImGui::Button("encoder-minmax") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_K))) {
    for(auto& axis : axis_) axis.update_minmax();
    LOG_F(INFO, "Updating encoder minmax....");
  }
  ImGui::SameLine();
  if(ImGui::Button("SaveConfig")) {
    save_config("../elmo-config2.toml");
  }

  ImGui::End();

  ImGui::Begin("Axis Viewer", &show_axis_control_);
  for(auto& a : axis_) {
    auto poweredon = a.is_poweron;
    if(ImGui::Checkbox(a.name.c_str(), &poweredon)) {
      if(poweredon)
        a.poweron();
      else
        a.poweroff();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("link=%s, pos=%.2f", a.joint_name.c_str(), a.position);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.1, 0.1, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0.1, 0.1, 1));
    ImGui::Button(ICON_FA_TOGGLE_OFF "OFF");
    ImGui::PopStyleColor(2);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.8, 0.1, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1, 0.9, 0.1, 1));
    ImGui::Button(ICON_FA_TOGGLE_ON "ON");
    ImGui::PopStyleColor(2);
  }

#if 0
  if(ImGui::CollapsingHeader("Power", true)) {
    ImGui::Text("parameters");
    int cur_limit = 400;
    float kp      = 0.1;
    float kd      = 0.1;
    float ki      = 0.3;
    ImGui::SliderInt("TorLimit", &cur_limit, 0, 5000);
    ImGui::SliderFloat("Kp", &kp, 0, 1);
    ImGui::SliderFloat("Kd", &kd, 0, 1);
    ImGui::SliderFloat("Ki", &ki, 0, 1);
    if(ImGui::Button("Set")) {
      LOG_F(INFO, "set pid parameter");
    }
  }

  if(ImGui::CollapsingHeader("Axis", true)) {
    ImGui::Text("Axis");
    ImGui::SameLine();
    int i = 0;
    for(auto& axis : axis_) {
      i++;
      ImGui::Text("%s", axis.name.c_str());
      if(ImGui::Button(axis.name.c_str())) {
        LOG_F(INFO, "axis %s start!!", axis.name.c_str());
        // axis.axis.poweron();
      }
      static int homing = 1000;
      if(ImGui::Button("Home")) {
        homing = 0;
      }
      if(homing < 1000) {
        ImGui::SameLine();
        homing++;
        LOG_F(INFO, "home start!!");
        auto client = hr4c::ModbusClient::Get();
        axis.set_target_pos(0);
        client->send_command_data();
        ImGui::Text("Homing %d", homing);
      }

      if(ImGui::Button("Stop")) axis.poweroff();
    }
  }
#endif
  ImGui::End();
}

#if 0
void Hr4cRobot::update_visuzlize(const std::shared_ptr<melchior::URDFModelPlugin>& model) {
  if(model == nullptr) return;
  auto urdf = model->m_urdf_model;
  if(urdf == nullptr) return;

  for(auto& axis : axis_) {
    auto joint = urdf->getJoint(axis.joint_name);
    if(joint == nullptr) continue;
    auto angle = axis.position;
    urdf->set_joint_angle(axis.joint_name, angle);
  }

  if(fsensor_) {
    auto m = Empty::Find("endeffector");
    if(!m) return;
    auto abspo = m->get_absolute_position();
    auto r     = melchior::get_main_renderer();
    auto force = fsensor_->get_forces();
    auto fv    = Vec3f(force[3], force[4], force[5]);
    r->arrow(abspo, abspo + fv * 10, Vec3b(255, 0, 0));
  }
}
#endif

void Hr4cRobot::load_config(const std::string& filepath) {
  LOG_F(INFO, "Save config : %s", filepath.c_str());

  std::ifstream ifs(filepath);
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  auto prop = json::jobject::parse(str.c_str());
  LOG_F(INFO, "config %s", prop.pretty(2).c_str());

#if 0
  if(prop.is_array("axis")) {
    auto axis_prop = prop.get_object("axis");
    LOG_F(INFO, "axis size %d", (int)axis_prop.size());
    for(int i = 0; i < axis_prop.size(); i++) {
      auto axis_data = axis_prop[i];
      if(axis_data.is_object()) {
        auto config = axis_data.get_object();
        auto id     = config.get_int_or("id", i);
        auto a      = AxisWrapper(id);
        a.load_config(config);
        axis_.push_back(a);
      } else {
        LOG_F(ERROR, "Could not find axis config %s", axis_data.str().c_str());
      }
    }
  }

  if(prop.is_object("socket")) {
    auto socket = prop.get_object("socket");
    ip_         = socket.get_string("ip");
    port_       = socket.get_int("port");
  } else {
    LOG_F(ERROR, "Could not find socket config");
  }
  if(prop.is_object("fsensor")) {
    auto fsensor = prop.get_object("fsensor");
    if(fsensor.is_string("devname")) devname_ = fsensor.get_string("devname");
  } else {
    LOG_F(ERROR, "Could not find fsensor config");
  }
#endif
}

void Hr4cRobot::save_config(const std::string& filepath) const {
#if 0
  LOG_F(INFO, "Save config : %s", filepath.c_str());
  PropertyDict a;
  for(auto& axis : axis_) {
    PropertyDict d = axis.save_config();
    a.push_back(d);
  }
  PropertyDict prop;
  prop["axis"] = a;

  PropertyDict socket;
  socket["ip"]   = ip_;
  socket["port"] = port_;
  prop["socket"] = socket;
  prop.dump_toml_file(filepath);
#endif
}

Hr4cRobot::Hr4cRobot() {}
Hr4cRobot::~Hr4cRobot() {}

void Hr4cRobot::poweron_all() {
  for(auto& axis : axis_) axis.poweron();
}

void Hr4cRobot::poweroff_all() {
  for(auto& axis : axis_) axis.poweroff();
}

bool Hr4cRobot::start() {
  if(is_connected_) return true;
  auto res = hr4c::start(ip_, port_);
  if(!res) {
    LOG_F(ERROR, "Could not start server, ip, port = %s, %d", ip_.c_str(), port_);
    hr4c::terminate();
  }
  is_connected_ = res;

  fsensor_ = std::make_shared<fsensor>(devname_.c_str());
  fsensor_->SerialStart();
  if(!fsensor_->get_status()) {
    LOG_F(ERROR, "Could not open fsensor %s", devname_.c_str());
    fsensor_ = nullptr;
    return false;
  }

  return res;
}


} // namespace hr4c
