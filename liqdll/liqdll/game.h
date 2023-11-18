#pragma once

#include "structs.h"
#include "overlay.h"

//cycles of general stat worker must achive in order to have render address be valid
#define VALID_RENDER_COUNT	200

//worker threads
//int GeneralRankWorker(void*);
//int GeneralStatWorker(void*);

struct Game {

	GraphicsInfo*							graphics_info;
	GlobalOptions							global_opts;

	ClientInfo*								client_info;
	CountryList*							country_list;
	DWORD									prov_size;

	ProvinceRenderHitEntry*					render_hit_list;

	int Init(DWORD);
	int GetClientCountry(Country**);
	int GetCountryById(DWORD, Country**);

	int GetArmyScreenCoord(Army*, Point*);
	int ArmyToScreenCoord(Army*, D3DXVECTOR2*);
	int GetArmyPathNodes(Army*, D3DXVECTOR2*, DWORD, DWORD, DWORD*);

	int GetStaticProvScreenPos(Province*, Point*);
	int GetMovingProvScreenPos(Province*, Point*);
	int GetDockProvScreenPos(Province*, Point*);

	CameraInfo *GetCamera() {
		return *(graphics_info->camera_info);
	}

	MapOrigin *GetMapOrigInfo() {
		return *(GetCamera()->map_origin_info);
	}

	int ProvToScreen(Province*, D3DXVECTOR2*);
	int ProvToWorldCoord(Province*, D3DXVECTOR3*);
	int ProvToWorldCoordById(DWORD, D3DXVECTOR3*);
};

extern Game *game;

bool CompareAtt(General*, General*);
bool CompareDef(General*, General*);

int GetGeneralTraitModsSum(General*, TraitModsSum*);
int GetSingleGeneralTraitModsSum(General*, TraitModType, int*);


char* GetGeneralName(General*);
char* GetArmyName(Army*);
char* GetProvinceName(Province*);