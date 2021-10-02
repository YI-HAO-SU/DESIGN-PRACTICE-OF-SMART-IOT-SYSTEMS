#pragma once
#include <stdio.h>
typedef struct {
	const char * cmdname;
	void(*f)(void *p);
}FunTBL;
struct Message {
	int n;
	char data[1500];
	int msgfn;
	char v;
};
void hello(void *p);
void CONNECT(void *p);
void CONNACK(void *p);
void PUBLISH(void *p);
void PUBACK(void *p);
void PUBREC(void *p);
void PUBREL(void *p);
void PUBCOMP(void *p);
void SUBSCRIBE(void *p);
void SUBACK(void *p);
void UNSUBSCRIBE(void *p);
void UNSUBACK(void *p);
void PINGREQ(void *p);
void PINGRESP(void *p);
void DISCONNECT(void *p);

int send_CONNECT(char *prtc, char *clientID, char  *usr, char *pwd);
int send_SUBSCRIBE(char  *dn);
int send_PUBLISH(char  *topic, int msgid, char * msg);

