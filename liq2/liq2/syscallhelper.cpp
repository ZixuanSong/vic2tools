#include "pch.h"

#include "Windows.h"
#include "tlhelp32.h"
#include "structs.h"
#include <iostream>

void PrintProcessEntry(LPPROCESSENTRY32W entry) {

	printf("\nProcess\nId: %d\nThreads: %d\nParent: %d\n", entry->th32ProcessID,
		entry->cntThreads,
		entry->th32ParentProcessID);
}

void PrintModule(LPMODULEENTRY32W entry) {
	printf("\nModule: %S\nBase: %X\nSize: %d\n", entry->szModule,
		entry->modBaseAddr,
		entry->modBaseSize);
}

void GetProcessHandle(HANDLE *process_handle_ptr, DWORD *proc_id) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		std::cout << "Can't create 32 snapshot. Exiting..." << std::endl;
		exit(1);
	}

	PROCESSENTRY32W process_entry_buff;
	process_entry_buff.dwSize = sizeof process_entry_buff;

	if (!Process32FirstW(snapshot, &process_entry_buff)) {
		std::cout << "Can't iterate first process. Exiting..." << std::endl;
		CloseHandle(snapshot);
		exit(1);
	}

	while (wcscmp(L"v2game.exe", process_entry_buff.szExeFile) != 0) {
		if (!Process32NextW(snapshot, &process_entry_buff)) {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				//process not found
				*process_handle_ptr = nullptr;
				CloseHandle(snapshot);
				return;
			}

			std::cout << "ProcessNext Error. Exiting..." << std::endl;
			CloseHandle(snapshot);
			exit(1);
		}
	}

	PrintProcessEntry(&process_entry_buff);
	HANDLE tmp_process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_entry_buff.th32ProcessID);
	*process_handle_ptr = tmp_process_handle;
	*proc_id = process_entry_buff.th32ProcessID;

	CloseHandle(snapshot);
}

void GetModuleInfo(DWORD process_id, WCHAR *module_name, ModuleInfo *out) {

	int retry = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);
	while (snapshot == INVALID_HANDLE_VALUE) {

		int err = GetLastError();
		if ((err != ERROR_BAD_LENGTH && err != ERROR_PARTIAL_COPY) || retry >= 3) {
			printf("Can't create 32 snapshot. error: %d", err);
			exit(1);
		}

		printf("\nCan't find module, retrying (%d/3) ...\n", ++retry);
		Sleep(5000);
		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);
	}

	MODULEENTRY32W module_entry;
	module_entry.dwSize = sizeof module_entry;

	if (!Module32FirstW(snapshot, &module_entry)) {
		std::cout << "Can't iterate first module. Exiting..." << std::endl;
		CloseHandle(snapshot);
		exit(1);
	}

	while (wcscmp(module_name, module_entry.szModule) != 0) {
		if (!Module32NextW(snapshot, &module_entry)) {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				//module not found

				printf("%S not found\n", module_name);
				out->handle = nullptr;
				CloseHandle(snapshot);
				return;
			}

			std::cout << "ProcessNext Error. Exiting..." << std::endl;
			CloseHandle(snapshot);
			exit(1);
		}
	}

	PrintModule(&module_entry);
	out->base = (DWORD)module_entry.modBaseAddr;
	out->handle = module_entry.hModule;
	CloseHandle(snapshot);
}