#pragma once
#define TCPCLIENT_MAX_NUM	20

enum CONNSTATE { csWAIT, csINIT, csCLOSED, csOPEN };

typedef struct _TCPCLIENT_INFO
{
	int		nIdx;
	CONNSTATE	ConnState;
	char		HostIP[16];
	int			HostPort;
	int			Mode;
	char		AdptName[64];
	void*	pTCPClient;
	int			m_ExitFlag;
	int			belock;
	long		clearTime;
}TCPCLIENT_INFO, *PTCPCLIENT_INFO;

class TcpClientsMgr
{
private:
	// tcp client connections manager thread
	static int TCPClientThreadFunc(void* lparam);
protected:
	int checkIdleObject();
	int checkIdleObject(char *ip);
	int deleted(int idx);
public:
	TCPCLIENT_INFO		m_TCPClientInfo[TCPCLIENT_MAX_NUM];
	unsigned int		m_threadIds[TCPCLIENT_MAX_NUM];
	int		m_nTCPClientNum;
public:
	TcpClientsMgr();
	~TcpClientsMgr();

	// Add Tcp Client Object
	//int AddTCPClientObject(char* pHostIP, int nHostPort);
	int AddTCPClientObjectWithMode(char* pHostIP, int nHostPort, int mode);
	// delete all Tcp Clients Objects
	int DeleteAllTCPClient();
	int DeleteTCPClient(int idx);
	int ClearTimeOutInvalidItem();

	// set the TCP Client Connection Object to be csINIT
	int Open(int Idx);
	int Close(int Idx);
	// Get Tcp Client Connection Status
	int GetTCPClientState(int Idx);

	// Start Tcp Client
	int  Start();
	int StartItem(int Idx);
	// close Tcp Client
	int Stop();
	int StopItem(int Idx);
};

extern class CTcpClientMgrBase g_TCPClnt[TCPCLIENT_MAX_NUM];

