#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <d3d11.h>
#include "MinHook/MinHook.h"
#include "../memory.hpp"
#include "../utils.hpp"
#include "../luaengine.hpp"
#pragma comment (lib, "d3d/MinHook/MinHook.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3d11.lib")

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static IDXGISwapChain* GetSwapChain()
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle(xorstr("dxgi.dll").crypt_get());
		Sleep(1);
	} while (!hDXGIDLL);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = FindWindow(xorstr("TankWindowClass").crypt_get(), NULL);
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	IDXGISwapChain* pSwapChain = 0;
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&pSwapChain,
		&pDevice,
		NULL,
		&pContext)))
	{
		pContext->Release();
		pDevice->Release();
		return pSwapChain;
	}
	else
		return NULL;
}

extern void* o_present;
HRESULT hk_present(IDXGISwapChain* dis, UINT syncInterval, UINT flags);

extern void* o_GetRawInputData;
UINT WINAPI hk_GetRawInputData(
	_In_ HRAWINPUT hRawInput,
	_In_ UINT uiCommand,
	_Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
	_Inout_ PUINT pcbSize,
	_In_ UINT cbSizeHeader);

//40 55 56 57 41 54 41 57 48 8B EC 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 45 0F B6 E0 4C 8D 4D 38 4C 8B F9 33 F6 45 33 C0 89 75 38 48 8B 4F 18 BA ? ? ? ? FF 15 ? ? ? ? 83 F8 FF 0F 84 ? ? ? ? 
static uint64_t grid_iat = mem::dwGameBase + 0x2436C50;
static void HookD3D11()
{
	IDXGISwapChain* m_pSwapChain = GetSwapChain();

	if (m_pSwapChain)
	{
		utils::ThreadStuff(true);
		DWORD_PTR* pSwapChainVtable = (DWORD_PTR*)m_pSwapChain;
		pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
		uint64_t dwPresent = (uint64_t)(DWORD_PTR*)pSwapChainVtable[8];

		MH_Initialize();
		MH_CreateHook((void*)dwPresent, (void*)hk_present, reinterpret_cast<void**>(&o_present));
		MH_CreateHook((void*)mem::RPM<uint64_t>(grid_iat), (void*)hk_GetRawInputData, reinterpret_cast<void**>(&o_GetRawInputData));
		MH_EnableHook(MH_ALL_HOOKS);
		utils::ThreadStuff(false);
	}
}

extern bool is_unloaded;
static void UnhookD3D11()
{
	is_unloaded = true;
	decrypter::global_key = 0;
	decrypter::manager_key = 0;

	{
		utils::ThreadStuff(true);
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		utils::ThreadStuff(false);
	}

	g_pLuaEngine_Update->~LuaEngine();
	g_pLuaEngine_Draw->~LuaEngine();
	g_pLuaEngine_MouseInput->~LuaEngine();
}