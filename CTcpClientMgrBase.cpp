#include "stdafx.h"
#include "ReceiveBuffer.h"
#include "CTcpClientMgrBase.h"


CTcpClientMgrBase::CTcpClientMgrBase()
	:TcpClientBaseUnit()
{
	mode = 0;
}


CTcpClientMgrBase::~CTcpClientMgrBase()
{
}

int CTcpClientMgrBase::getRetBufferByCommand(char command, char* buffer, int len, int timeout)
{
	if (mode == SERVERTYPE_4U_ANNUNIATOR)return m_rcvBuffer.getAnnuniatorUploadDataByCommand(command, buffer, len, timeout);
	else if (mode == SERVERTYPE_HIS_DEVICES)return m_rcvBuffer.getAnnuniatorUploadDataByCommand(command, buffer, len, timeout);
	return 0;
}
int CTcpClientMgrBase::getRetBufferByCommandEnder(char command, char* buffer, int len, int timeout)
{
	if (mode == SERVERTYPE_4U_ANNUNIATOR)return m_rcvBuffer.getAnnuniatorUploadDataByCommandWithEnder(command, buffer, len, SB4U_MODE, timeout);
	return 0;
}
int CTcpClientMgrBase::PackagePro(char* buf, int len)
{
	//TcpClientBaseUnit::PackagePro(buf, len);
	if (mode == SERVERTYPE_4U_ANNUNIATOR) { DebugInfo(buf, len, false); m_rcvBuffer.setAnnuniatorUploadData(buf, len); }
	else if (mode == SERVERTYPE_HIS_DEVICES) { DebugInfo(buf, len, true); m_rcvBuffer.setAnnuniatorUploadData(buf, len); }
	else DebugInfo(buf, len, false);
	return 0;
}
int CTcpClientMgrBase::SendData(char* buf, int len)
{
	if (mode == SERVERTYPE_4U_ANNUNIATOR) { DebugInfo(buf, len, false); }
	else if (mode == SERVERTYPE_HIS_DEVICES) { DebugInfo(buf, len, true); }
	else DebugInfo(buf, len, false);
	TcpClientBaseUnit::SendData(buf, len);
	return 0;
}
int CTcpClientMgrBase::SendDataByMode(char* buf, int len, int md)
{
	if (md == mode)
	{
		if (md == SERVERTYPE_4U_ANNUNIATOR) { DebugInfo(buf, len, false); }
		else if (md == SERVERTYPE_HIS_DEVICES) { DebugInfo(buf, len, true); }
		else DebugInfo(buf, len, false);
		SendData(buf, len);
	}
	return 0;
}
void CTcpClientMgrBase::DebugInfo(char* buf, int len, bool beAscii)
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