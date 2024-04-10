#include <iostream>
#include <loguru/loguru.hpp>
// シリアル通信用
#include <hr4c/fsensor/Fsensor.hpp>

// シリアル通信用
#include "pComResInternal.h"
#include "pCommon.h"
#include "rs_comm.h"
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

namespace hr4c {

fsensor::fsensor(const char* devname) { // /dev/ttyACM0
  rcv_n     = 0;
  p_rd      = 0;
  p_wr      = 0;
  Comm_RcvF = false;
  RcvSts    = STS_IDLE;
  status    = false;
  forces.resize(6, 0);
  init(devname);
  start = false;
  offset_calc();
  t = 0;
}

bool fsensor::init(const char* devname) {
  bool ret = false;
  fd       = open(devname, O_RDWR | O_NDELAY | O_NOCTTY);
  if(fd > 0) {
    delim = 0;
    Comm_Setup(460800, PAR_NON, BIT_LEN_8, 0, 0, CHR_ETX);
    // Comm_Setup( 230400, PAR_NON, BIT_LEN_8, 0, 0, CHR_ETX);
    status = true;
    SerialStop();
    usleep(10000);
    ret = true;
  } else {
    std::cerr << "Can't Open :" << devname << std::endl;
    // exit(1);
  }

  gain.resize(6);
  gain[0] = 300.0 / 10000;
  gain[1] = 300.0 / 10000;
  gain[2] = 300.0 / 10000;
  gain[3] = 4.0 / 10000;
  gain[4] = 4.0 / 10000;
  gain[5] = 2.0 / 10000;
  return ret;
}

fsensor::~fsensor() {
  SerialStop();
  Comm_Close();
}

void fsensor::Comm_Close() {
  if(fd > 0) {
    close(fd);
  }
  fd = 0;
  return;
}

void fsensor::Comm_Setup(long baud, int parity, int bitlen, int rts, int dtr, char code) {
  long brate;
  long cflg;

  switch(baud) {
    case 2400: brate = B2400; break;
    case 4800: brate = B4800; break;
    case 9600: brate = B9600; break;
    case 19200: brate = B19200; break;
    case 38400: brate = B38400; break;
    case 57600: brate = B57600; break;
    case 115200: brate = B115200; break;
    case 230400: brate = B230400; break;
#ifdef B460800
    case 460800: brate = B460800; break;
#endif
    default: brate = B9600; break;
  }
  // パリティ
  switch(parity) {
    case PAR_NON: cflg = 0; break;
    case PAR_ODD: cflg = PARENB | PARODD; break;
    default: cflg = PARENB; break;
  }
  // データ長
  switch(bitlen) {
    case 7: cflg |= CS7; break;
    default: cflg |= CS8; break;
  }
  // DTR
  switch(dtr) {
    case 1: cflg &= ~CLOCAL; break;
    default: cflg |= CLOCAL; break;
  }
  // RTS CTS
  switch(rts) {
    case 0: cflg &= ~CRTSCTS; break;
    default: cflg |= CRTSCTS; break;
  }

  // ポート設定フラグ
  struct termios tio; // ポート設定構造体
  tio.c_cflag     = cflg | CREAD;
  tio.c_lflag     = 0;
  tio.c_iflag     = 0;
  tio.c_oflag     = 0;
  tio.c_cc[VTIME] = 0;
  tio.c_cc[VMIN]  = 0;

  int ret = cfsetspeed(&tio, brate);
  if(ret < 0) std::cerr << "cfsetspeed error\n";

  if(fd > 0) {
    int ret  = tcflush(fd, TCIFLUSH);        // バッファの消去
    int ret2 = tcsetattr(fd, TCSANOW, &tio); // 属性の設定
  }
  delim = code; // デリミタコード
  return;
}

int fsensor::Comm_SendData(unsigned char* buff, int l) {
  if(fd <= 0) return -1;
  int ret = write(fd, buff, l);
  if(ret < 0) return -1;
  return OK;
}

int fsensor::Comm_GetRcvData(unsigned char* buff) {
  int l = rcv_buff[p_rd][0];
  if(p_wr == p_rd) return 0;
  memcpy(buff, &rcv_buff[p_rd][0], l);
  p_rd++;
  if(p_rd >= MAX_BUFF) p_rd = 0;
  // l=strlen((const char* )buff);
  return l;
}

int fsensor::Comm_CheckRcv() { return p_wr - p_rd; }

void fsensor::Comm_Rcv() {
  int i, rt = 0;
  unsigned char ch;
  unsigned char rbuff[MAX_LENGTH];

  while(1) {
    rt = read(fd, rbuff, 1);

    // 受信データあり
    if(rt > 0) {
      rbuff[rt] = 0;
      ch        = rbuff[0];

      switch(RcvSts) {
        case STS_IDLE:
          ucBCC = 0; /* BCC */
          rcv_n = 0;
          if(ch == CHR_DLE) RcvSts = STS_WAIT_STX;
          break;
        case STS_WAIT_STX:
          if(ch == CHR_STX) { /* STXがあれば次はデータ */
            RcvSts = STS_DATA;
          } else { /* STXでなければ元に戻る */
            RcvSts = STS_IDLE;
          }
          break;
        case STS_DATA:
          if(ch == CHR_DLE) { /* DLEがあれば次はETX */
            RcvSts = STS_WAIT_ETX;
          } else { /* 受信データ保存 */
            stmp[rcv_n] = ch;
            ucBCC ^= ch; /* BCC */
            rcv_n++;
          }
          break;
        case STS_WAIT_ETX:
          if(ch == CHR_DLE) { /* DLEならばデータである */
            stmp[rcv_n] = ch;
            ucBCC ^= ch; /* BCC */
            rcv_n++;
            RcvSts = STS_DATA;
          } else if(ch == CHR_ETX) { /* ETXなら次はBCC */
            RcvSts = STS_WAIT_BCC;
            ucBCC ^= ch;             /* BCC */
          } else if(ch == CHR_STX) { /* STXならリセット */
            ucBCC  = 0;              /* BCC */
            rcv_n  = 0;
            RcvSts = STS_DATA;
          } else {
            ucBCC  = 0; /* BCC */
            rcv_n  = 0;
            RcvSts = STS_IDLE;
          }
          break;
        case STS_WAIT_BCC:
          if(ucBCC == ch) { /* BCC一致 */
            // 作成された文字列をリングバッファへコピー
            memcpy(rcv_buff[p_wr], stmp, rcv_n);
            p_wr++;
            if(p_wr >= MAX_BUFF) p_wr = 0;
          }
          /* 次のデータ受信に備える */
          ucBCC  = 0; /* BCC */
          rcv_n  = 0;
          RcvSts = STS_IDLE;
          break;
        default: RcvSts = STS_IDLE; break;
      }

      if(rcv_n > MAX_LENGTH) {
        ucBCC  = 0;
        rcv_n  = 0;
        RcvSts = STS_IDLE;
      }
    } else {
      break;
    }

    // 受信完了フラグ
    if(p_rd != p_wr) {
      Comm_RcvF = true;
    } else {
      Comm_RcvF = false;
    }
  }
}

void fsensor::GetProductInfo() {

  if(fd > 0) {
    UCHAR SendBuff[512];
    unsigned short len;
    std::cout << "Get Product Info\n";
    len         = 0x04;
    SendBuff[0] = len;
    SendBuff[1] = 0xff;
    SendBuff[2] = CMD_GET_INF;
    SendBuff[3] = 0;
    SendData(SendBuff, len);

    UCHAR CommRcvBuff[256];
    bool EndF = false;
    while(!EndF) {
      Comm_Rcv();

      if(Comm_CheckRcv() != 0) { // 受信データ有
        CommRcvBuff[0] = 0;

        int rt = Comm_GetRcvData(CommRcvBuff);
        if(rt > 0) {
          ST_R_GET_INF* stGetInfo       = (ST_R_GET_INF*)CommRcvBuff;
          stGetInfo->scFVer[F_VER_SIZE] = 0;
          {
            char tmp[F_VER_SIZE + 1];
            for(int i = 0; i < F_VER_SIZE; i++) tmp[i] = stGetInfo->scFVer[i];
            tmp[F_VER_SIZE] = 0;
            version         = tmp;
          }

          stGetInfo->scSerial[SERIAL_SIZE] = 0;
          {
            char tmp[SERIAL_SIZE + 1];
            for(int i = 0; i < SERIAL_SIZE; i++) tmp[i] = stGetInfo->scSerial[i];
            tmp[SERIAL_SIZE] = 0;
            SerialNo         = tmp;
          }

          stGetInfo->scPName[P_NAME_SIZE] = 0;
          {
            char tmp[P_NAME_SIZE + 1];
            for(int i = 0; i < P_NAME_SIZE; i++) {
              tmp[i] = stGetInfo->scPName[i];
            }
            tmp[P_NAME_SIZE] = 0;
            Type             = tmp;
          }

          if(rt == 38 && Type.find("PFS") != string::npos) EndF = true;
        }
      } else {
        SendData(SendBuff, len);
      }
    }
    std::cout << "Version: " << version << std::endl;
    std::cout << "SerialNo:" << SerialNo << std::endl;
    std::cout << "Type:" << Type << std::endl;
  }
}

void fsensor::SerialStart() {
  if(fd > 0) {
    UCHAR SendBuff[512];
    unsigned short len;
    std::cout << "Start\n";

    len         = 0x04;
    SendBuff[0] = len;
    SendBuff[1] = 0xFF;
    SendBuff[2] = CMD_DATA_START;
    SendBuff[3] = 0;

    SendData(SendBuff, len);
    start = true;
  }
}

void fsensor::SerialStop() {
  if(fd > 0) {
    UCHAR SendBuff[512];
    unsigned short len;
    std::cout << "Stop\n";

    len         = 0x04;
    SendBuff[0] = len;
    SendBuff[1] = 0xff;
    SendBuff[2] = CMD_DATA_STOP;
    SendBuff[3] = 0;
    if(SendData(SendBuff, len) == OK) std::cout << "send data OK for stop\n";
  }
}

unsigned long fsensor::SendData(unsigned char* pucInput, unsigned short usSize) {

  USHORT usCnt;
  UCHAR ucWork;
  UCHAR ucBCC = 0;
  unsigned char CommSendBuff[1024];
  UCHAR* pucWrite = &CommSendBuff[0];
  USHORT usRealSize;

  // データ整形
  *pucWrite = CHR_DLE; // DLE
  pucWrite++;
  *pucWrite = CHR_STX; // STX
  pucWrite++;
  usRealSize = 2;

  for(usCnt = 0; usCnt < usSize; usCnt++) {
    ucWork = pucInput[usCnt];
    if(ucWork == CHR_DLE) { // データが0x10ならば0x10を付加
      *pucWrite = CHR_DLE;  // DLE付加
      pucWrite++;           // 書き込み先
      usRealSize++;         // 実サイズ
      // BCCは計算しない!
    }
    *pucWrite = ucWork; // データ
    ucBCC ^= ucWork;    // BCC
    pucWrite++;         // 書き込み先
    usRealSize++;       // 実サイズ
  }

  *pucWrite = CHR_DLE; // DLE
  pucWrite++;
  *pucWrite = CHR_ETX; // ETX
  ucBCC ^= CHR_ETX;    // BCC計算
  pucWrite++;
  *pucWrite = ucBCC; // BCC付加
  usRealSize += 3;

  if(Comm_SendData(&CommSendBuff[0], usRealSize) == OK) return OK;
  return -1;
}

bool fsensor::GetForceInfo() {
  bool ret = false;
  UCHAR CommRcvBuff[256];
  Comm_Rcv();
  if(Comm_CheckRcv() != 0) {
    memset(CommRcvBuff, 0, sizeof(CommRcvBuff));
    int rt = Comm_GetRcvData(CommRcvBuff);
    if(rt > 0) {
      ST_R_DATA_GET_F* stForce = (ST_R_DATA_GET_F*)CommRcvBuff;
      if(t < 5) {
        t++;
      } else {
        for(int i = 0; i < 6; i++) forces[i] = (double)stForce->ssForce[i] * gain[i];

        if(!offset_init) {
          for(int i = 0; i < 6; i++) f_offset[i] += forces[i];
          if(++offset_cnt == 100) {
            for(int i = 0; i < 6; i++) f_offset[i] /= 100.;
            offset_init = true;
          }
        } else {
#if 1
          for(int i = 0; i < 6; i++) forces[i] -= f_offset[i];
#endif
        }
        ret = true;
      }

      ST_RES_HEAD* stCmdHead = (ST_RES_HEAD*)CommRcvBuff;
      if(stCmdHead->ucCmd == CMD_DATA_STOP) {
        std::cout << "Receive Stop Response:" << std::endl;
        int l = stCmdHead->ucLen;
        for(int i = 0; i < l; i++) {
          printf("%02x ", CommRcvBuff[i]);
        }
        std::cout << std::endl;
        status = false;
      }
    }
  }
  return ret;
}

void fsensor::offset_calc() {
  offset_init = false;
  offset_cnt  = 0;
  f_offset.resize(6, 0);
}

} // namespace hr4c
