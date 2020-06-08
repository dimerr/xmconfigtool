#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include "netsdk.h"

using namespace std;

long g_LoginID=0;
char hostname[64]="";
char username[NET_NAME_PASSWORD_LEN]="";
char password[NET_NAME_PASSWORD_LEN]="";
char action[64];
char param1[32];
char param2[32];
int error=0;
DWORD dwRetLen = 0;
int nWaitTime = 10000;

bool isValidIpAddress(char *ipAddress)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
	return result != 0;
}

int  main(int argc, char* argv[])
{
	if(argc<4) {
		printf("\nUsage: %s [IP|CloudID] [username] [password] [action]\n Possible actions: debugcamera,format,password,localsearch\n\n",argv[0]);
		return 0;
	}

	strcpy (hostname, argv[1]);
	strcpy (username, argv[2]);
	strcpy (password, argv[3]);
	sprintf (action, argv[4]);
	sprintf (param1, argv[5]);
	sprintf (param2, argv[6]);

	H264_DVR_Init(NULL,NULL);
	H264_DVR_SetConnectTime(3000,1);

	//printf("Hostname: %s, Username: %s, Password: \"%s\"\n", hostname,username,password);

	H264_DVR_DEVICEINFO lpDeviceInfo;
	memset(&lpDeviceInfo,0,sizeof(lpDeviceInfo));
	if (isValidIpAddress(hostname)) {
		g_LoginID = H264_DVR_Login((char*)hostname, 34567, (char*)username,(char*)password,(LPH264_DVR_DEVICEINFO)(&lpDeviceInfo),&error);
	}
	else{
		g_LoginID = H264_DVR_Login_Cloud((char*)hostname, 34567, (char*)username,(char*)password,(LPH264_DVR_DEVICEINFO)(&lpDeviceInfo),&error,NULL);
	}
	printf("g_LoginID=%lui,nError:%d\n",g_LoginID,error);

	if ( g_LoginID <= 0 )
	{
		string strErr;
		switch (error)
		{
			case H264_DVR_PASSWORD_NOT_VALID:
			strErr = printf("Error.PwdErr");
			break;
			case H264_DVR_NOPOWER:
			strErr = printf("Error.NoPower");
			break;
			case H264_DVR_LOGIN_USER_NOEXIST:
			strErr = printf("Error.UserNotExisted");
			break;
			case H264_DVR_USER_LOCKED:
			strErr = printf("Login.Locked");
			break;
			case H264_DVR_USER_IN_BLACKLIST:
			strErr = printf("Error.InBlackList");
			break;
			case H264_DVR_USER_HAS_USED:
			strErr = printf("Error.HasLogined");
			break;
			case H264_DVR_CONNECT_DEVICE_ERROR:
			strErr = printf("Error.NotFound");
			break;
			case H264_DVR_CLOUD_LOGIN_ERR:
			strErr = printf("Log.Error") + lpDeviceInfo.sCloudErrCode;
			break;
			default:
			{
				char ch[10];
				strErr = printf("Log.Error") + ch;
			}
			break;
		}
		printf("error?");
	}

	BOOL bSuccess = H264_DVR_GetDevConfig(g_LoginID,E_SDK_CONFIG_SYSINFO,0,(char *)&lpDeviceInfo,sizeof(H264_DVR_DEVICEINFO),&dwRetLen,nWaitTime);
	//printf("bSuccess=%i,RetLen:%lui\n",bSuccess,dwRetLen);
	if (bSuccess && dwRetLen == sizeof (H264_DVR_DEVICEINFO))
	{
		printf("Hardware    : %s\n",lpDeviceInfo.sHardWare);
		printf("Software    : %s\n",lpDeviceInfo.sSoftWareVersion);
		printf("SerialNumber: %s\n",lpDeviceInfo.sSerialNumber);
	}

	if (!strcmp(action,"debugcamera")) {
		printf("DebugCamera\n");
	}
	if (!strcmp(action,"format")) {
		printf("Formatting disk...");
		SDK_StorageDeviceControl pStorageCtl;
		memset(&pStorageCtl,0,sizeof(pStorageCtl));
		pStorageCtl.iAction = SDK_STORAGE_DEVICE_CONTROL_CLEAR;
		pStorageCtl.iPartNo = 0;
		pStorageCtl.iSerialNo = 0;
		pStorageCtl.iType = SDK_STORAGE_DEVICE_CLEAR_DATA;
		int nRet = H264_DVR_SetDevConfig(g_LoginID,E_SDK_CONFIG_DISK_MANAGER,0,(char *)&pStorageCtl,sizeof(pStorageCtl),nWaitTime);
		if (nRet >=0 ){
			printf("success");
		}
	}
	if (!strcmp(action,"password")) {
		printf("Change password...");
		char szMD5[100] = {0};
		//H264_DVR_Encryptsword((char *)szMD5, param1);

		_CONF_MODIFY_PSW psw;
		strcpy (psw.sUserName, "admin");
		strcpy (psw.Password, password);
		strcpy (psw.NewPassword, (char *)szMD5);
		int nRet = H264_DVR_SetDevConfig(g_LoginID, E_SDK_CONFIG_MODIFY_PSW, 0, (char *)&psw, sizeof(_CONF_MODIFY_PSW));
		if (nRet >=0 ){
			printf("success");
		}
	}

	if (!strcmp(action,"localsearch")) {
		printf("Searching...");
		unsigned long lRetLen=0;
		SDK_NetDevList m_Devlist;
		memset( &m_Devlist,0,sizeof(m_Devlist));
		int nRet = H264_DVR_GetDevConfig(g_LoginID, E_SDK_CFG_NET_LOCALSEARCH, -1, (char*)&m_Devlist,sizeof(m_Devlist),&lRetLen,15000);
		if (nRet >=0 ){
			for (int i= 0; i<m_Devlist.vNetDevNum;i++)
			{
				printf ("Found: %i %d.%d.%d.%d %s %s \n",i,\
				  m_Devlist.vNetDevList[i].HostIP.c[0], \
					m_Devlist.vNetDevList[i].HostIP.c[1], \
				  m_Devlist.vNetDevList[i].HostIP.c[2], \
					m_Devlist.vNetDevList[i].HostIP.c[3], \
					m_Devlist.vNetDevList[i].sMac, \
					m_Devlist.vNetDevList[i].sSn);
			}
		}
	}

	if(g_LoginID>0){
		printf("\nLogin ok\n");
	}
	else {
		printf("Error\n");
	}

	if(g_LoginID>0)
	{
		H264_DVR_Logout(g_LoginID);
		printf("Logout...\n");
	}
	H264_DVR_Cleanup();

	return 0;
}
