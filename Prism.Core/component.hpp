#pragma once
#include <Windows.h>
#include <cstdint>
#include <bitset>

#include "struct.hpp"
#include "decrypter.hpp"
#include "enums.hpp"

#include <xorstr/include/xorstr.hpp>

class Component
{

public:
	uint64_t vfunc_table;		        // 0x0
	uint64_t component_parent;	        // 0x8
	byte component_id;			        // 0x10
	char pad_02[0xF];			        // 0x11

};

class ControlComponent : public Component
{

public:
	char pad_01[0x100F];				// 0x20
	uint8_t move_side;					// 0x102F
	uint8_t move_forward;				// 0x1030
	char pad_02[0x3];					// 0x1031
	uint32_t KeyCode;					// 0x1034
	char pad_03[0x58];					// 0x1038
	XMFLOAT3 Angle;						// 0x1090

	void SetKeyCode(int key)
	{
		this->KeyCode |= key;
	}

	XMFLOAT3 GetAngle()
	{
		return this->Angle;
	}

	void SetAngle(XMFLOAT3 _Angle)
	{
		this->Angle = _Angle;
	}

};

class MeshComponent : public Component
{

public:
	char pad_01[0x30];                  // 0x20
	XMFLOAT3 velocity;                  // 0x50
	char pad_02[0xE4];                  // 0x5C
	XMFLOAT3 location;                  // 0x140
	char pad_03[0x304];                 // 0x14C
	uint64_t encrypted_buf[10];         // 0x450
	char pad_04[0x220];                 // 0x4A0

	class SkeletalMesh
	{

	public:
		class SkelIds
		{
		public:
			char pad_01[0x30];          // 0x0
			uint64_t SkelIdList;        // 0x30
			char pad_02[0x1E];          // 0x38
			uint16_t SkelIdCount;       // 0x56
		} *SkelIds;                     // 0x0

		char pad_01[0x20];				// 0x8

		class StaticMesh
		{
		public:
			char pad_01[0x20];
			XMFLOAT3 location;
			char pad_02[0x4];
		} *StaticMesh;                  // 0x28

		char pad_02[0x18];              // 0x30

		uint16_t BoneCount;             // 0x48

	} *SkeletalMesh;                    // 0x6C0

	XMFLOAT3 GetVelocity()
	{
		return this->velocity;
	}

	// 41 ? 48 83 EC ? 0F 28 ? ? 48 ? ? ? ? ? ? 0F

	XMFLOAT3 GetLocation()
	{
		__try
		{
			uint64_t decrypted_buf[8];

			uint64_t xor_key[8] = {
				0xD70407D6BF1E81B9,
				0x4EFEDB381A6121B3,
				0x88A5A907DAAA24D7,
				0x36BFEA920F7D1245,
				0x1DAFBE68C75F033,
				0x6D9781EA5ECF189E,
				0xC4874BC9CDC42792,
				0x9CC4B72316F5DA2D
			};

			size_t size = sizeof(decrypted_buf) / sizeof(uint64_t);

			decrypter::DecryptData(encrypted_buf, size, xor_key, decrypted_buf);

			return *(XMFLOAT3*)((uint64_t)decrypted_buf + 0x30);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return XMFLOAT3{};
		}
	}

	XMFLOAT3 GetRotation()
	{
		__try
		{
			uint64_t decrypted_buf[8];

			uint64_t xor_key[8] = {
				0xD70407D6BF1E81B9,
				0x4EFEDB381A6121B3,
				0x88A5A907DAAA24D7,
				0x36BFEA920F7D1245,
				0x1DAFBE68C75F033,
				0x6D9781EA5ECF189E,
				0xC4874BC9CDC42792,
				0x9CC4B72316F5DA2D
			};

			size_t size = sizeof(decrypted_buf) / sizeof(uint64_t);

			decrypter::DecryptData(encrypted_buf, size, xor_key, decrypted_buf);

			return *(XMFLOAT3*)((uint64_t)decrypted_buf);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return XMFLOAT3{};
		}
	}

	XMMATRIX GetMeshMatrix()
	{
		__try
		{
			uint64_t decrypted_buf[8]{};

			static uint64_t xor_key[8] = {
				0xD70407D6BF1E81B9,
				0x4EFEDB381A6121B3,
				0x88A5A907DAAA24D7,
				0x36BFEA920F7D1245,
				0x1DAFBE68C75F033,
				0x6D9781EA5ECF189E,
				0xC4874BC9CDC42792,
				0x9CC4B72316F5DA2D
			};

			size_t size = sizeof(decrypted_buf) / sizeof(uint64_t);

			decrypter::DecryptData(encrypted_buf, size, xor_key, decrypted_buf);

			return *(XMMATRIX*)(decrypted_buf);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return XMMATRIX{};
		}
	}

	XMFLOAT3 GetBonePos(int id)
	{
		__try
		{
			for (int i = 0; i < this->SkeletalMesh->SkelIds->SkelIdCount; i++)
			{
				if (*(uint32_t*)(this->SkeletalMesh->SkelIds->SkelIdList + 4 * i) == id)
				{
					XMMATRIX mtx = GetMeshMatrix();

					XMFLOAT3 Bone = this->SkeletalMesh->StaticMesh[i].location;

					XMStoreFloat3(&Bone, XMVector3Transform(XMLoadFloat3(&Bone), GetMeshMatrix()));

					return Bone;
				}
			}

			return XMFLOAT3{};
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return XMFLOAT3{};
		}
	}

};

class TeamComponent : public Component
{

public:
	char pad_01[0x38];					// 0x20
	uint32_t Team;						// 0x58

	eTeam GetTeam()
	{
		return (eTeam)(this->Team & 0xF800000);
	}

};

class StatComponent : public Component
{

public:
	char pad_01[0xC0];					// 0x20
	uint64_t BattleTag;					// 0xE0
	uint32_t BattleTag_;				// 0xE8

	std::string GetBattleTag()
	{
		char result[MAX_PATH] = "";

		if (this->BattleTag_ == 1)
		{
			if (mem::RPM<uint32_t>(this->BattleTag + 0x14) <= 0x10)
				memcpy_s(result, MAX_PATH, (char*)this->BattleTag, MAX_PATH);
			else
				memcpy_s(result, MAX_PATH, (char*)(mem::RPM<uint64_t>(this->BattleTag)), MAX_PATH);
		}
		else
			memcpy_s(result, MAX_PATH, (char*)(mem::RPM<uint64_t>(this->BattleTag + 0x8)), MAX_PATH);

		return result;
	}

};

class HealthComponent : public Component
{

private:
	struct HealthInfo
	{
		float Max;
		float Curr;
		char pad_01[0xC];
	};

public:
	char pad_01[0xBC];					// 0x20
	XMFLOAT2 Health;					// 0xDC
	char pad_02[0x138];					// 0xE4
	XMFLOAT2 Armour;					// 0x21C
	char pad_03[0x138];					// 0x224
	XMFLOAT2 Barrier;					// 0x35C
	char pad_04[0x140];					// 0x364
	bool isBarrierProjected;			// 0x4A4
	bool isImmortal;					// 0x4A5

	XMFLOAT2 GetHealth()
	{
		__try
		{
			return Health;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

	XMFLOAT2 GetArmour()
	{
		__try
		{
			XMFLOAT2 Armour{};
			for (int i = 0; i < 10; i++)
			{
				HealthInfo armourEx = mem::RPM<HealthInfo>((uint64_t)&this->Armour + i * sizeof(HealthInfo));
				if (armourEx.Curr && armourEx.Max)
				{
					Armour.x += armourEx.Max;
					Armour.y += armourEx.Curr;
				}
			}

			return Armour;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

	XMFLOAT2 GetBarrier()
	{
		__try
		{
			XMFLOAT2 Barrier{};
			for (int i = 0; i < 10; i++)
			{
				HealthInfo barrierEx = mem::RPM<HealthInfo>((uint64_t)&this->Barrier + i * sizeof(HealthInfo));
				if (barrierEx.Curr && barrierEx.Max)
				{
					Barrier.x += barrierEx.Max;
					Barrier.y += barrierEx.Curr;
				}
			}

			return Barrier;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

	XMFLOAT2 GetLife()
	{
		__try
		{
			XMFLOAT2 Life{};
			XMFLOAT2 Health = GetHealth(), Armour = GetArmour(), Barrier = GetBarrier();
			XMStoreFloat2(&Life, XMLoadFloat2(&Health) + XMLoadFloat2(&Armour) + XMLoadFloat2(&Barrier));
			return Life;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

};

class IdentifierComponent : public Component
{

public:
	char pad_01[0xB0];				    // 0x20
	uint64_t HeroID;				    // 0xD0

	// just make an unordered_map that's initialized statically / once at runtime
	std::string GetHeroName()
	{
		auto Mesh = (MeshComponent*)decrypter::GetComponent(decrypter::GetCommonWithReaper(decrypter::DecryptParent((uint64_t)this)), eComp::COMP_MESH);
		switch (HeroID)
		{
		case eHero::HERO_REAPER:
			return xorstr("Reaper").crypt_get();
		case eHero::HERO_TRACER:
			return xorstr("Tracer").crypt_get();
		case eHero::HERO_MERCY:
			return xorstr("Mercy").crypt_get();
		case eHero::HERO_HANZO:
			return xorstr("Hanzo").crypt_get();
		case eHero::HERO_TORBJORN:
			return xorstr("Torbjorn").crypt_get();
		case eHero::HERO_REINHARDT:
			return xorstr("Reinhardt").crypt_get();
		case eHero::HERO_PHARAH:
			return xorstr("Pharah").crypt_get();
		case eHero::HERO_WINSTON:
			return xorstr("Winston").crypt_get();
		case eHero::HERO_WIDOWMAKER:
			return xorstr("Widowmaker").crypt_get();
		case eHero::HERO_BASTION:
			return xorstr("Bastion").crypt_get();
		case eHero::HERO_SYMMETRA:
			return xorstr("Symmetra").crypt_get();
		case eHero::HERO_ZENYATTA:
			return xorstr("Zenyatta").crypt_get();
		case eHero::HERO_GENJI:
			return xorstr("Genji").crypt_get();
		case eHero::HERO_ROADHOG:
			return xorstr("Roadhog").crypt_get();
		case eHero::HERO_MCCREE:
			return xorstr("McCree").crypt_get();
		case eHero::HERO_JUNKRAT:
			return xorstr("Junkrat").crypt_get();
		case eHero::HERO_ZARYA:
			return xorstr("Zarya").crypt_get();
		case eHero::HERO_SOLDIER76:
			return xorstr("Soldier 76").crypt_get();
		case eHero::HERO_LUCIO:
			return xorstr("Lucio").crypt_get();
		case eHero::HERO_DVA:
			if (Mesh->SkeletalMesh->BoneCount > 240)
				return xorstr("D.Va").crypt_get();
			else
				return xorstr("Hana").crypt_get();
		case eHero::HERO_MEI:
			return xorstr("Mei").crypt_get();
		case eHero::HERO_ANA:
			return xorstr("Ana").crypt_get();
		case eHero::HERO_SOMBRA:
			return xorstr("Sombra").crypt_get();
		case eHero::HERO_ORISA:
			return xorstr("Orisa").crypt_get();
		case eHero::HERO_DOOMFIST:
			return xorstr("Doomfist").crypt_get();
		case eHero::HERO_MOIRA:
			return xorstr("Moira").crypt_get();
		case eHero::HERO_BRIGITTE:
			return xorstr("Brigitte").crypt_get();
		case eHero::HERO_WRECKINGBALL:
			return xorstr("Wrecking Ball").crypt_get();
		case eHero::HERO_ASHE:
			return xorstr("Ashe").crypt_get();
		case eHero::HERO_ECHO:
			return xorstr("Echo").crypt_get();
		case eHero::HERO_BAPTISTE:
			return xorstr("Baptiste").crypt_get();
		case eHero::HERO_SIGMA:
			return xorstr("Sigma").crypt_get();
		case eHero::HERO_TRAININGBOT1:
			return xorstr("Training Bot1").crypt_get();
		case eHero::HERO_TRAININGBOT2:
			return xorstr("Training Bot2").crypt_get();
		case eHero::HERO_TRAININGBOT3:
			return xorstr("Training Bot3").crypt_get();
		case eHero::HERO_TRAININGBOT4:
			return xorstr("Training Bot4").crypt_get();
		default:
			return xorstr("default").crypt_get();
		}
	}
};

class HighlightComponent : public Component
{

public:
	uint64_t encrypted;					// 0x20

	void SetHighlightInfo(uint32_t HighlightType, uint32_t HighlightColor)
	{
		__try
		{
			uint64_t highlight = reinterpret_cast<uint64_t(__fastcall*)(uint64_t*)>(mem::dwGameBase + 0x5FFD70)(&this->encrypted);			//48 8B D8 48 8B 48 ? E8 ? ? ? ? 4C 8B 53 ? - 5
			uint64_t xor_key = spoof_call((PVOID)(mem::dwRetAddr), reinterpret_cast<uint64_t(__fastcall*)(uint64_t)>(mem::dwGameBase + 0x572000), mem::RPM<uint64_t>(highlight + 0x18)); //48 8B D8 48 8B 48 ? E8 ? ? ? ? 4C 8B 53 ? + 7
			mem::WPM<uint64_t>(highlight + 0x10, xor_key ^ HighlightType);

			uint64_t highlight_color = reinterpret_cast<uint64_t(__fastcall*)(uint64_t*)>(mem::dwGameBase + 0x5FFBE0)(&this->encrypted);	//? 8B CE E8 ? ? ? ? ? 8B CE 48 8B F8 E8 ? ? ? ? ? 8B CE 48 8B D8 E8 ? ? ? ? + 0x1F
			mem::WPM<uint32_t>(highlight_color + 0x10, HighlightColor);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

};

class VisibilityComponent : public Component
{

public:
	char pad_01[0x78];					// 0x20
	uint64_t encrypted_data;			// 0x98
	uint64_t encrypted_key;				// 0xA0

	bool IsVisible()
	{
		__try
		{
			uint64_t decrypted_key = spoof_call((PVOID)(mem::dwRetAddr), reinterpret_cast<uint64_t(__fastcall*)(uint64_t)>(mem::dwGameBase + 0x5FD920), this->encrypted_key); //48 8B ? ? ? 00 00 E8 ? ? ? ? 48 8B C8 48 33 ? ? ? 00
			uint64_t result = this->encrypted_data ^ decrypted_key;
			return result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

};

static SkillInfo null_skill{};
class SkillComponent : public Component
{

public:
	char pad_01[0xB0];				    // 0x20
	uint64_t skill_struct;				// 0xD0
	
	SkillInfo* GetSkillInfo(int skill_enums_id, int skill_enums_extra_id = 0)
	{
		//40 53 41 56 48 83 EC ? 33 DB 4C 8B F2 66 39 5A 02
		__try
		{
			uint16_t skillId[20] = {};
			switch ((uint16_t)skill_enums_id)
			{
			case eSkill::SKILL_1:
				skillId[1] = 0x28E3;
				break;
			case eSkill::SKILL_2:
				skillId[1] = 0x28E9;
				break;
			case eSkill::SKILL_L:
				skillId[1] = 0x2B1;
				break;
			case eSkill::SKILL_R:
				skillId[1] = 0x2B2;
				break;
			case eSkill::SKILL_ULT:
				skillId[1] = 0x156;
				break;
			case eSkill::SKILL_COOLTIME:
				skillId[1] = 0x3D;
				break;
			case eSkill::SKILL_REMAINTIME:
				skillId[1] = 0xD7;
				break;
			case eSkill::SKILL_DELAY:
				skillId[1] = 0xAD;
				break;
			default:
				skillId[0] = (uint16_t)skill_enums_extra_id, skillId[1] = (uint16_t)skill_enums_id;
				break;
			}

			auto info = (uint64_t)&this->skill_struct;
			if (!info)
				return nullptr;

			auto r9{0ull};
			if (!skillId[0])
			{
				r9 = mem::RPM<uint64_t>(info + 0x1F0);
			}
			else
			{
				auto r8 = mem::RPM<uint32_t>(info + 0x40);
				if (!r8)
					return nullptr;

				auto rax = mem::RPM<uint64_t>(info + 0x38);
				if (mem::RPM<uint16_t>(rax + 0x8) > skillId[0])
					skillId[0] += mem::RPM<uint16_t>(rax + 0x8) - 1;

				for (int i = 0; i < r8; i++)
				{
					if (mem::RPM<uint16_t>(rax + 0x8) == skillId[0])
					{
						r9 = mem::RPM<uint64_t>(rax);
						break;
					}
					rax += 0x10;
				}

				if (!r9)
					return nullptr;

				r9 = mem::RPM<uint64_t>(r9 + 0x98);
			}
			if (!r9)
				return nullptr;

			uint16_t skill_type = skillId[1];
			uint64_t r8 = 32 * ((skill_type & 0xF) + 1);
			int32_t eax = mem::RPM<int32_t>(r8 + r9 + 8) - 1;
			if (eax < 0)
				return (SkillInfo*)&null_skill;// reinterpret_cast<SkillInfo* (__fastcall*)(uint64_t, uint16_t)>(mem::dwGameBase + 0x1AEBD80)(r9, skill_type);
			uint64_t skill_table = mem::RPM<uint64_t>(r8 + r9) + eax * 0x10;
			while (mem::RPM<uint16_t>(skill_table) != skill_type)
			{
				skill_table -= 0x10;
				if (--eax < 0)
					return (SkillInfo*)&null_skill;// reinterpret_cast<SkillInfo* (__fastcall*)(uint64_t, uint16_t)>(mem::dwGameBase + 0x1AEBD80)(r9, skill_type);
			}
			uint64_t result = mem::RPM<uint64_t>(skill_table + 8);
			if (!result)
				return (SkillInfo*)&null_skill;// reinterpret_cast<SkillInfo* (__fastcall*)(uint64_t, uint16_t)>(mem::dwGameBase + 0x1AEBD80)(r9, skill_type);

			return (SkillInfo*)result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return nullptr;
		}
	}
	
};

class AngleComponent : Component
{

private:
	struct AngleInfo
	{
		char pad_01[0x20];			// 0x0
		XMFLOAT3 angle;				// 0x20
	};

public:
	char pad_01[0x80];				// 0x20
	AngleInfo* pAngle;				// 0xA0

	//48 89 5C 24 10 56 48 83 EC 20 48 63 81 ?
	XMFLOAT3 GetAngleVec()
	{
		return this->pAngle->angle;
	}

};

class ProjectileComponent : Component
{

public:
	uint32_t OwnerID;				// 0x20
	char pad_01[0xBC];				// 0x24
	XMFLOAT3 Location;				// 0xE0
	char pad_02[0x4];				// 0xEC
	XMFLOAT3 Velocity;				// 0xF0

	XMFLOAT3 GetVelocity()
	{
		return this->Velocity;
	}

	XMFLOAT3 GetLocation()
	{
		return this->Location;
	}

};
