#include "pch.h"
#include "typeFuns.h"

#include <time.h>
#include "jstring.h"

using namespace std;
#include <WINSOCK2.H>
#pragma comment(lib, "wsock32.lib")

FunTBL messageFun[] = {
				{"Reserved",hello},
				{"CONNECT",CONNECT},
				{"CONNACK",CONNACK},
				{"PUBLISH",PUBLISH},
				{"PUBACK",PUBACK},
				{"PUBREC",PUBREC},
				{"PUBREL",PUBREL},
				{"PUBCOMP",PUBCOMP},
				{"SUBSCRIBE",SUBSCRIBE},
				{"SUBACK",SUBACK},
				{"UNSUBSCRIBE",UNSUBSCRIBE},
				{"UNSUBACK",UNSUBACK},
				{"PINGREQ",PINGREQ},
				{"PINGRESP",PINGRESP},
				{"DISCONNECT",DISCONNECT},
				{"Reserved",hello},
				{NULL,NULL}
};
union {
	int v;
	unsigned char B[4];
}vn;
extern SOCKET ServerSocket;
extern int ExitServer;
extern char funstate[50];
void Xstrcpy_s(char * st, int n, const char * ss);
static char sout[100];
int send_CONNECT(char *prtc,char *clientID, char  *usr, char *pwd)
{
	union {
		char buf[500];
		struct {
			unsigned char MsgType;
			unsigned char RLen;
		};
	}pkt;

	union {
		char buf[100];
		struct {
			unsigned char Len[2];
			char prtc[98];
		};
	}pkt_prtc;

	union {
		char buf[100];
		struct {
			unsigned char Len[2];
			char clientID[98];
		};
	}pkt_clientID;

	union {
		char buf[100];
		struct {
			unsigned char Len[2];
			char usr[98];
		};
	}pkt_usr;

	union {
		char buf[100];
		struct {
			unsigned char Len[2];
			char pwd[98];
		};
	}pkt_pwd;

	union {
		int v;
		unsigned char B[4];
	}len;

	int i,j,n;

	for (i = 0;prtc[i];i++)pkt_prtc.prtc[i] = prtc[i];
	len.v = i;pkt_prtc.Len[0] = len.B[1];pkt_prtc.Len[1] = len.B[0];

	for (i = 0;clientID[i];i++)pkt_clientID.clientID[i] = clientID[i];
	len.v = i;pkt_clientID.Len[0] = len.B[1];pkt_clientID.Len[1] = len.B[0];

	for (i = 0;usr[i];i++)pkt_usr.usr[i] = usr[i];
	len.v = i;pkt_usr.Len[0] = len.B[1];pkt_usr.Len[1] = len.B[0];

	for (i = 0;pwd[i];i++)pkt_pwd.pwd[i] = pwd[i];
	len.v = i;pkt_pwd.Len[0] = len.B[1];pkt_pwd.Len[1] = len.B[0];

	pkt.MsgType = 0x10;
	i = 2;
	len.B[1]=pkt_prtc.Len[0] ;len.B[0]=pkt_prtc.Len[1];
	for (j = 0;j< len.v+2;j++)pkt.buf[i++] = pkt_prtc.buf[j];
	pkt.buf[i++] = 0x04;
	pkt.buf[i++] = (unsigned char)0xC2;//Connect Flags
	pkt.buf[i++] = 0x00;//Keep Alive
	pkt.buf[i++] = 0x3C;//= 60
	len.B[1] = pkt_clientID.Len[0];len.B[0] = pkt_clientID.Len[1];
	for (j = 0;j < len.v+2;j++)pkt.buf[i++] = pkt_clientID.buf[j];

	len.B[1] = pkt_usr.Len[0];len.B[0] = pkt_usr.Len[1];
	for (j = 0;j < len.v + 2;j++)pkt.buf[i++] = pkt_usr.buf[j];

	len.B[1] = pkt_pwd.Len[0];len.B[0] = pkt_pwd.Len[1];
	for (j = 0;j < len.v + 2;j++)pkt.buf[i++] = pkt_pwd.buf[j];

	len.v = i - 2;
	pkt.RLen = len.B[0];

	n = Mutex_send(ServerSocket, pkt.buf, i, 0);
	return n;
}

int send_SUBSCRIBE(char  *dn)
{
	int i;

	union CN {
		char buf[100];
		struct {
			unsigned char MsgType;
			unsigned char RLen;
			unsigned char tn[2];
			unsigned char DNLen[2];
		};
	}pkt;

	union {
		int v;
		unsigned char B[4];
	}dnlen;
	int n;

	pkt.MsgType = 0x82;
	pkt.tn[0] = 0x00;pkt.tn[1] = 0x03;

	for (i = 0;dn[i];i++) {
		pkt.buf[6 + i] = dn[i];
	}
	pkt.buf[6 + i] = 0x00;
	dnlen.v = i;
	pkt.DNLen[0] = dnlen.B[1];
	pkt.DNLen[1] = dnlen.B[0];
	pkt.RLen = dnlen.B[0] + 4+1;
	n = pkt.RLen + 2;
	n = Mutex_send(ServerSocket, pkt.buf, n, 0);
//	puts(dn);

	return n;
}

int send_PUBLISH(char  *topic,int msgid, char * msg)
{
	int i,j;

	union {
		char buf[1000];
		struct {
			unsigned char MsgType;
			unsigned char RLen[2];
			char payload[900];
		};
	}pkt;

	union {
		char buf[100];
		struct {
			unsigned char Len[2];
			char topic[98];
		};
	}pkt_topic;

	union {
		int v;
		unsigned char B[4];
	}tlen;
	int n;

	if (topic == NULL) return 0;

	pkt.MsgType = 0x30;
	for (i = 0;topic[i];i++)pkt_topic.topic[i] = topic[i];
	tlen.v = i;pkt_topic.Len[0] = tlen.B[1];pkt_topic.Len[1] = tlen.B[0];

	for (i = 0;i < (tlen.v + 2);i++)pkt.payload[i] = pkt_topic.buf[i];

	tlen.v = msgid;	pkt.payload[i++] = tlen.B[1];pkt.payload[i++] = tlen.B[0];

	for (j = 0;msg[j];j++)pkt.payload[i++] = msg[j];

	n=tlen.v = i; 
	if (tlen.v < 128) {
		pkt.RLen[0] = 0x32;pkt.RLen[1] = tlen.B[0];
		i = 1;
		n = n + 2;
	}
	else {
		tlen.v <<= 1; pkt.RLen[0] = tlen.B[1]; pkt.RLen[1] = (tlen.B[0]>>1)|0x80;
		pkt.MsgType = 0x32;
		n = n + 3;
	}

	n = Mutex_send(ServerSocket, &(pkt.buf[i]), n, 0);

	return n;
}

void hello(void *p)
{
//	sprintf_s(sout, 100, "[%s] ", "hello ");
//	putsXY(sout, 85,5);

}
void CONNECT(void *p)
{
//	sprintf_s(sout, 100, "[%s] ", "CONNECT ");
//	putsXY(sout, 85,5);
}
void CONNACK(void *p)
{
	Xstrcpy_s(funstate, 50, "CONNACK");
//	sprintf_s(sout, 100, "[%s] ", "CONNACK  ");
//	putsXY(sout, 85,5);

}
void PUBLISH(void *p)
{
	struct Message *buf;
	char topic[1024], msg[1024] ;
	int i, j;

	buf = (struct Message *)p;
	if (buf->n <= 0)return;
	vn.v = 0;
	vn.B[1] = (unsigned char)((buf->data)[0]);
	vn.B[0] = (unsigned char)((buf->data)[1]);
	i = 2;
	for (j = 0;j < vn.v;j++,i++)
		topic[j] = buf->data[i];
	topic[j] = 0x00;

	for(j=0;i<buf->n;i++,j++)
		msg[j] = buf->data[i];
	msg[j] = 0x00;

	printf("topic=[%s]\n", topic);
	printf("msg=[%s]\n", msg);

//	sprintf_s(sout, 100, "[%s] ", topic);
//	putsXY(sout, 60, 2);
	jstodb(topic, msg);
}
void PUBACK(void *p)
{
	Xstrcpy_s(funstate, 50, "PUBACK");
//	sprintf_s(sout, 100, "[%s] ", "PUBACK   ");
//	putsXY(sout, 85,5);
}
void PUBREC(void *p)
{
	Xstrcpy_s(funstate, 50, "PUBREC");
//	sprintf_s(sout, 100, "[%s] ", "PUBREC   ");
//	putsXY(sout, 85,5);

}
void PUBREL(void *p)
{
	char ppp[70] = { 0x70U,0x01 };//send PUBCOMP
	int n = 2;


	n = Mutex_send(ServerSocket, ppp, 2, 0);
	Xstrcpy_s(funstate, 50, "PUBREL");
//	sprintf_s(sout, 100, "[%s] ", "PUBREL ");
//	putsXY(sout, 85,5);

}
void PUBCOMP(void *p)
{
	Xstrcpy_s(funstate, 50, "PUBCOMP");
//	sprintf_s(sout, 100, "[%s] ", "PUBCOMP   ");
//	putsXY(sout, 85,5);
}

void SUBSCRIBE(void *p)
{
	int i;
	union CN {
		char buf[100];
		struct {
			unsigned char MsgType;
			unsigned char RLen;
			unsigned char tn[2];
			unsigned char DNLen[2];
		};
	}pkt;
	char * dn;
	union {
		int v;
		unsigned char B[4];
	}dnlen;
	int n;

	pkt.MsgType = 0x82;
	pkt.tn[0] = 0x00;pkt.tn[1] = 0x03;
	dn = (char *)p;
	for (i = 0;dn[i];i++) {
		pkt.buf[7 + i] = dn[i];
	}
	pkt.buf[7 + i] = 0x00;
	dnlen.v = i;
	pkt.DNLen[0] = dnlen.B[1];
	pkt.DNLen[1] = dnlen.B[0];
	pkt.RLen = dnlen.B[0]+4;
	n = pkt.RLen + 2;
	n = Mutex_send(ServerSocket, pkt.buf, n, 0);

//	sprintf_s(sout, 100, "[%s] ", "SUBSCRIBE   ");
//	putsXY(sout, 85,5);

}
void SUBACK(void *p)
{
	Xstrcpy_s(funstate, 50, "SUBACK");
//	sprintf_s(sout, 100, "[%s] ", "SUBACK  ");
//	putsXY(sout, 85,5);
}
void UNSUBSCRIBE(void *p)
{
	Xstrcpy_s(funstate, 50, "UNSUBSCRIBE");
//	sprintf_s(sout, 100, "[%s] ", "UNSUBSCRIBE   ");
//	putsXY(sout, 85,5);
}
void UNSUBACK(void *p)
{

	Xstrcpy_s(funstate, 50, "UNSUBACK");
//	sprintf_s(sout, 100, "[%s] ", "UNSUBACK ");
//	putsXY(sout, 85,5);
}
void PINGREQ(void *p)
{
	char ppp[70] = { 0xC0U,0x00 };//send PINGRESP
	int n = 2;
	Xstrcpy_s(funstate, 50, "PINGREQ");
//	sprintf_s(sout, 100, "[%s] ", "PINGREQ ");
//	putsXY(sout, 85, 5);

	while (n > 0) {
		Sleep(60 * 1000);
		n = Mutex_send(ServerSocket, ppp, 2, 0);
	}

}
void PINGRESP(void *p)
{
	Xstrcpy_s(funstate, 50, "PINGRESP");
//	sprintf_s(sout, 100, "[%s] ", "PINGRESP    ");
//	putsXY(sout, 85,5);
}
void DISCONNECT(void *p)
{
	Xstrcpy_s(funstate, 50, "DISCONNECT");
//	sprintf_s(sout, 100, "[%s] ", "DISCONNECT   ");
//	putsXY(sout, 85,5);
}
//----------------------------------------------------------------------------------------------
static long TstFlagf1 = 0;
int TestAndSetf1(void)
{
	long tmp;
	tmp = 1;
	_asm {
		MOV EAX, tmp
		xchg TstFlagf1, EAX
		MOV tmp, EAX
	}
	return tmp;
}
void ReleaseTASf1(void)
{
	TstFlagf1 = 0;
}
void Xstrcpy_s(char * st, int n, const char * ss)
{
	while (TestAndSetf1()) {
		if (ExitServer) return;
		Sleep(1);
	}
	strcpy_s(st, n, ss);

	ReleaseTASf1();
	return ;
}
