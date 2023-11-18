#pragma once

#include "Windows.h"
#include <vector>
#include <string>

struct ListNode {
	DWORD		data_addr;
	ListNode*	next;
	ListNode*	prev;
};

enum TraitModType {
	TM_NONE = 0,
	TM_ATTACK,
	TM_DEFEND,
	TM_MORALE,
	TM_ORGANIZATION,
	TM_RECON,
	TM_SPEED,
	TM_ATTRITION,
	TM_EXPERIENCE,
	TM_RELIABILITY
};

struct TraitMods {
	int none;
	int	attack;
	int defend;
	int morale;
	int org;
	int recon;
	int speed;
	int attrition;
	int exp;
	int reliability;
};

struct Trait {

};

struct General {
	char pad1[0x7C];
	DWORD			personality_addr;
	DWORD			background_addr;
	char pad2[0x4];
	DWORD			army_addr;
	char pad3[0x8];
	union {
		DWORD		name_addr;
		char		name[16];
	};
	DWORD			name_len;
	char pad4[0xC];
	DWORD			portrait_addr;
	char pad5[0x20];
	DWORD			prestige;
	int				morale_magnitude;
	DWORD			morale_mod_addr;
	char pad6[0x2C];
	int				organization_magnitude;
	DWORD			organization_mod_addr;
};

struct Brigade {

};

struct ArmyDisplay {
	char pad1[0x29];
	char				is_drawn;
	char pad2[0x2A];
	DWORD				matrix_1[16];
	DWORD				matrix_2[16];
	char pad4[0x74];
	DWORD province_id;
};

struct Army {
	char	pad1[0x4];
	char				is_selected;
	char	pad2[0x3];
	char	pad3[0x30];
	DWORD				brigade_tail;
	DWORD				brigade_head;
	DWORD				brigade_len;
	char	pad4[0x94];
	DWORD				general_addr;
	char	pad5[0x8];
	DWORD				path_list_tail;
	DWORD				path_list_head;
	DWORD				path_list_size;
	char	pad6[0x1C];
	DWORD				display_addr;
	char*				select_list_ptr;
	union {
		DWORD			name_addr;
		char			name[16];
	};
	DWORD				name_len;
};

struct Country {
	char pad1[0x1C];
	char				tag[4];
	DWORD				id;
	char pad2[0x790];
	DWORD				army_tail;
	DWORD				army_head;
	DWORD				army_len;
	char pad3[0x10];
	DWORD				leadership;
	char pad4[0x400];
	DWORD				general_tail;
	DWORD				general_head;
	DWORD				general_len;
	char pad5[0x25C];
	DWORD				research_point;
	char pad6[0x60];
	DWORD				presitge;
};

struct CountryList {
	DWORD	len;
	DWORD*	ptr_list_addr;
	DWORD*	ptr_list;

	CountryList() :
		ptr_list(nullptr)
	{
	}

	~CountryList() {
		if (ptr_list != nullptr) {
			HeapFree(GetProcessHeap(), 0, ptr_list);
			ptr_list = nullptr;
		}
	}
};

struct ProvinceDisplay {
	char pad1[0x18];
	DWORD				province_addr;
	char pad2[0xC8];
	union {
		DWORD			name_addr;
		char			name[16];
	};
	DWORD				name_len;
	char pad3[0x70];
	DWORD				render_addr;
	DWORD				render_addr2;
};

struct Province {
	char pad1[0x58];
	DWORD		province_id;
	DWORD		province_display_addr;
	char pad2[0x9C];
	DWORD		army_list_tail;
	DWORD		army_list_head;
	DWORD		army_list_size;
	char pad3[0x28];
	DWORD		owner_tag;
	DWORD		owner_id;
};

struct ProvinceList {
	DWORD	len;
	DWORD*	ptr_list;

	ProvinceList() :
		ptr_list(nullptr)
	{
	}

	~ProvinceList() {
		if (ptr_list != nullptr) {
			HeapFree(GetProcessHeap(), 0, ptr_list);
			ptr_list = nullptr;
		}
	}
};

struct ClientInfo {
	char	pad1[0xACC];
	DWORD	province_list_addr_start;
	DWORD	province_list_addr_end;
	char	pad2[0x88];
	char	client_country_tag[4];
	DWORD	client_country_id;
};

struct Coord {
	int x;
	int y;
};

struct GeneralDisplayData {
	WCHAR	display_string[128];
	DWORD	display_string_len;
	Coord	coord;
};



struct ModuleInfo {
	DWORD base;
	HMODULE handle;
};

struct ThreadStruct {
	DWORD		status;
	HANDLE		lock;
	void*		game;
	void*		data;
};
