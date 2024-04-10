#pragma once
#include <iostream>
#include <vector>

// =============================================================================
//	マクロ定義
// =============================================================================
#define MAX_BUFF 10
#define MAX_LENGTH 255

#define STS_IDLE 0
#define STS_WAIT_STX 1
#define STS_DATA 2
#define STS_WAIT_ETX 3
#define STS_WAIT_BCC 4

#define PRG_VER "Ver 1.0.0"

namespace hr4c {

class fsensor {
private:
  int fd;
  unsigned char delim; // 受信データデリミタ
  unsigned char rcv_buff[MAX_BUFF][MAX_LENGTH];
  unsigned char stmp[MAX_LENGTH];
  std::vector<double> forces;
  std::vector<double> f_offset;
  bool offset_init;
  int offset_cnt;
  int t;
  std::vector<double> gain;
  std::string version;
  std::string SerialNo;
  std::string Type;

  bool Comm_RcvF;
  int p_rd;
  int p_wr;
  int rcv_n;
  int RcvSts;
  unsigned char ucBCC;

  void Comm_Close();
  void Comm_Setup(long baud, int parity, int bitlen, int rts, int dtr, char code);
  int Comm_SendData(unsigned char* buff, int l);
  int Comm_GetRcvData(unsigned char* buff);
  int Comm_CheckRcv();
  void Comm_Rcv();
  unsigned long SendData(unsigned char* pucInput, unsigned short usSize);
  bool status;
  bool start;
  void offset_calc();
  bool init(const char* devname);

public:
  fsensor(const char* devname);
  fsensor()                          = delete;
  fsensor(const fsensor&)            = delete;
  fsensor& operator=(const fsensor&) = delete;

  ~fsensor();
  void SerialStart();
  void SerialStop();
  void GetProductInfo();
  bool GetForceInfo();

  inline std::vector<double> get_forces() { return forces; }
  inline bool get_status() { return status; }
  std::string get_serialNo() { return SerialNo; }
};

} // namespace hr4c
