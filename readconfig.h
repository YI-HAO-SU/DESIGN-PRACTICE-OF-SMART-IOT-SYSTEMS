#pragma once
typedef struct {
	int length;

	struct {
		char name[20];
		char cmdMsg[200];
	}cmd[20];
}CMD;
typedef struct {
	int length;

	struct {
		char Sname[40];
		char dbName[40];
	}Fields[20];
}Inputs;
int readConfig(char *cnfln);
