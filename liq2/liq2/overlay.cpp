#include "pch.h"
#include "windows.h"
#include "structs.h"
#include "overlay.h"

#include <iostream>

#define CENTERX (GetSystemMetrics(SM_CXSCREEN)/2)-(s_width/2)
#define CENTERY (GetSystemMetrics(SM_CYSCREEN)/2)-(s_height/2)

int s_width = 800;
int s_height = 600;
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;
HWND hWnd;
const MARGINS  margin = { 0,0,s_width,s_height };
LPD3DXFONT pFont;
WNDCLASSEX window_class;

void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = s_width;    // set the width of the buffer
	d3dpp.BackBufferHeight = s_height;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	D3DXCreateFont(d3ddev, 15, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFont);
}

void DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, LPWSTR str)
{
	RECT FontPos = { x, y, x + 2, y + 2 };
	g_pFont->DrawText(NULL, str, -1, &FontPos, DT_NOCLIP, color);
}

void DrawX(int x, int y, DWORD color, LPD3DXFONT g_pFont)
{
	RECT FontPos = { x, y, x + 1, y + 1 };
	g_pFont->DrawText(NULL, L"X", 1, &FontPos, DT_NOCLIP, color);
}


void render(DisplayInfo *display_info)
{
	d3ddev->BeginScene();    // begins the 3D scene
	
	switch (display_info->type) {
	case GENERAL_LIST: 
	{
		GeneralDisplayData *general_display_list = (GeneralDisplayData*)display_info->data_buffer;
		
		//printf("%d\n", display_info->arg1);
		for (DWORD i = 0; i < display_info->arg1; i++) {
			//printf("%d\t%d\n", general_display_list->coord.x, general_display_list->coord.y);
			//DrawX(general_display_list[i].coord.x, general_display_list[i].coord.y, D3DCOLOR_ARGB(255, 255, 0, 0), pFont);
			DrawString(general_display_list[i].coord.x, general_display_list[i].coord.y, D3DCOLOR_ARGB(255, 255, 0, 0), pFont, general_display_list[i].display_string);
		}
		break;
	}
	}

	//DrawString(10, 300, D3DCOLOR_ARGB(255, 255, 0, 0), pFont, L"test");
	d3ddev->EndScene();    // ends the 3D scene
	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}
int OverlayInit() {
	RECT rc;

	HWND newhwnd = FindWindow(NULL, L"Victoria 2");
	if (newhwnd != NULL) {
		GetWindowRect(newhwnd, &rc);
		s_width = rc.right - rc.left;
		s_height = rc.bottom - rc.top;
	}
	else {
		return -1;
	}
	
	ZeroMemory(&window_class, sizeof(WNDCLASSEX));

	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = WindowProc;
	window_class.hInstance = NULL;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	window_class.lpszClassName = L"WindowClass";

	if (!RegisterClassEx(&window_class)) {
		return -2;
	}


	hWnd = CreateWindowEx(0,
		L"WindowClass",
		L"",
		WS_EX_TOPMOST | WS_POPUP,
		rc.left, rc.top,
		s_width, s_height,
		NULL,
		NULL,
		NULL,
		NULL);

	if (hWnd == NULL) {
		return -3;
	}

	SetWindowLong(hWnd, GWL_EXSTYLE, (int)GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);


	/*LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
	
		printf("couldn't pD3D\n");
		return 1;
	}
	
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;

	LPDIRECT3DDEVICE9 device;

	HRESULT res = pD3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &device);

	if (FAILED(res)) {
		printf("Couldn't create device\n");
		return 1;
	}

	device->EndScene();*/
	initD3D(hWnd);

	return 1;
}

int OverlayShow(DisplayInfo* display_info, ThreadStruct *thread_control) {
	ShowWindow(hWnd, SW_RESTORE);
	MSG msg;
	
	RECT vic2_rect;
	while (TRUE)
	{
		HWND vic2 = FindWindow(NULL, L"Victoria 2");
		if (vic2 == NULL) {
			goto end;
		}
		
		GetWindowRect(vic2, &vic2_rect);
		SetWindowPos(hWnd, HWND_TOPMOST, vic2_rect.left, vic2_rect.top, (vic2_rect.bottom - vic2_rect.top), (vic2_rect.right - vic2_rect.left), SWP_NOSIZE);

		// clear the window alpha
		d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		if (GetForegroundWindow() == vic2)
		{
			if (WaitForSingleObject(thread_control->lock, 0) == WAIT_TIMEOUT) {
				continue;
			}

			if (display_info->data_buffer == NULL) {
				continue;
			}

			render(display_info);

			ReleaseMutex(thread_control->lock);
		}

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			exit(1);
		}

		Sleep(1);
	}
end:
	//DeleteObject(hWnd);
	return msg.wParam;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_PAINT:
		{
			DwmExtendFrameIntoClientArea(hWnd, &margin);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}