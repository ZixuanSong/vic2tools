#pragma once
#include "windows.h"
#include "d3d9.h"
#include "d3dx9.h"
#include <vector>

/*
		GAME STRUCTS
*/

struct Province;
struct Army;

struct GlobalOptions {
	BYTE *fow;
};

struct MapOrigin {
	char pad1[0x320];
	D3DXMATRIX			proj_view_mat;
	char pad2[0x16C];
	float				x;
	float				y;
};

struct CameraInfo {
	char pad1[0x114];
	D3DXVECTOR3			eye;
	D3DXVECTOR3			look_at;
	D3DXVECTOR3			up;
	char pad2[0xAC];
	D3DXMATRIX			view_mat;
	char pad3[0xE4];
	MapOrigin**			map_origin_info;
};

struct GraphicsInfo {
	char pad1[0x60];
	IDirect3D9*			directx_interface;
	IDirect3DDevice9*	device;
	char pad2[0xA0];
	CameraInfo**		camera_info;
};

template <typename T>
struct ListNode {
	T				data;
	ListNode<T>*	next;
	ListNode<T>*	prev;
};

struct ModelInfo {
	char pad1[0x1C0];
	int x;
	int y;
};

struct ScreenInfo {
	char pad1[0xA0];
	ModelInfo* model_info;
};

struct Render {
	char pad1[0x28];
	ScreenInfo* screen_info;
};

struct ProvinceDisplay {
	char pad1[0x18];
	Province*			province;
	char pad2[0x28];
	float				text_x;
	float				text_y;
	char pad3[0x30];
	int 				unit_x;
	int 				unit_y;
	char pad4[0x60];
	union {
		char*			name_ptr;
		char			name[16];
	};
	DWORD				name_len;
	char pad5[0x70];
	Render*				static_render;
	Render*				moving_render;
	char pad6[0xC];
	Render*				dock_render;
};

struct Province {
	char pad1[0x58];
	DWORD				province_id;
	ProvinceDisplay*	province_display;
	char pad2[0x9C];
	ListNode<Army*>*	army_list_tail;
	ListNode<Army*>*	army_list_head;
	DWORD				army_list_size;
	char pad3[0x28];
	char				owner_tag[4];
	DWORD				owner_id;
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

struct TraitMod{
	
};

struct TraitModList {
	int none;
	TraitMod*	tm_none;
	int	attack;
	TraitMod*	tm_attack;
	int defend;
	TraitMod*	tm_defend;
	int morale;
	TraitMod*	tm_morale;
	int org;
	TraitMod*	tm_organization;
	int recon;
	TraitMod*	tm_recon;
	int speed;
	TraitMod*	tm_speed;
	int attrition;
	TraitMod*	tm_attrition;
	int exp;
	TraitMod*	tm_experience;
	int reliability;
	TraitMod*	tm_reliability;
};

struct Trait {
	char pad1[0x18];
	TraitModList*	tm_list;
	char pad2[0x10];
	union {
		char*		name_ptr;
		char		name[16];
	};
	DWORD			name_len;
};

struct General {
	char pad1[0x7C];
	Trait*			personality;
	Trait*			background;
	char pad2[0x4];
	Army*			army;
	char pad3[0x8];
	union {
		char*		name_ptr;
		char		name[16];
	};
	DWORD			name_len;
	char pad4[0xC];
	void*			portrait_addr;
	char pad5[0x20];
	DWORD			prestige;
	int				morale_magnitude;
	TraitMod*		morale_mod;
	char pad6[0x2C];
	int				organization_magnitude;
	TraitMod*		organization_mod;
};

struct Brigade {

};

struct ArmyDisplay {
	char pad1[0x29];
	char				is_within_screen;
	char pad2[0x2A];
	D3DXMATRIX			matrix_1;
	D3DXMATRIX			matrix_2;
	char pad4[0x74];
	DWORD				province_id;
};

struct ArmyVtable{
	char pad1[0x3C];
	char (*is_naval_unit)();
};

struct Army {
	ArmyVtable*				vtable;
	char					is_selected;
	char	pad2[0x3];
	char	pad3[0x30];
	ListNode<Brigade*>*		brigade_tail;
	ListNode<Brigade*>*		brigade_head;
	DWORD					brigade_len;
	char	pad4[0x94];
	General*				general;
	char	pad5[0x8];
	ListNode<DWORD>*		path_list_tail;
	ListNode<DWORD>*		path_list_head;		//destination
	DWORD					path_list_size;
	char	pad6[0x1C];
	ArmyDisplay*			display_addr;
	void*				select_list_ptr;
	union {
		char*			name_ptr;
		char			name[16];
	};
	DWORD				name_len;
};

struct Country {
	char pad1[0x1C];
	char				tag[4];
	DWORD				id;
	char pad2[0x790];
	ListNode<Army*>*	army_tail;
	ListNode<Army*>*	army_head;
	DWORD				army_len;
	char pad3[0x10];
	DWORD				leadership;
	char pad4[0x400];
	ListNode<General*>*	general_tail;
	ListNode<General*>*	general_head;
	DWORD				general_len;
	char pad5[0x25C];
	DWORD				research_point;
	char pad6[0x60];
	DWORD				presitge;
};

struct ClientInfo {
	char	pad1[0xACC];
	DWORD**				province_list_start;
	DWORD**				province_list_end;
	char	pad2[0x88];
	char				client_country_tag[4];
	DWORD				client_country_id;
};

struct CountryList {
	DWORD		len;
	DWORD**		ptr_list_start;
	DWORD**		ptr_list_end;
};

/*
		PROGRAM STRUCTS
*/

struct TraitModsSum {
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

struct ProvinceRenderHitEntry {
	Render*		static_render;
	DWORD		static_last_tick;
	Render*		moving_render;
	DWORD		moving_last_tick;
	Render*		dock_render;
	DWORD		dock_last_tick;
};

struct Point {
	int x;
	int y;
};

struct PointF {
	float x;
	float y;
};

/*
enum DisplayType {
	GENERAL_RANK,
	GENERAL_STAT
};

struct DisplayInfo {
	HANDLE			lock;
	DisplayType		type;
	DWORD			arg1;
	DWORD			arg2;
	DWORD			arg3;
};


struct GlobalDisplayList {
	HANDLE lock;
	std::vector<DisplayInfo*> display_info_list;
};

struct GeneralStatInternalEntry {
	char on_screen;
	short moving;
	int attack;
	int defense;
	int prestige;
};

struct GeneralStatDisplayEntry {
	char	attack[2];
	char	defense[2];
	DWORD	prestige_len;
	char	prestige[8];
	Point coord;
};

struct ThreadControl {
	bool			run;	//signal thread running/stopping
	void*			game;	//game object
	void*			disp;	//display object _out_
};*/
