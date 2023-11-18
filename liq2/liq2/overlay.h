#pragma once

#include "d3d9.h"
#include "d3dx9.h"
#include "dwmapi.h"
#include "windows.h"

enum DisplayType {
	GENERAL_LIST
};

struct DisplayInfo {
	DisplayType		type;
	void*			data_buffer;
	void*			data_buffer2;
	int				arg1;
	int				arg2;
};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int OverlayInit();
int OverlayShow(DisplayInfo*, ThreadStruct*);