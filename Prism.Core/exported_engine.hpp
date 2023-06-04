#pragma once
#include <Windows.h>
#include <sstream>

#include "function.hpp"
#include "engine.hpp"
#include "d3d/renderer.hpp"

#include "luaengine.hpp"
#include "utils.hpp"
#include "var_manager.hpp"
#pragma comment(lib, "winmm.lib")

#pragma pack(push, 1)
struct RayCastResult
{
	bool bHitted;
	Player hittedPlayer;
	uint32_t hittedBone;
	XMFLOAT3 hittedVec;
	XMFLOAT3 hittedRot;
};
#pragma pack(pop)

class ExportedEngine
{
public:

	Player GetLocalPlayer()
	{
		return engine::LocalPlayer;
	}

	Player GetPlayerAt(int index)
	{
		return engine::PlayerList.at(index);
	}

	int GetPlayerCount()
	{
		return engine::PlayerList.size();
	}

	Triggers GetTriggersAt(int index)
	{
		return engine::TriggersList.at(index);
	}

	int GetTriggersCount()
	{
		return engine::TriggersList.size();
	}

	Projectile GetProjectileAt(int index)
	{
		return engine::ProjectileList.at(index);
	}

	int GetProjectileCount()
	{
		return engine::ProjectileList.size();
	}

	Matrix GetViewMatrix()
	{
		return engine::g_ViewMatrix;
	}

	float GetFov()
	{
		return engine::g_LocalFov;
	}

	void SetFov(float _fov)
	{
		engine::g_LocalFov = _fov;
	}

	bool WorldToScreen(XMFLOAT3 worldPos, XMFLOAT2* out)
	{
		return engine::WorldToScreen(worldPos, out);
	}

	float IsInFov(XMFLOAT3 worldPos)
	{
		return engine::IsInFov(worldPos, 360.0).second;
	}

	uint32_t GetTick()
	{
		timeBeginPeriod(1);
		uint32_t tick = timeGetTime();
		timeEndPeriod(1);
		return tick;
	}

	RayCastResult LineOfSight(int mask_enums_id = 1)
	{
		std::vector<Player> onlyLocal{ engine::LocalPlayer };

		XMFLOAT3 ViewRay{};
		XMFLOAT3 camera_vec = engine::g_ViewMatrix.GetCameraVec(), camera_ang = engine::g_ViewMatrix.GetCameraAngle();
		XMStoreFloat3(&ViewRay, XMLoadFloat3(&camera_vec) + XMLoadFloat3(&camera_ang) * 1000);
		std::pair<eRayCast, RayCastOut> result = engine::RayCast(camera_vec, ViewRay, onlyLocal, mask_enums_id);

		if (result.first == eRayCast::HIT_SUCCESS_OTHER)
			return RayCastResult{ true, Player(result.second.hittedEntity, decrypter::GetReaperWithCommon(result.second.hittedEntity)), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };
		else if (result.first == eRayCast::HIT_FAILED)
			return RayCastResult{ true, Player(), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };

		return RayCastResult{ false, Player(), 0, XMFLOAT3{}, XMFLOAT3{} };
	}

	RayCastResult RayCast(XMFLOAT3 from, XMFLOAT3 to, int mask_enums_id = 1)
	{
		std::vector<Player> onlyLocal{ engine::LocalPlayer };

		std::pair<eRayCast, RayCastOut> result = engine::RayCast(from, to, onlyLocal, mask_enums_id);

		if (result.first == eRayCast::HIT_SUCCESS_OTHER)
			return RayCastResult{ true, Player(result.second.hittedEntity, decrypter::GetReaperWithCommon(result.second.hittedEntity)), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };
		else if (result.first == eRayCast::HIT_FAILED)
			return RayCastResult{ true, Player(), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };

		return RayCastResult{ false, Player(), 0, XMFLOAT3{}, XMFLOAT3{} };
	}

	RayCastResult RayCastWithIgnore(XMFLOAT3 from, XMFLOAT3 to, Player player, int mask_enums_id = 1)
	{
		std::vector<Player> toIgnore{ player };

		std::pair<eRayCast, RayCastOut> result = engine::RayCast(from, to, toIgnore, mask_enums_id);

		if (result.first == eRayCast::HIT_SUCCESS_OTHER)
			return RayCastResult{ true, Player(result.second.hittedEntity, decrypter::GetReaperWithCommon(result.second.hittedEntity)), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };
		else if (result.first == eRayCast::HIT_FAILED)
			return RayCastResult{ true, Player(), result.second.hittedBone, result.second.hittedVec, result.second.hittedRot };

		return RayCastResult{ false, Player(), 0, XMFLOAT3{}, XMFLOAT3{} };
	}

	BestBoneResult FindBestBone(Player player, float fov, int prefId, int mask_enums_id = 1)
	{
		__try
		{
			return engine::FindBestBone(player, fov, prefId, mask_enums_id);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return BestBoneResult{ false, 0, 0.f };
		}
	}

	uint64_t GetMapId()
	{
		return engine::GetMapId();
	}

	InputInfo* GetInputInfo()
	{
		return engine::GetInputInfo();
	}

	float GetSensitivity()
	{
		return engine::GetSensitivity();
	}

	void SetSensitivity(float value)
	{
		engine::SetSensitivity(value);
	}

	ControlComponent* GetController()
	{
		return (ControlComponent*)decrypter::GetComponent(engine::LocalPlayer.GetReaper(), eComp::COMP_CONTROL);
	}

	std::string GetPath()
	{
		return g_Path;
	}
};

std::vector<ID3D11ShaderResourceView*> resrc_list;
class ExportedRenderer
{
public:

	float GetWidth()
	{
		return cRenderer::Width;
	}

	float GetHeight()
	{
		return cRenderer::Height;
	}

	void DrawOutlinedText(const char* text, float x, float y, float size, int color, bool center)
	{
		cRenderer::DrawOutlinedText(text, ImVec2(x, y), size, color, center);
	}

	void DrawBox(XMFLOAT2 from, XMFLOAT2 to, int color)
	{
		cRenderer::DrawBox(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color);
	}

	void DrawBoxFilled(XMFLOAT2 from, XMFLOAT2 to, int color, float rounding, int roundingFlags)
	{
		cRenderer::DrawBoxFilled(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color, rounding, roundingFlags);
	}

	void DrawLine(XMFLOAT2 from, XMFLOAT2 to, int color, float thickness)
	{
		cRenderer::DrawLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color, thickness);
	}

	void Draw3DLine(XMFLOAT3 from, XMFLOAT3 to, int color, float thickness)
	{
		cRenderer::Draw3DLine(from, to, color, thickness);
	}

	void DrawCircle(float x, float y, float radius, int color, float thickness)
	{
		cRenderer::DrawCircle(ImVec2(x, y), radius, color, thickness);
	}

	void DrawCircleFilled(float x, float y, float radius, int color)
	{
		cRenderer::DrawCircleFilled(ImVec2(x, y), radius, color);
	}

	void DrawResource(float x, float y, int w, int h, ID3D11ShaderResourceView* texture)
	{
		if (texture)
			cRenderer::DrawImage(x, y, w, h, texture);
	}

	ID3D11ShaderResourceView* LoadResource(std::string path)
	{
		ID3D11ShaderResourceView* resrc = cRenderer::CreateSpriteFromFile(g_Path + path);
		if (resrc != nullptr)
		{
			resrc->AddRef();
			resrc_list.push_back(resrc);
		}
		return resrc;
	}

	void DrawFOV(float fov, int color, float thickness)
	{
		float _fov = abs(tan(fov * XM_PI / 180.f / 2.f) / tan(engine::g_Fov * XM_PI / 180.f / 2.f) * cRenderer::Width);
		cRenderer::DrawCircle(ImVec2(cRenderer::Width / 2, cRenderer::Height / 2), _fov, color, thickness);
	}
};

class ExportedVariableManager
{
public:

	void Clear()
	{
		g_pVarManager->Clear();
	}

	int Size()
	{
		return g_pVarManager->Size();
	}

	bool IsEmpty()
	{
		return g_pVarManager->IsEmpty();
	}

	bool IsExist(const char* key)
	{
		return g_pVarManager->IsExist(key);
	}

	bool Erase(const char* key)
	{
		return g_pVarManager->Erase(key);
	}

	bool get_var_as_bool(const char* key)
	{
		return g_pVarManager->get_var_as_bool(key);
	}

	void set_var_as_bool(const char* key, bool value)
	{
		return g_pVarManager->set_var_as_bool(key, value);
	}

	int get_var_as_int(const char* key)
	{
		return g_pVarManager->get_var_as_int(key);
	}

	void set_var_as_int(const char* key, int value)
	{
		return g_pVarManager->set_var_as_int(key, value);
	}

	__int64 get_var_as_int64(const char* key)
	{
		return g_pVarManager->get_var_as_int64(key);
	}

	void set_var_as_int64(const char* key, __int64 value)
	{
		return g_pVarManager->set_var_as_int64(key, value);
	}

	float get_var_as_float(const char* key)
	{
		return g_pVarManager->get_var_as_float(key);
	}

	void set_var_as_float(const char* key, float value)
	{
		return g_pVarManager->set_var_as_float(key, value);
	}

	double get_var_as_double(const char* key)
	{
		return g_pVarManager->get_var_as_double(key);
	}

	void set_var_as_double(const char* key, double value)
	{
		return g_pVarManager->set_var_as_double(key, value);
	}
};

ExportedEngine* g_pEngine = new ExportedEngine;
ExportedRenderer* g_pRenderer = new ExportedRenderer;
ExportedVariableManager* g_pVariableManager = new ExportedVariableManager;

void Register(lua_State* L)
{
	getGlobalNamespace(L)
		.beginNamespace(xorstr("Math").crypt_get())
			.addFunction(xorstr("NormalizeVec3").crypt_get(), NormalizeVec3)
			.addFunction(xorstr("LerpVec3").crypt_get(), LerpVec3)
			.addFunction(xorstr("Clamp").crypt_get(), Clamp)
			.beginClass<XMFLOAT3>(xorstr("XMFLOAT3").crypt_get())
				.addProperty(xorstr("x").crypt_get(),
					std::function <float(const XMFLOAT3*)>(
						[](const XMFLOAT3* vec) {return vec->x; }),
					std::function <void(XMFLOAT3*, float)>(
						[](XMFLOAT3* vec, float v) {vec->x = v; }))
				.addProperty(xorstr("y").crypt_get(),
					std::function <float(const XMFLOAT3*)>(
						[](const XMFLOAT3* vec) {return vec->y; }),
					std::function <void(XMFLOAT3*, float)>(
						[](XMFLOAT3* vec, float v) {vec->y = v; }))
				.addProperty(xorstr("z").crypt_get(),
					std::function <float(const XMFLOAT3*)>(
						[](const XMFLOAT3* vec) {return vec->z; }),
					std::function <void(XMFLOAT3*, float)>(
						[](XMFLOAT3* vec, float v) {vec->z = v; }))
				.addFunction(xorstr("__eq").crypt_get(),		//https://github.com/vinniefalco/LuaBridge/blob/d07c50e76f65921b87bf70c806d5bafa20c0cdb0/Tests/Source/ClassTests.cpp#L554
					std::function <bool(XMFLOAT3*, XMFLOAT3*)>(
						[](XMFLOAT3* vec, XMFLOAT3* vec2) { return (vec->x == vec2->x && vec->y == vec2->y && vec->z + vec2->z); }))
				.addFunction(xorstr("__add").crypt_get(),
					std::function <XMFLOAT3(XMFLOAT3*, XMFLOAT3*)>(
						[](XMFLOAT3* vec, XMFLOAT3* vec2) { return XMFLOAT3(vec->x + vec2->x, vec->y + vec2->y, vec->z + vec2->z); }))
				.addFunction(xorstr("__sub").crypt_get(),
					std::function <XMFLOAT3(XMFLOAT3*, XMFLOAT3*)>( 
						[](XMFLOAT3* vec, XMFLOAT3* vec2) { return XMFLOAT3(vec->x - vec2->x, vec->y - vec2->y, vec->z - vec2->z); }))
				.addFunction(xorstr("__mul").crypt_get(),
					std::function <XMFLOAT3(XMFLOAT3*, XMFLOAT3*)>(
						[](XMFLOAT3* vec, XMFLOAT3* vec2) { return XMFLOAT3(vec->x * vec2->x, vec->y * vec2->y, vec->z * vec2->z); }))
				.addFunction(xorstr("__div").crypt_get(),
					std::function <XMFLOAT3(XMFLOAT3*, XMFLOAT3*)>(
						[](XMFLOAT3* vec, XMFLOAT3* vec2) { return XMFLOAT3(vec->x / vec2->x, vec->y / vec2->y, vec->z / vec2->z); }))
				.addConstructor<XMFLOAT3(*) (float, float, float)>()
			.endClass()
			.beginClass <XMFLOAT2>(xorstr("XMFLOAT2").crypt_get())
				.addProperty(xorstr("x").crypt_get(),
					std::function <float(const XMFLOAT2*)>(
						[](const XMFLOAT2* vec) {return vec->x; }),
					std::function <void(XMFLOAT2*, float)>(
						[](XMFLOAT2* vec, float v) {vec->x = v; }))
				.addProperty(xorstr("y").crypt_get(),
					std::function <float(const XMFLOAT2*)>(
						[](const XMFLOAT2* vec) {return vec->y; }),
					std::function <void(XMFLOAT2*, float)>(
						[](XMFLOAT2* vec, float v) {vec->y = v; }))
				.addFunction(xorstr("__eq").crypt_get(),		//https://github.com/vinniefalco/LuaBridge/blob/d07c50e76f65921b87bf70c806d5bafa20c0cdb0/Tests/Source/ClassTests.cpp#L554
					std::function <bool(XMFLOAT2*, XMFLOAT2*)>(
						[](XMFLOAT2* vec, XMFLOAT2* vec2) { return (vec->x == vec2->x && vec->y == vec2->y); }))
				.addFunction(xorstr("__add").crypt_get(),
					std::function <XMFLOAT2(XMFLOAT2*, XMFLOAT2*)>(
						[](XMFLOAT2* vec, XMFLOAT2* vec2) { return XMFLOAT2(vec->x + vec2->x, vec->y + vec2->y); }))
				.addFunction(xorstr("__sub").crypt_get(),
					std::function <XMFLOAT2(XMFLOAT2*, XMFLOAT2*)>(
						[](XMFLOAT2* vec, XMFLOAT2* vec2) { return XMFLOAT2(vec->x - vec2->x, vec->y - vec2->y); }))
				.addFunction(xorstr("__mul").crypt_get(),
					std::function <XMFLOAT2(XMFLOAT2*, XMFLOAT2*)>(
						[](XMFLOAT2* vec, XMFLOAT2* vec2) { return XMFLOAT2(vec->x * vec2->x, vec->y * vec2->y); }))
				.addFunction(xorstr("__div").crypt_get(),
					std::function <XMFLOAT2(XMFLOAT2*, XMFLOAT2*)>(
						[](XMFLOAT2* vec, XMFLOAT2* vec2) { return XMFLOAT2(vec->x / vec2->x, vec->y / vec2->y); }))
				.addConstructor<void(*) (float, float)>()
			.endClass()
			.beginClass <Matrix>(xorstr("Matrix").crypt_get())
				.addFunction(xorstr("GetCameraVec").crypt_get(), &Matrix::GetCameraVec)
				.addFunction(xorstr("GetCameraAngle").crypt_get(), &Matrix::GetCameraAngle)
			.endClass()
			.addFunction(xorstr("Predict").crypt_get(), &Predict)
		.endNamespace()
		.beginNamespace(xorstr("Struct").crypt_get())
			.beginClass<BestBoneResult>(xorstr("BestBoneResult").crypt_get())
				.addProperty(xorstr("bFound").crypt_get(),
					std::function <bool(const BestBoneResult*)>(
						[](const BestBoneResult* in) { return in->bFound; }),
					std::function <void(BestBoneResult*, bool)>(
						[](BestBoneResult* in, bool v) { in->bFound = v; }))
				.addProperty(xorstr("foundBone").crypt_get(),
					std::function <int(const BestBoneResult*)>(
						[](const BestBoneResult* in) { return in->foundBone; }),
					std::function <void(BestBoneResult*, int)>(
						[](BestBoneResult* in, int v) { in->foundBone = v; }))
				.addProperty(xorstr("lastFov").crypt_get(),
					std::function <float(const BestBoneResult*)>(
						[](const BestBoneResult* in) { return in->lastFov; }),
					std::function <void(BestBoneResult*, float)>(
						[](BestBoneResult* in, float v) { in->lastFov = v; }))
			.endClass()
			.beginClass<RayCastResult>(xorstr("RayCastResult").crypt_get())
				.addProperty(xorstr("bHitted").crypt_get(),
					std::function <bool(const RayCastResult*)>(
						[](const RayCastResult* in) { return in->bHitted; }),
					std::function <void(RayCastResult*, bool)>(
						[](RayCastResult* in, bool v) { in->bHitted = v; }))
				.addProperty(xorstr("hittedPlayer").crypt_get(),
					std::function <Player(const RayCastResult*)>(
						[](const RayCastResult* in) { return in->hittedPlayer; }),
					std::function <void(RayCastResult*, Player)>(
						[](RayCastResult* in, Player v) { in->hittedPlayer = v; }))
				.addProperty(xorstr("hittedBone").crypt_get(),
					std::function <uint32_t(const RayCastResult*)>(
						[](const RayCastResult* in) { return in->hittedBone; }),
					std::function <void(RayCastResult*, uint32_t)>(
						[](RayCastResult* in, uint32_t v) { in->hittedBone = v; }))
				.addProperty(xorstr("hittedVec").crypt_get(),
					std::function <XMFLOAT3(const RayCastResult*)>(
						[](const RayCastResult* in) { return in->hittedVec; }),
					std::function <void(RayCastResult*, XMFLOAT3)>(
						[](RayCastResult* in, XMFLOAT3 v) { in->hittedVec = v; }))
				.addProperty(xorstr("hittedRot").crypt_get(),
					std::function <XMFLOAT3(const RayCastResult*)>(
						[](const RayCastResult* in) { return in->hittedRot; }),
					std::function <void(RayCastResult*, XMFLOAT3)>(
						[](RayCastResult* in, XMFLOAT3 v) { in->hittedRot = v; }))
			.endClass()
			.beginClass<SkillInfo>(xorstr("SkillInfo").crypt_get())
				.addProperty(xorstr("isUsing").crypt_get(),
					std::function <uint32_t(const SkillInfo*)>(
						[](const SkillInfo* in) { return in->isUsing; }),
					std::function <void(SkillInfo*, uint32_t)>(
						[](SkillInfo* in, uint32_t v) { in->isUsing = v; }))
				.addProperty(xorstr("flUltGauge").crypt_get(),
					std::function <float(const SkillInfo*)>(
						[](const SkillInfo* in) { return in->flUltGauge; }),
					std::function <void(SkillInfo*, float)>(
						[](SkillInfo* in, float v) { in->flUltGauge = v; }))
				.addProperty(xorstr("isBlocked").crypt_get(),
					std::function <uint32_t(const SkillInfo*)>(
						[](const SkillInfo* in) { return in->isBlocked; }),
					std::function <void(SkillInfo*, uint32_t)>(
						[](SkillInfo* in, uint32_t v) { in->isBlocked = v; }))
				.addFunction(xorstr("GetCoolTime").crypt_get(), &SkillInfo::GetCoolTime)
			.endClass()
			.beginClass<InputInfo>(xorstr("InputInfo").crypt_get())
				.addProperty(xorstr("key_forward").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_forward; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_forward = v; }))
				.addProperty(xorstr("key_back").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_back; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_back = v; }))
				.addProperty(xorstr("key_left").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_left; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_left = v; }))
				.addProperty(xorstr("key_right").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_right; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_right = v; }))
				.addProperty(xorstr("key_jump").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_jump; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_jump = v; }))
				.addProperty(xorstr("key_duck").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_duck; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_duck = v; }))
				.addProperty(xorstr("key_reload").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_reload; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_reload = v; }))
				.addProperty(xorstr("key_mouse_left").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_mouse_left; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_mouse_left = v; }))
				.addProperty(xorstr("key_mouse_right").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_mouse_right; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_mouse_right = v; }))
				.addProperty(xorstr("key_interact").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_interact; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_interact = v; }))
				.addProperty(xorstr("key_melee").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_melee; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_melee = v; }))
				.addProperty(xorstr("key_ability_1").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_ability_1; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_ability_1 = v; }))
				.addProperty(xorstr("key_ability_2").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_ability_2; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_ability_2 = v; }))
				.addProperty(xorstr("key_ability_3").crypt_get(),
					std::function <bool(const InputInfo*)>(
						[](const InputInfo* in) { return in->key_ability_3; }),
					std::function <void(InputInfo*, bool)>(
						[](InputInfo* in, bool v) { in->key_ability_3 = v; }))
			.endClass()
			.beginClass<MouseInput>(xorstr("MouseInput").crypt_get())
				.addProperty(xorstr("dx").crypt_get(),
					std::function <int(const MouseInput*)>(
						[](const MouseInput* in) { return in->dx; }),
					std::function <void(MouseInput*, int)>(
						[](MouseInput* in, int v) { in->dx = v; }))
				.addProperty(xorstr("dy").crypt_get(),
					std::function <int(const MouseInput*)>(
						[](const MouseInput* in) { return in->dy; }),
					std::function <void(MouseInput*, int)>(
						[](MouseInput* in, int v) { in->dy = v; }))
				.addProperty(xorstr("btn").crypt_get(),
					std::function <int(const MouseInput*)>(
						[](const MouseInput* in) { return in->btn; }),
					std::function <void(MouseInput*, int)>(
						[](MouseInput* in, int v) { in->btn = v; }))
			.endClass()
			.beginClass<ID3D11ShaderResourceView>(xorstr("ResourceInfo").crypt_get())
			.endClass()
		.endNamespace()
		.beginNamespace(xorstr("Game").crypt_get())
			.addVariable(xorstr("Engine").crypt_get(), &g_pEngine, false)
			.addVariable(xorstr("Renderer").crypt_get(), &g_pRenderer, false)
			.addVariable(xorstr("VarManager").crypt_get(), &g_pVariableManager, false)
			.beginNamespace(xorstr("Utils").crypt_get())
				.addFunction(xorstr("IsKeyPressed").crypt_get(), utils::IsKeyPressed)
				.addFunction(xorstr("SendInput").crypt_get(), &utils::SendInput)
			.endNamespace()
			.beginNamespace(xorstr("Settings").crypt_get())
				.addVariable(xorstr("settings_limit_fps").crypt_get(), &function::settings_limit_fps, true)
				.addVariable(xorstr("settings_draw_fov").crypt_get(), &function::settings_draw_fov, true)
				.addVariable(xorstr("settings_esp_text").crypt_get(), &function::settings_esp_text, true)
				.addVariable(xorstr("settings_esp_skeleton").crypt_get(), &function::settings_esp_skeleton, true)
				.addVariable(xorstr("settings_esp_skeleton_color_enemy_visible").crypt_get(), &function::settings_esp_skeleton_color_enemy_visible, true)
				.addVariable(xorstr("settings_esp_skeleton_color_enemy_invisible").crypt_get(), &function::settings_esp_skeleton_color_enemy_invisible, true)
				.addVariable(xorstr("settings_esp_skeleton_useraycast").crypt_get(), &function::settings_esp_skeleton_useraycast, true)
				.addVariable(xorstr("settings_esp_highlight").crypt_get(), &function::settings_esp_highlight, true)
				.addVariable(xorstr("settings_esp_highlight_color_enemy_visible").crypt_get(), &function::settings_esp_highlight_color_enemy_visible, true)
				.addVariable(xorstr("settings_esp_highlight_color_enemy_invisible").crypt_get(), &function::settings_esp_highlight_color_enemy_invisible, true)
				.addVariable(xorstr("settings_esp_highlight_color_team").crypt_get(), &function::settings_esp_highlight_color_team, true)
			.endNamespace()
		.endNamespace()
		.beginClass<ExportedEngine>(xorstr("Engine").crypt_get())
			.addFunction(xorstr("GetLocalPlayer").crypt_get(), &ExportedEngine::GetLocalPlayer)
			.addFunction(xorstr("GetPlayerAt").crypt_get(), &ExportedEngine::GetPlayerAt)
			.addFunction(xorstr("GetPlayerCount").crypt_get(), &ExportedEngine::GetPlayerCount)
			.addFunction(xorstr("GetTriggersAt").crypt_get(), &ExportedEngine::GetTriggersAt)
			.addFunction(xorstr("GetTriggersCount").crypt_get(), &ExportedEngine::GetTriggersCount)
			.addFunction(xorstr("GetProjectileAt").crypt_get(), &ExportedEngine::GetProjectileAt)
			.addFunction(xorstr("GetProjectileCount").crypt_get(), &ExportedEngine::GetProjectileCount)
			.addFunction(xorstr("GetViewMatrix").crypt_get(), &ExportedEngine::GetViewMatrix)
			.addFunction(xorstr("GetFov").crypt_get(), &ExportedEngine::GetFov)
			.addFunction(xorstr("SetFov").crypt_get(), &ExportedEngine::SetFov)
			.addFunction(xorstr("WorldToScreen").crypt_get(), &ExportedEngine::WorldToScreen)
			.addFunction(xorstr("IsInFov").crypt_get(), &ExportedEngine::IsInFov)
			.addFunction(xorstr("GetTick").crypt_get(), &ExportedEngine::GetTick)
			.addFunction(xorstr("LineOfSight").crypt_get(), &ExportedEngine::LineOfSight)
			.addFunction(xorstr("RayCast").crypt_get(), &ExportedEngine::RayCast)
			.addFunction(xorstr("RayCastWithIgnore").crypt_get(), &ExportedEngine::RayCastWithIgnore)
			.addFunction(xorstr("FindBestBone").crypt_get(), &ExportedEngine::FindBestBone)
			.addFunction(xorstr("GetMapId").crypt_get(), &ExportedEngine::GetMapId)
			.addFunction(xorstr("GetInputInfo").crypt_get(), &ExportedEngine::GetInputInfo)
			.addFunction(xorstr("GetSensitivity").crypt_get(), &ExportedEngine::GetSensitivity)
			.addFunction(xorstr("SetSensitivity").crypt_get(), &ExportedEngine::SetSensitivity)
			.addFunction(xorstr("GetController").crypt_get(), &ExportedEngine::GetController)
			.addFunction(xorstr("GetPath").crypt_get(), &ExportedEngine::GetPath)
		.endClass()
		.beginClass<ExportedRenderer>(xorstr("Renderer").crypt_get())
			.addFunction(xorstr("GetWidth").crypt_get(), &ExportedRenderer::GetWidth)
			.addFunction(xorstr("GetHeight").crypt_get(), &ExportedRenderer::GetHeight)
			.addFunction(xorstr("DrawText").crypt_get(), &ExportedRenderer::DrawOutlinedText)
			.addFunction(xorstr("DrawBox").crypt_get(), &ExportedRenderer::DrawBox)
			.addFunction(xorstr("DrawBoxFilled").crypt_get(), &ExportedRenderer::DrawBoxFilled)
			.addFunction(xorstr("DrawLine").crypt_get(), &ExportedRenderer::DrawLine)
			.addFunction(xorstr("Draw3DLine").crypt_get(), &ExportedRenderer::Draw3DLine)
			.addFunction(xorstr("DrawCircle").crypt_get(), &ExportedRenderer::DrawCircle)
			.addFunction(xorstr("DrawCircleFilled").crypt_get(), &ExportedRenderer::DrawCircleFilled)
			.addFunction(xorstr("DrawResource").crypt_get(), &ExportedRenderer::DrawResource)
			.addFunction(xorstr("LoadResource").crypt_get(), &ExportedRenderer::LoadResource)
			.addFunction(xorstr("DrawFOV").crypt_get(), &ExportedRenderer::DrawFOV)
		.endClass()
		.beginClass<ExportedVariableManager>(xorstr("VarManager").crypt_get())
			.addFunction(xorstr("Clear").crypt_get(), &ExportedVariableManager::Clear)
			.addFunction(xorstr("Size").crypt_get(), &ExportedVariableManager::Size)
			.addFunction(xorstr("IsEmpty").crypt_get(), &ExportedVariableManager::IsEmpty)
			.addFunction(xorstr("IsExist").crypt_get(), &ExportedVariableManager::IsExist)
			.addFunction(xorstr("Erase").crypt_get(), &ExportedVariableManager::Erase)
			.addFunction(xorstr("get_var_as_bool").crypt_get(), &ExportedVariableManager::get_var_as_bool)
			.addFunction(xorstr("set_var_as_bool").crypt_get(), &ExportedVariableManager::set_var_as_bool)
			.addFunction(xorstr("get_var_as_int").crypt_get(), &ExportedVariableManager::get_var_as_int)
			.addFunction(xorstr("set_var_as_int").crypt_get(), &ExportedVariableManager::set_var_as_int)
			.addFunction(xorstr("get_var_as_int64").crypt_get(), &ExportedVariableManager::get_var_as_int64)
			.addFunction(xorstr("set_var_as_int64").crypt_get(), &ExportedVariableManager::set_var_as_int64)
			.addFunction(xorstr("get_var_as_float").crypt_get(), &ExportedVariableManager::get_var_as_float)
			.addFunction(xorstr("set_var_as_float").crypt_get(), &ExportedVariableManager::set_var_as_float)
			.addFunction(xorstr("get_var_as_double").crypt_get(), &ExportedVariableManager::get_var_as_double)
			.addFunction(xorstr("set_var_as_double").crypt_get(), &ExportedVariableManager::set_var_as_double)
		.endClass()
		.beginClass<Player>(xorstr("Player").crypt_get())
			.addConstructor<void(*) (void)>()
			.addFunction(xorstr("__eq").crypt_get(),		//https://github.com/vinniefalco/LuaBridge/blob/d07c50e76f65921b87bf70c806d5bafa20c0cdb0/Tests/Source/ClassTests.cpp#L554
				std::function <bool(Player*, Player*)>(
					[](Player* vec, Player* vec2) { return (vec->GetCommon() == vec2->GetCommon()) && (vec->GetReaper() == vec2->GetReaper()); }))
			.addFunction(xorstr("IsValid").crypt_get(), &Player::IsValid)
			.addFunction(xorstr("GetPlayerId").crypt_get(), &Player::GetPlayerId)
			.addFunction(xorstr("GetMesh").crypt_get(), &Player::GetMesh)
			.addFunction(xorstr("GetTeam").crypt_get(), &Player::GetTeam)
			.addFunction(xorstr("GetStat").crypt_get(), &Player::GetStat)
			.addFunction(xorstr("GetHealth").crypt_get(), &Player::GetHealth)
			.addFunction(xorstr("GetIdentifier").crypt_get(), &Player::GetIdentifier)
			.addFunction(xorstr("GetHighlight").crypt_get(), &Player::GetHighlight)
			.addFunction(xorstr("GetVisibility").crypt_get(), &Player::GetVisibility)
			.addFunction(xorstr("GetAngle").crypt_get(), &Player::GetAngle)
			.addFunction(xorstr("GetSkill").crypt_get(), &Player::GetSkill)
			.addFunction(xorstr("GetBoneId").crypt_get(), &Player::GetBoneId)
			.addFunction(xorstr("IsEnemy").crypt_get(), &Player::IsEnemy)
		.endClass()
		.beginClass<Triggers>(xorstr("Triggers").crypt_get())
			.addFunction(xorstr("GetPlayer").crypt_get(), &Triggers::GetPlayer)
			.addFunction(xorstr("GetTriggersId").crypt_get(), &Triggers::GetTriggersId)
			.addFunction(xorstr("GetMesh").crypt_get(), &Triggers::GetMesh)
			.addFunction(xorstr("GetTeam").crypt_get(), &Triggers::GetTeam)
			.addFunction(xorstr("GetHealth").crypt_get(), &Triggers::GetHealth)
			.addFunction(xorstr("IsEnemy").crypt_get(), &Triggers::IsEnemy)
		.endClass()
		.beginClass<Projectile>(xorstr("Projectile").crypt_get())
			.addFunction(xorstr("GetProjectileId").crypt_get(), &Projectile::GetProjectileId)
			.addFunction(xorstr("GetProjectileInfo").crypt_get(), &Projectile::GetProjectile)
			.addFunction(xorstr("GetTeam").crypt_get(), &Projectile::GetTeam)
			.addFunction(xorstr("IsEnemy").crypt_get(), &Projectile::IsEnemy)
			.addFunction(xorstr("GetOwner").crypt_get(), &Projectile::GetOwner)
		.endClass()
		.beginClass<ControlComponent>(xorstr("Control").crypt_get())
			.addProperty(xorstr("move_side").crypt_get(),
				std::function <uint8_t(const ControlComponent*)>(
					[](const ControlComponent* in) { return in->move_side; }),
				std::function <void(ControlComponent*, uint8_t)>(
					[](ControlComponent* in, uint8_t v) { in->move_side = v; }))
			.addProperty(xorstr("move_forward").crypt_get(),
				std::function <uint8_t(const ControlComponent*)>(
					[](const ControlComponent* in) { return in->move_forward; }),
				std::function <void(ControlComponent*, uint8_t)>(
					[](ControlComponent* in, uint8_t v) { in->move_forward = v; }))
			.addFunction(xorstr("SetKeyCode").crypt_get(), &ControlComponent::SetKeyCode)
			.addFunction(xorstr("GetAngle").crypt_get(), &ControlComponent::GetAngle)
			.addFunction(xorstr("SetAngle").crypt_get(), &ControlComponent::SetAngle)
		.endClass()
		.beginClass<MeshComponent>(xorstr("Mesh").crypt_get())
			.addFunction(xorstr("GetVelocity").crypt_get(), &MeshComponent::GetVelocity)
			.addFunction(xorstr("GetLocation").crypt_get(), &MeshComponent::GetLocation)
			.addFunction(xorstr("GetBonePos").crypt_get(), &MeshComponent::GetBonePos)
		.endClass()
		.beginClass<StatComponent>(xorstr("Stat").crypt_get())
			.addFunction(xorstr("GetBattleTag").crypt_get(), &StatComponent::GetBattleTag)
		.endClass()
		.beginClass<HealthComponent>(xorstr("Health").crypt_get())
			.addFunction(xorstr("GetHealth").crypt_get(), &HealthComponent::GetHealth)
			.addFunction(xorstr("GetArmour").crypt_get(), &HealthComponent::GetArmour)
			.addFunction(xorstr("GetBarrier").crypt_get(), &HealthComponent::GetBarrier)
			.addFunction(xorstr("GetLife").crypt_get(), &HealthComponent::GetLife)
			.addProperty(xorstr("isBarrierProjected").crypt_get(),
				std::function <bool(const HealthComponent*)>(
					[](const HealthComponent* in) { return in->isBarrierProjected; }),
				std::function <void(HealthComponent*, bool)>(
					[](HealthComponent* in, bool v) { in->isBarrierProjected = v; }))
			.addProperty(xorstr("isImmortal").crypt_get(),
				std::function <bool(const HealthComponent*)>(
					[](const HealthComponent* in) { return in->isImmortal; }),
				std::function <void(HealthComponent*, bool)>(
					[](HealthComponent* in, bool v) { in->isImmortal = v; }))
		.endClass()
		.beginClass<IdentifierComponent>(xorstr("Identifier").crypt_get())
			.addData(xorstr("HeroID").crypt_get(), &IdentifierComponent::HeroID)
			.addFunction(xorstr("GetHeroName").crypt_get(), &IdentifierComponent::GetHeroName)
		.endClass()
		.beginClass<HighlightComponent>(xorstr("Highlight").crypt_get())
			.addFunction(xorstr("SetHighlightInfo").crypt_get(), &HighlightComponent::SetHighlightInfo)
		.endClass()
		.beginClass<VisibilityComponent>(xorstr("Visibility").crypt_get())
			.addFunction(xorstr("IsVisible").crypt_get(), &VisibilityComponent::IsVisible)
		.endClass()
		.beginClass<SkillComponent>(xorstr("Skill").crypt_get())
			.addFunction(xorstr("GetSkillInfo").crypt_get(), &SkillComponent::GetSkillInfo)
		.endClass()
		.beginClass<AngleComponent>(xorstr("Angle").crypt_get())
			.addFunction(xorstr("GetAngleVec").crypt_get(), &AngleComponent::GetAngleVec)
		.endClass()
		.beginClass<ProjectileComponent>(xorstr("ProjectileInfo").crypt_get())
			.addFunction(xorstr("GetVelocity").crypt_get(), &ProjectileComponent::GetVelocity)
			.addFunction(xorstr("GetLocation").crypt_get(), &ProjectileComponent::GetLocation)
		.endClass();
}