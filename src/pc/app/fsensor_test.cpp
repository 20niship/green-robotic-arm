#include <hr4c/fsensor/Fsensor.hpp>
using namespace hr4c;
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
  if(argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <device>" << std::endl;
    return 1;
  }

  auto devname = argv[1];
  auto sensor  = fsensor(devname);

  sensor.SerialStart();
  // sensor.GetProductInfo();

  int i = 0;
  while(i < 1000) {
    sensor.GetForceInfo();
    auto forces = sensor.get_forces();
    for(auto f : forces) {
      std::cout << f << " ";
    }
    i++;
    std::cout << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  sensor.SerialStop();
  return 0;
}
