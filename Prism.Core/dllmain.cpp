#include <mutex>

#include "d3d/renderer.hpp"
#include "d3d/hooker.hpp"

#include "function.hpp"

#include <license.hpp>

std::once_flag once;
void* o_present;

extern void cRenderer::CreateRenderTarget();
extern void cRenderer::ClearRenderTarget();

int login_failed_count{};
bool is_unloaded = false;
HRESULT hk_present(IDXGISwapChain* dis, UINT syncInterval, UINT flags)
{
	std::call_once(once, [dis]() {
		cRenderer::Initialize(dis);
		utils::Protect();
		});

	if (is_unloaded)
		return reinterpret_cast<decltype(&hk_present)>(o_present)(dis, syncInterval, flags);

	if (utils::IsKeyPressed(VK_F1))
		g_pLuaEngine_Draw->SafeReset(true);

	if (utils::IsKeyPressed(VK_INSERT))
		cRenderer::b_Draw = !cRenderer::b_Draw, Sleep(300);

	engine::g_ViewMatrix = engine::GetViewMatrix();

	if (cRenderer::b_Draw)
	{
		cRenderer::BeginScene();

		function::FunctionExecutor();

		cRenderer::EndScene();
	}

	return reinterpret_cast<decltype(&hk_present)>(o_present)(dis, syncInterval, flags);
}

void* o_GetRawInputData;
UINT WINAPI hk_GetRawInputData(
	_In_ HRAWINPUT hRawInput,
	_In_ UINT uiCommand,
	_Out_writes_bytes_to_opt_(*pcbSize, return) LPVOID pData,
	_Inout_ PUINT pcbSize,
	_In_ UINT cbSizeHeader)
{
	__try
	{
		UINT r = reinterpret_cast<decltype(&hk_GetRawInputData)>(o_GetRawInputData)(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
		auto Data = (PRAWINPUT)pData;

		if (is_unloaded)
			return r;

		if (utils::IsKeyPressed(VK_F1))
			g_pLuaEngine_MouseInput->SafeReset(false);

		if (Data && Data->header.dwType == RIM_TYPEMOUSE)
		{
			if (Data->header.hDevice == NULL)
			{
				if (!utils::hDevice)
					Data->header.hDevice = (HANDLE)0x10035;
				else
					Data->header.hDevice = utils::hDevice;
			}

			{
				MouseInput in{};
				in.dx = Data->data.mouse.lLastX;
				in.dy = Data->data.mouse.lLastY;
				in.btn = Data->data.mouse.usButtonFlags;
				MouseInput res = function::OnMouseInput(in);
				Data->data.mouse.lLastX = res.dx;
				Data->data.mouse.lLastY = res.dy;
				Data->data.mouse.usButtonFlags = res.btn;
			}
		}
		return r;
	}
	__except (1) {}
}


char* g_Key{};
char* g_Path{};
time_t last_login{};
void checkLicense()
{
	/*redacted*/
}

bool temp_test = true;
std::once_flag once_;
time_t last_update{};
void update_loop()
{
#ifndef VMP
	VMProtectBeginUltra("update");
#endif
	for (;;)
	{
		Sleep(1);
		__try
		{
			std::call_once(once_, []() {
				g_pLuaEngine_Update->SafeReset(false);
				g_pLuaEngine_Draw->SafeReset(true);
				g_pLuaEngine_MouseInput->SafeReset(false);
				HookD3D11();
				});

			if (utils::IsKeyPressed(VK_HOME))
				temp_test = !temp_test, Sleep(300);

			if (temp_test)
			{
				if ((GetTickCount64() - last_login) > 60000)
					checkLicense();

				if ((GetTickCount64() - last_update) > 100)
				{
					last_update = GetTickCount64();
					engine::Update();
				}

				if (utils::IsKeyPressed(VK_F1))
					g_pLuaEngine_Update->SafeReset(false);

				function::OnUpdate();
			}

			if (utils::IsKeyPressed(VK_CONTROL) && utils::IsKeyPressed(VK_DELETE))
			{
				UnhookD3D11();
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}
#ifndef VMP
	VMProtectEnd();
#endif
}

uint64_t g_Instance;
BOOL DllMain(HINSTANCE hInstance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#ifndef VMP
		VMProtectBeginMutation("dllmain");
#else
		AllocConsole();
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
#endif
		g_Instance = (uint64_t)hInstance;
		g_Key = reinterpret_cast<char*>(*(uint64_t*)reserved);
		g_Path = reinterpret_cast<char*>(*(uint64_t*)((uint64_t)reserved + 0x10));
		if (g_Key && g_Path)
		{
			mem::dwRetAddr = mem::find_spoof_addr();
			if (mem::dwRetAddr)
			{
				if (utils::Init(g_Instance))
				{
					_beginthread((_beginthread_proc_type)update_loop, 0, nullptr);
				}
			}
		}
#ifndef VMP
		VMProtectEnd();
#endif
	}

	return true;
}