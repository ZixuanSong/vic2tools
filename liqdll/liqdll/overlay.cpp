#include "stdafx.h"
#include "overlay.h"

#include <iostream>
#include <algorithm>

#define SCREEN_BOUND_MARGIN 5

OverlayData overlay_data;
OverlayOpts overlay_opts;
//GlobalDisplayList g_disp_list;

//hook related
typedef HRESULT (_stdcall *orig_func) (IDirect3DDevice9*);
orig_func orig_func_ptr = nullptr;
DWORD *g_entry_addr = nullptr;
HANDLE g_hook_thread = nullptr;

int OverlayInit(GraphicsInfo *gfx_info) {

	//setup identity matrix
	D3DXMatrixIdentity(&overlay_data.identity);

	//set up screen resolution
	gfx_info->device->GetViewport(&overlay_data.view_port);
	overlay_data.screen_rect.left = 0;
	overlay_data.screen_rect.top = 0;
	overlay_data.screen_rect.right = overlay_data.view_port.Width;
	overlay_data.screen_rect.bottom = overlay_data.view_port.Height;

	printf("Screen size: %d x %d\n", overlay_data.screen_rect.right, overlay_data.screen_rect.bottom);

	//set up projection matrix
	overlay_data.fov_radian = 0.7850f;
	overlay_data.aspect_ratio = (float) overlay_data.view_port.Width / (float) overlay_data.view_port.Height;
	D3DXMatrixPerspectiveFovLH(&overlay_data.proj_matrix, overlay_data.fov_radian, overlay_data.aspect_ratio, 8.0f, 1200.0f);

	//initialize g disp list
	//g_disp_list.lock = CreateMutex(NULL, false, NULL);
	//g_disp_list.display_info_list.reserve(2);

	//options
	overlay_opts.general_rank = false;
	overlay_opts.general_stat_all = false;
	overlay_opts.general_stat_player = false;
	overlay_opts.gfx_debug = false;
	overlay_opts.army_path_all = false;
	overlay_opts.army_path_visible = false;

	//general stats
	D3DXCreateFont(gfx_info->device, GENERAL_STAT_FONT_HEIGHT, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &overlay_data.general_stat_font);

	//general rank font and rect
	D3DXCreateFont(gfx_info->device, GENERAL_RANK_FONT_HEIGHT, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &overlay_data.general_rank_font);
	overlay_data.general_rank_rect.left = 5; //leave a little margin
	overlay_data.general_rank_rect.top = overlay_data.screen_rect.bottom / 2;
	overlay_data.general_rank_rect.right = overlay_data.screen_rect.right;
	overlay_data.general_rank_rect.bottom = overlay_data.screen_rect.bottom;

	//army path
	D3DXCreateLine(gfx_info->device, &overlay_data.path_line);
	overlay_data.path_line->SetWidth(2.0f);

	//tick font and rect
	D3DXCreateFont(gfx_info->device, 15, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &overlay_data.tick_font);
	overlay_data.tick_rect.left = overlay_data.screen_rect.right - 100;
	overlay_data.tick_rect.top = 5;
	overlay_data.tick_rect.right = overlay_data.screen_rect.right;
	overlay_data.tick_rect.bottom = 20;
	
	return 1;
}

/*
void RenderGeneralRank(IDirect3DDevice9* device, DisplayInfo *disp) {
	
	overlay_data.general_rank_rect.top = overlay_data.screen_rect.bottom - (disp->arg3 * GENERAL_RANK_FONT_HEIGHT);
	//leave a little margin
	overlay_data.general_rank_rect.top -= 5; 
	overlay_data.general_rank_font->DrawTextA(NULL, (char*)disp->arg1, disp->arg2, &overlay_data.general_rank_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
}

void RenderGeneralStat(IDirect3DDevice9* device, DisplayInfo *disp) {
	GeneralStatDisplayEntry *disp_list = (GeneralStatDisplayEntry*)disp->arg1;
	DWORD entry_count = disp->arg2;

	RECT rect_buff;
	
	//printf("%d\t0x%X\n", entry_count, disp_list);
	for (int i = 0; i < entry_count; i++) {

		
		rect_buff.left = disp_list[i].coord.x;
		rect_buff.top = disp_list[i].coord.y;
		rect_buff.right = rect_buff.left + 300;
		rect_buff.bottom = rect_buff.top + GENERAL_STAT_FONT_HEIGHT;

		
		overlay_data.general_stat_font->DrawTextA(NULL, disp_list[i].prestige, disp_list[i].prestige_len, &rect_buff, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 255));
		//printf("%s\n", disp_list[i].prestige);
	
		if (disp_list[i].attack[1] != '0') {
			rect_buff.left += GENERAL_ATTACK_OFFSET;
			if (disp_list[i].attack[0] == '+') {
				overlay_data.general_stat_font->DrawTextA(NULL, disp_list[i].attack, 2, &rect_buff, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
			}
			else {
				overlay_data.general_stat_font->DrawTextA(NULL, disp_list[i].attack, 2, &rect_buff, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
			}
		}
		
		if (disp_list[i].defense[1] != '0') {
			rect_buff.left += GENERAL_DEFEND_OFFSET;
			if (disp_list[i].defense[0] == '+') {
				overlay_data.general_stat_font->DrawTextA(NULL, disp_list[i].defense, 2, &rect_buff, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
			}
			else {
				overlay_data.general_stat_font->DrawTextA(NULL, disp_list[i].defense, 2, &rect_buff, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
			}
		}
	}
}*/

void RenderGeneralRank(IDirect3DDevice9* device) {

	static char* disp_buff = NULL;

	//allocate display buffer
	if (disp_buff == NULL) {
		//2kb of display string should be enough
		disp_buff = (char*)HeapAlloc(GetProcessHeap(), 0, sizeof(char) * 2048);
	}

	static std::vector<General*> free_sort_list;
	static std::vector<General*> assigned_sort_list;

	Country *country;
	game->GetClientCountry(&country);

	DWORD write_idx = 0;
	DWORD new_lines = 0;

	ListNode<General*> *curr_node;

	//country with no generals (rare)
	if (country->general_len == 0) {

		new_lines = 1;
		write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%s\n", "No general");
		disp_buff[write_idx++] = '\0';

		goto display;
	}

	
	free_sort_list.clear();
	assigned_sort_list.clear();
	free_sort_list.reserve(country->general_len);
	assigned_sort_list.reserve(country->general_len);

	//get a list of generals
	curr_node = country->general_head;
	while (curr_node != NULL) {
		if (curr_node->data->army == NULL) {
			free_sort_list.push_back(curr_node->data);
		}
		else {
			assigned_sort_list.push_back(curr_node->data);
		}
		
		curr_node = curr_node->next;
	}

	write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%s\n\n", "Attack:");
	new_lines += 2;

	int tmp_stat;

	//sort by attack
	std::sort(free_sort_list.begin(), free_sort_list.end(), CompareAtt);
	std::sort(assigned_sort_list.begin(), assigned_sort_list.end(), CompareAtt);

	for (int i = 0; i < assigned_sort_list.size(); i++) {
		if (i == GENERAL_RANK_DISP_LIMIT) {
			break;
		}

		tmp_stat = 0;
		GetSingleGeneralTraitModsSum(assigned_sort_list[i], TM_ATTACK, &tmp_stat);
		write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%d %s  (%s)\n", tmp_stat / 1000, GetGeneralName(assigned_sort_list[i]), GetArmyName(assigned_sort_list[i]->army));
		new_lines++;
	}

	write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%c", '\n');
	new_lines++;

	for (int i = 0; i < free_sort_list.size(); i++) {
		if (i == GENERAL_RANK_DISP_LIMIT) {
			break;
		}

		tmp_stat = 0;
		GetSingleGeneralTraitModsSum(free_sort_list[i], TM_ATTACK, &tmp_stat);
		write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%d %s\n", tmp_stat / 1000, GetGeneralName(free_sort_list[i]));
		new_lines++;
	}

	write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "\n%s\n\n", "Defense:");
	new_lines += 3;

	//sort by def
	std::sort(free_sort_list.begin(), free_sort_list.end(), CompareDef);
	std::sort(assigned_sort_list.begin(), assigned_sort_list.end(), CompareDef);

	for (int i = 0; i < assigned_sort_list.size(); i++) {
		if (i == GENERAL_RANK_DISP_LIMIT) {
			break;
		}

		tmp_stat = 0;
		GetSingleGeneralTraitModsSum(assigned_sort_list[i], TM_DEFEND, &tmp_stat);
		write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%d %s (%s)\n", tmp_stat / 1000, GetGeneralName(assigned_sort_list[i]), GetArmyName(assigned_sort_list[i]->army));
		new_lines++;
	}


	write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%c", '\n');
	new_lines++;

	for (int i = 0; i < free_sort_list.size(); i++) {
		if (i == GENERAL_RANK_DISP_LIMIT) {
			break;
		}

		tmp_stat = 0;
		GetSingleGeneralTraitModsSum(free_sort_list[i], TM_DEFEND, &tmp_stat);
		write_idx += sprintf_s(&disp_buff[write_idx], 2048 - write_idx, "%d %s\n", tmp_stat / 1000, GetGeneralName(free_sort_list[i]));
		new_lines++;
	}

	disp_buff[write_idx++] = '\0';

display:
	//set up display rect
	overlay_data.general_rank_rect.top = overlay_data.screen_rect.bottom - (new_lines * GENERAL_RANK_FONT_HEIGHT);
	//leave a little margin
	overlay_data.general_rank_rect.top -= 5;

	overlay_data.general_rank_font->DrawTextA(NULL, disp_buff, write_idx, &overlay_data.general_rank_rect, DT_NOCLIP, D3DCOLOR_ARGB(130, 111, 239, 51));
}

void RenderGeneralStatAll(IDirect3DDevice9* device) {

	Country *country_ptr;
	Army *army_ptr;
	ArmyDisplay *army_disp_ptr;
	ListNode<Army*> *curr_node;

	DWORD country_len = game->country_list->len;
	DWORD **country_list_start = game->country_list->ptr_list_start;

	Point static_coord_buff;
	Point moving_coord_buff;

	int att_buff;
	int def_buff;
	char att_disp[2];
	att_disp[0] = 'A';
	char def_disp[2];
	def_disp[0] = 'D';
	RECT disp_rect;
	Point screen_coord;

	//traverse country list
	for (int i = 0; i < country_len; i++) {
		country_ptr = (Country*)country_list_start[i];
		
		//ignore country with no army
		if (country_ptr->army_len == 0) {
			continue;
		}

		//traverse army
		curr_node = country_ptr->army_head;
		while (curr_node != NULL) {

			army_ptr = curr_node->data;
			army_disp_ptr = army_ptr->display_addr;

			//army stack is not displayed on screen
			if (army_ptr->vtable->is_naval_unit() == 0 && army_disp_ptr->is_within_screen == 0) {
				goto next;
			}

			if (game->GetArmyScreenCoord(army_ptr, &screen_coord) < 0) {
				goto next;
			}
			disp_rect.left = screen_coord.x;
			disp_rect.top = screen_coord.y;

			//format attack
			att_buff = 0;
			GetSingleGeneralTraitModsSum(army_ptr->general, TM_ATTACK, &att_buff);
			if (att_buff != 0) {
				att_disp[1] = '0' + (abs(att_buff) / 1000);
				//no offset from origin
				if (att_buff > 0) {
					overlay_data.general_stat_font->DrawTextA(NULL, att_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
				}
				else if (att_buff < 0) {
					overlay_data.general_stat_font->DrawTextA(NULL, att_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
				}
			}

			//format defense
			def_buff = 0;
			GetSingleGeneralTraitModsSum(army_ptr->general, TM_DEFEND, &def_buff);
			if (def_buff != 0) {
				def_disp[1] = '0' + (abs(def_buff) / 1000);
				//offset
				disp_rect.left += GENERAL_DEFEND_OFFSET;
				if (def_buff >= 0) {
					overlay_data.general_stat_font->DrawTextA(NULL, def_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
				}
				else {
					overlay_data.general_stat_font->DrawTextA(NULL, def_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
				}
			}

		next:
			curr_node = curr_node->next;
		}
	}
}

void RenderGeneralStatPlayer(IDirect3DDevice9* device) {
	Country *country_ptr = (Country*)game->country_list->ptr_list_start[game->client_info->client_country_id];
	Army *army_ptr;
	ArmyDisplay *army_disp_ptr;
	ListNode<Army*> *curr_node;

	Point static_coord_buff;
	Point moving_coord_buff;

	int att_buff;
	int def_buff;
	char att_disp[2];
	att_disp[0] = 'A';
	char def_disp[2];
	def_disp[0] = 'D';
	RECT disp_rect;
	Point screen_coord;

	//player has no army
	if (country_ptr->army_len == 0) {
		return;
	}

	//traverse army
	curr_node = country_ptr->army_head;
	while (curr_node != NULL) {

		army_ptr = curr_node->data;
		army_disp_ptr = army_ptr->display_addr;

		//army stack is not displayed on screen
		if (army_ptr->vtable->is_naval_unit() == 0 && army_disp_ptr->is_within_screen == 0) {
			goto next;
		}

		if (game->GetArmyScreenCoord(army_ptr, &screen_coord) < 0) {
			goto next;
		}
		disp_rect.left = screen_coord.x;
		disp_rect.top = screen_coord.y;

		//format attack
		att_buff = 0;
		GetSingleGeneralTraitModsSum(army_ptr->general, TM_ATTACK, &att_buff);
		if (att_buff != 0) {
			att_disp[1] = '0' + (abs(att_buff) / 1000);
			//no offset from origin
			if (att_buff > 0) {
				overlay_data.general_stat_font->DrawTextA(NULL, att_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
			}
			else if (att_buff < 0) {
				overlay_data.general_stat_font->DrawTextA(NULL, att_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
			}
		}

		//format defense
		def_buff = 0;
		GetSingleGeneralTraitModsSum(army_ptr->general, TM_DEFEND, &def_buff);
		if (def_buff != 0) {
			def_disp[1] = '0' + (abs(def_buff) / 1000);
			//offset
			disp_rect.left += GENERAL_DEFEND_OFFSET;
			if (def_buff >= 0) {
				overlay_data.general_stat_font->DrawTextA(NULL, def_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));
			}
			else {
				overlay_data.general_stat_font->DrawTextA(NULL, def_disp, 2, &disp_rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
			}
		}

	next:
		curr_node = curr_node->next;
	}
	
}

void RenderGfxDebug(IDirect3DDevice9* device) {

	char buff[4];
	buff[0] = '=';

	//CameraInfo* cam = game->GetCamera();
	//D3DXMatrixLookAtLH(&overlay_data.view_matrix, &cam->eye, &cam->look_at, &cam->up);

	//MapOrigin *map_origin = game->GetMapOrigInfo();
	Province* konigsberg = (Province*) game->client_info->province_list_start[3123];

	D3DXVECTOR3 obj_coord;

	obj_coord.x = konigsberg->province_display->unit_x;
	obj_coord.y = 0;
	obj_coord.z = konigsberg->province_display->unit_y;

	//D3DXVECTOR3 screen_coord;

	/*D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX proj;
	device->GetTransform(D3DTS_WORLD, &world);
	device->GetTransform(D3DTS_VIEW, &view);
	device->GetTransform(D3DTS_PROJECTION, &proj);

	D3DXVec3Project(&screen_coord, &obj_coord, &overlay_data.view_port, (const D3DXMATRIX*)&proj, (const D3DXMATRIX*)&view, (const D3DXMATRIX*)&world);*/

	Army *head = ((Country*)game->country_list->ptr_list_start[game->client_info->client_country_id])->army_head->data;
	if (!strcmp(GetArmyName(head), "billwilson")) {
		memcpy(buff, GetArmyName(head), head->name_len);
		printf("%s\n", buff);
	}

	D3DXVECTOR2 screen_coord;
	ArmyDisplay* army_disp = head->display_addr;
	obj_coord.x = army_disp->matrix_2._41;
	obj_coord.y = army_disp->matrix_2._42;
	obj_coord.z = army_disp->matrix_2._43;

	WorldToScreen(&screen_coord, obj_coord);

	printf("X: %d, Y: %d\n", screen_coord.x, screen_coord.y);

	RECT rect;

	rect.left =	screen_coord.x;
	rect.top = screen_coord.y;
	rect.right = rect.left + 50;
	rect.bottom = rect.top + 50;
	overlay_data.general_rank_font->DrawTextA(NULL, "X", 1, &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
}

void RenderArmyPathAll(IDirect3DDevice9* device) {

	//if camera doesn't show units dont consider render army path
	if (game->GetCamera()->eye.y > 500.0f) {
		return;
	}

	static D3DXVECTOR2 path_nodes[100];

	Country *country_ptr;
	DWORD country_len = game->country_list->len;
	DWORD **country_list_start = game->country_list->ptr_list_start;
	D3DXVECTOR3 world_coord;
	
	overlay_data.path_line->Begin();

	for (int i = 0; i < country_len; i++) {
		country_ptr = (Country*)country_list_start[i];

		if (country_ptr->army_len == 0) {
			continue;
		}

		ListNode<Army*> *curr_node = country_ptr->army_head;
		DWORD path_nodes_size;

		Point screen_buff;
		while (curr_node != NULL) {
			if (curr_node->data->path_list_size == 0) {
				goto next;
			}

			game->GetArmyPathNodes(curr_node->data, path_nodes, 50, 0, &path_nodes_size);

			//no path was populated 
			if (path_nodes_size == 0) {
				goto next;
			}

			/*
			printf("Size: %d\n", path_nodes_size);
			for (int i = 0; i < path_nodes_size; i++) {
				printf("X: %.2f, Y: %.2f", path_nodes[i].x, path_nodes[i].y);
			}
			*/

			if (curr_node->data->vtable->is_naval_unit()) {
				overlay_data.path_line->Draw(path_nodes, path_nodes_size, D3DCOLOR_ARGB(70, 38, 244, 255));
			}
			else {
				overlay_data.path_line->Draw(path_nodes, path_nodes_size, D3DCOLOR_ARGB(110, 252, 211, 30));
			}

			
			game->ProvToWorldCoordById(curr_node->data->path_list_head->data, &world_coord);
			WorldToScreen(&path_nodes[0], world_coord);
			if (WithinScreen(path_nodes[0])) {
				path_nodes[1].x = path_nodes[0].x + 6.0f;
				path_nodes[1].y = path_nodes[0].y + 6.0f;
				path_nodes[0].x = path_nodes[0].x - 6.0f;
				path_nodes[0].y = path_nodes[0].y - 6.0f;

				overlay_data.path_line->Draw(path_nodes, 2, D3DCOLOR_ARGB(200, 0, 0, 255));

				path_nodes[0].y += 12.0f;
				path_nodes[1].y -= 12.0f;
				overlay_data.path_line->Draw(path_nodes, 2, D3DCOLOR_ARGB(200, 0, 0, 255));
			}

		next:
			curr_node = curr_node->next;
		}
	}

	overlay_data.path_line->End();
	
}

void RenderArmyPathVisible(IDirect3DDevice9* device) {

	if (game->GetCamera()->eye.y > 500.0f) {
		return;
	}

	static D3DXVECTOR2 path_nodes[4];

	Country *country_ptr;
	DWORD country_len = game->country_list->len;
	DWORD **country_list_start = game->country_list->ptr_list_start;
	D3DXVECTOR3 world_coord;

	overlay_data.path_line->Begin();

	for (int i = 0; i < country_len; i++) {

		//don't care about player armies
		if (i == game->client_info->client_country_id) {
			continue;
		}

		country_ptr = (Country*)country_list_start[i];

		if (country_ptr->army_len == 0) {
			continue;
		}

		ListNode<Army*> *curr_node = country_ptr->army_head;
		DWORD path_nodes_size;

		Point screen_buff;
		while (curr_node != NULL) {
			if (curr_node->data->path_list_size == 0) {
				goto next;
			}

			//don't draw armies can't be seen
			if (!(curr_node->data->display_addr->is_within_screen)) {
				goto next;
			}

			game->GetArmyPathNodes(curr_node->data, path_nodes, 50, 1, &path_nodes_size);

			//no path was populated 
			if (path_nodes_size == 0) {
				goto next;
			}

			if (curr_node->data->vtable->is_naval_unit()) {
				overlay_data.path_line->Draw(path_nodes, path_nodes_size, D3DCOLOR_ARGB(70, 38, 244, 255));
			}
			else {
				overlay_data.path_line->Draw(path_nodes, path_nodes_size, D3DCOLOR_ARGB(110, 252, 211, 30));
			}

			//destination is the first node of path list tail
			game->ProvToWorldCoordById(curr_node->data->path_list_tail->data, &world_coord);
			WorldToScreen(&path_nodes[0], world_coord);
			if (WithinScreen(path_nodes[0])) {
				path_nodes[1].x = path_nodes[0].x + 6.0f;
				path_nodes[1].y = path_nodes[0].y + 6.0f;
				path_nodes[0].x = path_nodes[0].x - 6.0f;
				path_nodes[0].y = path_nodes[0].y - 6.0f;

				overlay_data.path_line->Draw(path_nodes, 2, D3DCOLOR_ARGB(200, 0, 0, 255));

				path_nodes[0].y += 12.0f;
				path_nodes[1].y -= 12.0f;
				overlay_data.path_line->Draw(path_nodes, 2, D3DCOLOR_ARGB(200, 0, 0, 255));
			}

		next:
			curr_node = curr_node->next;
		}
	}

	overlay_data.path_line->End();
}

void RenderCost(IDirect3DDevice9* device, DWORD tick) {

	static DWORD last_cost = 0;
	char buff[32];
	int len;

	if (tick != 0) {
		len = sprintf_s(buff, 32, "Ticks: %d", tick);
		last_cost = tick;
	}
	else {
		len = sprintf_s(buff, 32, "Ticks: %d", last_cost);
	}

	overlay_data.tick_font->DrawTextA(NULL, buff, len, &overlay_data.tick_rect, DT_NOCLIP, D3DCOLOR_ARGB(150, 255, 0, 0));
}

HRESULT _stdcall MyEndScene(IDirect3DDevice9* device) {

	/*
		Called every game loop cycle
		Executes within game loop, watch for performance hit!
	*/

	DWORD tick = GetTickCount();

	if (overlay_opts.general_stat_all) {
		RenderGeneralStatAll(device);
	}
	else if (overlay_opts.general_stat_player) {
		RenderGeneralStatPlayer(device);
	}

	if (overlay_opts.army_path_all) {
		RenderArmyPathAll(device);
	}
	else if (overlay_opts.army_path_visible) {
		RenderArmyPathVisible(device);
	}

	if (overlay_opts.general_rank) {
		RenderGeneralRank(device);
	}

	if (overlay_opts.gfx_debug) {
		RenderGfxDebug(device);
	}

	//cost counter
	tick = GetTickCount() - tick;
	RenderCost(device, tick);

	return orig_func_ptr(device);

	//can't aquire lock for global display list
	/*if (WaitForSingleObject(g_disp_list.lock, 0) == WAIT_TIMEOUT) {
		return orig_func_ptr(device);
	}

	//aquired g disp list lock
	DisplayInfo *disp_info;
	for (int i = 0; i < g_disp_list.display_info_list.size(); i++) {
		disp_info = g_disp_list.display_info_list[i];

		//can't aquire lock for this display info
		if (WaitForSingleObject(disp_info->lock, 0) == WAIT_TIMEOUT) {
			continue;
		}
		
		//aquired disp lock
		switch (disp_info->type) {

		case GENERAL_RANK:
			RenderGeneralRank(device, disp_info);
			break;

		case GENERAL_STAT:
			RenderGeneralStat(device, disp_info);
			break;
		}

		ReleaseMutex(disp_info->lock);
	}

	ReleaseMutex(g_disp_list.lock);*/
	
}

void ReHook() {
	for (;;) {
		if (*g_entry_addr != (DWORD)&MyEndScene) {
			DWORD old_protection;
			int ret;
			ret = VirtualProtect((LPVOID)g_entry_addr, 4, PAGE_EXECUTE_READWRITE, &old_protection);

			*g_entry_addr = (DWORD)&MyEndScene;
			VirtualProtect((LPVOID)g_entry_addr, 4, old_protection, NULL);
		}
		Sleep(20);
	}
}

int HookVTableEntry(DWORD *entry_addr, DWORD orig_func_addr) {
	
	g_entry_addr = entry_addr;

	DWORD old_protection;
	int ret;
	ret = VirtualProtect((LPVOID)entry_addr, 4, PAGE_EXECUTE_READWRITE, &old_protection);
	if (ret < 0) {
		printf("VIrtual Protect failed: %d\n", GetLastError());
		return -1;
	}

	printf("Overwriting 0x%X at 0x%X to 0x%X\n", *entry_addr, entry_addr, &MyEndScene);
	//if (orig_func_ptr == nullptr) {
		orig_func_ptr = (orig_func)orig_func_addr;
	//}
	*entry_addr = (DWORD) &MyEndScene;
	VirtualProtect((LPVOID)entry_addr, 4, old_protection, NULL);

	//start rehook thread
	if (g_hook_thread == nullptr) {
		g_hook_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ReHook, NULL, 0, NULL);
	}
	return 1;
}

int FixToScreenBound(Point* out) {
	if (out->x < 0) {
		out->x = 0;
	}

	if (out->x > overlay_data.screen_rect.right) {
		out->x = overlay_data.screen_rect.right;
	}

	if (out->y < 0) {
		out->y = 0;
	}

	if (out->y > overlay_data.screen_rect.bottom) {
		out->y = overlay_data.screen_rect.bottom;
	}
	return 1;
}

bool WithinScreen(D3DXVECTOR2& coord) {
	
	if (coord.x >= 0 - SCREEN_BOUND_MARGIN 
		&& coord.y >= SCREEN_BOUND_MARGIN 
		&& coord.x <= overlay_data.view_port.Width + SCREEN_BOUND_MARGIN 
		&& coord.y <= overlay_data.view_port.Height + SCREEN_BOUND_MARGIN) {
		return true;
	}

	return false;
}

void PrintMatrix(D3DMATRIX& mat) {
	for (int i = 0; i < 4; i++) {
		printf("[");
		for (int j = 0; j < 4; j++) {
			printf("%.2f\t", mat.m[i][j]);
		}
		printf("]\n");
	}
}

void WorldToScreen(D3DXVECTOR2 *out, D3DXVECTOR3& world_coord) {
	
	D3DXVECTOR3 tmp_out;
	D3DXVec3TransformCoord(&tmp_out, &world_coord, &(game->GetMapOrigInfo()->proj_view_mat));
	float x_half = overlay_data.view_port.Width / 2.0f;
	float y_half = overlay_data.view_port.Height / 2.0f;

	out->x = overlay_data.view_port.X + (1.0f + tmp_out.x) * x_half;
	out->y = overlay_data.view_port.Y + (1.0f - tmp_out.y) * y_half;
	
}

void WorldToScreenEx(D3DXVECTOR3 *out, D3DXVECTOR3& world_coord, D3DVIEWPORT9& view_port, D3DMATRIX& proj_matrix, D3DMATRIX& view_matrix) {

	D3DXMATRIX view_proj;
	D3DXVECTOR3 tmp_out;

	D3DXMatrixMultiply(&view_proj, (const D3DXMATRIX*)&view_matrix, (const D3DXMATRIX*)&proj_matrix);
	D3DXVec3TransformCoord(&tmp_out, &world_coord, &view_proj);

	//printf("--\n");
	//PrintMatrix(view_proj);

	float x_half = view_port.Width / 2.0f;
	float y_half = view_port.Height / 2.0f;

	out->x = view_port.X + (1.0f + tmp_out.x) * x_half;
	out->y = view_port.Y + (1.0f - tmp_out.y) * y_half;
	//out->z = 0;
}