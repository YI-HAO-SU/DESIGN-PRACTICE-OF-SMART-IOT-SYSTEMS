#include "pch.h"
#include "jstring.h"
#include <stdio.h> 
#include <string>
#include "dbfile.h"
int makeFieldData(char *s, char * s1, char *s2);
int strAG(char *s2, int n);
int StoI(char * s);
extern char **devNameOut;
extern char **devName;
extern int devN;
extern char *CnState;

int jstodb(char *topic, char * msg)
{
	int i,j,ii,t,id;
	char *p, s1[50], s2[550],dbn[50];
	char buf[1000],sout[500];
	int k;
	string ts;
	
	p = NULL;
	for (i = 0;i < devN;i++) {
		sprintf_s(s1, "/%s/", devName[i]);
		if ((p=strstr(topic, s1)) != NULL) {
			id = i;
			CnState[i] = 0;
			break;
		}
	}
	if (p == NULL) return 0;

	p = instr(msg, (char *)"{");
	if (p == NULL)return 0;

	i = 0;
	ts = "";dbn[0] = 0x00;

	while (p = sgets(p, buf)) {
//		printf("[%d]:%s\n", i, buf);
		switch(i){
		case 1:
//			printf("[%d]", i);
			if (strstr(buf, "\"time\"") == NULL) {
				printf(">>[%s][%s]\n", "\"time\"", buf);
				return 0;
			}
			if (writefield(buf, s1, s2)) {
				if (s2[0] == '0'&&s2[1] == '0') {
					cmdInterpreter(devNameOut[id],(char *)"SynT");
					return 0;
				}
				ts = s1;
				ts += ":";
				ts += s2;
				ts += "|";

				j = 0;
				for (ii = 0;ii < 10;ii++) {
					if (s2[ii] != '/') {
						dbn[j] = s2[ii];
						j++;
					}
				}
				dbn[j] = 0x00;
//				sprintf_s(sout,100,"[%s]", s2);
			}
//			printf("\n");
			break;
		case 3:
			//			printf("[%d]", i);
//			if (strstr(buf, "\"current limit\"") == NULL) break;
			if (strstr(buf, "\"current limit\"") == NULL) {
				printf(">>[%s][%s]\n", "\"current limit\"", buf);
				return 0;
			}
			if (makeFieldData(buf, s1, s2)) {
				for (ii = 0;ii < 10;ii++) {
					if (s1[ii] != ' ')
						s1[ii] = s1[ii];
					else
						s1[ii] = '_';
				}
//				sprintf_s(sout, 100, "[%s:%s]",s1, s2);
			}
			//			printf("\n");

			break;
		case 5:
			//			printf("[%d]", i);
//			if (strstr(buf, "\"switch\"") == NULL) break;
			if (strstr(buf, "\"switch\"") == NULL) {
				printf(">>[%s][%s]\n", "\"switch\"", buf);
				return 0;
			}
			if (writefield(buf, s1, s2)) {
				ts += s1;
				ts += ":";
				ts += s2;
				ts += "|";
				k = strlen(sout);
//				sprintf_s(&(sout[k]), 100, "%s:%s =>", s1, s2);
				updateDevState(devName[id],(char *) "swState", s2);
			}
			break;
		case 9:
//			printf("[%d]", i);
//			if (strstr(buf, "\"current data\"") == NULL) break;
			if (strstr(buf, "\"current data\"") == NULL) {
				printf(">>[%s][%s]\n", "\"current data\"", buf);
				return 0;
			}
			if (t=writeAGfield(buf, 15, s1, s2)) {
				ts += s1;
				ts += ":";
				ts += s2;
				ts += "|";
			}
			k = strlen(sout);
//			sprintf_s(&(sout[k]), 100, "[%s]", s2);

//			buf[t] = ':';
			if (writefield(buf, s1, s2)) {
				ts += s1;
				ts += "_s:";
				ts += s2;
				ts += "|";

			}
//			printf("\n");
			break;
		case 18:
			if (writefield(buf, s1, s2)) {
//				printf("[%s]:[%s]\n", s1, s2);
				if (instr(s2, (char *)"false") == NULL) {
					ts += "Temp:";
					ts += s2;
					ts += "|";
//					puts("OK!\n");
				}
			}
			break;
		default:

			break;
		}
		i++;
		if (p[0] == 0x00)break;
		
	}
//	putsXY(sout, 60, 3);
	if (dbn[0] != 0x00) {
		ts += "<end>";
		write_recs_to_db(topic, dbn, ts);
	}

	return i;
}
int makeFieldData(char *s, char * s1, char *s2)
{
	int i;

	if (s[0] != '"')return 0;
	str_split(s, s1, s2);
	for (i = 0;s1[i];i++)
		if (s1[i] == ':') {
			s1[i] = 0x00;
			break;
		}
	//	printf("[%s]:[%s]", s1, s2);
	return 1;
}
int writefield(char *s, char * s1,char *s2) 
{
	int i,t;

	if (s[0] != '"')return 0;
	t=str_split(s, s1, s2);
	for(i=0;s1[i];i++)
		if (s1[i] == ' ') {
			s1[i] = 0x00;
			break;
		}
//	printf("[%s]:[%s]", s1, s2);
	return t;
}
int writeAGfield(char *s,int n, char * s1, char *s2)
{
	int k=0,i,t;
	double v;

	if (s[0] != '"')return 0;
	t=str_split(s, s1, s2);
	for (i = 0;s1[i];i++)
		if (s1[i] == ' ') {
			s1[i] = 0x00;
			break;
		}
	k = strAG(s2, n);
	v = k * 3.26;
	sprintf_s(s2,50,"%8.1f", v);
	return t;
}
int strAG(char *s, int n)
{
	int i,k,sum,I[100];
	char *ss[100];

	k = 0;
	ss[k] = s;k++;
	for (i = 0;s[i];i++) {
		if (s[i] == ',') {
			s[i] = 0x00;

			if (s[i + 1] != 0x00) {
				ss[k] = &(s[i + 1]);
				I[k-1] = i;
				k++;
			}
		}
	}
	sum = 0;
	for (i = k - n;i < k;i++) {
		sum += StoI(ss[i]);
	}
	for (i = 0;i < (k-1);i++) {
		s[I[i]]=',';
	}
	return sum;
}
int StoI(char * s)
{
	int i;
	int t = 0;

	for (i = 0;s[i];i++) {
		t = t * 10 + (s[i] - '0');
	}
	return t;
}
char * sgets(char * s, char * sout)
{
	int i;
	char *p;

	for (i = 0;s[i] && s[i] != 0x0A;i++)
		sout[i] = s[i];
	sout[i] = 0x00;
	if (s[i] == 0x00)p = &(s[i]);
	else p = &(s[i + 1]);
	return p;
}
char *instr(char *s, char * s1)
{
	char * p;
	int n;
	char c;

	p = NULL;
	n = strlen(s1);
	//	printf("[%s][%d]\n",s1,n);getchar();
	while (strlen(s) >= n) {
		c = s[n];
		s[n] = 0x00;
		if (strcmp(s, s1) == 0) {
			p = s;
			s[n] = c;
			return p;
		}
		else {
			s[n] = c;
			s++;
		}
	}
	return p;
}

int str_split(char * buf, char *s1, char * s2)
{
	int i, j = 0,t;
	for (i = 0, j = 0;buf[i] != ':';i++) {
		if (buf[i] != '\"') {
			s1[j++] = buf[i];
		}
	}
	s1[j] = 0x00;t = j;
	i++;j = 0;
	for (;buf[i] != '\n' && buf[i];i++) {
		if (buf[i] != '\"'&&buf[i] != 'Z'&&buf[i] != '}'&&buf[i] != '\''&&buf[i] != '['&&buf[i] != ']') {
			s2[j++] = buf[i];
		}
	}
	if (s2[j - 1] == ',')j--;
	s2[j] = 0x00;

	return t;
}
