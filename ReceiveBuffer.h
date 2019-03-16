#pragma once
#include <list>
#define MAXLENGTH_SB4U_CMD	200
#define ENDER1_MODE1	0xAA
#define ENDER2_MODE1	0x55
#define SB4U_MODE		1
struct ReceivedPacket
{
	long rcvTime;
	short length;
	char rcvData[MAXLENGTH_SB4U_CMD];
};
typedef std::list<ReceivedPacket> list_RecvData;
class ReceiveBuffer		// Data structure for Annuniator info
{
public:
	ReceiveBuffer();
	~ReceiveBuffer();
public:
	int getAnnuniatorUploadDataByCommand(char command, char* buffer, int len);
	int getAnnuniatorUploadDataByCommand(char command, char* buffer, int len, int timeout);
	int getAnnuniatorUploadDataByCommandWithEnder(char command, char* buffer, int len, int mode, int timeout);
	void setAnnuniatorUploadData(char* data, int len);
	void Clear();
	void ClearTimeOutPackage();
protected:
	int checkEndMode(int mode, char* buffer, int length);
protected:
	list_RecvData bufferList;
	int timeout;
	bool beWriting;
};

