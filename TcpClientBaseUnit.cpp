#include "stdafx.h"
#include "unistd.h"	// unix system library, should be defined yourself in windows
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include "ReceiveBuffer.h"
#include "TcpClientBaseUnit.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

TcpClientBaseUnit::TcpClientBaseUnit()
{
	m_ExitThreadFlag = 0;
	m_nRecvLen = 0;
	m_sockclose = 0;
	m_writeable = false;
	m_bDebugOutput = true;
	m_port = 0;
	m_sockfd = 0;
	//m_thread = NULL;
	memset(m_remoteHost, 0, sizeof(m_remoteHost));
}


TcpClientBaseUnit::~TcpClientBaseUnit()
{
	//if (m_thread != NULL)
	//{
	//	delete m_thread;
	//	m_thread = NULL;
	//}
}
int TcpClientBaseUnit::SocketThreadFunc(void* lparam)
{
	TcpClientBaseUnit *pSocket;
	pSocket = (TcpClientBaseUnit*)lparam;
	int beZero = 3;
	fd_set fdRead;
	int ret;
	struct timeval aTime;
	while (1)
	{
		if (pSocket->m_ExitThreadFlag)
		{
			printf("Tcp Client Quit\r\n");
			break;
		}
		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_sockfd, &fdRead);
		aTime.tv_sec = 0;
		aTime.tv_usec = 50000;
		ret = select(pSocket->m_sockfd + 1, &fdRead, NULL, NULL, &aTime);
		if (ret < 0)
		{
			printf("tcp Client select=%d\r\n", ret);
			pSocket->Close();
			//_close(pSocket->m_sockfd);
			//pSocket->m_sockclose = 1;
			break;
		}
		if (ret > 0)
		{
			if (FD_ISSET(pSocket->m_sockfd, &fdRead))
			{
				memset(pSocket->RecvBuf, 0, sizeof(pSocket->RecvBuf));
				pSocket->m_nRecvLen = recv(pSocket->m_sockfd, pSocket->RecvBuf, sizeof(pSocket->RecvBuf), 0);
				if (pSocket->m_nRecvLen < 0)
				{
					printf("Tcp Client Socket Error\r\n");
					pSocket->Close();
					//_close(pSocket->m_sockfd);
					//pSocket->m_sockclose = 1;
					break;
				}
				else if (pSocket->m_nRecvLen == 0)
				{
					beZero--;
					//if (beZero == 0)
					{
						printf("Ckise socket %d\r\n", pSocket->m_sockfd);
						pSocket->Close();
					}
					break;
				}
				else
				{
					beZero = 3;
					pSocket->RecvBuf[pSocket->m_nRecvLen] = 0;
					pSocket->PackagePro(pSocket->RecvBuf, pSocket->m_nRecvLen);
				}
			}
		}
	}
	printf("Tcp Client Socket ThreadFun finished\r\n");
	return 0;
}
int TcpClientBaseUnit::Open(char* ServerIP, int ServerPort)	// windows version
{
	WORD version = MAKEWORD(2, 2);
	WSADATA lpData;
	int intEr = WSAStartup(version, &lpData);	// set the winsock version, and initialized it
	if (intEr != 0)
	{
		printf("Winsock init failed\r\n");
		return 0;
	}
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sockfd == INVALID_SOCKET)
	{
		printf("Tcp Client Socket fail\r\n");
		return -1;
	}
	printf("Tcp Client socket success! socketfd=%d\r\n", m_sockfd);
	strcpy(m_remoteHost, ServerIP);
	m_port = ServerPort;
	m_ExitThreadFlag = 0;
	return 1;
}
//int TcpClientBaseUnit::Open(char* ServerIP, int ServerPort)
//{
//	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
//	if (m_sockfd == -1)
//	{
//		printf("Tcp Client Socket fail\r\n");
//		return -1;
//	}
//	printf("Tcp Client socket success! socketfd=%d\r\n", m_sockfd);
//	strcpy(m_remoteHost, ServerIP);
//	m_port = ServerPort;
//	m_ExitThreadFlag = 0;
//	return 1;
//}
int TcpClientBaseUnit::Close()
{
	if (m_sockclose == 1) return 1;
	m_ExitThreadFlag = 1;
	m_sockclose = 1;
	int err = closesocket(m_sockfd);
	if (err < 0)
	{
		return 0;
	}
	WSACleanup();
	return 1;
}
int TcpClientBaseUnit::Connect()
{
	struct sockaddr_in addr;
	int ret;
	struct timeval timeVal;
	fd_set		fdSet;
	int flag;
	unsigned long iMode = 1;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	//addr.sin_addr.S_un.S_addr = inet_addr(m_remoteHost);

	in_addr inaddr;
	inet_pton(AF_INET, m_remoteHost, (void*)&inaddr);
	//inet_pton(AF_INET, "127.0.0.1", (void*)&inaddr);
	addr.sin_addr = inaddr;
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		Close();
		return 0;
	}

	// Connect
	ret = connect(m_sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		printf("Tcp Client Connect failed\r\n");
		return 0;
	}
	// make socket non-blocking
	flag = ioctlsocket(m_sockfd, FIONBIO, &iMode);
	if (flag != NO_ERROR)
	{
		printf("Tcp Client Set Async failed");
	}
	m_sockclose = 0;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FD_ZERO(&fdSet);
	FD_SET(m_sockfd, &fdSet);
	ret = select((int)m_sockfd + 1, NULL, &fdSet, NULL, &timeVal);
	if (ret == 0)
	{
		printf("Tcp Client Connect select timeout with result=%d\r\n", ret);
		return 0;
	}
	if (ret != 1)
	{
		printf("Tcp Client Connect select failed with result=%d\r\n", ret);
		return 0;
	}
	thread t(SocketThreadFunc, (void*)this);
	t.detach();
	return 1;
}
int TcpClientBaseUnit::SendData(char * buf, int len)
{
	int nBytes = 0;
	int nSendBytes = 0;
	if (m_writeable == false)
	{
		printf("Tcp Client no ready for writing\r\n");
		return 0;
	}
	//if (m_bDebugOutput)
	//{
	//	printf("TcpClient s:");
	//	DebugInfo(buf, len, false);
	//}
	while (nSendBytes < len)
	{
		nBytes = send(m_sockfd, buf + nSendBytes, len - nSendBytes, 0);
		if (nBytes < 0)
		{
			return nBytes;
		}
		nSendBytes = nSendBytes + nBytes;
		if (nSendBytes < len)
		{
			chrono::milliseconds	delayms(10);
			this_thread::sleep_for(delayms);
		}
	}
	return 1;
}
int TcpClientBaseUnit::PackagePro(char* buf, int len)
{
	if (m_bDebugOutput)
	{
		DebugInfo(buf, len, false);
	}
	return 0;
}
int TcpClientBaseUnit::PackagePro(char* buf, int len, bool beAscii)
{
	if (m_bDebugOutput)
	{
		DebugInfo(buf, len, beAscii);
	}
	return 0;
}
void TcpClientBaseUnit::DebugInfo(char* buf, int len, bool beAscii)
{
	if (beAscii)printf("%s\r\n", buf);
	else
	{
		for (int i = 0; i < len; i++)
		{
			printf("%02X-", buf[i]);
		}
		if (len > 0)printf("TCP Client\r\n");
	}
}
