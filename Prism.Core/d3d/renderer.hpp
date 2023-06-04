#pragma once
#include <Windows.h>
#include <string>
#include <d3d11.h>
#include <iostream>
#include <vector>

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui_impl_dx11.h"

#include "../engine.hpp"
#include "../component.hpp"

namespace cRenderer
{

	extern IDXGISwapChain* m_Swapchain;
	extern ID3D11Device* m_Device;
	extern ID3D11DeviceContext* m_Context;
	extern ID3D11RenderTargetView* m_RTV;
	extern HWND hWnd;
	extern ImFont* m_pFont;

	extern int Width;
	extern int Height;

	extern bool b_Draw;

	void Initialize(IDXGISwapChain* swapChain);

	void ClearRenderTarget();
	void CreateRenderTarget();

	void BeginScene();
	void EndScene();

	void PreResize();
	void PostResize();

	bool IsFocused();

	float DrawOutlinedText(const std::string& text, const ImVec2& position, float size, uint32_t color, bool center);
	float DrawOutlinedText(const std::wstring& wtext, const ImVec2& position, float size, uint32_t color, bool center);
	void DrawLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness = 1.0f);
	void Draw3DLine(XMFLOAT3 from, XMFLOAT3 to, uint32_t color, float thickness = 1.0f);
	void DrawCircle(const ImVec2& position, float radius, uint32_t color, float thickness = 1.0f);
	void DrawCircleScale(const ImVec2& position, float radius, uint32_t color, const ImVec2& scalepos, const ImVec2& scaleHeadPosition, float thickness = 1.0f);
	void DrawCircleFilled(const ImVec2& position, float radius, uint32_t color);
	void DrawBox(const ImVec2& position, const ImVec2& HeadPosition, uint32_t color);
	void DrawBoxFilled(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding = 0.0f, uint32_t roundingCornersFlags = ImDrawCornerFlags_All);
	void RenderCircle(const ImVec2& position, float radius, uint32_t color, float thickness, uint32_t segments);
	void DrawImage(int x, int y, int w, int h, ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView* CreateSpriteFromFile(std::string FilePath);
	bool DrawSkeleton(Player player, uint32_t visColor, uint32_t invisColor, float thickness, bool bUseRayCast);
	void DrawHealthBar(HealthComponent* Health, MeshComponent* Mesh);

}