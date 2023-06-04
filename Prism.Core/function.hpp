#pragma once
#include "d3d/renderer.hpp"
#include "d3d/hooker.hpp"

#include "luaengine.hpp"

namespace function
{

	extern bool lua_error;
	extern std::string lua_err_string;

	extern float settings_limit_fps;
	extern bool settings_draw_fov;
	extern bool settings_esp_text;
	extern bool settings_esp_skeleton;
	extern uint32_t settings_esp_skeleton_color_enemy_visible;
	extern uint32_t settings_esp_skeleton_color_enemy_invisible;
	extern bool settings_esp_skeleton_useraycast;
	extern bool settings_esp_highlight;
	extern uint32_t settings_esp_highlight_color_enemy_visible;
	extern uint32_t settings_esp_highlight_color_enemy_invisible;
	extern uint32_t settings_esp_highlight_color_team;

	void FunctionExecutor();
	void ESP();
	void OnDraw();
	void OnUpdate();
	MouseInput OnMouseInput(MouseInput pt);

	void Debug();

}