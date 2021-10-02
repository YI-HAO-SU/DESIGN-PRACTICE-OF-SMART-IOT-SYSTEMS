// MQTTClientS.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include "pch.h"
#include <iostream>

#include <conio.h>

#include <process.h>
#include <time.h>
#include "typeFuns.h"
#include "dbfile.h"
#include "MsgQ.h"

using namespace std;
#include <WINSOCK2.H>
#pragma comment(lib, "wsock32.lib")

// Set current cursor position.
#include <TimeAPI.h>
#pragma comment(lib, "Winmm.lib")
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename ("EOF", "adoEOF")
#include <Windows.h>

void str_split(char * buf, char *s1, char * s2);

#define			ServerPort					4000	//port
#define			MaxBuf						64		//每次接受或傳送的緩衝大小
#define			configuration				"configS"

WSADATA			WsaData;
char			BufSend[MaxBuf];					//放置要傳送的訊息

SOCKET			ServerSocket;						//Server的socket
int				ExitServer;							//控制是否離開						

SOCKADDR_IN		AddrServer;							//Server的SOCKADDR_IN結構


void			ExitClient(void);					//關閉所有的socket和thread
int			connectServer(void);				//傳送訊息給Server
int testFlag = 0;
long cur = 0, last = timeGetTime();
char funstate[50];
struct Message_Out {
	char atrName[100];
	char vtype;
	union {
		char * s;
		int i;
		BOOL b;
	}value;
};
int make_publish_msg(struct Message_Out * msgTBL, char *msg);
////////////////////////////////////////////////////////////////////////////////////////////////////////
extern FunTBL messageFun[];
//char devNameIn[2][100], devNameOut[2][100], cnID[100], devName[2][100];
char cnID[100];
char **devNameIn, **devNameOut, **devName, *CnState;
int devN = 0;
int readConfig(void);
//===========================================================================================
void ReleaseTAS1(void);
void ReleaseTAS(void);
int getCTimeSec(char * cctime);
void WerFault_check(void*);
int main(int argc, char* argv[])																	  //
{
	char cctime[20];
	int rflag = 1;

	timeBeginPeriod(1);
	getCTimeSec(cctime);

	sprintf_s(cnID, 100, "nsysuele_MPD%s", cctime);
	devN=readConfig();
	if (devN == 0) return 0;
	//-----------------------------------------------初始化-----------------------------------------------//
//	_beginthread(WerFault_check, 0, (void*)"*");//
	initMQ();

	while (rflag) {
		if (0 != WSAStartup(MAKEWORD(1, 1), &WsaData)) {												  //																								
			break;																				  //
		}
		rflag = connectServer();
		Sleep(100);
		WSACleanup();
	}//
//------------------------------------------------結束------------------------------------------------//
	ExitClient();																					  //
																		  //
	return 0;																						  //
}																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void	ExitClient()																				  //
{																									  //
	cout << "System exiting..." << endl;															  //
	WSACleanup();																					  //																				  //
}																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
int readConfig(void) 
{
	FILE *fp;

	char buf[100];
	int i, j,n=0;

	if (fileExist((char *)configuration)) {
		fopen_s(&fp,configuration, "r");
		if (fp != NULL) {
			for (j = 0;fgets(buf, 80, fp) > 0;j++);
			fclose(fp);n = j;
		}
		if (n == 0) {
			exit(0);
		}
		else {
			devName = (char **)malloc(n * sizeof(char *));
			devNameIn = (char **)malloc(n * sizeof(char *));
			devNameOut = (char **)malloc(n * sizeof(char *));
			CnState= (char *)malloc(n * sizeof(char));
			for (i = 0;i < n;i++) {
				devName[i] = (char *)malloc(100 * sizeof(char ));
				devNameIn[i] = (char *)malloc(100 * sizeof(char ));
				devNameOut[i] = (char *)malloc(100 * sizeof(char ));
				devName[i][0] = devNameIn[i][0] = devNameOut[i][0] = 0x00;
				CnState[i] = 0;
			}
		}
		fopen_s(&fp, configuration, "r");
		if (fp != NULL) {
			for (j = 0;fgets(buf, 99, fp) > 0;j++) {
				for (i = 0;buf[i] && buf[i] != 0x0D && buf[i] != 0x0A;i++)
					devName[j][i] = buf[i];
				devName[j][i] = 0x00;
				sprintf_s(devNameIn[j], 99, "nsysu_ele/%s/in", devName[j]);
				sprintf_s(devNameOut[j], 99, "nsysu_ele/%s/out", devName[j]);
			}
			fclose(fp);
		}
	}
	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Kgetline(char *buf, int n);
void PingReq(void *p);
void DataRecv(void *p);
void CmdCheck(void *p);
void ActiveCheck(void *p);
void processMsg(void * p);
int recflag = 1, pingflag = 1;
int connectServer(void)																			  // 
{

	int SPort = 1883;
	char * p;
	int rVal = 0, n,i,id,j;
	//-----------------------------------------建立Server的SCOKET-----------------------------------------//
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                         //
	if (INVALID_SOCKET == ServerSocket) {																  //																											 
		ExitClient();																				  //
	}																								  //
//-----------------------------------------------CONNECT----------------------------------------------//		
	AddrServer.sin_family = AF_INET;																  //
	char ServerIP[20];																				  //
	funstate[0] = 0x00;
	strcpy_s(ServerIP, "140.117.156.44");
//	strcpy_s(ServerIP, "192.168.1.55");
//	strcpy_s(ServerIP, "127.0.0.1");
	//
	AddrServer.sin_addr.s_addr = inet_addr(ServerIP);												  //
	AddrServer.sin_port = htons(SPort);														  //
	if (SOCKET_ERROR == connect(ServerSocket, (LPSOCKADDR)&AddrServer, sizeof(AddrServer))) {			  //	
		cout << "Connection Fail!!!\n";
		ExitClient();	
		return 1;//
	}																								  //
	cout << "Connect successfully!" << endl;

	ReleaseTAS1();
	ReleaseTAS();

	recflag = 1;

	_beginthread(processMsg, 0, (void*)"*");
	_beginthread(DataRecv, 0, (void*)"*");		//
//========================================================================	
	n = send_CONNECT((char *)"MQTT", cnID, (char *)"PM_System", (char *)"mpd@8010a_ESP8266");	//註冊封包
	printf("\n=========1=[%d]==Send Connection=====\n", n);
	while (strcmp(funstate, (char *)"CONNACK") != 0) { Sleep(1); }																	//空轉會爆RAM被吃光
	printf("=========2=[%d]====CONNACK===========\n", n);
	//getchar();
	funstate[0] = 0x00;
	for (i = 0;i < devN;i++) {
		n = send_SUBSCRIBE(devNameIn[i]);
		printf("=========3.1=[%d][%s]===============\n", i, devNameIn[i]);

		while (strcmp(funstate, (char *)"SUBACK") != 0) {
			Sleep(1);
		}
		Sleep(10);
	}
	printf("<%s>", devNameIn[0]);
	printf("=========4=[%d]===============\n", n);
	testFlag = 0;											 //可以看到回來的封包
	send_PUBLISH(devNameIn[0], 1, (char*)"Hello");
	getchar();
	pingflag = 1;
	_beginthread(PingReq, 0, (void*)"*");		//
	_beginthread(CmdCheck, 0, (void*)"*");		//
	_beginthread(ActiveCheck, 0, (void*)"*");		//

//------------------------------------------傳送給Server訊息------------------------------------------// 		 
	ExitServer = 0;BufSend[0] = 0x00;
	cout << ">>";
	while (!ExitServer) {
		rVal = 1;
		if (Kgetline(BufSend, MaxBuf)) {
			
			id = -1;
			p = strstr(BufSend, " ");
			if (p != NULL) {
				*p = 0x00;
				for (i = 0;i < devN;i++) {
					if (strcmp(BufSend, devName[i]) == 0) {
						id = i;
						p++;
						for (j = 0;p[j];j++)
							BufSend[j] = p[j];
						BufSend[j] = 0x00;
						break;
					}
				}
			}
			if (id == -1)
				p = NULL;
			else
				p = devNameOut[id];

			if ((rVal = cmdInterpreter(p,BufSend)) >= 0) {
				break;
			}
			BufSend[0] = 0x00;
			cout << ">>";
		}
		else {
			Sleep(1);
		}
		if (pingflag == 0 || recflag == 0) {
			rVal = 1;
			break;
		}
		//		if(strcmp(funstate, "PUBREL") == 0) {
		//			rVal = 1;
		//			break;
		//		}
				//
	}																								  //
//------------------------------------------------結束------------------------------------------------//
	recflag = 0;
	pingflag = 0;
	closesocket(ServerSocket);
	ExitServer = 1;
//	printf("\nExit Connect!![%s]!\n", devNameIn);																	  //																		  //
	Sleep(5 * 1000);																				  //
	return rVal;																					  //
}
int cmdInterpreter(char *devNOut,char *BufSend)
{
	static int msgid = 4;
	char msg[200], cctime[40];
	enum atrName { SWITCH = 0 };
	struct Message_Out msgTBL[] = {
		{"switch",'b',NULL},
		{"###",'#',NULL},
	};
	int rVal,i;
	char * tp;

	msg[0] = 0x00;
	rVal = -1;
	if (strcmp(BufSend, "SynT") ==0) {
		getLCTimeLog(cctime);
//		sprintf_s(msg, "{\"time\":\"%s\",\"current limit\":512,\"modbus moniter\":[{\"slave\":4,\"function\":4,\"start\":3,\"length\":1}]}", cctime);//2020/05/11/14/30/06
		sprintf_s(msg, "{\"time\":\"%s\",\"current limit\":512}", cctime);//2020/05/11/14/30/06
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
		}
		else {
			for (i = 0;i < devN;i++) {
				send_PUBLISH(devNameOut[i], msgid++, msg);
			}

		}
		
		sprintf_s(msg, "{\"modbus moniter\":[{\"slave\":4,\"function\":4,\"start\":3,\"length\":1}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
		}
		else {
			for (i = 0;i < devN;i++) {
				send_PUBLISH(devNameOut[i], msgid++, msg);
			}
		}
		rVal = -2;
	}
	if (strcmp(BufSend, "off") ==0) {
		msgTBL[SWITCH].value.b = false;
		make_publish_msg(msgTBL, msg);
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}
	}
	if (strcmp(BufSend, "on") == 0) {
		msgTBL[SWITCH].value.b = true;
		make_publish_msg(msgTBL, msg);
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}
	}

	if ((tp=strstr(BufSend, "SetHTemp")) != NULL) {
		//		msgTBL[SWITCH].value.b = true;
		//		make_publish_msg(msgTBL, msg);
		
		if ((tp = strstr(tp, " "))!=NULL) {
			for (i = 0;tp[i] != NULL && tp[i] == ' '; i++);

			sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":0,\"length\":1,\"in\":[%s]}]}", &(tp[i]));
			if (devNOut != NULL) {
				send_PUBLISH(devNOut, msgid++, msg);
				rVal = -2;
			}
		}
	}
	if (strcmp(BufSend, "AlarmOn") == 0) {
		//		msgTBL[SWITCH].value.b = true;
		//		make_publish_msg(msgTBL, msg);
/*
		sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":0,\"length\":1,\"in\":[100]}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}
*/
		sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":1,\"length\":1,\"in\":[0]}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}

		sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":16,\"length\":1,\"in\":[3]}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}
	}
	if (strcmp(BufSend, "AlarmOff") == 0) {
		//		msgTBL[SWITCH].value.b = true;
		//		make_publish_msg(msgTBL, msg);
		sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":0,\"length\":1,\"in\":[800]}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}
		sprintf_s(msg, "{\"modbus request\":[{\"slave\":4,\"function\":6,\"start\":16,\"length\":1,\"in\":[2]}]}");
		if (devNOut != NULL) {
			send_PUBLISH(devNOut, msgid++, msg);
			rVal = -2;
		}

	}

	if (strcmp(BufSend, "cls") == 0) {
		system("cls");
		rVal = -2;
	}
	if (strcmp(BufSend, "reconnect") == 0) {
		rVal = 1;
	}
	if (strcmp(BufSend, "exit") == 0) {
		rVal = 0;
	}
	if(devNOut!=NULL)printf("[%s][%s][%d]\n", devNOut, BufSend,rVal);
	else printf("[System][%s][%d]\n",  BufSend,rVal);
	return rVal;
}
int Kgetline(char *buf, int n)
{
	int i;
	char c;

	i = strlen(buf);
	if (i == n)return 1;
	if (_kbhit()) {
		c = _getch();
		switch (c) {
		case 0x08:
			if (i > 0) {
				buf[i - 1] = 0x00;
				putchar(0x08);putchar(' ');putchar(0x08);
			}
			break;
		case 0x0D:
			putchar('\n');
			return 1;
		default:
			buf[i] = c;
			putchar(c);
			buf[i + 1] = 0x00;
		}
	}

	return 0;
}
void CmdCheck(void *p)
{
	char   cmd[200];
	int  tcount = 0, count = 0, rVal,i=0,d;
	string ts;

	d = (1000 / devN);
	while (recflag) {
		Sleep(d);
		if (tcount >= 1) {
			rVal = -1;
			cmd[0] = 0x00;

			getSWcmd(devName[i], cmd);

			if (cmd[0] != 0x00 && cmd[0] != ' ') {
				rVal = cmdInterpreter(devNameOut[i],cmd);
				if (rVal > 0) {
					ExitServer = 1;
					break;
				}
				else {
					writeSWcmd(devName[i], (char *)"");
				}
			}

			getClientcmd(devName[i], cmd);

			if (cmd[0] != 0x00 && cmd[0] != ' ') {
				rVal = cmdInterpreter(devNameOut[i], cmd);
				if (rVal > 0) {
					ExitServer = 1;
					break;
				}
				else {
					writeClientcmd(devName[i], (char *)"");
				}
			}
			i = (i + 1) % devN;
			//			getLocalTimeLog(cctime);
			//			sprintf_s(buf, 100, "swCmd[%s]<%s:%s>[%d]                       ", cctime, devName, cmd, rVal);
			//			putsXY(buf, 60, 9);
			tcount = 0;
		}
		else {
			tcount++;
		}

	}
	pingflag = 0;
}
void ActiveCheck(void *p)
{
	char cctime[100], buf[300], *s1, *s2, *s3, *s4, state[20], cmd[20];
	int i,n, d, WeekDay, tcount = 0, count = 0, rVal, tflag = 0,id;
	string ts;

	string s;
	id = 0;d = (20000 / devN);
	while (recflag) {
		Sleep(d);

		rVal = -1;
		WeekDay = getCTimeLog(cctime);
		getActiveSch(devName[id], WeekDay, &ts);

		istringstream iss(ts);
		n = 0;tflag = -1;
		getSWState(devName[id], state);
		while (getline(iss, s, '|')) {

			strcpy_s(buf, 300, s.c_str());
			for (i = 0;buf[i] && buf[i] != '/';i++);
			s1 = buf; s2 = (buf[i]) ? (buf[i] = 0x00, &(buf[++i])) : &(buf[i]);
			for (;buf[i] && buf[i] != '/';i++);
			s3 = (buf[i]) ? (buf[i] = 0x00, &(buf[++i])) : &(buf[i]);
			for (;buf[i] && buf[i] != '/';i++);
			s4 = (buf[i]) ? (buf[i] = 0x00, &(buf[++i])) : &(buf[i]);
			sscanf_s(s3, "%d", &rVal);
			
			if (rVal > 0) {
				if (strcmp(cctime, s1) >= 0 && strcmp(cctime, s2) <= 0) {
					tflag = 1;
				}
				else {
					if (s4[0] == 'T') {
						delActiveSch(devName[id], WeekDay, s1, (char *)"T");
					}
					tflag = (tflag == 1) ? 1 : 0;
				}
			}

			if (rVal < 0) {
				if ((strcmp(cctime, s1) >= 0 && strcmp(cctime, "24:00") <= 0) ||
					(strcmp(cctime, "00:00") >= 0 && strcmp(cctime, s2) <= 0)) {
					tflag = 1;
				}
				else {
					if (s4[0] == 'T') {
						delActiveSch(devName[id], WeekDay, s1, (char *) "T");
					}
					tflag = (tflag == 1) ? 1 : 0;
				}
			}
		}
		if (tflag != -1) {
			cmd[0] = 0x00;
			if (strcmp(state, "false") == 0 && tflag == 1) {
				strcpy_s(cmd, "on");
			}
			if (strcmp(state, "true") == 0 && tflag == 0) {
				strcpy_s(cmd, "off");
			}
			if (cmd[0] != 0x00)cmdInterpreter(devNameOut[id], cmd);
		}
		id = (id + 1) % devN;
	}

}
void PingReq(void *p)
{
	char ppp[70] = {0xC0U,0x00 }, cctime[100], buf[200], *pt;
	int n = 2, tcount = 0, count = 0,i;

	while (n > 0 && recflag) {
		Sleep(1000);
		if ((tcount & 0x0F) == 0) {
			for (i = 0;i < devN;i++) {
				CnState[i]++;
			}
		}
		if (tcount== 18) {
			buf[0] = 0x00;
			pt = buf;
			for (i = 0;i < devN;i++) {
				sprintf_s(pt, 15, "[%s:%d]", devName[i], CnState[i]);
				while (*pt)pt++;
			}
			putsXY(buf, 10, 0);
		}
		if (tcount >= 33) {
			count = 0;
			do {
				n = Mutex_send(ServerSocket, ppp, 2, 0);
				if (n <= 0) {
					Sleep(3);
					count++;
				}
			} while (count < 10 && n <= 0);

			//			getLocalTimeLog(cctime);
			//			sprintf_s(buf, 100, "[%s]<%d : %d>                       ", cctime, n,count);
			//			putsXY(buf, 60, 8);			
			tcount = 0;
		}
		else {
			tcount++;
		}

	}
	getLocalTimeLog(cctime);
	printf("\n\n=>>[%s]<%d : %d>Exit Ping!!", cctime, n,count);

	pingflag = 0;
}

void DataRecv(void *p)
{
	int i, n, pn, n1, count = 0;
	static struct Message * buf = NULL, buft;
	unsigned char msgfn, flag,fflag;
	char  cctime[50], s[100];
	union {
		int v;
		unsigned char B[4];
	}vn;

	do {
		vn.v = 0;
		fflag = 0;
//		buf = &buft;
		if(buf == NULL) {
			buf = getMsgBuf();
			if (buf == NULL) {
				buf = &buft;
			}
			else {
				fflag = 1;
			}
		}
		else {
			fflag = 1;
		}

		n1 = recv(ServerSocket, buf->data, 3, 0);
		if (recflag == 0)break;

		for (i = 0; i < n1; i++) {
			printf("%02X ", (unsigned char)((*buf).data[0]));
		}
		putchar('\n');

		flag = (unsigned char)((*buf).data[0]);
		msgfn = (unsigned char)((*buf).data[0]) >> 4;
		vn.B[0] = (unsigned char)((*buf).data[1]);

		pn = vn.v;

		//		printf("=>>[%02XH]",msgfn);
		if (((unsigned char)((*buf).data[1]) & 0x80) == 0x80) {
			vn.v <<= 1;
			vn.B[1] = (unsigned char)((*buf).data[2]);
			vn.v >>= 1;
			i = 0;
			pn = vn.v;
		}
		else {
			((*buf).data[0]) = ((*buf).data[2]);
			pn = (vn.v == 0) ? 0 : vn.v - 1;
			i = 1;
		}
		((*buf).data[i]) = 0x00;
		//		printf("pn=[%d],%d", pn,i);
		if (pn > 0) {
			n = recv(ServerSocket, &(buf->data[i]), pn, 0);
			buf->n = n + i;
		}
		else {
			buf->data[0] = 0x00;
			buf->n = 0;
		}

		if (flag != 0x00) {
			if (fflag && msgfn == 3) {
				buf->msgfn = msgfn;
				insertMsgQ(buf);
				buf = NULL;
			}
			else {
//				getLocalTimeLog(cctime);
//				sprintf_s(s, 100, "[%s][%d] ", cctime,msgfn);
//				putsXY(s, 60, 5);
				messageFun[msgfn].f((void *)(buf));
			}
		}

		if (n1 <= 0) {
			Sleep(1000);
			count++;
		}
		else {
			count = 0;
		}
	} while (n1 > 0 || count < 10);
	recflag = 0;

	printf("\n\n=>>[%02XH][%d][%d]", msgfn, n1, count);
	printf("Exit Data Reciever!!");
	ExitServer = 1;
	exit(0);
}//

void processMsg(void * p)
{
	struct Message * buf;
	char  cctime[50],s[100];

	do {
		if (!isMsgQEmpty()) {
			buf = delMsgQ();
//			getLocalTimeLog(cctime);
//			sprintf_s(s, 100, "[%s][%d] ", cctime, buf->msgfn);
//			putsXY(s, 60, 5);

			messageFun[buf->msgfn].f((void *)(buf));
			releaseMQBuf(buf);
		}
		else {
			Sleep(10);
		}
	} while (recflag);
}
//////////////////////////////////////////////

int make_publish_msg(struct Message_Out * msgTBL, char *msg)
{
	int  i = 0, k = 0;

	msg[i++] = '{';
	while (strcmp(msgTBL[k].atrName, "###") != 0)
	{
		if (i > 2)msg[i++] = ',';
		switch (msgTBL[k].vtype) {
		case 's':
			sprintf_s(&(msg[i]), 100, "\"%s\":\"%s\"", msgTBL[k].atrName, (msgTBL[k].value).s);
			break;
		case 'i':
			sprintf_s(&(msg[i]), 100, "\"%s\":%d", msgTBL[k].atrName, (msgTBL[k].value).i);
			break;
		case 'b':
			if ((msgTBL[k].value).b) {
				sprintf_s(&(msg[i]), 100, "\"%s\":%s", msgTBL[k].atrName, "true");
			}
			else {
				sprintf_s(&(msg[i]), 100, "\"%s\":%s", msgTBL[k].atrName, "false");
			}
			break;
		case 'M':
			break;
		}
		i = strlen(msg);
		k++;
	}
	msg[i++] = '}';msg[i++] = 0x00;
	return i;
}
//-----------------------------------------------------------------------------------
//=====================================================================
// Set current cursor position.
static long TstFlag = 0;
int TestAndSet(void)
{
	long tmp;
	tmp = 1;
	_asm {
		MOV EAX, tmp
		xchg TstFlag, EAX
		MOV tmp, EAX
	}
	return tmp;
}
void ReleaseTAS(void)
{
	TstFlag = 0;
}


void GotoXY(int x, int y)
{
	// Set the cursor position.
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Cord;
	Cord.X = x;
	Cord.Y = y;
	SetConsoleCursorPosition(StdOut, Cord);
}
void GetXY(int * x, int * y)
{
	// Set the cursor position.
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = { 0 };
	GetConsoleScreenBufferInfo(StdOut, &ScreenBufferInfo);


	*x = ScreenBufferInfo.dwCursorPosition.X;
	*y = ScreenBufferInfo.dwCursorPosition.Y;

}
void putsXY(char *s, int x, int y)
{
	int xt, yt;

	while (TestAndSet()) {
		if (ExitServer) return;
		Sleep(1);
	}
	GetXY(&xt, &yt);
	GotoXY(x-10, y);
	printf("%s  ", s);
	GotoXY(xt, yt);

	ReleaseTAS();
}
//----------------------------------------------------------------------------------------------
void WerFault_check(void*)
{
	FILE *fp;
	char list[16];

	while (1) {
		Sleep(473);
		system("TASKLIST > .\\task_check");
		while(!fileExist((char *)".\\task_check")){
			Sleep(1);
		}
		//ShellExecute(NULL,  _T("open"), _T("cmd.exe"),_T("TASKLIST > C:\\ProxyBuf\\task_check"), NULL, SW_SHOW);
		system("findstr \"WerFault.exe\" .\\task_check > .\\task_exist");
		while (!fileExist((char *)".\\task_exist")) {
			Sleep(1);
		}
		//ShellExecute(NULL,  _T("open"), _T("cmd.exe"),_T("findstr \"WerFault.exe\" C:\\ProxyBuf\\task_check > C:\\ProxyBuf\\task_exist"), NULL, SW_SHOW);
		fopen_s(&fp,".\\task_exist", "rb");
		if (fp) {

			fread(list, sizeof(char), 16, fp);
			for (int i = 0;i < 16;i++) {
				if (list[i] == ' ') {
					list[i] = 0x00;
				}
			}
			//printf("%s\n",list);
			if (!strcmp("WerFault.exe", list)) {

				//ShellExecute(NULL,  _T("open"), _T("cmd.exe"),_T("TASKKILL /F /IM WerFault.exe > nul"), NULL, SW_SHOW);
				system("TASKKILL /F /IM WerFault.exe > nul");
				for (int i = 0;i < 16;i++) {
					list[i] = 0x00;
				}
			}
			fclose(fp);
			fopen_s(&fp,".\\task_check", "w");
			if (fp)  fclose(fp);
			fopen_s(&fp,".\\task_exist", "w");
			if (fp)	fclose(fp);
		}
	}
}

//----------------------------------------------------------------------------------------------
void getLCTimeLog(char * cctime)
{
	time_t timer;
	struct tm tblock;

	timer = time(NULL);
	localtime_s(&tblock, &timer);
	sprintf_s(cctime, 100, "%d/%02d/%02d/%02d/%02d/%02d", (tblock.tm_year + 1900), tblock.tm_mon + 1, tblock.tm_mday, tblock.tm_hour, tblock.tm_min, tblock.tm_sec);
}
void getLocalTimeLog(char * cctime)
{
	time_t timer;
	struct tm tblock;

	timer = time(NULL);
	localtime_s(&tblock, &timer);
	sprintf_s(cctime, 100, "%d/%02d/%02d %02d:%02d:%02d", (tblock.tm_year + 1900) - 1911, tblock.tm_mon + 1, tblock.tm_mday, tblock.tm_hour, tblock.tm_min, tblock.tm_sec);
}
int getCTimeLog(char * cctime)
{
	time_t timer;
	struct tm tblock;

	timer = time(NULL);
	localtime_s(&tblock, &timer);
	sprintf_s(cctime, 100, "%02d:%02d", tblock.tm_hour, tblock.tm_min);
	return tblock.tm_wday;
}
int getCTimeSec(char * cctime)
{
	time_t timer;
	struct tm tblock;

	timer = time(NULL);
	localtime_s(&tblock, &timer);
	sprintf_s(cctime, 100, "%05d", (tblock.tm_hour * 60 + tblock.tm_min) * 60 + tblock.tm_sec);
	return tblock.tm_wday;
}
//----------------------------------------------------------------------------------------------
static long TstFlag1 = 0;
int TestAndSet1(void)
{
	long tmp;
	tmp = 1;
	_asm {
		MOV EAX, tmp
		xchg TstFlag1, EAX
		MOV tmp, EAX
	}
	return tmp;
}
void ReleaseTAS1(void)
{
	TstFlag1 = 0;
}
int Mutex_send(SOCKET sckt, char *buf, int len, int flag)
{
	int n,count=0;

	while (TestAndSet1()) {
		if (ExitServer) return -1;
		Sleep(1);
	}
	do {
		n = send(sckt, buf, len, flag);
		if (n <= 0) {
			count++;
			Sleep(3);
		}
	} while (n <= 0 && count < 10);
	if (n < 0)ExitServer = 1;
	ReleaseTAS1();
	return n;
}

