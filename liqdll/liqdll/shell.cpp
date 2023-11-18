#include "stdafx.h"
#include "shell.h"
#include "overlay.h"
#include <iostream>

void ShellInit() {
	FILE *in;
	FILE *out;
	freopen_s(&in, "CONIN$", "r", stdin);
	freopen_s(&out, "CONOUT$", "w", stdout);

	// Note that there is no CONERR$ file
	HANDLE hStdout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hStdin = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
	SetStdHandle(STD_INPUT_HANDLE, hStdin);
}

int ParseInput(std::string& input, Game *game) {
	
	/*
		Reading game memory here is race condition prone!!!
	*/
	
	if (!input.compare("dumpgfx")) {
		printf("Graphics Struct at: 0x%X\ninterface: 0x%X\ndevice: 0x%X\n", game->graphics_info, 
																			game->graphics_info->directx_interface, 
																			game->graphics_info->device);

		if (game->graphics_info->device != nullptr || (DWORD) game->graphics_info->device != 0xBAADF00D) {
			DWORD *vtable_start = (DWORD*) ((DWORD*)game->graphics_info->device)[0];
			printf("Device vtable at: 0x%X\n", (DWORD) vtable_start);
			for (int i = 0; i < 118; i++) {
				printf("[%d]\t0x%X", i, vtable_start[i]);

				if (i == 42) {
					printf("\tEndScene()");
				}

				printf("\n");
			}
		}

		CameraInfo *c_info = *game->graphics_info->camera_info;
		printf("Camera at: 0x%X\n", c_info);
		printf("Eye vector : [%.2f\t%.2f\t%.2f]\n", c_info->eye.x, c_info->eye.y, c_info->eye.z);

		MapOrigin *map_origin = *(c_info->map_origin_info);
		printf("Map origin at: 0x%X\nX: %.2f, Y: %.2f\n", map_origin, map_origin->x, map_origin->y);

		printf("View matrix:\n");
		PrintMatrix(overlay_data.view_matrix);

		printf("Projection matrix:\n");
		PrintMatrix(overlay_data.proj_matrix);

		return 1;
	}

	if (!input.compare("dumpgenlist")) {
		ListNode<General*>* curr_node;
		Country* country = (Country*) game->country_list->ptr_list_start[game->client_info->client_country_id];
		if (country->general_len == 0) {
			printf("No general\n");
			return 1;
		}

		printf("Start at head:\n");
		curr_node = country->general_head;
		while (curr_node != NULL) {
			printf("%s\n", GetGeneralName(curr_node->data));
			curr_node = curr_node->next;
		}

		printf("\nStart at tail:\n");
		curr_node = country->general_tail;
		while (curr_node != NULL) {
			printf("%s\n", GetGeneralName(curr_node->data));
			curr_node = curr_node->prev;
		}

		return 1;
	}

	if (!input.compare("dumpinfo")) {

		//dump static level structs
		printf("ClientInfo: 0x%X\ncountry list: 0x%X [%d]\n\n", game->client_info, game->country_list, game->country_list->len);

		//dump client info members
		DWORD country_id = game->client_info->client_country_id;
		char *tag = game->client_info->client_country_tag;
		printf("Currently playing: %s [%d]\n\n", tag, country_id);

		//dumping country list and country
		Country *country = (Country*)game->country_list->ptr_list_start[country_id];
		printf("Client Country at: 0x%X\nPrestige: %d\n\n", country, country->presitge);

		//dumping general and trait
		General* head_general = country->general_head->data;
		TraitModsSum sum;
		std::memset(&sum, 0, sizeof(sum));
		GetGeneralTraitModsSum(head_general, &sum);

		char *name;
		if (head_general->name_len >= 16) {
			name = head_general->name_ptr;
		}
		else {
			name = head_general->name;
		}
		printf("General count:%d\nHead general at: 0x%X\nHead general name: %s\nPrestige: %d\n", country->general_len, head_general, name, head_general->prestige);
		printf("Trait modifiers:\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\n", sum.attack, sum.defend, sum.morale, sum.org, sum.recon, sum.speed, sum.attrition, sum.exp, sum.reliability);

		//dump province list and province render
		Province* prov = (Province*)game->client_info->province_list_start[695];

		printf("Province list start: 0x%X [%d]\nProv 695: 0x%X\n", game->client_info->province_list_start, game->client_info->province_list_end - game->client_info->province_list_start, prov);
		printf("Name: %s\nOwned by: %s\nParked army: %d\n", GetProvinceName(prov), prov->owner_tag, prov->army_list_size);
		printf("Static render: 0x%X\nMoving render: 0x%X\n", prov->province_display->static_render, prov->province_display->moving_render);
		if (prov->province_display->static_render != NULL) {
			printf("X: %d\tY: %d\n", prov->province_display->static_render->screen_info->model_info->x, prov->province_display->static_render->screen_info->model_info->y);
		}
		printf("\n");

		//dump army info
		Army* army = country->army_head->data;
		printf("Total number of armies: %d\nHead at: 0x%X\nName: %s\n", country->army_len, army, GetArmyName(army));
		printf("Disp at: 0x%X\nProv: %d\n", army->display_addr, army->display_addr->province_id);
		printf("Is naval unit: %d\n\n", army->vtable->is_naval_unit());

		return 1;
	}

	if (!input.compare("hookdevice")) {
		DWORD *device_vtable_start = (DWORD*)((DWORD*)game->graphics_info->device)[0];

		if (HookVTableEntry(&device_vtable_start[42], device_vtable_start[42]) < 0) {
			return -1;
		}
		return 1;
	}

	if (!input.compare("fow")) {
		*(game->global_opts.fow) = !*(game->global_opts.fow);
		return 1;
	}

	if (!input.compare("gsa")) {
		overlay_opts.general_stat_all = !overlay_opts.general_stat_all;
		return 1;
	}

	if (!input.compare("gs")) {
		overlay_opts.general_stat_player = !overlay_opts.general_stat_player;
		return 1;
	}

	if (!input.compare("gr")) {
		overlay_opts.general_rank = !overlay_opts.general_rank;
		return 1;
	}

	if (!input.compare("gfxdebug")) {
		overlay_opts.gfx_debug = !overlay_opts.gfx_debug;
		return 1;
	}

	if (!input.compare("apa")) {
		overlay_opts.army_path_all = !overlay_opts.army_path_all;
		return 1;
	}

	if (!input.compare("ap")) {
		overlay_opts.army_path_visible = !overlay_opts.army_path_visible;
		return 1;
	}

	/*
	if (!input.compare("rg")) {
		
		
		static DisplayInfo *disp = nullptr;
		static ThreadControl *tc = nullptr;
		static HANDLE worker_thread = NULL;

		if (disp != nullptr) {
			//there exist an entry in g disp list
			//remove disk from g disp list
			WaitForSingleObject(g_disp_list.lock, INFINITE);

			auto iter = g_disp_list.display_info_list.begin();
			while (iter != g_disp_list.display_info_list.end()) {
				if (*iter == disp) {
					break;
				}

				iter++;
			}
			g_disp_list.display_info_list.erase(iter);

			ReleaseMutex(g_disp_list.lock);

			//shut down worker thread
			tc->run = 0;
			WaitForSingleObject(worker_thread, INFINITE);
			CloseHandle(worker_thread);
			worker_thread = NULL;

			//free thread control
			HeapFree(GetProcessHeap(), 0, tc);
			tc = nullptr;

			//free disp
			WaitForSingleObject(disp->lock, INFINITE);
			CloseHandle(disp->lock);

			HeapFree(GetProcessHeap(), 0, disp);
			disp = nullptr;

			printf("rank general worker stopped\n");
			return 1;
		}

		//set up a new display info for general rank
		disp = (DisplayInfo*)HeapAlloc(GetProcessHeap(), 0, sizeof(DisplayInfo));
		std::memset(disp, 0, sizeof(DisplayInfo));

		disp->type = GENERAL_RANK;
		disp->lock = CreateMutex(NULL, true, NULL);
		
		//aquire global display list lock
		DWORD ret;
		ret = WaitForSingleObject(g_disp_list.lock, INFINITE);
		if(ret == WAIT_FAILED){
			printf("Acquire g disp lock failed: %d\n", GetLastError());
			HeapFree(GetProcessHeap(), 0, disp);
			disp = nullptr;
			return -1;
		}

		g_disp_list.display_info_list.push_back(disp);
		//release global display list lock
		ReleaseMutex(g_disp_list.lock);

		//set up thread control for worker thread
		tc = (ThreadControl*)HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadControl));
		tc->run = 1;
		tc->disp = disp;
		tc->game = game;

		//set up worker thread
		worker_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&GeneralRankWorker, tc, 0, NULL);
		if (worker_thread == NULL) {
			printf("Can't create worker: %d\n", GetLastError());
			HeapFree(GetProcessHeap(), 0, disp);
			HeapFree(GetProcessHeap(), 0, tc);
			disp = nullptr;
			tc = nullptr;
			return -1;
		}

		//release disp lock
		ReleaseMutex(disp->lock);
		printf("Worker thread started\n");

		return 1;
	}*/

	/*
	if (!input.compare("gs")) {

		
		static DisplayInfo *disp = nullptr;
		static ThreadControl *tc = nullptr;
		static HANDLE worker_thread = NULL;

		if (disp != nullptr) {
			return 1;
		}

		//set up a new display info for general rank
		disp = (DisplayInfo*)HeapAlloc(GetProcessHeap(), 0, sizeof(DisplayInfo));
		std::memset(disp, 0, sizeof(DisplayInfo));

		disp->type = GENERAL_STAT;
		disp->lock = CreateMutex(NULL, false, NULL);

		//set up thread control for worker thread
		tc = (ThreadControl*)HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadControl));
		tc->run = 1;
		tc->disp = disp;
		tc->game = game;

		//set up worker thread
		worker_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&GeneralStatWorker, tc, 0, NULL);
		if (worker_thread == NULL) {
			printf("Can't create worker: %d\n", GetLastError());
			goto fail;
		}
		printf("Worker thread started\n");

		//aquire global display list lock
		if(WaitForSingleObject(g_disp_list.lock, INFINITE) == WAIT_FAILED) {
			printf("Acquire g disp lock failed: %d\n", GetLastError());
			goto fail_stop;
		}

		g_disp_list.display_info_list.push_back(disp);
		//release global display list lock
		ReleaseMutex(g_disp_list.lock);

		return 1;

	fail_stop:
		tc->run = 0;
		WaitForSingleObject(worker_thread, INFINITE);
	fail:
		CloseHandle(disp->lock);
		HeapFree(GetProcessHeap(), 0, disp);
		HeapFree(GetProcessHeap(), 0, tc);
		disp = nullptr;
		tc = nullptr;
		return -1;
	}*/

	printf("Unknown option\n");
	return -1;
}

void RunShell(Game *game) {
	std::string in;
	int ret;

	for (;;) {
		printf("> ");
		std::getline(std::cin, in);
		
		ret = ParseInput(in, game);

		//errored, start new loop
		if (ret < 0) {
			continue;
		}
		//forcefully exit to liq entry
		if (ret == 0) {
			break;
		}
	}
}