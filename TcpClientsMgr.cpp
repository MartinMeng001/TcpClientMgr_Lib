#include "stdafx.h"
#include "unistd.h"
#include <string.h>
#include <winsock2.h>
#include <thread>
#include "ReceiveBuffer.h"
#include "TcpClientBaseUnit.h"
#include "CTcpClientMgrBase.h"
#include "TcpClientsMgr.h"
using namespace std;

extern long GetCurrentTimeBySec();
class TcpClientsMgr g_tcpClientMgr;
class CTcpClientMgrBase g_TCPClnt[TCPCLIENT_MAX_NUM];
TcpClientsMgr::TcpClientsMgr()
{
	m_nTCPClientNum = 0;
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		m_threadIds[i] = 0;
		memset(&m_TCPClientInfo[i], 0, sizeof(TCPCLIENT_INFO));
	}
}

TcpClientsMgr::~TcpClientsMgr()
{
}
int TcpClientsMgr::checkIdleObject()
{
	int ret = 0;
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		if (m_TCPClientInfo[i].pTCPClient == NULL)
		{
			ret = i + 1;
			break;
		}
	}
	return ret;
}
int TcpClientsMgr::checkIdleObject(char *ip)
{
	int ret = 0;
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		if (strcmp(m_TCPClientInfo[i].HostIP, ip) == 0) return 0;
		if (m_TCPClientInfo[i].belock != 0)continue;
		if (m_TCPClientInfo[i].pTCPClient == NULL)
		{
			ret = i + 1;
			break;
		}
	}
	return ret;
}
int TcpClientsMgr::AddTCPClientObjectWithMode(char* pHostIP, int nHostPort, int mode)
{
	//int num = checkIdleObject();
	ClearTimeOutInvalidItem();
	int num = checkIdleObject(pHostIP);
	if (num==0)return 0;
	if (m_TCPClientInfo[num - 1].m_ExitFlag == 1)return 0;
	strcpy(m_TCPClientInfo[num-1].HostIP, pHostIP);
	m_TCPClientInfo[num-1].HostPort = nHostPort;
	m_TCPClientInfo[num - 1].nIdx = num - 1;
	m_TCPClientInfo[num-1].ConnState = csWAIT;
	m_TCPClientInfo[num-1].pTCPClient = (void*)&g_TCPClnt[num-1];
	m_TCPClientInfo[num-1].m_ExitFlag = 0;
	m_TCPClientInfo[num-1].Mode = mode;
	m_TCPClientInfo[num - 1].belock = 1;
	g_TCPClnt[num-1].setMode(mode);
	m_nTCPClientNum++;
	return num;
}
int TcpClientsMgr::DeleteTCPClient(int idx)
{
	int ret = 0;
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		if ((i + 1) == idx)
		{
			if (m_TCPClientInfo[i].pTCPClient != NULL)
			{
				m_TCPClientInfo[i].belock = 2;
				m_TCPClientInfo[i].m_ExitFlag = 1;
				m_TCPClientInfo[i].clearTime = GetCurrentTimeBySec();

				ret = 1;
				break;
			}
		}
	}
	return ret;
}
int TcpClientsMgr::deleted(int idx)
{
	m_TCPClientInfo[idx].pTCPClient = NULL;
	m_nTCPClientNum--;
	if (m_nTCPClientNum < 0)m_nTCPClientNum = 0;
	memset((char*)&m_TCPClientInfo[idx], 0, sizeof(TCPCLIENT_INFO));
	return 0;
}
int TcpClientsMgr::ClearTimeOutInvalidItem()
{
	int timeout = 3;
	long currTime = GetCurrentTimeBySec();
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		if ((m_TCPClientInfo[i].belock > 1) && (m_TCPClientInfo[i].clearTime>0))
		{
			if ((currTime - m_TCPClientInfo[i].clearTime) > timeout)
			{
				memset(&m_TCPClientInfo[i], 0, sizeof(TCPCLIENT_INFO));
			}
		}
	}
	return 0;
}
int TcpClientsMgr::DeleteAllTCPClient()
{
	for (int i = 0; i < TCPCLIENT_MAX_NUM; i++)
	{
		DeleteTCPClient(i + 1);
	}
	m_nTCPClientNum = 0;
	return 1;
}
int  TcpClientsMgr::Start()
{
	for (int i = 0; i < m_nTCPClientNum; i++)
	{
		thread t(TCPClientThreadFunc, (void*)&m_TCPClientInfo[i]);	
		t.detach();
		//m_threadIds[i] = (unsigned int)t.get_id();
	}
	return 0;
}
int TcpClientsMgr::Stop()
{
	for (int i = 0; i < m_nTCPClientNum; i++)
	{
		m_TCPClientInfo[i].m_ExitFlag = 1;
	}
	return 0;
}
int TcpClientsMgr::StartItem(int Idx)
{
	if (m_nTCPClientNum == 0) return 0;
	thread t(TCPClientThreadFunc, (void*)&m_TCPClientInfo[Idx]);
	t.detach();
	return 1;
}
int TcpClientsMgr::StopItem(int Idx)
{
	if (m_nTCPClientNum == 0) return 0;
	m_TCPClientInfo[Idx].m_ExitFlag = 1;
	return 1;
}
int TcpClientsMgr::Open(int Idx)
{
	//if (Idx >= m_nTCPClientNum) return -1;
	if (m_TCPClientInfo[Idx].ConnState != csWAIT) return -1;
	m_TCPClientInfo[Idx].ConnState = csINIT;
	return 0;
}
int TcpClientsMgr::Close(int Idx)
{
	//if (Idx >= m_nTCPClientNum) return -1;
	m_TCPClientInfo[Idx].m_ExitFlag = 1;
	return 0;
}

int TcpClientsMgr::GetTCPClientState(int Idx)
{
	//if (Idx >= m_nTCPClientNum) return -1;
	return (int)m_TCPClientInfo[Idx].ConnState;
}

int TcpClientsMgr::TCPClientThreadFunc(void* lparam)
{
	PTCPCLIENT_INFO pTCPClntInfo = (PTCPCLIENT_INFO)lparam;
	CTcpClientMgrBase* pTCPClnt;
	int ConnectNum = 0, TimeCnt = 0;
	fd_set		fdRead;
	int				ret=0;
	struct timeval	aTime;

	pTCPClnt = (CTcpClientMgrBase*)pTCPClntInfo->pTCPClient;

	while (1)
	{
		if (pTCPClntInfo->m_ExitFlag==1)
		{
			pTCPClntInfo->m_ExitFlag = 0;
			if (pTCPClntInfo->ConnState > csINIT)
			{
				pTCPClnt->TcpClientBaseUnit::Close();			
				//memset((char*)pTCPClntInfo, 0, sizeof(TCPCLIENT_INFO));
			}
			g_tcpClientMgr.deleted(pTCPClntInfo->nIdx);
			ret = -1;
			break;
		}
		//if (pTCPClntInfo->ConnState == csWAIT)
		//{
		//	chrono::milliseconds delayms(10);
		//	this_thread::sleep_for(delayms);
		//	continue;
		//}
		aTime.tv_sec = 0;
		aTime.tv_usec = 50000; // 100ms
		FD_ZERO(&fdRead);
		FD_SET(0, &fdRead);
		ret = select(0, &fdRead, NULL, NULL, &aTime);
		if (pTCPClntInfo->belock != 1)
		{
			pTCPClntInfo->m_ExitFlag = 1;
			continue;
		}
		switch (pTCPClntInfo->ConnState)
		{
		case csWAIT:
			pTCPClnt->m_writeable = false;
			pTCPClntInfo->ConnState = csINIT;
			break;
		case csINIT:
			pTCPClnt->m_writeable = false;
			pTCPClnt->Open(pTCPClntInfo->HostIP, pTCPClntInfo->HostPort);
			pTCPClntInfo->ConnState = csCLOSED;
			printf("TCPClnt Connect...\r\n");
			break;
		case csCLOSED:
			pTCPClnt->m_writeable = false;
			if (pTCPClnt->Connect())
			{
				pTCPClntInfo->ConnState = csOPEN;
				printf("TCPClnt Opened\r\n");
				ConnectNum = 0;
				TimeCnt = 0;
			}
			else
			{
				ConnectNum++;
			}
			if (ConnectNum > 3)	// 
			{
				pTCPClnt->Close();
				//return 0;
			}
			break;
		case csOPEN:
			if (pTCPClnt->m_sockclose)
			{
				pTCPClnt->m_writeable = false;
				pTCPClntInfo->ConnState = csWAIT;
			}
			else
			{
				pTCPClnt->m_writeable = true;
				TimeCnt++;
			}
			
			break;
		}
	}
	return ret;
}
