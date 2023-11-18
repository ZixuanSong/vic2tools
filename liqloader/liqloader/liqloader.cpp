// liqloader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Windows.h"
#include "tlhelp32.h"

#include <iostream>


const WCHAR *dll_name = L"liqqy.dll";

void PrintProcessEntry(LPPROCESSENTRY32W entry) {

	printf("\nProcess\nId: %d\nThreads: %d\nParent: %d\n", entry->th32ProcessID,
		entry->cntThreads,
		entry->th32ParentProcessID);
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

int main()
{
	int ret;

	printf("Waiting for v2game process...\n");

	DWORD process_id;
	HANDLE process_handle = nullptr;

	WCHAR *path_buffer;
	DWORD path_len = MAX_PATH;

	LPVOID path_alloc_addr;
	FARPROC loadlib_addr;

	HANDLE remote_thread;

	while (process_handle == nullptr) {
		GetProcessHandle(&process_handle, &process_id);
	}

	printf("Validating liqdll.dll:\n");

	path_buffer = (WCHAR*) HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * MAX_PATH);
	if (path_buffer == NULL) {
		printf("alloc error: %d\n", GetLastError());
		goto fail;
	}

	ret = GetCurrentDirectoryW(path_len, path_buffer);
	if (ret == 0) {
		printf("curr dir error: %d\n", GetLastError());
		goto fail;
	}
	path_len = ret;
	path_buffer[path_len++] = L'\\';

	std::memcpy(&path_buffer[path_len], dll_name, wcslen(dll_name) * 2);
	path_len += wcslen(dll_name);
	path_buffer[path_len++] = L'\0';
	printf("%S\n", path_buffer);

	ret = GetFileAttributesW(path_buffer);
	if (ret == INVALID_FILE_ATTRIBUTES) {
		printf("file attribute error: %d\n", GetLastError());
		goto fail;
	}

	printf("Remote allocating...\n");
	path_alloc_addr = VirtualAllocEx(process_handle, NULL, sizeof(WCHAR) * path_len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (path_alloc_addr == NULL) {
		printf("alloc error: %d\n", GetLastError());
		goto fail;
	}

	printf("Writing path to: 0x%X\n", (DWORD)path_alloc_addr);

	if (!WriteProcessMemory(process_handle, path_alloc_addr, path_buffer, sizeof(WCHAR) * path_len, NULL)) {
		printf("memory write error: %d\n", GetLastError());
	}

	printf("Finding function addr\n");
	loadlib_addr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryW");
	if (loadlib_addr == NULL) {
		printf("Couldn't find loadlib addr: %d\n", GetLastError());
		goto fail;
	}

	printf("starting load dll thread\n");
	remote_thread = CreateRemoteThread(process_handle, NULL, 0, (LPTHREAD_START_ROUTINE)loadlib_addr, path_alloc_addr, 0, NULL);
	if (remote_thread == NULL) {
		printf("create thread error: %d\n", GetLastError());
		goto fail;
	}

	
	WaitForSingleObject(remote_thread, INFINITE);
	GetExitCodeThread(remote_thread, (DWORD*) &ret);
	printf("liqqy.dll base: 0x%X\n", (DWORD)ret);

	CloseHandle(remote_thread);
	return 0;

fail:
	getchar();
	return 1;
}
