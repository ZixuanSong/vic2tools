#include "pch.h"

#include "game.h"
#include "syscallhelper.h"

#include <iostream>
#include <cmath>
#include <string>


void GameInfo::Init() {
	process_handle = nullptr;
	DWORD process_id;

	printf("Looking for v2game.exe...");
	while (process_handle == nullptr) {
		GetProcessHandle(&process_handle, &process_id);
	}

	GetModuleInfo(process_id, (WCHAR *)L"v2game.exe", &v2game);
	GetModuleInfo(process_id, (WCHAR *)L"kernel32.dll", &kernel32);

	fow_offset = 0xB092FB;
	PrvStrToIdxFunc_offset = 0x6AF0C5;
	CountryList_offset = 0xE587E4;
	client_offset = 0xE588E8;
	army_display_offset = 0x10C;

	trait_mod_names[0] = "NON";
	trait_mod_names[1] = "ATT";
	trait_mod_names[2] = "DEF";
	trait_mod_names[3] = "MOR";
	trait_mod_names[4] = "ORG";
	trait_mod_names[5] = "REC";
	trait_mod_names[6] = "SPD";
	trait_mod_names[7] = "ATR";		
	trait_mod_names[8] = "EXP";
	trait_mod_names[9] = "REL";

	LoadClientInfo();
	LoadProvinceList();
	LoadCountryList();
}

GameInfo::~GameInfo() {
	CloseHandle(process_handle);
}

void GameInfo::LoadClientInfo() {
	DWORD addr;
	ReadProcessMemory(process_handle, (LPVOID) (v2game.base + client_offset), &addr, sizeof(DWORD), NULL);
	ReadProcessMemory(process_handle, (LPVOID) addr, &client_info, sizeof(client_info), NULL);
}

void GameInfo::LoadProvinceList() {
	province_list.len = (client_info.province_list_addr_end - client_info.province_list_addr_start) / 4;
	province_list.ptr_list = (DWORD*)HeapAlloc(GetProcessHeap(), 0, sizeof(DWORD) * province_list.len);
	ReadProcessMemory(process_handle, (LPVOID) client_info.province_list_addr_start, province_list.ptr_list, sizeof(DWORD) * province_list.len, NULL);
	render_hit_list.resize(province_list.len);

	ProvinceRenderHitInfo empty;
	empty.static_address = 0;
	empty.static_hits = 0;
	empty.moving_address = 0;
	empty.moving_hits = 0;
	std::fill(render_hit_list.begin(), render_hit_list.end(), empty);
}

void GameInfo::LoadCountryList() {
	DWORD list_addr;
	ReadProcessMemory(process_handle, (LPVOID) (v2game.base + CountryList_offset), &list_addr, sizeof(DWORD), NULL);
	ReadProcessMemory(process_handle, (LPVOID) list_addr, &country_list, sizeof(country_list) - 4, NULL);

	country_list.ptr_list = (DWORD*) HeapAlloc(GetProcessHeap(), 0, sizeof(DWORD) * country_list.len);
	ReadProcessMemory(process_handle, (LPVOID)country_list.ptr_list_addr, country_list.ptr_list, sizeof(DWORD) * country_list.len, NULL);
}

void GameInfo::LoadCountry(DWORD index, Country *out) {
	ReadProcessMemory(process_handle, (LPVOID) country_list.ptr_list[index], out, sizeof(Country), NULL);
}

void GameInfo::LoadCountryEx(DWORD index, void *out, DWORD offset, DWORD size) {
	ReadProcessMemory(process_handle, (LPVOID)(country_list.ptr_list[index] + offset), out, size, NULL);
}

void GameInfo::LoadProvince(DWORD id, Province *out) {

	DWORD province_addr = province_list.ptr_list[id];
	ReadProcessMemory(process_handle, (LPVOID)province_addr, out, sizeof(Province), NULL);
}

void GameInfo::LoadProvinceDisplayById(DWORD id, ProvinceDisplay *out) {
	Province prov_buff;
	LoadProvince(id, &prov_buff);
	LoadProvinceDisplayByProvince(prov_buff, out);
}

void GameInfo::LoadProvinceDisplayByProvince(Province& province, ProvinceDisplay *out) {
	ReadProcessMemory(process_handle, (LPVOID)province.province_display_addr, out, sizeof(ProvinceDisplay), NULL);
}

int 
GameInfo::LoadProvinceStaticScreenPos(DWORD prov_index, ProvinceDisplay& province_display, int *x_out, int *y_out) {
	
	ProvinceRenderHitInfo *rh_entry = &render_hit_list[prov_index];

	if (province_display.render_addr == 0 || province_display.render_addr == 0xBAADF00D) {
		rh_entry->static_address = 0;
		rh_entry->static_hits = 0;
		return -1;
	}

	if (province_display.render_addr != rh_entry->static_address) {
		rh_entry->static_address = province_display.render_addr;
		rh_entry->static_hits = 0;
		return -2;
	}

	if (rh_entry->static_hits < RENDER_VALID_CYCLES) {
		rh_entry->static_hits++;
		return -3;
	}

	if (LoadProvinceScreenPos(province_display.render_addr, x_out, y_out) < 0) {
		printf("screen pos\n");
		return -4;
	}

	return 1;
}

int
GameInfo::LoadProvinceMovingScreenPos(DWORD prov_index, ProvinceDisplay& province_display, int *x_out, int *y_out) {
	
	ProvinceRenderHitInfo *rh_entry = &render_hit_list[prov_index];

	if (province_display.render_addr2 == 0 || province_display.render_addr2 == 0xBAADF00D) {
		rh_entry->moving_address = 0;
		rh_entry->moving_hits = 0;
		return -1;
	}

	if (province_display.render_addr2 != rh_entry->moving_address) {
		rh_entry->moving_address = province_display.render_addr2;
		rh_entry->moving_hits = 0;
		return -2;
	}

	if (rh_entry->moving_hits < RENDER_VALID_CYCLES) {
		rh_entry->moving_hits++;
		return -3;
	}

	if (LoadProvinceScreenPos(province_display.render_addr2, x_out, y_out) < 0) {
		printf("screen pos\n");
		return -4;
	}

	return 1;
}


int 
GameInfo::LoadArmyList(DWORD country_index, Army **out, DWORD *count) {
	char buffer[16];

	LoadCountryEx(country_index, buffer, 0x7B4, 16);

	//do we really need tail????
	DWORD tail_addr = *((DWORD*) &buffer[0]);
	DWORD head_addr = *((DWORD*) &buffer[4]);
	DWORD size = *((DWORD*) &buffer[8]);
	
	DWORD loaded = 0;
	Army *new_list;

	*out = nullptr;
	*count = 0;

	if (size == 0) {
		return 1;
	}	
	
	new_list = (Army*) HeapAlloc(GetProcessHeap(), 0, sizeof(Army) * size);

	//error past this line needs to free list
	
	ListNode curr_node;
	if (!ReadProcessMemory(process_handle, (LPVOID) head_addr, &curr_node, sizeof(ListNode), NULL)) {
		goto fail_free;
	}

	for(;;) {

		//printf("Army: 0x%X\n", curr_node.data_addr);
		if (!ReadProcessMemory(process_handle, (LPVOID) curr_node.data_addr, &new_list[loaded], sizeof(Army), NULL)) {
			goto fail_free;
		}

		loaded++;
		
		if (curr_node.next == 0) {
			break;
		}

		if (!ReadProcessMemory(process_handle, (LPVOID)curr_node.next, &curr_node, sizeof(ListNode), NULL)) {
			goto fail_free;
		}
	}

	*out = new_list;
	*count = loaded;
	return 1;

fail_free:
	HeapFree(GetProcessHeap(), 0, new_list);
	return -1;
}

void GameInfo::LoadArmyName(Army& army, char *out, DWORD *len) {
	if (army.name_len >= 16) {
		ReadProcessMemory(process_handle, (LPVOID)army.name_addr, out, army.name_len, NULL);
	}
	else {
		std::memcpy(out, &army.name[0], army.name_len);
	}
	*len = army.name_len;
	out[army.name_len] = '\0';
}

void GameInfo::LoadArmyDisplay(Army& army,  ArmyDisplay *out) {
	ReadProcessMemory(process_handle, (LPVOID) army.display_addr, out, sizeof(ArmyDisplay), NULL);
}

void GameInfo::LoadArmyInfoEx(Army& army, char *out, DWORD offset, DWORD size) {
	ReadProcessMemory(process_handle, (LPVOID) (army.display_addr + offset), out, sizeof(size), NULL);
}

int
GameInfo::LoadArmyScreenPos(Army& army_struct, int *x_out, int *y_out) {

//find province display
DWORD prov_id;
ArmyDisplay army_display_buffer;

LoadArmyDisplay(army_struct, &army_display_buffer);
prov_id = army_display_buffer.province_id;
if (prov_id <= 0 || prov_id > province_list.len) {
	printf("province id\n");
	return -1;
}

ProvinceDisplay prov_display_buffer;
LoadProvinceDisplayById(prov_id, &prov_display_buffer);

int ret;
if (army_struct.path_list_size > 0) {
	ret = LoadProvinceMovingScreenPos(prov_id, prov_display_buffer, x_out, y_out);
}
else {
	ret = LoadProvinceStaticScreenPos(prov_id, prov_display_buffer, x_out, y_out);
}

if (ret < 0) {
	return -2;
}

return 1;
}

int
GameInfo::LoadGeneral(Army& army, General* out) {
	ReadProcessMemory(process_handle, (LPVOID)army.general_addr, out, sizeof(General), NULL);
	return 1;
}

int
GameInfo::LoadGeneralTraitMods(General& general, TraitMods *out) {

	char buff[0x50];
	DWORD addr_buff;

	std::memset(out, 0, sizeof(TraitMods));
	//get personality mods
	TraitMods personality;
	ReadProcessMemory(process_handle, (LPVOID)(general.personality_addr + 0x18), &addr_buff, sizeof(DWORD), NULL);
	ReadProcessMemory(process_handle, (LPVOID)addr_buff, buff, 0x50, NULL);

	for (int i = 1; i < sizeof(TraitMods) / 4; i++) {
		((int*)out)[i] += *((int*)&buff[i * 8]);
	}

	//get background mods
	TraitMods background;
	ReadProcessMemory(process_handle, (LPVOID)(general.background_addr + 0x18), &addr_buff, sizeof(DWORD), NULL);
	ReadProcessMemory(process_handle, (LPVOID)addr_buff, buff, 0x50, NULL);

	for (int i = 1; i < sizeof(TraitMods) / 4; i++) {
		((int*)out)[i] += *((int*)&buff[i * 8]);
	}


	//sum prestige mod
	((int*)out)[TM_MORALE] += general.morale_magnitude;
	((int*)out)[TM_ORGANIZATION] += general.organization_magnitude;
	return 1;
}

int
GameInfo::FormGeneralModifierString(General& general, WCHAR *out, DWORD *out_len) {

	//std::wstring format_buffer;
	//format_buffer.reserve(128);

	DWORD start = 0;

	/*WCHAR name[64];
	if (general.name_len >= 16) {
		char tmp_name[64];
		ReadProcessMemory(process_handle, (LPVOID)general.name_addr, tmp_name, general.name_len, NULL);
		MultiByteToWideChar(CP_UTF8, 0, tmp_name, general.name_len, out, general.name_len);
	}
	else {
		MultiByteToWideChar(CP_UTF8, 0, general.name, general.name_len, out, general.name_len);
	}


	out[general.name_len] = L'\0';
	start = general.name_len;
	out[start++] = L'\n';*/

	if (general.prestige > 0) {
		start += swprintf_s(&out[start], 128, L"Prestige: %.2f%%\n", (float)general.prestige / 10);
	}

	TraitMods trait_mods;
	LoadGeneralTraitMods(general, &trait_mods);

	start += swprintf_s(&out[start], 128, L"ATT: %d    DEF: %d\n", trait_mods.attack / 1000, trait_mods.defend / 1000);

	for (int i = TM_MORALE; i < 10; i++) {
		if (((int*)&trait_mods)[i] != 0) {
			start += swprintf_s(&out[start], 128, L"%.3S: %.2f%%\n", trait_mod_names[i], ((float)((int*)&trait_mods)[i]) / 10);
		}
	}
	out[start] = L'\0';

	

	/*
	
	index = general.name_len + 1;

	std::wstring prestige = std::to_wstring(((float)general.prestige) / 10);
	std::memcpy(&out[index], prestige.data(), prestige.size() * 2);
	index += prestige.size();

	out[index] = L'\0';
	*out_len = index;*/

	
	//printf("%S\n", out);
	//printf("ATT:%d\tDEF:%d\n%d\t%d\t%d\t%d\t%d\t%d\t%d\n", trait_mods.attack, trait_mods.defend, trait_mods.morale,trait_mods.org,trait_mods.recon,trait_mods.speed,trait_mods.attrition,trait_mods.exp,trait_mods.reliability);

	return 1;
}

// private-----------------------------------------------------------------

int
GameInfo::LoadProvinceScreenPos(DWORD render_struct_addr, int *x_out, int *y_out) {

	DWORD addr_buff;
	char buff[8];

	//get screen info addr
	ReadProcessMemory(process_handle, (LPVOID)(render_struct_addr + 0x28), &addr_buff, sizeof(DWORD), NULL);

	//get model info addr
	ReadProcessMemory(process_handle, (LPVOID)(addr_buff + 0xA0), &addr_buff, sizeof(DWORD), NULL);

	//load X and Y from model info struct
	ReadProcessMemory(process_handle, (LPVOID)(addr_buff + 0x1C0), &buff, 8, NULL);

	int read_x = *((int*)&buff[0]);
	int read_y = *((int*)&buff[4]);

	//transformation
	read_y += 58;

	
	//validity check
	if (read_x < 0) {
		read_x = 0;
	}

	if (read_y < 0) {
		read_y = 0;
	}

	if (read_x > 1024) {
		read_x = 1024;
	}

	if (read_y > 768) {
		read_y = 768;
	}

	*x_out = read_x;
	*y_out = read_y;
	
	return 1;
}