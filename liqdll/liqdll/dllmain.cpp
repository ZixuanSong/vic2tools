// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "structs.h"
#include "shell.h"
#include "game.h"
#include "overlay.h"

#include <stdio.h>

DWORD WINAPI LiqEntry(void *arg) {

	WCHAR buff[64];
	HMODULE v2game = GetModuleHandleA("v2game.exe");
	HMODULE me = GetModuleHandleA("liqqy.dll");

	swprintf_s(buff, 64, L"liqqy.dll base: 0x%X\nv2game base: 0x%X", (DWORD) me, (DWORD) v2game);
	MessageBox(NULL, buff, L"Liqqy", MB_OK);

	if (AllocConsole() == 0) {
		MessageBox(NULL, L"Failed to open console. liqqy dll will now exit.", L"Liqqy", MB_OK);
		return 1;
	}

	ShellInit();

	printf("Initializing game structs...\n");

	game = (Game*) HeapAlloc(GetProcessHeap(), 0, sizeof(Game));
	if (game->Init((DWORD)v2game) < 0) {
		printf("Failed\n");
		goto end;
	}

	printf("Initializing overlay...\n");

	if (OverlayInit(game->graphics_info) < 0) {
		printf("Failed\n");
		goto end;
	}

	printf("Ready\n\n");

	RunShell(game);
	FreeConsole();

end:
	return 0;
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		HANDLE main_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&LiqEntry, NULL, 0, NULL);
		CloseHandle(main_thread);
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

