#pragma once
#include <vector>
#include <mutex>

#include "memory.hpp"
#include "struct.hpp"
#include "enums.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "utils.hpp"

#ifndef VMP
#include <VMProtectSDK.h>
#endif

namespace engine
{

	extern uint64_t g_Manager;
	extern uint64_t g_CameraMgr;
	extern uint64_t g_EntityTArray;

	extern std::vector<Player> PlayerList;
	extern std::vector<Triggers> TriggersList;
	extern std::vector<Projectile> ProjectileList;
	extern Player LocalPlayer;

	extern Matrix g_ViewMatrix;
	extern float g_Fov;
	extern float g_LocalFov;

	void Update();

	Matrix GetViewMatrix();
	bool WorldToScreen(XMFLOAT3 worldPos, XMFLOAT2* OutPos);
	std::pair<bool, float> IsInFov(XMFLOAT3 worldPos, float fov);

	std::pair<eRayCast, RayCastOut> RayCast(const XMFLOAT3& from, const XMFLOAT3& to, const std::vector<Player>& entityList, int mask);
	BestBoneResult FindBestBone(Player player, float fov, int prefId, int mask);

	uint64_t GetMapId();
	InputInfo* GetInputInfo();
	float GetSensitivity();
	void SetSensitivity(float value);

}