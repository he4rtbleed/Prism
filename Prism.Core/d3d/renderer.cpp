#include "renderer.hpp"
#include "fonts.hpp"

IDXGISwapChain* cRenderer::m_Swapchain;
ID3D11Device* cRenderer::m_Device;
ID3D11DeviceContext* cRenderer::m_Context;
ID3D11RenderTargetView* cRenderer::m_RTV;
HWND cRenderer::hWnd;
ImFont* cRenderer::m_pFont;

int cRenderer::Width;
int cRenderer::Height;

bool cRenderer::b_Draw = true;

void cRenderer::Initialize(IDXGISwapChain* swapChain)
{
	m_Swapchain = swapChain;
	if (FAILED(m_Swapchain->GetDevice(__uuidof(ID3D11Device), (void**)(&m_Device))))
		return;

	m_Device->GetImmediateContext(&m_Context);

	hWnd = FindWindow(xorstr("TankWindowClass").crypt_get(), NULL);

	m_pFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fonts_compressed_data, fonts_compressed_size, 14.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

	ImGui::CreateContext();
	ImGui_ImplDX11_Init(hWnd, m_Device, m_Context);
	ImGui_ImplDX11_CreateDeviceObjects();
}

void cRenderer::ClearRenderTarget()
{
	if (m_RTV)
	{
		m_RTV->Release();
		m_RTV = NULL;
	}
}

void cRenderer::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	m_Swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	m_Device->CreateRenderTargetView(pBackBuffer, NULL, &m_RTV);
	pBackBuffer->Release();

	D3D11_TEXTURE2D_DESC desc;
	pBackBuffer->GetDesc(&desc);
	Width = desc.Width;
	Height = desc.Height;
}

void cRenderer::BeginScene()
{
	cRenderer::CreateRenderTarget();

	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiSetCond_Always);
}

void cRenderer::EndScene()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::Render();
	m_Context->OMSetRenderTargets(1, &m_RTV, NULL);
	ImGui_ImplDX11_RenderDrawLists(ImGui::GetDrawData());

	cRenderer::ClearRenderTarget();
}

void cRenderer::PreResize()
{
	RECT rct;
	GetClientRect(hWnd, &rct);
	Width = rct.right - rct.left;
	Height = rct.bottom - rct.top;
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ClearRenderTarget();
}

void cRenderer::PostResize()
{
	CreateRenderTarget();
	ImGui_ImplDX11_CreateDeviceObjects();
}

bool cRenderer::IsFocused()
{
	return (GetForegroundWindow() == cRenderer::hWnd);
}

void cRenderer::DrawBox(const ImVec2& pos, const ImVec2& boxres, uint32_t color)
{
	if (!b_Draw)
		return;
	DrawLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + boxres.x, pos.y), color, 1.f);
	DrawLine(ImVec2(pos.x, pos.y), ImVec2(pos.x, pos.y + boxres.y), color, 1.f);
	DrawLine(ImVec2(pos.x, pos.y + boxres.y), ImVec2(pos.x + boxres.x, pos.y + boxres.y), color, 1.f);
	DrawLine(ImVec2(pos.x + boxres.x, pos.y), ImVec2(pos.x + boxres.x, pos.y + boxres.y), color, 1.f);
}

float cRenderer::DrawOutlinedText(const std::string& text, const ImVec2& position, float size, uint32_t color, bool center)
{
	if (!b_Draw)
		return 1.f;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;
	float y = 0.0f;
	int i = 0;
	ImVec2 textSize = m_pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

	if (center)
	{
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

		window->DrawList->AddText(m_pFont, size, { position.x - textSize.x / 2.0f, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
	}
	else
	{
		window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

		window->DrawList->AddText(m_pFont, size, { position.x, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
	}
	return 1.f;
}

float cRenderer::DrawOutlinedText(const std::wstring& wtext, const ImVec2& position, float size, uint32_t color, bool center)
{
	if (!b_Draw)
		return 1.f;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	std::string text(wtext.begin(), wtext.end());

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;
	float y = 0.0f;
	int i = 0;
	ImVec2 textSize = m_pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

	if (center)
	{
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

		window->DrawList->AddText(m_pFont, size, { position.x - textSize.x / 2.0f, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
	}
	else
	{
		window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
		window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

		window->DrawList->AddText(m_pFont, size, { position.x, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
	}
	return 1.f;
}

void cRenderer::DrawBoxFilled(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding, uint32_t roundingCornersFlags)
{
	if (!b_Draw)
		return;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xFF;
	float r = (color >> 16) & 0xFF;
	float g = (color >> 8) & 0xFF;
	float b = (color) & 0xFF;

	window->DrawList->AddRectFilled(from, to, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), rounding, roundingCornersFlags);
}

void cRenderer::DrawLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness)
{
	if (!b_Draw)
		return;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddLine(from, to, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), thickness);
}

void cRenderer::Draw3DLine(XMFLOAT3 from, XMFLOAT3 to, uint32_t color, float thickness)
{
	if (!b_Draw)
		return;
	XMFLOAT2 start{}, end{};
	if (engine::WorldToScreen(from, &start) && engine::WorldToScreen(to, &end))
		DrawLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), color, thickness);
}

void cRenderer::DrawCircle(const ImVec2& position, float radius, uint32_t color, float thickness)
{
	if (!b_Draw)
		return;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircle(position, radius, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64, thickness);
}

void cRenderer::DrawCircleScale(const ImVec2& position, float radius, uint32_t color, const ImVec2& scalepos, const ImVec2& scaleheadPosition, float thickness)
{
	if (!b_Draw)
		return;
	float rad = (scaleheadPosition.y + 15 - scalepos.y) / 10.5f;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircle(position, rad, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64, thickness);
}

void cRenderer::DrawCircleFilled(const ImVec2& position, float radius, uint32_t color)
{
	if (!b_Draw)
		return;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircleFilled(position, radius, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64);
}

void cRenderer::RenderCircle(const ImVec2& position, float radius, uint32_t color, float thickness, uint32_t segments)
{
	if (!b_Draw)
		return;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircle(position, radius, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), segments, thickness);
}

void cRenderer::DrawImage(int x, int y, int w, int h, ID3D11ShaderResourceView* texture)
{
	__try
	{
		if (!b_Draw)
			return;
		float OriginX = ImGui::GetCursorPosX();
		float OriginY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosX(x);  ImGui::SetCursorPosY(y);
		ImGui::Image((void*)texture, ImVec2((float)w, (float)h));
		ImGui::SetCursorPosX(OriginX);  ImGui::SetCursorPosY(OriginY);
	}
	__except (1) {}
}

#define STB_IMAGE_IMPLEMENTATION
#include "imgui/stb_image.h"
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	__try
	{
		// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
		if (image_data == NULL)
			return false;

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = image_width;
		desc.Height = image_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = image_data;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		cRenderer::m_Device->CreateTexture2D(&desc, &subResource, &pTexture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		cRenderer::m_Device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
		pTexture->Release();

		*out_width = image_width;
		*out_height = image_height;
		stbi_image_free(image_data);

		return true;
	}
	__except (1) {}
}

ID3D11ShaderResourceView* cRenderer::CreateSpriteFromFile(std::string FilePath)
{
	int my_image_width = 0;
	int my_image_height = 0;
	ID3D11ShaderResourceView* my_texture = nullptr;
	LoadTextureFromFile(FilePath.c_str(), &my_texture, &my_image_width, &my_image_height);
	return my_texture;
}

bool cRenderer::DrawSkeleton(Player player, uint32_t visColor, uint32_t invisColor, float thickness, bool bUseRayCast)
{
	auto Identifier = player.GetIdentifier();
	auto Mesh = player.GetMesh();
	bool IsMeka = (Identifier->HeroID == eHero::HERO_DVA) && (Mesh->SkeletalMesh->BoneCount > 240);
	bool IsBastion = (Identifier->HeroID == eHero::HERO_BASTION);
	bool IsBot = (Identifier->HeroID >= eHero::HERO_TRAININGBOT1 && Identifier->HeroID <= eHero::HERO_TRAININGBOT4);
	int skeleton[][4] = {
		{ 0x10, 0x11, 0, 0 },
		{ 0x10, 0xD, IsBot ? 0 : 0xE, (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x1C },
		{ 0x10, 0x36, IsBot ? 0 : 0x37, (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x3A },
		{ 0x10, IsMeka ? 0x6 : IsBot ? 0x3 : IsBastion ? 0x4 : 0x5, IsBot ? 0x92D : 0x4, IsBot ? 0 : 0x3 },
		{ IsBot ? 0 : 0x3, IsBot ? 0 : 0x55, IsBot ? 0 : 0x59, IsBot ? 0 : 0x5A },
		{ IsBot ? 0 : 0x3, IsBot ? 0 : 0x5F, IsBot ? 0 : 0x63, IsBot ? 0 : 0x64 }
	};

	bool isVisible = bUseRayCast ? false : player.GetVisibility()->IsVisible();

	for (auto part : skeleton)
	{
		XMFLOAT3 previous{};
		for (int i = 0; i < 4; i++)
		{
			if (!part[i]) break;

			XMFLOAT3 current = Mesh->GetBonePos(part[i]);

			std::vector<Player> entityList{};
			entityList.push_back(engine::LocalPlayer);
			entityList.push_back(Player(decrypter::DecryptParent((uint64_t)Mesh), 0));
			bool hitted = isVisible;
			if (bUseRayCast)
			{
				hitted = engine::RayCast(engine::g_ViewMatrix.GetCameraVec(), current, entityList, eMask::MASK_IGNOREALL).first == eRayCast::HIT_SUCCESS;
				if (!isVisible && hitted)
					isVisible = true;
			}

			if (previous.x == 0.f)
			{
				previous = current;
				continue;
			}
			XMFLOAT2 p1{}, c1{};
			bool bp1 = engine::WorldToScreen(previous, &p1);
			bool bc1 = engine::WorldToScreen(current, &c1);
			if (bp1 && bc1)
				cRenderer::DrawLine(ImVec2(p1.x, p1.y), ImVec2(c1.x, c1.y), hitted ? visColor : invisColor, thickness);
			previous = current;
		}
	}

	return isVisible;
}

void cRenderer::DrawHealthBar(HealthComponent* Health, MeshComponent* Mesh)
{
	XMFLOAT3 head = Mesh->GetBonePos(0x11);
	XMFLOAT3 root = Mesh->GetLocation();

	XMFLOAT2 head_{}, root_{};
	if (engine::WorldToScreen(head, &head_) && engine::WorldToScreen(root, &root_))
	{
		XMFLOAT2 Life = Health->GetLife();
		float health = (Life.y / Life.x) * 100.f;
		float h = head_.y - root_.y;
		float offset = (h / 4.f) - 5.f;
		float w = h / 64.f;
		float flBoxes = std::ceil(health / 10.f);
		float flHeight = h / 10.f;
		UINT hp = UINT(h - (UINT)((h * health) / 100)); // Percentage
		int Red = int(255 - (health * 2.55));
		int Green = int(health * 2.55);

		cRenderer::DrawLine(ImVec2{ min(root_.x, head_.x) + offset, root_.y }, ImVec2{ min(root_.x, head_.x) + offset, root_.y + h }, 0xFF000000, 3.0f);
		cRenderer::DrawLine(ImVec2{ min(root_.x, head_.x) + offset, root_.y }, ImVec2{ min(root_.x, head_.x) + offset, root_.y + flHeight * flBoxes }, 0xFFFF0000, 2.0f);
	}
}