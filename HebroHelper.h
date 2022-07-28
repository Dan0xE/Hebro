#pragma once
#ifndef HEBROHELPER
#define HEBROHELPER

#include <iostream>
#include <Windows.h>
#include <string>
#include <TlHelp32.h>


//int getPID(const char* procname)
int getPID(const char* process)
{
	process = "explorer.exe";
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	int pid = 0;
	BOOL hResult;

	//get all processes
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		//printf("Couldn't create a Snapshot of running Processes. Maybe your AV is blocking it\n");
		return 0;
	}

	//int
	pe.dwSize = sizeof(PROCESSENTRY32);

	//info about first process
	hResult = Process32First(hSnapshot, &pe);

	//retrieve info
		while (hResult)
		{
			if (strcmp(process, pe.szExeFile) == 0)
			{
				pid = pe.th32ProcessID;
				break;
			}
			hResult = Process32Next(hSnapshot, &pe);
		}
	
	
	CloseHandle(hSnapshot);
	//std::cout << pid;
	return pid;
	//destroy handle

	
};

#endif
