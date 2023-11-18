#pragma once
#include "structs.h"

#include <vector>

//anti flickering

#define RENDER_VALID_CYCLES 20

struct ProvinceRenderHitInfo {
	DWORD static_address;
	DWORD static_hits;
	DWORD moving_address;
	DWORD moving_hits;
};

struct GameInfo {
	
	//process related
	HANDLE process_handle;
	ModuleInfo v2game;
	ModuleInfo kernel32;

	//game structures
	ClientInfo client_info;
	CountryList country_list;

	ProvinceList province_list;
	std::vector<ProvinceRenderHitInfo> render_hit_list;

	const char *trait_mod_names[10];

	//program settings
	char fow;

	//offsets
	DWORD fow_offset;
	DWORD PrvStrToIdxFunc_offset;
	DWORD CountryList_offset;
	DWORD ProvinceList_offset;
	DWORD client_offset;
	DWORD army_display_offset;
	DWORD army_general_offset;

	~GameInfo();

	void Init();

	void	LoadClientInfo();
	void	LoadProvinceList();
	void	LoadCountryList();

	void	LoadCountry(DWORD, Country*);
	void	LoadCountryEx(DWORD, void*, DWORD, DWORD);

	void	LoadProvince(DWORD, Province*);
	void	LoadProvinceDisplayById(DWORD, ProvinceDisplay*);
	void	LoadProvinceDisplayByProvince(Province&, ProvinceDisplay*);
	int		LoadProvinceStaticScreenPos(DWORD, ProvinceDisplay&, int*, int*);
	int		LoadProvinceMovingScreenPos(DWORD, ProvinceDisplay&, int*, int*);

	//return heap allocated memory. bad practice but to minamize readprocessmemory...
	int		LoadArmyList(DWORD, Army**, DWORD*);

	void	LoadArmyInfoEx(Army&, char*, DWORD, DWORD);
	void	LoadArmyName(Army&, char*, DWORD*);
	void	LoadArmyDisplay(Army&, ArmyDisplay*);
	int		LoadArmyScreenPos(Army&, int*, int*);

	int		LoadGeneral(Army&, General*);
	int		FormGeneralModifierString(General&, WCHAR*, DWORD*);
	int		LoadGeneralTraitMods(General&, TraitMods*);

private:
	int		LoadProvinceScreenPos(DWORD, int*, int*);
};


