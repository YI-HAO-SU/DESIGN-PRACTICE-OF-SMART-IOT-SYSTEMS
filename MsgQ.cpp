#include "pch.h"
#include <iostream>

#include <stdio.h>

#include <process.h>
#include <time.h>
#include "typeFuns.h"
#include "MsgQ.h"

static struct Message * MQM, **MQ;
static int front, tail;

void initMQ(void)
{
	int i;

	MQM = (struct Message *) malloc(16 * sizeof(struct Message));
	for (i = 0;i < 16;i++) {
		MQM[i].v = 0;
	}
	MQ= (struct Message **) malloc(16 * sizeof(struct Message *));
	front = tail = 0;
}
struct Message *getMsgBuf(void)
{
	int i;
	struct Message * p;
	p = NULL;
	for (i = 0;i < 16;i++) {
		if (MQM[i].v == 0) {
			p = &(MQM[i]);
			p->v = 1;
			break;
		}
	}
	return p;
}
void releaseMQBuf(struct Message *buf)
{
	buf->v = 0;
}

void insertMsgQ(struct Message *buf)
{
	MQ[tail++] = buf;
	tail = tail & 0x0F;
}
int isMsgQEmpty(void)
{
	if (front == tail)return 1;
	return 0;
}
struct Message * delMsgQ(void)
{
	struct Message * p;
	p = MQ[front++];
	front = front & 0x0F;
	return p;
}