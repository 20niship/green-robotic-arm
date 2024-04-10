#include <hr4c/core/ModbusClient.hpp>
#include <loguru/loguru.hpp>

namespace hr4c {

bool ModbusClient::start(const std::string ip, int port) {
  // m_ctx = modbus_new_rtu(m_devicename.c_str(), m_serial_speed, m_parity, m_databit, m_stopbit);
  m_ctx = modbus_new_tcp(ip.c_str(), port);
  if(m_ctx == NULL) {
    LOG_F(ERROR, "modbus_new_tcp failed!!, ip: %s, port: %d", ip.c_str(), port);
    return false;
  }

  modbus_set_debug(m_ctx, FALSE);
  // modbus_set_error_recovery(m_ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);
  if(modbus_set_slave(m_ctx, m_slave_id) == -1) {
    LOG_F(ERROR, "modbus_set_slave failed!! slave_id: %d", m_slave_id);
    LOG_F(ERROR, "modbus error: %s", modbus_strerror(errno));
    modbus_free(m_ctx);
    return false;
  }

  for(int i = 0; i < MODBUS_AXIS_DATA_NUM; i++) m_tab_reg[i] = 0;
  for(int i = 0; i < MODBUS_COMMAND_DATA_NUM; i++) m_cmd_reg[i] = 0;

  LOG_F(INFO, "ModbusServer started successfully with ip: %s, port: %d", ip.c_str(), port);
  return true;
}

bool ModbusClient::connect() {
  if(modbus_connect(m_ctx) == -1) {
    LOG_F(ERROR, "Connection failed: %s", modbus_strerror(errno));
    modbus_free(m_ctx);
    return false;
  }
  connected_ = true;
  return true;
}

bool ModbusClient::read_axis_data() {
  int rc;
  rc = modbus_read_registers(m_ctx, m_register_address, MODBUS_AXIS_DATA_NUM, m_tab_reg.data());
  if(rc == -1) {
    LOG_F(ERROR, "modbus read axis data failed!! : %s", modbus_strerror(errno));
    return false;
  }
  return true;
}

bool ModbusClient::read_command_data() {
  int rc;
  rc = modbus_read_registers(m_ctx, m_register_address, MODBUS_COMMAND_DATA_NUM, m_tab_reg.data());
  if(rc == -1) {
    LOG_F(ERROR, "modbus read command data failed!! : %s", modbus_strerror(errno));
    return false;
  }
  return true;
}

bool ModbusClient::send_command_data() const {
  int rc;
  rc = modbus_write_registers(m_ctx, m_register_address, MODBUS_COMMAND_DATA_NUM, m_cmd_reg.data());
  if(rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return false;
  }
  return true;
}

bool ModbusClient::send_axis_data() const {
  int rc;
  rc = modbus_write_registers(m_ctx, m_register_address, MODBUS_AXIS_DATA_NUM, m_tab_reg.data());
  if(rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return false;
  }
  return true;
}

bool ModbusClient::disconnect() {
  modbus_close(m_ctx);
  modbus_free(m_ctx);
  return true;
}

ModbusClient::ModbusClient() {}
ModbusClient::~ModbusClient() { disconnect(); }

ModbusClient* ModbusClient::singleton_ = nullptr;

} // namespace hr4c
