#include "pch.h"
#include "structs.h"
#include "syscallhelper.h"
#include "game.h"
#include "overlay.h"
#include "Windows.h"

#include <iostream>
#include <string>

int GetGeneralDisplayListThread(LPVOID arg) {
	ThreadStruct *thread_arg = (ThreadStruct*)arg;
	GameInfo *game = (GameInfo*)thread_arg->game;
	DisplayInfo *display_info = (DisplayInfo*)thread_arg->data;

	display_info->arg1 = 0;

	DWORD country_id = game->client_info.client_country_id;
	Army *army_list;
	DWORD army_list_size;


	GeneralDisplayData *display_data_list = (GeneralDisplayData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GeneralDisplayData) * 10);
	DWORD display_data_list_size = 10;

	int ret;
	for (;;) {
		if (thread_arg->status == 0) {
			display_info->data_buffer = NULL;
			break;
		}

		//get army list
		ret = game->LoadArmyList(country_id, &army_list, &army_list_size);
		if (ret < 0) {
			display_info->data_buffer = NULL;
			return -1;
		}

		if (army_list_size > display_data_list_size) {
			DWORD new_size = display_data_list_size * 2;
			if (army_list_size > new_size) {
				new_size = army_list_size;
			}
			
			display_data_list = (GeneralDisplayData*) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, display_data_list, sizeof(GeneralDisplayData) * new_size);
			display_data_list_size = new_size;

		}

		General general_buffer;

		DWORD loaded_display_count = 0;
		WCHAR buffer[128];
		DWORD buffer_len;

		if (WaitForSingleObject(thread_arg->lock, 0) == WAIT_TIMEOUT) {
			HeapFree(GetProcessHeap(), 0, army_list);
			continue;
		}

		for (DWORD i = 0; i < army_list_size; i++) {

			//get generals
			if (game->LoadGeneral(army_list[i], &general_buffer) < 0) {
				continue;
			}

			

			if (game->FormGeneralModifierString(general_buffer, 
												display_data_list[loaded_display_count].display_string, 
												&display_data_list[loaded_display_count].display_string_len) < 0) 
			{
				continue;
			}

			//get screen positions
			if (game->LoadArmyScreenPos(army_list[i], 
										&(display_data_list[loaded_display_count].coord.x), 
										&(display_data_list[loaded_display_count].coord.y)) < 0) 
			{
				continue;
			}

			loaded_display_count++;
		}

		ReleaseMutex(thread_arg->lock);
		HeapFree(GetProcessHeap(), 0, army_list);
		
		display_info->arg1 = 0;
		display_info->data_buffer = (void*)display_data_list;
		display_info->arg1 = loaded_display_count;
		Sleep(1);
	}

	HeapFree(GetProcessHeap(), 0, display_data_list);
	return 0;
}

void Shell(GameInfo* game) {
	
	std::string in;
	for (;;) {
		std::cout << "> ";
		std::getline(std::cin, in);
		if (!in.compare("fow")) {
			game->fow = !(game->fow);
			if (!WriteProcessMemory(game->process_handle, (LPVOID) (game->v2game.base + game->fow_offset), &(game->fow), 1, NULL)) {
				std::cout << "Memory write error" << std::endl;
				return;
			}
		}
		else if (!in.compare("exit")) {
			return;
		}
		else if (!in.compare("testk32")) {

			HMODULE k32_handle = GetModuleHandleW(L"kernel32.dll");
			if (k32_handle == NULL) {
				printf("Couldn't find k32handle: %d\n", GetLastError());
				continue;
			}
		
			if ((DWORD) game->kernel32.handle != (DWORD)k32_handle) {
				printf("k32 base mismatch\n");
			}

			FARPROC func_addr = GetProcAddress(k32_handle, "GetModuleHandleA");

			if (func_addr == NULL) {
				printf("Coudn't get function addr: %d\n", GetLastError());
				continue;
			}

			printf("Function address: %X\n", (DWORD) func_addr);
			HANDLE thread = CreateRemoteThread(game->process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE) func_addr, NULL, NULL, NULL);
			if (thread == NULL) {
				printf("Can't create remote thread: %d\n", GetLastError());
				continue;
			}

			WaitForSingleObject(thread, INFINITE);

			DWORD ret;
			GetExitCodeThread(thread, &ret);
			CloseHandle(thread);

			printf("Exit code: %X\n", ret);
		}
		else if (!in.compare("testv2game1")) {

			printf("Enter province id: ");
			std::getline(std::cin, in);

			/*
				push string pointer
				mov eax, img_base + 0x6AF0C5
				call eax
				add esp, 0x4
				ret
			*/

			char primer[] =	{	0x68, 0xAA, 0xAA, 0xAA, 0xAA,
								0xB8, 0xAA, 0xAA, 0xAA, 0xAA,
								0xFF, 0xD0,
								0x83, 0xC4, 0x04,
								0xC3};

			std::memcpy(&primer[6], &game->PrvStrToIdxFunc_offset, sizeof(DWORD));

			LPVOID new_alloc_addr = VirtualAllocEx(game->process_handle, NULL, in.size() + 1 + sizeof(primer), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (new_alloc_addr == NULL) {
				printf("VirtualAlloc failed: %d\n", GetLastError());
				continue;
			}

			//write string
			if (!WriteProcessMemory(game->process_handle, new_alloc_addr, in.data(), in.size() + 1, NULL)) {
				printf("Memory write failed 1: %d\n", GetLastError());
				VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);
				continue;
			}

			std::memcpy(&primer[1], (DWORD*)&new_alloc_addr, sizeof(DWORD));

			LPVOID primer_addr = (LPVOID)((DWORD)new_alloc_addr + in.size() + 1);
			//write primer
			if (!WriteProcessMemory(game->process_handle, primer_addr, primer, sizeof(primer), NULL)) {
				printf("Memory write failed 2: %d\n", GetLastError());
				VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);
				continue;
			}

			//char buff[128];
			//ReadProcessMemory(proc_handle, new_alloc_addr, buff, 128, NULL);

			HANDLE thread = CreateRemoteThread(game->process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)primer_addr, NULL, NULL, NULL);
			if (thread == NULL) {
				printf("Can't create remote thread: %d\n", GetLastError());
				VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);
				continue;
			}

			WaitForSingleObject(thread, INFINITE);

			DWORD ret;
			GetExitCodeThread(thread, &ret);

			CloseHandle(thread);
			VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);

			printf("Exit code: %d\n", ret);
		}
		else if (!in.compare("testv2game2")) {

			/*
				mov ecx, [img_base + 0xE588E8]
				mov ecx, [ecx + B24]
				mov edx, [ecx]
				mov eax, [edx + 90]
				call eax
				ret
			*/

			char primer[] = {	0x8B, 0x0D, 0xAA, 0xAA, 0xAA, 0xAA,
								0x8B, 0x89, 0x24, 0x0B, 0x00, 0x00,
								0x8B, 0x11,
								0x8B, 0x82, 0x90, 0x00, 0x00, 0x00,
								0xFF, 0xD0,
								0xC3 };

			DWORD e588e8_addr = game->v2game.base + 0xE588E8;

			std::memcpy(&primer[2], &e588e8_addr, sizeof(DWORD));

			LPVOID new_alloc_addr = VirtualAllocEx(game->process_handle, NULL, sizeof(primer), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (new_alloc_addr == NULL) {
				printf("VirtualAlloc failed: %d\n", GetLastError());
				continue;
			}

			//write primer
			if (!WriteProcessMemory(game->process_handle, new_alloc_addr, primer, sizeof(primer), NULL)) {
				printf("Memory write failed 1: %d\n", GetLastError());
				VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);
				continue;
			}

			HANDLE thread = CreateRemoteThread(game->process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE) new_alloc_addr, NULL, NULL, NULL);
			if (thread == NULL) {
				printf("Can't create remote thread: %d\n", GetLastError());
				VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);
				continue;
			}

			WaitForSingleObject(thread, INFINITE);

			DWORD ret;
			GetExitCodeThread(thread, &ret);

			CloseHandle(thread);
			VirtualFreeEx(game->process_handle, new_alloc_addr, 0, MEM_RELEASE);

			if (ret == 0) {
				printf("Non valid selection\n");
				continue;
			}

			printf("Army Stack struct addr: 0x%X\n", ret);

			DWORD name_addr = ret + 0x114;

			char buff[128];
			char len;

			ReadProcessMemory(game->process_handle, (LPVOID) name_addr, buff, 20, NULL);
			len = buff[16];

			if (len >= 16) {
				DWORD tmp;
				std::memcpy(&tmp, buff, 4);
				ReadProcessMemory(game->process_handle, (LPVOID) tmp, buff, len, NULL);
				buff[len] = '\0';
			}

			printf("Name: %s\n", buff);
		}
		else if (!in.compare("listgeneral")) {
			int ret;

			DisplayInfo *display_info = (DisplayInfo*) HeapAlloc(GetProcessHeap(), 0, sizeof(DisplayInfo));
			if (display_info == NULL) {
				printf("Failed to alloc display: %d\n", GetLastError());
				continue;
			}

			display_info->type = GENERAL_LIST;
			display_info->data_buffer = NULL;

			ThreadStruct *thread_struct = (ThreadStruct*)HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadStruct));
			if (thread_struct == NULL) {
				printf("Failed to alloc thread struct: %d\n", GetLastError());
				continue;
			}


			thread_struct->status = 1;
			thread_struct->game = (void*) game;
			thread_struct->data = (void*) display_info;
			thread_struct->lock = CreateMutex(NULL, false, NULL);

			if (thread_struct->lock == NULL) {
				printf("Failed to create lock: %d\n", GetLastError());
				continue;
			}

			//---------------- free display and thread struct past this line -------------------

			HANDLE worker_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &GetGeneralDisplayListThread, thread_struct, 0, NULL);
			if (worker_thread == NULL) {
				printf("Failed to create thread: %d\n", GetLastError());
				CloseHandle(thread_struct->lock);
				HeapFree(GetProcessHeap(), 0, display_info);
				HeapFree(GetProcessHeap(), 0, thread_struct);
				continue;
			}

			Sleep(1000);
			OverlayShow(display_info, thread_struct);

			thread_struct->status = 0;
			WaitForSingleObject(worker_thread, INFINITE);

			CloseHandle(worker_thread);
			CloseHandle(thread_struct->lock);
			HeapFree(GetProcessHeap(), 0, display_info);
			HeapFree(GetProcessHeap(), 0, thread_struct);
		}
		else {
			std::cout << "Unknown option" << std::endl;
		}
	}
}

int main()
{
	GameInfo *game_info = (GameInfo*) HeapAlloc(GetProcessHeap(), 0, sizeof(GameInfo));
	if (game_info == NULL) {
		printf("bad game info alloc %d\n", GetLastError());
		exit(1);
	}

	//game_info->Init();
	OverlayInit();

	printf("\n");
	
	//Shell(game_info);



	HeapFree(GetProcessHeap(), 0, game_info);
}
