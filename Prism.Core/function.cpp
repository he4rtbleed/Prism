#include <vector>
#include <sstream>
#include <ctime>

#include "function.hpp"
#include "decrypter.hpp"
#include "engine.hpp"
#include "component.hpp"
#include "enums.hpp"

void function::FunctionExecutor()
{
	__try
	{
#ifdef VMP
		Debug();
#endif
		ESP();
		OnDraw();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

bool function::lua_error = false;
std::string function::lua_err_string;

bool function::settings_draw_fov = true;
bool function::settings_esp_text = true;
bool function::settings_esp_skeleton = true;
uint32_t function::settings_esp_skeleton_color_enemy_visible = 0xFF00FF00;
uint32_t function::settings_esp_skeleton_color_enemy_invisible = 0xFFFF0000;
bool function::settings_esp_skeleton_useraycast = true;
bool function::settings_esp_highlight = true;
uint32_t function::settings_esp_highlight_color_enemy_visible = 0xFF00FF00;
uint32_t function::settings_esp_highlight_color_enemy_invisible = 0xFF0000FF;
uint32_t function::settings_esp_highlight_color_team = 0xFFFFFFFF;
void function::ESP()
{
	if (lua_error)
		cRenderer::DrawOutlinedText(function::lua_err_string, ImVec2(cRenderer::Width / 2, 30), 20, 0xFFFF0000, true);

	if (!engine::g_Manager)
		return;

	float RotXZ = atan2f(engine::g_ViewMatrix.m13, engine::g_ViewMatrix.m33);
	float XAXis_X = engine::g_ViewMatrix.m11 * cosf(RotXZ) - engine::g_ViewMatrix.m31 * sinf(RotXZ);
	engine::g_Fov = fabs(atanf(1.0f / XAXis_X) * 2.0f * 180.0f / XM_PI);

	if (settings_draw_fov)
	{
		float _fov = abs(tan(engine::g_LocalFov * XM_PI / 180.f / 2.f) / tan(engine::g_Fov * XM_PI / 180.f / 2.f) * cRenderer::Width);
		cRenderer::DrawCircle(ImVec2(cRenderer::Width / 2, cRenderer::Height / 2), _fov, 0xFFFFFFFF);
	}

	int i{}, j{};
	for (Player player : engine::PlayerList)
	{
		auto Health = player.GetHealth();
		auto Mesh = player.GetMesh();
		auto Identifier = player.GetIdentifier();
		auto Highlight = player.GetHighlight();
		auto Visibility = player.GetVisibility();

		if (!Health || !Mesh || !Identifier || !Highlight || !Visibility)
			continue;

		bool isEnemy = player.IsEnemy();

		XMFLOAT2 Life = Health->GetLife();
		if (Life.y)
		{
			XMFLOAT2 root{};
			if (engine::WorldToScreen(Mesh->GetLocation(), &root))
			{
				if (isEnemy)
				{
					if (settings_esp_text)
						cRenderer::DrawOutlinedText(Identifier->GetHeroName(), ImVec2(root.x, root.y), 16, 0xFFFFFFFF, true);
					bool isVisible = settings_esp_skeleton ? cRenderer::DrawSkeleton(player, settings_esp_skeleton_color_enemy_visible, settings_esp_skeleton_color_enemy_invisible, 1.f, settings_esp_skeleton_useraycast) : Visibility->IsVisible();
					if (settings_esp_highlight)
						Highlight->SetHighlightInfo(isVisible ? 1 : 2, isVisible ? settings_esp_highlight_color_enemy_visible : settings_esp_highlight_color_enemy_invisible);
				}
				else
				{
					if (settings_esp_highlight)
						Highlight->SetHighlightInfo(2, settings_esp_highlight_color_team);
				}
			}
		}

		if (utils::IsKeyPressed(VK_TAB))
		{
			auto Stat = player.GetStat();
			if (Stat)
				cRenderer::DrawOutlinedText(Stat->GetBattleTag(), ImVec2(cRenderer::Width - 175 - 175 * !isEnemy, 100 + 18 * (isEnemy ? i++ : j++)), 16, 0xFFFFFFFF, false);
		}
	}
}

void OnDraw()
{
	luabridge::LuaRef runHandler = luabridge::getGlobal(g_pLuaEngine_Draw->L(), xorstr("OnDraw").crypt_get());
	runHandler();
}

void OnUpdate()
{
	luabridge::LuaRef runHandler = luabridge::getGlobal(g_pLuaEngine_Update->L(), xorstr("OnUpdate").crypt_get());
	runHandler();
}

MouseInput OnMouseInput(MouseInput pt)
{
	luabridge::LuaRef runHandler = luabridge::getGlobal(g_pLuaEngine_MouseInput->L(), xorstr("OnMouseInput").crypt_get());
	return runHandler(pt);
}

void function::OnDraw()
{
	__try
	{
		::OnDraw();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

float function::settings_limit_fps = 7.0;
std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
void function::OnUpdate()
{
	__try
	{
		a = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> work_time = a - b;

		if (work_time.count() < function::settings_limit_fps)
		{
			std::chrono::duration<double, std::milli> delta_ms(function::settings_limit_fps - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}

		b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - a;

		::OnUpdate();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

MouseInput function::OnMouseInput(MouseInput pt)
{
	__try
	{
		return ::OnMouseInput(pt);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return pt;
	}
}

void function::Debug()
{
#ifdef VMP
	std::stringstream debugStr;
	debugStr << std::hex << mem::dwRetAddr << std::endl;
	debugStr << std::hex << engine::g_Manager << std::endl;
	debugStr << std::hex << engine::g_EntityTArray << std::endl;
	debugStr << std::dec << engine::PlayerList.size() << std::endl;
	debugStr << std::hex << engine::Update << std::endl;
	debugStr << std::hex << engine::RayCast << std::endl;
	debugStr << std::hex << engine::FindBestBone << std::endl;
	debugStr << std::hex << "LOCAL - " << engine::LocalPlayer.GetCommon() << ", " << engine::LocalPlayer.GetReaper() << std::endl;
	debugStr << std::hex << "REAPERLINKER - " << decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_REAPERLINKER) << std::endl;
	debugStr << std::hex << "COMMONLINKER - " << decrypter::GetComponent(engine::LocalPlayer.GetReaper(), eComp::COMP_COMMONLINKER) << std::endl;

	auto Control = ((ControlComponent*)decrypter::GetComponent(engine::LocalPlayer.GetReaper(), eComp::COMP_CONTROL));
	if (Control)
	{
		debugStr << std::hex << "CONTROL - " << Control << std::endl;
		debugStr << std::hex << "\t" << Control->KeyCode << std::endl;
		debugStr << std::hex << "\t" << Control->Angle.x << ", " << Control->Angle.y << ", " << Control->Angle.z << std::endl;
	}
	auto Mesh = ((MeshComponent*)decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_MESH));
	if (Mesh)
	{
		debugStr << std::hex << "MESH - " << Mesh << std::endl;
		debugStr << std::hex << "\t" << Mesh->SkeletalMesh << std::endl;
		if (Mesh->SkeletalMesh)
			debugStr << std::hex << "\t" << Mesh->SkeletalMesh->BoneCount << std::endl;
	}
	auto Team = ((TeamComponent*)decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_TEAM));
	if (Team)
	{
		debugStr << std::hex << "TEAM - " << Team << std::endl;
		debugStr << std::hex << "\t" << Team->Team << std::endl;
		debugStr << std::hex << "\t" << Team->GetTeam() << std::endl;
	}
	auto Stat = ((StatComponent*)decrypter::GetComponent(engine::LocalPlayer.GetReaper(), eComp::COMP_STAT));
	if (Stat)
	{
		debugStr << std::hex << "STAT - " << Stat << std::endl;
		debugStr << std::hex << "\t" << Stat->GetBattleTag() << std::endl;
	}
	auto Skill = ((SkillComponent*)decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_SKILL));
	if (Skill)
	{
		debugStr << std::hex << "SKILL - " << Skill << std::endl;
	}
	auto Health = ((HealthComponent*)decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_HEALTH));
	if (Health)
	{
		debugStr << std::hex << "HEALTH - " << Health << std::endl;
		debugStr << std::hex << "\t" << Health->GetHealth().x << ", " << Health->GetHealth().y << std::endl;
		debugStr << std::hex << "\t" << Health->GetArmour().x << ", " << Health->GetArmour().y << std::endl;
		debugStr << std::hex << "\t" << Health->GetBarrier().x << ", " << Health->GetBarrier().y << std::endl;
		debugStr << std::hex << "\t" << Health->GetLife().x << ", " << Health->GetLife().y << std::endl;
		debugStr << std::hex << "\t" << Health->isBarrierProjected << ", " << Health->isImmortal << std::endl;
	}
	auto Identifier = ((IdentifierComponent*)decrypter::GetComponent(engine::LocalPlayer.GetReaper(), eComp::COMP_IDENTIFIER));
	if (Identifier)
	{
		debugStr << std::hex << "IDENTIFIER - " << Identifier << std::endl;
		debugStr << std::hex << "\t" << Identifier->HeroID << std::endl;
		//debugStr << std::hex << "\t" << Identifier->GetHeroName() << std::endl;
	}
	auto Highlight = ((HighlightComponent*)decrypter::GetComponent(engine::LocalPlayer.GetCommon(), eComp::COMP_HIGHLIGHT));
	if (Highlight)
	{
		debugStr << std::hex << "HIGHLIGHT - " << Highlight << std::endl;
	}
	debugStr << std::endl;
	debugStr << engine::g_ViewMatrix.m11 << "\t" << engine::g_ViewMatrix.m12 << "\t" << engine::g_ViewMatrix.m13 << "\t" << engine::g_ViewMatrix.m14 << std::endl;
	debugStr << engine::g_ViewMatrix.m21 << "\t" << engine::g_ViewMatrix.m22 << "\t" << engine::g_ViewMatrix.m23 << "\t" << engine::g_ViewMatrix.m24 << std::endl;
	debugStr << engine::g_ViewMatrix.m31 << "\t" << engine::g_ViewMatrix.m32 << "\t" << engine::g_ViewMatrix.m33 << "\t" << engine::g_ViewMatrix.m34 << std::endl;
	debugStr << engine::g_ViewMatrix.m41 << "\t" << engine::g_ViewMatrix.m42 << "\t" << engine::g_ViewMatrix.m43 << "\t" << engine::g_ViewMatrix.m44 << std::endl;
	debugStr << std::endl;

	cRenderer::DrawOutlinedText(debugStr.str(), ImVec2(10, 58), 16, 0xFFFFFFFF, false);
#endif
}