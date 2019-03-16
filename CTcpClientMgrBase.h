#pragma once
#include "ReceiveBuffer.h"
#include "TcpClientBaseUnit.h"
class CTcpClientMgrBase : public TcpClientBaseUnit
{
public:
	CTcpClientMgrBase();
	~CTcpClientMgrBase();

	int getRetBufferByCommand(char command, char* buffer, int len, int timeout);
	int getRetBufferByCommandEnder(char command, char* buffer, int len, int timeout);
	virtual int PackagePro(char* buf, int len);
	virtual int SendData(char* buf, int len);
	int SendDataByMode(char* buf, int len, int md);
	void setMode(int m) { mode = m; }
protected:
	void DebugInfo(char* buf, int len, bool beAscii);
private:
	ReceiveBuffer m_rcvBuffer;
	int mode;
};

