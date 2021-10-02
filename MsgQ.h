#pragma once
void initMQ(void);
struct Message *getMsgBuf(void);
void insertMsgQ(struct Message *buf);
int isMsgQEmpty(void);
struct Message * delMsgQ(void);
void releaseMQBuf(struct Message *buf);