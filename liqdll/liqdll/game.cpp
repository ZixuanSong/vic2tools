#include "stdafx.h"
#include "game.h"

#include <algorithm>
#include <string>

Game *game;

//general rank display worker

bool CompareAtt(General *a, General* b) {
	int a_attack = 0, b_attack = 0;
	GetSingleGeneralTraitModsSum(a, TM_ATTACK, &a_attack);
	GetSingleGeneralTraitModsSum(b, TM_ATTACK, &b_attack);
	return (a_attack > b_attack);
}

bool CompareDef(General *a, General* b) {
	int a_defend = 0, b_defend = 0;
	GetSingleGeneralTraitModsSum(a, TM_DEFEND, &a_defend);
	GetSingleGeneralTraitModsSum(b, TM_DEFEND, &b_defend);
	return (a_defend > b_defend);
}


int GeneralRankWorker(void *arg) {

	/*
		Disp structure:
		arg1 - pointer to a buffer of all texts to display
		arg2 - length of buffer in characters
		arg3 - number of lines to help format how far up the screen to display
	*/

	/*
	ThreadControl *tc = (ThreadControl*)arg;
	Game *game = (Game*) tc->game;
	DisplayInfo *disp = (DisplayInfo*) tc->disp;

	DWORD country_id;
	Country* country;

	//std container for std::sort
	std::vector<General*> sort_list;
	ListNode<General*> *curr_node;
	DWORD display_count;


	//swapping scheme
	char *swap_disp_buff[2];
	char *curr_disp_buff;
	char *old_disp_buff;
	DWORD write_idx;

	//one time allocation
	swap_disp_buff[0] = (char*)HeapAlloc(GetProcessHeap(), 0, 1024);
	swap_disp_buff[1] = (char*)HeapAlloc(GetProcessHeap(), 0, 1024);
	curr_disp_buff = swap_disp_buff[0];

	while (tc->run) {
		
		//get country
		game->GetClientCountry(&country);

		write_idx = 0;

		//country with no generals (rare)
		if (country->general_len == 0) {

			display_count = 0;

			write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%s\n", "No general");
			curr_disp_buff[write_idx++] = '\0';

			goto end;
		}

		//get a list of generals
		sort_list.clear();
		sort_list.reserve(country->general_len);
		curr_node = country->general_head;
		while (curr_node != NULL) {
			sort_list.push_back(curr_node->data);
			curr_node = curr_node->next;
		}

		if (sort_list.size() >= 5) {
			display_count = 5;
		}
		else {
			display_count = sort_list.size();
		}
	
		write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%s\n\n", "Sorted by attack:");

		int tmp_stat;

		//sort by attack
		std::sort(sort_list.begin(), sort_list.end(), CompareAtt);
		for (int i = 0; i < display_count; i++) {

			tmp_stat = 0;
			GetSingleGeneralTraitModsSum(sort_list[i], TM_ATTACK, &tmp_stat);
			write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%d %s", tmp_stat / 1000, GetGeneralName(sort_list[i]));

			if (sort_list[i]->army != NULL) {
				write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, " (%s)", GetArmyName(sort_list[i]->army));
			}

			write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%c", '\n');
		}

		write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%s\n\n", "\nSorted by Defense:");

		//sort by def
		std::sort(sort_list.begin(), sort_list.end(), CompareDef);
		for (int i = 0; i < display_count; i++) {

			tmp_stat = 0;
			GetSingleGeneralTraitModsSum(sort_list[i], TM_DEFEND, &tmp_stat);
			write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%d %s", tmp_stat / 1000, GetGeneralName(sort_list[i]));

			if (sort_list[i]->army != NULL) {
				write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, " (%s)", GetArmyName(sort_list[i]->army));
			}

			write_idx += sprintf_s(&curr_disp_buff[write_idx], 1024, "%c", '\n');
		}

		curr_disp_buff[write_idx++] = '\0';

end:
		//entering critical section
		WaitForSingleObject(disp->lock, INFINITE);

		disp->arg1 = (DWORD) curr_disp_buff;
		disp->arg2 = write_idx - 1;
		disp->arg3 = 4 + display_count * 2;

		//leaving critical section - release lock
		ReleaseMutex(disp->lock);

		//swap disp buff
		if (curr_disp_buff == swap_disp_buff[0]) {
			curr_disp_buff = swap_disp_buff[1];
		}
		else {
			curr_disp_buff = swap_disp_buff[0];
		}

		//pulse every 2 seconds. doesn't need to update that much as general creation/death is relatively rare 
		//a longer sleep time will prevent race conditions as other thread will have enough time to clean up data structures before switching
		//to this thread
		Sleep(2500);
	}

	WaitForSingleObject(disp->lock, INFINITE);
	//free swap buffs;
	HeapFree(GetProcessHeap(), 0, swap_disp_buff[0]);
	HeapFree(GetProcessHeap(), 0, swap_disp_buff[1]);

	ReleaseMutex(disp->lock);

	return 0;*/
}

//general stat display worker thread
int GeneralStatWorker(void *arg) {

	/*
		arg1 - pointer to general stat display list
		arg2 - list length
	*/

	/*
	ThreadControl *tc = (ThreadControl*)arg;
	Game *game = (Game*)tc->game;
	DisplayInfo *disp = (DisplayInfo*)tc->disp;

	Province *prov_ptr;
	//Country *country_ptr;
	Army *army_ptr;
	ArmyDisplay *army_disp_ptr;
	//General *general_ptr;
	ListNode<Army*> *curr_node;

	DWORD prov_list_len = game->client_info->province_list_end - game->client_info->province_list_start;
	

	//swapping scheme
	GeneralStatDisplayEntry *swap_disp_buff[2];
	GeneralStatDisplayEntry *curr_disp_buff;
	swap_disp_buff[0] = (GeneralStatDisplayEntry*)HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(GeneralStatDisplayEntry));
	swap_disp_buff[1] = (GeneralStatDisplayEntry*)HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(GeneralStatDisplayEntry));
	curr_disp_buff = swap_disp_buff[0];
	DWORD entry_count;

	GeneralStatInternalEntry *internal_list;
	DWORD estimated_count;
	DWORD internal_count;

	GeneralStatDisplayEntry gen_stat_disp_buff;
	Point static_coord_buff;
	Point moving_coord_buff;
	int static_flag;
	int moving_flag;

	while (tc->run) {

		entry_count = 0;
		
		printf("here1\n");

		//traverse country list
		for (int i = 0; i < prov_list_len; i++) {
			prov_ptr = (Province*)game->client_info->province_list_start[i];
			
			//printf("here2\n");
			//ignore country with no army
			if (prov_ptr->army_list_size == 0) {
				continue;
			}

			static_flag = game->GetStaticProvScreenPos(prov_ptr, &static_coord_buff);
			moving_flag = game->GetMovingProvScreenPos(prov_ptr, &moving_coord_buff);

			//dont display armies behind banner
			if (static_flag == 1 && static_coord_buff.y < BANNER_HEIGHT) {
				continue;
			}
			if (moving_flag == 1 && moving_coord_buff.y < BANNER_HEIGHT) {
				continue;
			}

			//province doesn't render anything don't care
			if (static_flag < 0 && moving_flag < 0) {
				continue;
			}

			if (static_flag == 1) {
				FixToScreenBound(&static_coord_buff);
			}

			if (moving_flag == 1) {
				FixToScreenBound(&moving_coord_buff);
			}

			printf("here3\n");
			estimated_count = prov_ptr->army_list_size;
			internal_list = (GeneralStatInternalEntry*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, estimated_count * sizeof(GeneralStatInternalEntry));
			internal_count = 0;
			
			SwitchToThread();

			//entering race condition prone section
			if (prov_ptr->army_list_head == NULL) {
				if (internal_list != NULL) {
					HeapFree(GetProcessHeap(), 0, internal_list);
					internal_list = NULL;
				}
				continue;
			}
			
			printf("here4.1\n");
			//traverse army
			curr_node = prov_ptr->army_list_head;
			while (curr_node != NULL) {

				internal_list[internal_count].moving = (short)curr_node->data->path_list_size;
				internal_list[internal_count].on_screen = curr_node->data->display_addr->is_within_screen;
				internal_list[internal_count].prestige = curr_node->data->general->prestige;
				internal_list[internal_count].attack = 0;
				internal_list[internal_count].defense = 0;
				GetSingleGeneralTraitModsSum(curr_node->data->general, TM_ATTACK, &(internal_list[internal_count].attack));
				GetSingleGeneralTraitModsSum(curr_node->data->general, TM_DEFEND, &(internal_list[internal_count].defense));

				internal_count++;

				if (internal_count == estimated_count) {
					break;
				}

				if (curr_node->next == NULL || curr_node->next == (ListNode<Army*>*) 0xBAADF00D) {
					break;
				}
				curr_node = curr_node->next;
			}

			//exiting race condition prone section
			
			printf("here5\n");

			//parse information
			for (int i = 0; i < internal_count; i++) {

				//moving
				if (internal_list[i].moving) {
					if (moving_flag < 0) {
						continue;
					}

					curr_disp_buff[entry_count].coord.x = moving_coord_buff.x;
					curr_disp_buff[entry_count].coord.y = moving_coord_buff.y;
				}
				//not moving
				else {
					if (static_flag < 0) {
						continue;
					}

					curr_disp_buff[entry_count].coord.x = static_coord_buff.x;
					curr_disp_buff[entry_count].coord.y = static_coord_buff.y;
				}

				printf("here5.1\n");
				if (internal_list[i].attack >= 0) {
					curr_disp_buff[entry_count].attack[0] = '+';
				}
				else {
					curr_disp_buff[entry_count].attack[0] = '-';
				}
				curr_disp_buff[entry_count].attack[1] = '0' + (abs(internal_list[i].attack / 1000));

				printf("here5.2\n");
				if (internal_list[i].defense >= 0) {
					curr_disp_buff[entry_count].defense[0] = '+';
				}
				else {
					curr_disp_buff[entry_count].defense[0] = '-';
				}
				curr_disp_buff[entry_count].defense[1] = '0' + (abs(internal_list[i].defense / 1000));
				printf("here5.3\n");
				
				curr_disp_buff[entry_count].prestige_len = sprintf_s(curr_disp_buff[entry_count].prestige, 8, "%.2f%%", (float)internal_list[i].prestige / 10);
				
				printf("here5.4\n");
				//if attack, defense, and prestige are all 0 then there is no reason to display it
				if (curr_disp_buff[entry_count].attack[1] == L'0' && curr_disp_buff[entry_count].defense[1] == L'0' && internal_list[i].prestige == 0) {
					continue;
				}
				
				
				printf("here5.5\n");
				//printf("%c%c\t%c%c", curr_disp_buff[entry_count].attack[0], curr_disp_buff[entry_count].attack[1], curr_disp_buff[entry_count].defense[0], curr_disp_buff[entry_count].defense[0]);

				entry_count++;
			}

			HeapFree(GetProcessHeap(), 0, internal_list);
			internal_list = NULL;
			printf("here6\n");
		}

		
		//entering critical section acquire lock
		WaitForSingleObject(disp->lock, INFINITY);

		disp->arg1 = (DWORD)curr_disp_buff;
		disp->arg2 = entry_count;

		//leaving critical section release lock
		ReleaseMutex(disp->lock);

		//swap disp buff
		if (curr_disp_buff == swap_disp_buff[0]) {
			curr_disp_buff = swap_disp_buff[1];
		}
		else {
			curr_disp_buff = swap_disp_buff[0];
		}

		//printf("here7\n");
	}

	//entering critical section acquire lock
	WaitForSingleObject(disp->lock, INFINITY);

	HeapFree(GetProcessHeap(), 0, swap_disp_buff[0]);
	HeapFree(GetProcessHeap(), 0, swap_disp_buff[1]);

	//leaving critical section release lock
	ReleaseMutex(disp->lock);

	return 1;*/
}

int GeneralStatWorkerOld(void *arg) {
	
	/*
		arg1 - pointer to general stat display list
		arg2 - list length
	*/

	/*
	ThreadControl *tc = (ThreadControl*)arg;
	Game *game = (Game*)tc->game;
	DisplayInfo *disp = (DisplayInfo*)tc->disp;

	Province *prov_ptr;
	Country *country_ptr;
	Army *army_ptr;
	ArmyDisplay *army_disp_ptr;
	General *general_ptr;
	ListNode<Army*> *curr_node;
	DWORD country_len = game->country_list->len;
	DWORD **country_list_start = game->country_list->ptr_list_start;

	//swapping scheme
	GeneralStatDisplayEntry *swap_disp_buff[2];
	GeneralStatDisplayEntry *curr_disp_buff;
	swap_disp_buff[0] = (GeneralStatDisplayEntry*) HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(GeneralStatDisplayEntry));
	swap_disp_buff[1] = (GeneralStatDisplayEntry*) HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(GeneralStatDisplayEntry));
	printf("0x%X\t0x%X\n", swap_disp_buff[0], swap_disp_buff[1]);
	curr_disp_buff = swap_disp_buff[0];
	DWORD entry_count;

	GeneralStatInternalEntry *internal_list;
	DWORD estimated_count;
	DWORD internal_count;

	GeneralStatDisplayEntry gen_stat_disp_buff;
	Point static_coord_buff;
	Point moving_coord_buff;

	while (tc->run) {

		entry_count = 0;
		printf("here2\n");
		//traverse country list
		for (int i = 0; i < country_len; i++) {
			country_ptr = (Country*)country_list_start[i];
			//printf("here2\n");
			//ignore country with no army
			if (country_ptr->army_len == 0) {
				continue;
			}

			estimated_count = country_ptr->army_len;
			internal_list = (GeneralStatInternalEntry*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, estimated_count * sizeof(GeneralStatInternalEntry));
			internal_count = 0;
			printf("here3\n");
			SwitchToThread();

			//entering race condition prone section
			if (country_ptr->army_head == NULL) {
				HeapFree(GetProcessHeap(), 0, internal_list);
				continue;
			}
			printf("here4\n");
			//traverse army
			curr_node = country_ptr->army_head;
			while (curr_node != NULL) {

				internal_list[internal_count].moving = (short)curr_node->data->path_list_size;
				internal_list[internal_count].on_screen = curr_node->data->display_addr->is_within_screen;
				internal_list[internal_count].prov_id = curr_node->data->display_addr->province_id;
				internal_list[internal_count].prestige = curr_node->data->general->prestige;
				GetSingleGeneralTraitModsSum(curr_node->data->general, TM_ATTACK, &(internal_list[internal_count].attack));
				GetSingleGeneralTraitModsSum(curr_node->data->general, TM_DEFEND, &(internal_list[internal_count].defense));

				internal_count++;

				if (internal_count == estimated_count) {
					break;
				}
			next:
				if (curr_node->next == NULL || curr_node->next == (ListNode<Army*>*) 0xBAADF00D) {
					break;
				}
				curr_node = curr_node->next;
			}

			//exiting race condition prone section
			printf("here5 0x%X\n", internal_list);

			//parse information
			for (int i = 0; i < internal_count; i++) {
				if (internal_list[i].prov_id < 1 || internal_list[i].prov_id >= game->prov_size) {
					continue;
				}

				//moving
				if (internal_list[internal_count].moving) {
					//moving render coord is not ready
					if (game->GetMovingProvScreenPos((Province*)game->client_info->province_list_start[internal_list[i].prov_id], &moving_coord_buff) < 0) {
						continue;
					}

					//don't display anything above banner
					if (moving_coord_buff.y < BANNER_HEIGHT) {
						continue;
					}

					gen_stat_disp_buff.coord.x = moving_coord_buff.x;
					gen_stat_disp_buff.coord.y = moving_coord_buff.y;
				}
				//not moving
				else {
					//static render coord is not ready
					if (game->GetStaticProvScreenPos((Province*)game->client_info->province_list_start[internal_list[i].prov_id], &static_coord_buff) < 0) {
						continue;
					}

					//don't display anything above banner
					if (static_coord_buff.y < BANNER_HEIGHT) {
						continue;
					}

					gen_stat_disp_buff.coord.x = static_coord_buff.x;
					gen_stat_disp_buff.coord.y = static_coord_buff.y;
				}


				if (internal_list[internal_count].attack >= 0) {
					gen_stat_disp_buff.attack[0] = '+';
				}
				else {
					gen_stat_disp_buff.attack[0] = '-';
				}
				gen_stat_disp_buff.attack[1] = '0' + (abs(internal_list[internal_count].attack / 1000));


				if (internal_list[internal_count].defense >= 0) {
					gen_stat_disp_buff.defense[0] = '+';
				}
				else {
					gen_stat_disp_buff.defense[0] = '-';
				}
				gen_stat_disp_buff.defense[1] = '0' + (abs(internal_list[internal_count].attack / 1000));

				sprintf_s(gen_stat_disp_buff.prestige, 8, "%0.2f%%", ((float)internal_list[internal_count].prestige) / 10);

				//if attack, defense, and prestige are all 0 then there is no reason to display it
				if (gen_stat_disp_buff.attack[1] == '0' && gen_stat_disp_buff.defense[1] == '0' && internal_list[internal_count].prestige == 0) {
					continue;
				}

				std::memcpy(&curr_disp_buff[entry_count], &gen_stat_disp_buff, sizeof(GeneralStatDisplayEntry));
				entry_count++;
			}

			HeapFree(GetProcessHeap(), 0, internal_list);

		}

		printf("==========================================here6\n");
		//entering critical section acquire lock
		WaitForSingleObject(disp->lock, INFINITY);

		disp->arg1 = (DWORD)curr_disp_buff;
		disp->arg2 = entry_count;

		//leaving critical section release lock
		ReleaseMutex(disp->lock);

		//swap disp buff
		if (curr_disp_buff == swap_disp_buff[0]) {
			curr_disp_buff = swap_disp_buff[1];
		}
		else {
			curr_disp_buff = swap_disp_buff[0];
		}

		//printf("here7\n");
	}

	//entering critical section acquire lock
	WaitForSingleObject(disp->lock, INFINITY);

	HeapFree(GetProcessHeap(), 0, swap_disp_buff[0]);
	HeapFree(GetProcessHeap(), 0, swap_disp_buff[1]);

	//leaving critical section release lock
	ReleaseMutex(disp->lock);

	return 1;
	*/
}

int
Game::Init(DWORD v2game_base) {
	global_opts.fow = (BYTE*)v2game_base + 0xB092FB;
	graphics_info = (GraphicsInfo*) *((DWORD*) (v2game_base + 0xE589B8));

	client_info = (ClientInfo*) *((DWORD*)(v2game_base + 0xE588E8));
	country_list = (CountryList*) *((DWORD*)(v2game_base + 0xE587E4));

	DWORD prov_size = client_info->province_list_end - client_info->province_list_start;

	printf("Filling %d province hits...\n", prov_size);
	render_hit_list = (ProvinceRenderHitEntry*)HeapAlloc(GetProcessHeap(), 0, prov_size * sizeof(ProvinceRenderHitEntry));

	for (int i = 0; i < prov_size; i++) {
		render_hit_list[i].static_render = NULL;
		render_hit_list[i].static_last_tick = 0;
		render_hit_list[i].moving_render = NULL;
		render_hit_list[i].moving_last_tick = 0;
		render_hit_list[i].dock_render = NULL;
		render_hit_list[i].dock_last_tick = 0;
	}

	return 1;
}

int 
Game::GetClientCountry(Country **out) {
	DWORD country_id = client_info->client_country_id;
	*out = (Country*)country_list->ptr_list_start[country_id];
	return 1;
}

int 
Game::GetCountryById(DWORD id, Country **out) {
	*out = (Country*)country_list->ptr_list_start[id];
	return 1;
}

int 
Game::GetArmyScreenCoord(Army* army, Point* out) {
	if (army->vtable->is_naval_unit() == 1) {
		//check dock first
		if (GetDockProvScreenPos((Province*)client_info->province_list_start[army->display_addr->province_id], out) > 0) {
			if (out->y < BANNER_HEIGHT) {
				return -2;
			}
			return 1;
		}

		//if not docked check curr province static render
		if (GetStaticProvScreenPos((Province*)client_info->province_list_start[army->display_addr->province_id], out) > 0) {
			if (out->y < BANNER_HEIGHT) {
				return -2;
			}
			return 1;
		}
		//ship is neither docked nor in an ocean tile
		return -3;
	}
	
	//land unit

	//if moving
	if (army->path_list_size > 0) {
		if (GetMovingProvScreenPos((Province*)client_info->province_list_start[army->display_addr->province_id], out) < 0) {
			return -1;
		}

		//don't display anything above banner
		if (out->y < BANNER_HEIGHT) {
			return -2;
		}
	}
	else {
		//static render coord is not ready
		if (GetStaticProvScreenPos((Province*)client_info->province_list_start[army->display_addr->province_id], out) < 0) {
			return -1;
		}

		//don't display anything above banner
		if (out->y < BANNER_HEIGHT) {
			return -2;
		}
	}
	return 1;
}

int 
Game::ArmyToScreenCoord(Army* army, D3DXVECTOR2* out) {

	D3DXVECTOR3 *world_coord = (D3DXVECTOR3*) &army->display_addr->matrix_2._41;
	WorldToScreen(out, *world_coord);
	return 1;
}

int
Game::GetArmyPathNodes(Army* army, D3DXVECTOR2* path_nodes, DWORD buffer_count, DWORD final_step , DWORD* node_count) {

	//only populate when part or whole of the path is visible on the screen
	//path size is guarenteed to be > 1
	
	DWORD curr_step = 1;	//tail node is the first step

	DWORD count = 0;
	D3DXVECTOR2 last_path_node;
	bool last_node_in_screen;
	D3DXVECTOR3 world_coord;

	//process the current army position node
	ArmyToScreenCoord(army, &last_path_node);
	if (army->display_addr->is_within_screen) {
		last_node_in_screen = true;
		path_nodes[count++] = last_path_node;
	}
	else {
		last_node_in_screen = false;
	}

	D3DXVECTOR2 curr_path_node;
	ListNode<DWORD> *curr_node = army->path_list_tail;
	//traverse from army position to destination
	while (curr_node != NULL) {

		//get current provice's screen pos
		ProvToWorldCoordById(curr_node->data, &world_coord);
		WorldToScreen(&curr_path_node, world_coord);

		if (WithinScreen(curr_path_node)) {

			if (!last_node_in_screen) {
				
				//edge case: duplicate one off screen node

				path_nodes[count++] = last_path_node;
				
			}

			path_nodes[count++] = curr_path_node;

			last_node_in_screen = true;
		}
		else {
			if (last_node_in_screen) {
				path_nodes[count++] = curr_path_node;
			}

			last_node_in_screen = false;
		}

		//stop after the final step is finished processing
		//if final_step is 0 then there is no final step because
		//curr_step always starts at 1 and increases
		if (curr_step == final_step) {
			break;
		}

		curr_step++;
		last_path_node = curr_path_node;
		curr_node = curr_node->prev;
	}

	*node_count = count;
	return 1;
}

int 
Game::GetStaticProvScreenPos(Province *prov, Point *out) {

	DWORD prov_id = prov->province_id;

	if (prov->province_display->static_render == NULL) {
		return -1;
	}

	if (prov->province_display->static_render != render_hit_list[prov_id].static_render) {
		render_hit_list[prov_id].static_render = prov->province_display->static_render;
		render_hit_list[prov_id].static_last_tick = GetTickCount();
		return -2;
	}

	if (GetTickCount() - render_hit_list[prov_id].static_last_tick < VALID_RENDER_COUNT) {
		return -3;
	}
	
	out->x = prov->province_display->static_render->screen_info->model_info->x - 10;
	out->y = prov->province_display->static_render->screen_info->model_info->y + 28;
	FixToScreenBound(out);
	return 1;
}

int 
Game::GetMovingProvScreenPos(Province *prov, Point *out) {
	DWORD prov_id = prov->province_id;

	if (prov->province_display->moving_render == NULL) {
		return -1;
	}

	if (prov->province_display->moving_render != render_hit_list[prov_id].moving_render) {
		render_hit_list[prov_id].moving_render = prov->province_display->moving_render;
		render_hit_list[prov_id].moving_last_tick = GetTickCount();
		return -2;
	}

	if (GetTickCount() - render_hit_list[prov_id].moving_last_tick < VALID_RENDER_COUNT) {
		return -3;
	}

	out->x = prov->province_display->moving_render->screen_info->model_info->x - 10;
	out->y = prov->province_display->moving_render->screen_info->model_info->y + 28;
	FixToScreenBound(out);
	return 1;
}

int
Game::GetDockProvScreenPos(Province *prov, Point *out) {
	DWORD prov_id = prov->province_id;

	if (prov->province_display->dock_render == NULL) {
		return -1;
	}

	if (prov->province_display->dock_render != render_hit_list[prov_id].dock_render) {
		render_hit_list[prov_id].dock_render = prov->province_display->dock_render;
		render_hit_list[prov_id].dock_last_tick = GetTickCount();
		return -2;
	}

	if (GetTickCount() - render_hit_list[prov_id].dock_last_tick < VALID_RENDER_COUNT) {
		return -3;
	}

	out->x = prov->province_display->dock_render->screen_info->model_info->x - 10;
	out->y = prov->province_display->dock_render->screen_info->model_info->y + 28;

	FixToScreenBound(out);
	return 1;
}

int
Game::ProvToScreen(Province* prov, D3DXVECTOR2* out) {
	D3DXVECTOR3 world_coord;
	ProvToWorldCoord(prov, &world_coord);
	world_coord.y = 0.5f;
	WorldToScreen(out, world_coord);
	return 1;
}

int 
Game::ProvToWorldCoordById(DWORD id, D3DXVECTOR3* out) {
	Province *prov = (Province*) client_info->province_list_start[id];
	ProvToWorldCoord(prov, out);
	return 1;
}

int
Game::ProvToWorldCoord(Province* prov, D3DXVECTOR3* out) {

	float prov_x = (float)prov->province_display->unit_x;
	float prov_y = (float)prov->province_display->unit_y;
	//PointF *map = &overlay_data.adjusted_map_origin;
	MapOrigin *map = GetMapOrigInfo();

	//where is the province on the current map with respect to world origin X
	D3DXVECTOR3 right_world_coord;
	D3DXVECTOR3 left_world_coord;

	//right side
	if (prov_x >= map->x) {
		right_world_coord.x = prov_x - map->x;
		left_world_coord.x = prov_x - map->x - 5616.0f;
	}
	//left side
	else {
		left_world_coord.x = prov_x - map->x;
		right_world_coord.x = 5616.0f - map->x + prov_x;
	}

	//if right world is closer than left
	if (abs(right_world_coord.x) <= abs(left_world_coord.x)) {
		out->x = right_world_coord.x;
	}
	//if left world is shorter than right
	else {
		out->x = left_world_coord.x;
	}
	
	out->z = prov_y - map->y;
	out->y = 1.0f;

	return 1;
}

int GetGeneralTraitModsSum(General *general, TraitModsSum *out) {

	//caller responsible for zeroing out

	//sum up background
	for (int i = 1; i < sizeof(TraitModsSum) / sizeof(int); i++) {
		((int*)out)[i] += ((int*) (general->background->tm_list))[i * 2];
	}

	//sum up personality
	for (int i = 1; i < sizeof(TraitModsSum) / sizeof(int); i++) {
		((int*)out)[i] += ((int*) (general->personality->tm_list))[i * 2];
	}

	//sum prestige mod
	((int*)out)[TM_MORALE] += general->morale_magnitude;
	((int*)out)[TM_ORGANIZATION] += general->organization_magnitude;

	return 1;
}

int GetSingleGeneralTraitModsSum(General *general, TraitModType type, int *out) {
	//caller responsible for zeroing out

	*out += ((int*) (general->background->tm_list))[type * 2];
	*out += ((int*) (general->personality->tm_list))[type * 2];

	if (type == TM_MORALE) {
		*out += general->morale_magnitude;
	}

	if (type == TM_ORGANIZATION) {
		*out += general->organization_magnitude;
	}

	return 1;
}

char* GetGeneralName(General *general) {
	if (general->name_len >= 16) {
		return general->name_ptr;
	}
	return general->name;
}

char* GetArmyName(Army *army) {
	if (army->name_len >= 16) {
		return army->name_ptr;
	}
	return army->name;
}

char* GetProvinceName(Province* prov) {
	if (prov->province_display->name_len >= 16) {
		return prov->province_display->name_ptr;
	}
	return prov->province_display->name;
}
