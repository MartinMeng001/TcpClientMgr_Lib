#include "stdafx.h"
#include <chrono>
#include <thread>
#include "tcpClientsMgrInfc.h"
using namespace std;
using namespace chrono;
extern class TcpClientsMgr g_tcpClientMgr;
int setClient(char* ip, int port, int mode)
{
	int num=g_tcpClientMgr.AddTCPClientObjectWithMode(ip, port, mode);
	if (num > 0)
	{
		g_tcpClientMgr.StartItem(num - 1);
		g_tcpClientMgr.Open(num - 1);
		return num;
	}
	return 0;
}
int  clearClient(int idx)
{
	return g_tcpClientMgr.DeleteTCPClient(idx);
}
int getInfoByType(char* cmd, int length, int type, char* recvbuf, int length2, int timeout, int index)
{
	int checkNetCount = timeout*100;
	while (checkNetCount > 0)
	{
		if (g_tcpClientMgr.GetTCPClientState(index - 1) == csOPEN)
		{
			checkNetCount = 1;
			break;
		}
		checkNetCount--;
		milliseconds delayms(10);
		this_thread::sleep_for(delayms);
	}
	if (checkNetCount == 0) return 0;
	g_TCPClnt[index - 1].SendData(cmd, length);
	if (recvbuf == NULL) return 1;
	if (type == SERVERTYPE_4U_ANNUNIATOR)return g_TCPClnt[index - 1].getRetBufferByCommand(cmd[5], recvbuf, length2, timeout);
	else  if (type == SERVERTYPE_HIS_DEVICES)return g_TCPClnt[index - 1].getRetBufferByCommand('O', recvbuf, length2, timeout);
	return 0;
}
int getMultiInfoByType(char* cmd, int length, int type, char* recvbuf, int length2, int timeout, int index)
{
	int checkNetCount = 100* timeout;
	while (checkNetCount > 0)
	{
		if (g_tcpClientMgr.GetTCPClientState(index - 1) == csOPEN)
		{
			checkNetCount = 1;
			break;
		}
		checkNetCount--;
		milliseconds delayms(10);
		this_thread::sleep_for(delayms);
	}
	if (checkNetCount == 0) return 0;
	g_TCPClnt[index - 1].SendData(cmd, length);
	if (recvbuf == NULL) return 1;
	if (type == SERVERTYPE_4U_ANNUNIATOR)return g_TCPClnt[index - 1].getRetBufferByCommandEnder(cmd[5], recvbuf, length2, timeout);
	return 0;
}