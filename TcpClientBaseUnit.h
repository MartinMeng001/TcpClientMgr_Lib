#pragma once
#include <WinSock2.h>
#define SERVERTYPE_4U_ANNUNIATOR	1
#define SERVERTYPE_HIS_DEVICES			2
class TcpClientBaseUnit
{
private:
	//thread* m_thread;
	static int SocketThreadFunc(void* lparam);
public:
	// Tcp Communication Socket
	//int m_sockfd;
	SOCKET m_sockfd;	// windows version
	int m_sockclose;
	bool m_writeable;
	int m_ExitThreadFlag;
	// Remote Host IP
	char m_remoteHost[255];
	int m_port;
	bool m_bDebugOutput;
	char RecvBuf[1500];
	int m_nRecvLen;
public:
	TcpClientBaseUnit();
	~TcpClientBaseUnit();

	int Open(char* ServerIP, int ServerPort);
	int Close();
	int Connect();

	virtual int SendData(char* buf, int len);
	virtual int PackagePro(char* buf, int len);
	virtual int PackagePro(char* buf, int len, bool beAscii);
	void DebugInfo(char* buf, int len, bool beAscii);
};

