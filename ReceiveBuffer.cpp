#include "stdafx.h"
#include <list>
#include <time.h>
#include <chrono>
#include <thread>
#include "ReceiveBuffer.h"
using namespace std;
using namespace chrono;
long GetCurrentTimeByMS()
{
	auto nowtime = chrono::system_clock::now();
	auto nowtimems = duration_cast<milliseconds>(nowtime.time_since_epoch());
	//milliseconds nowtimems(nowtime.time_since_epoch);

	return long(nowtimems.count());
}
long GetCurrentTimeBySec()
{
	//long nowtime = time(NULL);
	//return nowtime;
	auto nowtime = chrono::system_clock::now();
	auto nowtimesec = duration_cast<seconds>(nowtime.time_since_epoch());
	return long(nowtimesec.count());
}
ReceiveBuffer::ReceiveBuffer()
{
	Clear();
	timeout = 2;
	beWriting = false;
}


ReceiveBuffer::~ReceiveBuffer()
{
}
void ReceiveBuffer::ClearTimeOutPackage()
{
	long currTime = GetCurrentTimeBySec();
	list_RecvData::iterator data_it;
	for (data_it = bufferList.begin(); data_it != bufferList.end();)
	{
		if ((currTime - data_it->rcvTime) > timeout)
		{
			data_it = bufferList.erase(data_it);
		}
		else data_it++;
	}
}
int ReceiveBuffer::getAnnuniatorUploadDataByCommand(char command, char* buffer, int len, int timeout)
{
	int ret = 0;
	printf("find command from buffer\r\n");
	long curSec = GetCurrentTimeBySec();
	while ((curSec + timeout) > GetCurrentTimeBySec())
	{
		ret = getAnnuniatorUploadDataByCommand(command, buffer, len);
		if (ret > 0)
		{
			return ret;
		}
		milliseconds delayms(10);
		this_thread::sleep_for(delayms);
	}
	printf("buffer check end\r\n");
	return 0;
}
int ReceiveBuffer::getAnnuniatorUploadDataByCommandWithEnder(char command, char* buffer, int len, int mode, int timeout)
{
	int ret = 0;
	int length = 0;
	printf("find command from buffer\r\n");
	long curSec = GetCurrentTimeBySec();
	while ((curSec + timeout) > GetCurrentTimeBySec())
	{
		ret = getAnnuniatorUploadDataByCommand(command, buffer+length, len-length);
		if (ret > 0)
		{
			length += ret;
			if (checkEndMode(mode, buffer, length) == 1)
			{
				ret = length;
				return ret;
			}
		}
		milliseconds delayms(2);
		this_thread::sleep_for(delayms);
	}
	printf("buffer check end\r\n");
	return 0;
}
int ReceiveBuffer::checkEndMode(int mode, char* buffer, int length)
{
	if (mode == SB4U_MODE)
	{
		if (((unsigned char)buffer[length - 2] == ENDER1_MODE1) && ((unsigned char)buffer[length - 1] == ENDER2_MODE1))return 1;
	}
	return 0;
}
int ReceiveBuffer::getAnnuniatorUploadDataByCommand(char command, char* buffer, int len)
{
	int ret = 0;
	if (beWriting == true) return 0;
	ClearTimeOutPackage();
	memset(buffer, 0, len);
	list_RecvData::iterator data_it;
	for (data_it = bufferList.begin(); data_it != bufferList.end();)
	{
		if (data_it->rcvData[0] == command)
		{
			char debugdata[100];
			memset(debugdata, 0, sizeof(debugdata));
			//sprintf(debugdata, "found data:%d-%d\r\n", data_it->length, len);
			//OutputDebugStringA(debugdata);
			if (data_it->length > len)
			{
				ret = 0;
				break;
			}
			ret = data_it->length;
			memcpy(buffer, data_it->rcvData,  data_it->length);
			data_it = bufferList.erase(data_it);
			break;
		}
		else data_it++;
	}
	return ret;
}
void ReceiveBuffer::setAnnuniatorUploadData(char* data, int len)
{
	if (len > MAXLENGTH_SB4U_CMD) return;
	beWriting = true;
	ReceivedPacket newdata;
	memset(newdata.rcvData, 0, sizeof(newdata.rcvData));
	memcpy(newdata.rcvData, data, len);
	newdata.length = len;
	newdata.rcvTime = GetCurrentTimeBySec();
	bufferList.push_front(newdata);
	ClearTimeOutPackage();
	beWriting = false;
}
void ReceiveBuffer::Clear()
{
	list_RecvData::iterator data_it;
	for (data_it = bufferList.begin(); data_it != bufferList.end();)
	{
		data_it = bufferList.erase(data_it);
	}
}
