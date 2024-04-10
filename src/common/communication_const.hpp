/*
 * communication_const.hpp
 * @brief modbusでPCとMaestro通信時のフォーマット定数を定義する
 * @date 2023/7/7
 * @auther 20niship
 */

#pragma once

namespace hr4c {

// MODBUS CONFIG
// ※Modbusの使用上255byte / 2 = 128 以下にすること
#define MODBUS_WRITE_IN_INDEX 0    // write in is regArr[MODBUS_WRITE_IN_INDEX]
#define MODBUS_WRITE_IN_CNT 120    // to regArr[MODBUS_WRITE_IN_INDEX + MODBUS_WRITE_IN_CNT -1]
#define MODBUS_READ_OUTPUT_INDEX 0 // read_out is regArr[MODBUS_READ_OUTPUT_INDEX]
#define MODBUS_READ_CNT 120        // to regArr[MODBUS_READ_OUTPUT_INDEX + MODBUS_READ_CNT -1]

// MODBUS ARRAY ID
// #define MODBUS_TIME_START_INDEX 0 // 0 <= id <8 is h, m, s, sec

enum mbusStartIdlist : int {
  eTimeStartId = 0, // regArr[eTimeStart] ~ regArr[eTimeStart+8] is Time

  eAx1 = 4,  // regArr[eAx1] ~ regArr[eAx1+6] is pos1, vel1, tor1,
  eAx2 = 18, // regArr[eAx1] ~ regArr[eAx1+6] is pos1, vel1, tor1,

  eActualPos = 0,
  eActualVel = 2,
  eActualTor = 4,
  eActualCur = 6,
  eKpPos     = 8,
  eKpVel     = 10,
  eKiVel     = 12,

  eCommandType        = 0,
  eCommandTorqueStart = 24,
};

enum class mbuxCommandType : int {
  eTorque        = 0,
  ePowerOn       = 1, // 1 <= id <= 6 で value > 0のときにPowerON
  ePowerOff      = 2, // 1 <= id <= 6 のなかでvalue > 0のときにPowerOFF
  eReset         = 3,
  eEmergencyStop = 4, // すべてPowerOff
  eCurrent       = 5, // 1 <= id <= 6 is current
};

constexpr int EachAxisSize = mbusStartIdlist::eAx2 - mbusStartIdlist::eAx1;

} // namespace hr4c
