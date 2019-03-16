#pragma once
#include "TcpClientBaseUnit.h"
#include "CTcpClientMgrBase.h"
#include "TcpClientsMgr.h"

extern class CTcpClientMgrBase g_TCPClnt[TCPCLIENT_MAX_NUM];
extern int setClient(char* ip, int port, int mode);
extern int clearClient(int idx);
extern int getInfoByType(char* cmd, int length, int type, char* recvbuf, int length2, int timeout, int index);
extern int getMultiInfoByType(char* cmd, int length, int type, char* recvbuf, int length2, int timeout, int index);

// Complex Interface
// RealTime Data interface
// Running Status interface