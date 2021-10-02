// 開始使用的秘訣: 
//   1. 使用 [方案總管] 視窗，新增/管理檔案
//   2. 使用 [Team Explorer] 視窗，連線到原始檔控制
//   3. 使用 [輸出] 視窗，參閱組建輸出與其他訊息
//   4. 使用 [錯誤清單] 視窗，檢視錯誤
//   5. 前往 [專案] > [新增項目]，建立新的程式碼檔案，或是前往 [專案] > [新增現有項目]，將現有程式碼檔案新增至專案
//   6. 之後要再次開啟此專案時，請前往 [檔案] > [開啟] > [專案]，然後選取 .sln 檔案

#ifndef PCH_H
#define PCH_H

#include <WINSOCK2.H>
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

// TODO: 在此參考您的程式所需要的其他標頭
void getLocalTimeLog(char * cctime);
int getCTimeLog(char * cctime);
void getLCTimeLog(char * cctime);
void GotoXY(int x, int y);
void GetXY(int * x, int * y);
void putsXY(char *s, int x, int y);
int Mutex_send(SOCKET sckt, char *buf, int len, int flag);
int cmdInterpreter(char * devNOut,char *BufSend);

#endif //PCH_H
