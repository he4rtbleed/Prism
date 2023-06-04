#pragma once
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#pragma comment (lib, "lua53.lib")

#include <mutex>

#include <d3d11.h>
#include <LuaBridge/LuaBridge.h>
#include <xorstr\include\xorstr.hpp>

using namespace luabridge;

extern char* g_Path;
extern uint64_t g_Instance;
extern std::vector<ID3D11ShaderResourceView*> resrc_list;

class LuaEngine
{
public:
	LuaEngine() : m_L(luaL_newstate()) { luaL_openlibs(m_L); }
	LuaEngine(const LuaEngine& other);
	LuaEngine& operator=(const LuaEngine&);
	~LuaEngine() { lua_close(m_L); }

	lua_State* L();

	void ExecuteFile(const char* file);

	void ExecuteString(const char* expression);

	void Reset();

	void SafeReset(bool isDraw);

	std::mutex m;
private:
	lua_State* m_L;
	void report_errors(int state);
};

extern LuaEngine* g_pLuaEngine_Draw;
extern LuaEngine* g_pLuaEngine_Update;
extern LuaEngine* g_pLuaEngine_MouseInput;