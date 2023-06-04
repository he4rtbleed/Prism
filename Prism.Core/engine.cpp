#include "engine.hpp"
#include "d3d/renderer.hpp"

uint64_t engine::g_Manager;
uint64_t engine::g_CameraMgr;
uint64_t engine::g_EntityTArray;

std::vector<Player> engine::PlayerList;
std::vector<Triggers> engine::TriggersList;
std::vector<Projectile> engine::ProjectileList;
Player engine::LocalPlayer;

Matrix engine::g_ViewMatrix;
float engine::g_Fov;
float engine::g_LocalFov;

void engine::Update()
{
#ifndef VMP
	VMProtectBeginMutation("engine.update");
#endif
	g_Manager = decrypter::GetGManager();
	g_CameraMgr = decrypter::GetCameraMgr(g_Manager);
	g_EntityTArray = decrypter::GetEntityTArray(g_Manager);

	//
	// tbd: add a mutex so when you're copying from the buffer to the original container other threads don't get their iterators invalidated
	//
	std::vector<Player> bufferedPlayerList{};
	std::vector<Triggers> bufferedTriggersList{};
	std::vector<Projectile> bufferedProjectileList{};

	for (int i = 0; i < 0x1000; i++)
	{
		TArray CurrTArray = mem::RPM<TArray>(g_EntityTArray + 0x10 * i);

		if (CurrTArray.base_address && CurrTArray.num_elements && CurrTArray.num_elements < 100)
		{
			for (int y = 0; y < CurrTArray.num_elements; y++)
			{
				uint32_t CurrUID = mem::RPM<uint32_t>(CurrTArray.base_address + 0x10 * y);
				uint64_t CurrEntity = mem::RPM<uint64_t>(CurrTArray.base_address + 0x10 * y + 8);

				if (mem::RPM<uint32_t>(CurrEntity + OFFSET_PARENTPTR_UNIQUEID) == CurrUID)				//IsEntity
				{
					uint64_t Common_Linker = decrypter::GetCommonWithReaper(CurrEntity);
					uint64_t Triggers_Linker = decrypter::GetComponent(CurrEntity, eComp::COMP_TRIGGERS);
					uint64_t Projectile_Linker = decrypter::GetComponent(CurrEntity, eComp::COMP_PROJECTILE);

					if (Common_Linker)																			//IsPlayer
					{
						uint64_t Reaper = CurrEntity;

						if (CurrUID == mem::RPM<uint32_t>(g_Manager + 0x298))							//HERE	//41 ? ? 41 ? ? ? ? ? 00 48 8B 7C 24 20 0F 94 C0 C3
							engine::LocalPlayer = Player(Common_Linker, Reaper);
						else
							bufferedPlayerList.push_back(Player(Common_Linker, Reaper));
					}
					else if (Triggers_Linker)
					{
						bufferedTriggersList.push_back(Triggers(CurrEntity));
					}
					else if (Projectile_Linker)
					{
						bufferedProjectileList.push_back(Projectile(CurrEntity));
					}
				}
			}
		}
	}

	engine::PlayerList = bufferedPlayerList;
	engine::TriggersList = bufferedTriggersList;
	engine::ProjectileList = bufferedProjectileList;
#ifndef VMP
	VMProtectEnd();
#endif
}

Matrix engine::GetViewMatrix()
{
	__try
	{
		if (!g_CameraMgr || IsBadReadPtr((void*)g_CameraMgr, sizeof(g_CameraMgr)))
			return Matrix{};

		//48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 48 89 4C 24 08 57 41 54 41 55 41 56 41 57 48 81 EC ? ? 00 00 0F 28 05 ? ? ? ? 48 8D
			//48 85 C9 74 ? 48 8D 95 ? ? 00 00 E8 ? ? ? ? 85 C0 //caller
		uint64_t* encrypted_buf = (uint64_t*)(g_CameraMgr + 0x58);

		uint64_t decrypted_buf[8]{};

		uint64_t first_key = spoof_call((const void*)(mem::dwRetAddr), (uint64_t(_fastcall*)(uint64_t))(mem::dwGameBase + 0x689CF0), encrypted_buf[8]);

		uint64_t xor_keys[8]
		{
			0x95A722F6CDA2A6CC,
			0x0C5DFE1868DD06C6,
			0xCA068C27A81603A2,
			0x741CCFB27DC13530,
			0x4379DEC6FEC9D746,
			0x2F34A4CA2C733FEB,
			0x86246EE9BF7800E7,
			0xDE6792036449FD58
		};

		decrypter::DecryptData(encrypted_buf, 8, xor_keys, decrypted_buf, first_key);

		XMMATRIX viewport = mem::RPM<XMMATRIX>((uint64_t)decrypted_buf);
		XMMATRIX projection = mem::RPM<XMMATRIX>(g_CameraMgr + 0xA0);
		XMMATRIX view = XMMatrixMultiply(viewport, projection);
		return mem::RPM<Matrix>((uint64_t)&view);
	}
	__except (1) {}
}

bool engine::WorldToScreen(XMFLOAT3 worldPos, XMFLOAT2* OutPos)
{
	float screenX = (g_ViewMatrix.m11 * worldPos.x) + (g_ViewMatrix.m21 * worldPos.y) + (g_ViewMatrix.m31 * worldPos.z) + g_ViewMatrix.m41;
	float screenY = (g_ViewMatrix.m12 * worldPos.x) + (g_ViewMatrix.m22 * worldPos.y) + (g_ViewMatrix.m32 * worldPos.z) + g_ViewMatrix.m42;
	float screenW = (g_ViewMatrix.m14 * worldPos.x) + (g_ViewMatrix.m24 * worldPos.y) + (g_ViewMatrix.m34 * worldPos.z) + g_ViewMatrix.m44;

	float camX = cRenderer::Width * 0.5f;
	float camY = cRenderer::Height * 0.5f;

	float x = camX + (camX * screenX / screenW);
	float y = camY - (camY * screenY / screenW);

	OutPos->x = x;
	OutPos->y = y;

	if (screenW < 0.001f)
		return false;

	return true;
}

std::pair<bool, float> engine::IsInFov(XMFLOAT3 worldPos, float fov)
{
	XMFLOAT3 delta{};
	XMFLOAT3 camera_vec = engine::g_ViewMatrix.GetCameraVec();
	XMStoreFloat3(&delta, XMLoadFloat3(&worldPos) - XMLoadFloat3(&camera_vec));

	float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&delta)));

	delta.x /= dist;
	delta.y /= dist;
	delta.z /= dist;

	static constexpr auto radian_per_degree = 180.f / XM_PI;
	XMFLOAT3 camera_ang = engine::g_ViewMatrix.GetCameraAngle();
	float len = XMVectorGetX(XMVector3Length(XMLoadFloat3(&camera_ang) - XMLoadFloat3(&delta))) * radian_per_degree;

	return std::pair<bool, float>(len <= fov, len);
}

bool InitRayCastStruct(RayCastIn* rtIn, uint64_t g_Manager)
{
	__try
	{
		//E8 ? ? ? ? 0F 28 44 24 ? 33
		uint64_t a1 = (uint64_t)rtIn;
		*(uint32_t*)(a1 + 0x6C) = 0x80000004;
		*(uint64_t*)(a1 + 0x60) = a1 + 0x70;
		*(uint64_t*)(a1 + 0x90) = a1 + 0xA0;
		*(uint32_t*)(a1 + 0x9C) = 0x80000004;
		*(uint64_t*)(a1 + 0xD0) = a1 + 0xE0;
		*(uint32_t*)(a1 + 0xDC) = 0x80000008;
		*(uint64_t*)(a1 + 0x1C0) = g_Manager;

		uint64_t vTable = mem::RPM<uint64_t>(g_Manager);
		if (!vTable)
			return false;

		uint64_t vFunc = mem::RPM<uint64_t>(vTable + 0x68);
		if (!vFunc)
			return false;

		uint64_t result = spoof_call((PVOID)mem::dwRetAddr, reinterpret_cast<uint64_t(_fastcall*)(uint64_t)>(vFunc), g_Manager);
		if (!result)
			return false;

		vTable = mem::RPM<uint64_t>(result);
		if (!vTable)
			return false;

		vFunc = mem::RPM<uint64_t>(vTable + 0x8);
		if (!vFunc)
			return false;

		*(uint64_t*)(a1 + 0x1B8) = reinterpret_cast<uint64_t(_fastcall*)(uint64_t)>(vFunc)(result);
		*(uint32_t*)(a1 + 0x20) = 0x3F800000;
		*(uint64_t*)(a1 + 0x198) = 0x00010000;

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

std::pair<eRayCast, RayCastOut> engine::RayCast(const XMFLOAT3& from, const XMFLOAT3& to, const std::vector<Player>& entityList, int mask)
{
	std::pair<eRayCast, RayCastOut> result{};
	result.first = eRayCast::HIT_FAILED;
	if (g_Manager)
	{
        auto fnInitStruct = (void(_fastcall*)(uint64_t*, uint64_t*))(mem::dwGameBase + 0xBB59F0);			//E8 ? ? ? ? FF C3 48 83 C7 10 3B 1D ? ? ? ? 0F
		auto fnRayCast = (eRayCast(_fastcall*)(RayCastIn*, RayCastOut*, int))(mem::dwGameBase + 0xBA5A10); //E8 ? ? ? ? 8B F8 48 8B 76
		auto fnGetHittedBone = (uint32_t(_fastcall*)(uint64_t, RayCastOut))(mem::dwGameBase + 0xD14190); //E8 ? ? ? ? 49 8B 4C 24 08 48 8D 95 ? ? 00 00

        RayCastIn rtIn{};
		RayCastOut rtOut{};

		if (InitRayCastStruct(&rtIn, engine::g_Manager))
		{
			rtIn.from = from;
			rtIn.to = to;

			switch (mask)
			{
			case eMask::MASK_DEFAULT:
				rtIn.mask = 0;
				break;
			case eMask::MASK_IGNORESHIELD:
				rtIn.mask = 0x690000000000051;
				break;
			case eMask::MASK_FAILED_ON_PLAYER_IGNORE_OBJECT:
				rtIn.mask = 0x690000000000015;
				break;
			case eMask::MASK_FAILED_ON_OBJECT_IGNORE_PLAYER:
				rtIn.mask = 0x690000000000016;
				break;
			case eMask::MASK_IGNOREALL:
				rtIn.mask = 0x690000000000017;
				break;
			default:
				break;
			}

			for (Player entity : entityList)
			{
				uint64_t Common = entity.GetCommon();
				if (Common)
					fnInitStruct(&rtIn.offsetIn, &Common);
			}

			result.first = fnRayCast(&rtIn, &rtOut, 0);
			if (result.first && rtOut.hittedEntity)
				rtOut.hittedBone = fnGetHittedBone(rtOut.hittedEntity, rtOut);
			result.second = rtOut;
		}
	}

	return result;
}

BestBoneResult engine::FindBestBone(Player player, float fov, int prefId, int mask)
{
	auto Identifier = player.GetIdentifier();
	auto Mesh = player.GetMesh();
	bool IsMeka = (Identifier->HeroID == eHero::HERO_DVA) && (Mesh->SkeletalMesh->BoneCount > 240);
	bool IsBastion = (Identifier->HeroID == eHero::HERO_BASTION);
	bool IsBot = (Identifier->HeroID >= eHero::HERO_TRAININGBOT1 && Identifier->HeroID <= eHero::HERO_TRAININGBOT4);
	int bones[] = { 0x11, 0x10,0xD, IsBot ? 0 : 0xE, (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x1C, 0x36, IsBot ? 0 : 0x37, (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x3A, IsMeka ? 0x6 : IsBot ? 0x3 : IsBastion ? 0x4 : 0x5, IsBot ? 0x92D : 0x4, IsBot ? 0 : 0x3, IsBot ? 0 : 0x55, IsBot ? 0 : 0x59, IsBot ? 0 : 0x5A, IsBot ? 0 : 0x5F, IsBot ? 0 : 0x63, IsBot ? 0 : 0x64 };

	// this vector isn't really needed
	std::vector<Player> entityList{};
	entityList.push_back(engine::LocalPlayer);
	entityList.push_back(player);
	XMFLOAT3 bonePos = Mesh->GetBonePos(prefId);
	std::pair<bool, float> result = engine::IsInFov(bonePos, fov);
	if (result.first)
		if (engine::RayCast(engine::g_ViewMatrix.GetCameraVec(), bonePos, entityList, mask).first == eRayCast::HIT_SUCCESS)
			return BestBoneResult{ true, prefId, result.second };

	for (int bone : bones)
	{
		if (bone && bone != prefId)
		{
			XMFLOAT3 bonePos = Mesh->GetBonePos(bone);
			std::pair<bool, float> result = engine::IsInFov(bonePos, fov);
			if (result.first)
				if (engine::RayCast(engine::g_ViewMatrix.GetCameraVec(), bonePos, entityList, mask).first == eRayCast::HIT_SUCCESS)
					return BestBoneResult{ true, bone, result.second };
		}
	}

	return BestBoneResult{false, 0, 0.f};
}

uint64_t engine::GetMapId()
{
	//48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ?
	uint64_t ptr = decrypter::GetGlobalPtr(0x30);
	return mem::RPM<uint64_t>(ptr + 0x18); //0800000000000688 - practice range map id
}

InputInfo* engine::GetInputInfo()
{
	uint64_t ptr = decrypter::GetGlobalPtr(0x5);
	return (InputInfo*)ptr;
}

float engine::GetSensitivity()
{
	uint64_t ptr = decrypter::GetGlobalPtr(0x5);
	return mem::RPM<float>(ptr + 0x6A4);
}

void engine::SetSensitivity(float value)
{
	uint64_t ptr = decrypter::GetGlobalPtr(0x5);
	mem::WPM<float>(ptr + 0x6A4, value);
}

bool Player::IsEnemy()
{
	auto Team = GetTeam();
	auto LocalTeam = engine::LocalPlayer.GetTeam();
	if (Team && LocalTeam)
		return Team->GetTeam() == eTeam::TEAM_DEATHMATCH || Team->GetTeam() != LocalTeam->GetTeam();

	return false;
}

bool Triggers::IsEnemy()
{
	auto Team = GetTeam();
	auto LocalTeam = engine::LocalPlayer.GetTeam();
	if (Team && LocalTeam)
		return Team->GetTeam() == eTeam::TEAM_DEATHMATCH || Team->GetTeam() != LocalTeam->GetTeam();

	return false;
}

bool Projectile::IsEnemy()
{
	auto Team = GetTeam();
	auto LocalTeam = engine::LocalPlayer.GetTeam();
	if (Team && LocalTeam)
		return Team->GetTeam() == eTeam::TEAM_DEATHMATCH || Team->GetTeam() != LocalTeam->GetTeam();

	return false;
}

Player Projectile::GetOwner()
{
	ProjectileComponent* comp = this->GetProjectile();
	if (comp)
	{
		uint64_t Common = decrypter::GetEntityWithUID(engine::g_Manager, comp->OwnerID);
		if (Common)
		{
			uint64_t Reaper = decrypter::GetReaperWithCommon(Common);
			if (Reaper)
			{
				return Player(Common, Reaper);
			}
		}
	}

	return Player();
}
