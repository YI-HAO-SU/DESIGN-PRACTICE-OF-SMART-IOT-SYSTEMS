#include "pch.h"
#include <iostream>

#include <conio.h>
#include "dbfile.h"
#include "readconfig.h"

char **devNameIn, **devNameOut, **devName, *CnState;
CMD *cmdMsg;
Inputs *InP;
char*	BrokerIP;
int readCMD(FILE * fp, char *buf, int i);
int readInputs(char *buf, int i);
int readConfig(char *cnfln)
{
	FILE *fp;

	char buf[200];
	int i, j, n = 0;

	if (fileExist((char *)cnfln)) {
		fopen_s(&fp, cnfln, "r");
		if (fp != NULL) {
			for (j = 0;fgets(buf, 80, fp) > 0;j++)
				if (buf[0] == '[' || buf[0] == 0x00 || buf[0] == 0x0D || buf[0] == 0x0A)break;
			fclose(fp);n = j - 1;
		}
		if (n == 0) {
			exit(0);
		}
		else {
			devName = (char **)malloc(n * sizeof(char *));
			devNameIn = (char **)malloc(n * sizeof(char *));
			devNameOut = (char **)malloc(n * sizeof(char *));
			cmdMsg = (CMD *)malloc(n * sizeof(CMD));
			InP = (Inputs *)malloc(n * sizeof(Inputs));
			CnState = (char *)malloc(n * sizeof(char));
			for (i = 0;i < n;i++) {
				devName[i] = (char *)malloc(100 * sizeof(char));
				devNameIn[i] = (char *)malloc(100 * sizeof(char));
				devNameOut[i] = (char *)malloc(100 * sizeof(char));
				devName[i][0] = devNameIn[i][0] = devNameOut[i][0] = 0x00;
				CnState[i] = 0;
				cmdMsg[i].length = 0;
				InP[i].length = 0;
			}
			BrokerIP = (char *)malloc(20 * sizeof(char));
		}

		fopen_s(&fp, cnfln, "r");
		fgets(buf, 99, fp);
		for (i = 0;buf[i] && buf[i] != 0x0D && buf[i] != 0x0A;i++)
			BrokerIP[i] = buf[i];

		if (fp != NULL) {
			for (j = 0;j < n   ;j++) {
				if (fgets(buf, 99, fp) == NULL) break;
				for (i = 0;buf[i] && buf[i] != 0x0D && buf[i] != 0x0A;i++)
					devName[j][i] = buf[i];
				devName[j][i] = 0x00;
				sprintf_s(devNameIn[j], 99, "nsysu_ele/%s/in", devName[j]);
				sprintf_s(devNameOut[j], 99, "nsysu_ele/%s/out", devName[j]);
			}
			//			fclose(fp);
		}

		if (fgets(buf, 199, fp) != NULL) {
			do {
				if (buf[0] == '[') {
					for (i = 1;buf[i] && buf[i] != 0x0D && buf[i] != 0x0A && buf[i] != ']';i++);
					if (buf[i] == ']') {
						buf[i] = 0x00;
//						printf(">>[%s]", &(buf[1]));
						for (j = 0;j < n;j++) {
							if (strcmp(devName[j], &(buf[1])) == 0) {
//								printf("[%s]\n", devName[j]);
								readCMD(fp,buf, j);
								break;
							}
						}
					}
				}
			} while (!feof(fp));			
		}
		fclose(fp);
	}
	return n;
}

int readCMD(FILE * fp, char *buf, int i)
{	
	int k=0,j,n,m,flag;
	k = 0;
	while (!feof(fp)) {
		if(fgets(buf, 199, fp)==NULL)break;
		if (buf[0] == '[') break;
		if (buf[0] == '<') {
			readInputs(buf, i);
			continue;
		}
		for (j = 0;buf[j] != '='&&buf[j];j++);
		if (buf[j] == '=') {
			buf[j] = 0x00;	
//			printf("[%s]\n", buf);
			strcpy_s(cmdMsg[i].cmd[k].name, buf);
			flag = 0;
			for (n = j + 1, m = 0;buf[n] && buf[n] != 0x0D && buf[n] != 0x0A;n++) {
				if (buf[n] == '<')flag = 1;
				if (buf[n] == '>')flag = 2;
				switch(flag) {
				case 0:
					cmdMsg[i].cmd[k].cmdMsg[m++] = buf[n];
					break;
				case 1:
					cmdMsg[i].cmd[k].cmdMsg[m++] = '%';
					cmdMsg[i].cmd[k].cmdMsg[m++] = 's';
					break;
				case 2:
					flag = 0;
					break;
				default:
					cmdMsg[i].cmd[k].cmdMsg[m++] = buf[n];
				}
			}
			cmdMsg[i].cmd[k].cmdMsg[m] = 0x00;
			k++;
		}
	}
	cmdMsg[i].length = k;
//	printf("[%d]=%d\n", i, cmdMsg[i].length);
	return k;
}
int readInputs(char *buf, int i)
{
	char *p;
	int k;

	if ((p = strstr(buf, ">")) == NULL)return 0;
	*p = 0x00;
	if ((p = strstr(buf, "=")) == NULL)return 0;
	*p = 0x00;p++;
	k = InP[i].length;
	strcpy_s(InP[i].Fields[k].Sname, &(buf[1]));
	strcpy_s(InP[i].Fields[k].dbName, p);
	
//	printf("[%d]=%d <%s=%s>\n", i, InP[i].length, InP[i].Fields[k].Sname, InP[i].Fields[k].dbName);
	InP[i].length = k + 1;
	return 1;
}