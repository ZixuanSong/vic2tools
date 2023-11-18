#pragma once
#include "D3d9.h"
#include "d3dx9.h"
#include "game.h"

#define GENERAL_RANK_DISP_LIMIT 5
#define GENERAL_RANK_FONT_HEIGHT 20

#define BANNER_HEIGHT 90

#define GENERAL_STAT_FONT_HEIGHT 15

#define GENERAL_PRESTIGE_OFFSET 20	//offset from defend text start
#define GENERAL_DEFEND_OFFSET 20	//offset from the origin

struct OverlayData {
	
	//general
	D3DXMATRIX		identity;
	PointF			adjusted_map_origin;

	//game gfx related
	RECT			screen_rect;
	D3DVIEWPORT9	view_port;
	float			fov_radian;
	float			aspect_ratio;
	D3DXMATRIX		view_matrix;
	D3DXMATRIX		proj_matrix;


	//general rank
	LPD3DXFONT		general_rank_font;
	RECT			general_rank_rect;

	//general stat
	LPD3DXFONT		general_stat_font;

	//army path
	ID3DXLine*		path_line;

	//tick counter
	LPD3DXFONT		tick_font;
	RECT			tick_rect;
};

struct OverlayOpts {
	bool	general_rank;
	bool	general_stat_all;
	bool	general_stat_player;
	bool	army_path_all;
	bool	army_path_visible;
	bool	gfx_debug;
};

extern OverlayData overlay_data;
extern OverlayOpts overlay_opts;
//extern GlobalDisplayList g_disp_list;

void ReHook();
int HookVTableEntry(DWORD*, DWORD);
HRESULT _stdcall MyEndScene(IDirect3DDevice9*);

int OverlayInit(GraphicsInfo*);
int FixToScreenBound(Point*);
bool WithinScreen(D3DXVECTOR2&);
void PrintMatrix(D3DMATRIX&);
void WorldToScreen(D3DXVECTOR2*, D3DXVECTOR3&);
void WorldToScreenEx(D3DXVECTOR3*, D3DXVECTOR3&, D3DVIEWPORT9&, D3DMATRIX&, D3DMATRIX&);


