#include <iostream>
#include <melchior/app.hpp>
#include <melchior/colors.hpp>
#include <melchior/component.hpp>
#include <melchior/filesystem.hpp>
#include <melchior/model.hpp>
#include <melchior/project.hpp>
#include <melchior/urdf/urdf.hpp>
#include <melchior/window.hpp>

using namespace melchior;

void create_joints(const std::shared_ptr<URDF::UrdfModel>& model) {
  HR4C_ASSERT(model != nullptr);

  URDF::Joint joint;
  joint.limits.lower = -3.14;
  joint.limits.upper = 3.14;
  joint.type         = URDF::Joint::JointType::REVOLUTE;

  joint.name = "j0";
  joint.axis = {0, 0, 1};
  model->create_joint_simple_mode(joint, "base-yaw-cylinder");

  joint.axis = {0, 1, 0};
  joint.name = "j1";
  model->create_joint_simple_mode(joint, "link1");

  joint.axis = {0, 1, 0};
  joint.name = "j2";
  model->create_joint_simple_mode(joint, "link2");

  joint.axis = {0, 1, 0};
  joint.name = "j2b";
  model->create_joint_simple_mode(joint, "link2b");

  joint.axis = {0, 1, 0};
  joint.name = "j3";
  model->create_joint_simple_mode(joint, "link3");

  joint.axis = {0, 0, 1};
  joint.name = "j4";
  model->create_joint_simple_mode(joint, "link4");

  joint.axis = {0, 1, 0};
  joint.name = "j5";
  model->create_joint_simple_mode(joint, "link6");

  joint.axis = {0, 0, 1};
  joint.name = "j6";
  model->create_joint_simple_mode(joint, "link7");
}


#include <hr4c/core/robot.hpp>

using namespace melchior;
int main(int argc, char** argv) {
  auto robot             = hr4c::Hr4cRobot();
  std::string configfile = argc > 1 ? argv[1] : "../elmo_config.toml";
  robot.load_config(configfile);

  robot.start();
  /* robot.poweron_all(); */

  melchior::init();
  auto wnd = melchior::get_main_window();

  auto model  = Model::LoadFromFile("../../../model/main.glb");
  auto loader = model->addComponent<URDFModelPlugin>("urdf", std::make_shared<URDFModelPlugin>());
  loader->create_new_urdf_model();
  auto s = Scene::GetActiveScene();
  s->add_model(model);

  auto urdf = loader->m_urdf_model;
  create_joints(urdf);


  while(!wnd->should_close()) {
    wnd->new_frame();

    {
      auto j1 = urdf->get_joint_angle("j1");
      auto j2 = urdf->get_joint_angle("j2");
      urdf->set_joint_angle("j2b", j1 - j2);
      urdf->set_joint_angle("j3", j2 - 0.2);
    }

    robot.update();
    robot.update_visuzlize(loader);
    robot.update_gui();
    melchior::update(true);
  }
  melchior::terminate();
  return 0;
}

