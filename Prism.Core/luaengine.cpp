#include "luaengine.hpp"
#include <iostream>
#include "exported_engine.hpp"
#include "function.hpp"

LuaEngine* g_pLuaEngine_Draw = new LuaEngine();
LuaEngine* g_pLuaEngine_Update = new LuaEngine();
LuaEngine* g_pLuaEngine_MouseInput = new LuaEngine();

lua_State* LuaEngine::L()
{
	return m_L;
}

void LuaEngine::report_errors(int state)
{
	if (state)
	{
		function::lua_error = true;
		function::lua_err_string = lua_tostring(m_L, -1);
		lua_pop(m_L, 1); //remove error
	}else
		function::lua_error = false;
}

void LuaEngine::ExecuteFile(const char* file)
{
	if (!file || !m_L)
		return;

	int state = luaL_dofile(m_L, file);
	report_errors(state);
}


void LuaEngine::ExecuteString(const char* expression)
{
	if (!expression || !m_L)
	{
		std::cerr << xorstr("ERR: null expression passed to script engine!").crypt_get() << std::endl;
		return;
	}

	int state = luaL_dostring(m_L, expression);
	report_errors(state);
}

void LuaEngine::Reset()
{
	if (m_L)
		lua_close(m_L);
	m_L = luaL_newstate();
	luaL_openlibs(m_L);
	Register(m_L);
	ExecuteFile((std::string(g_Path) + xorstr("main.lua").crypt_get()).c_str());
}

void HotFix()
{
	std::vector<uint64_t> hotfixes = mem::arrayscan("40 53 48 83 EC 20 48 8B D9 48 8D 0D", g_Instance, 0x100000);
	for (uint64_t hotfix : hotfixes)
	{
		for (int i = 0; i < 0x50; i++)
		{
			if (mem::RPM<uint64_t>(hotfix + i) == 0x00005825048B4865)
			{
				DWORD OldProtect{};
				VirtualProtect((void*)hotfix, 2, PAGE_EXECUTE_READWRITE, &OldProtect);
				mem::WPM<uint8_t>(hotfix, 0xC3);
				VirtualProtect((void*)hotfix, 2, OldProtect, &OldProtect);
			}
		}
	}
}

int failed_count{};
void LuaEngine::SafeReset(bool isDraw)
{
	__try
	{
		if (isDraw)
		{
			for (ID3D11ShaderResourceView* resrc : resrc_list)
			{
				if (resrc)
				{
					ULONG ref = resrc->Release();
					while (ref > 0)
						ref = resrc->Release();
					resrc = nullptr;
				}
			}
			resrc_list.clear();
		}

		Reset();
		failed_count = 0;
		Sleep(300);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		HotFix();
		failed_count++;
		if (failed_count > 20)
			ExitProcess(0);
		SafeReset(isDraw);
	}
}