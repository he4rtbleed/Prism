#pragma once
#include <Windows.h>
#include "component.hpp"

class Player
{
private:
	uint64_t Common{}, Reaper{};

public:

	Player() { Common = NULL, Reaper = NULL; }
	Player(uint64_t a1, uint64_t a2) { Common = a1, Reaper = a2; }

	bool IsValid()
	{
		if (Common && Reaper)
			return true;
		return false;
	}

	uint64_t GetPlayerId()
	{
		uint64_t info = mem::RPM<uint64_t>(this->Common + 0x30) & 0xFFFFFFFFFFFFFFC0;
		return mem::RPM<uint64_t>(info + 0x10);
	}

	uint64_t GetCommon()
	{
		return Common;
	}

	uint64_t GetReaper()
	{
		return Reaper;
	}

	MeshComponent* GetMesh()
	{
		return (MeshComponent*)decrypter::GetComponent(this->Common, eComp::COMP_MESH);
	}

	TeamComponent* GetTeam()
	{
		return (TeamComponent*)decrypter::GetComponent(this->Common, eComp::COMP_TEAM);
	}

	StatComponent* GetStat()
	{
		return (StatComponent*)decrypter::GetComponent(this->Reaper, eComp::COMP_STAT);
	}

	HealthComponent* GetHealth()
	{
		return (HealthComponent*)decrypter::GetComponent(this->Common, eComp::COMP_HEALTH);
	}

	IdentifierComponent* GetIdentifier()
	{
		return (IdentifierComponent*)decrypter::GetComponent(this->Reaper, eComp::COMP_IDENTIFIER);
	}

	HighlightComponent* GetHighlight()
	{
		return (HighlightComponent*)decrypter::GetComponent(this->Common, eComp::COMP_HIGHLIGHT);
	}

	VisibilityComponent* GetVisibility()
	{
		return (VisibilityComponent*)decrypter::GetComponent(this->Reaper, eComp::COMP_VISIBILITY);
	}

	AngleComponent* GetAngle()
	{
		return (AngleComponent*)decrypter::GetComponent(this->Common, eComp::COMP_ANGLE);
	}

	SkillComponent* GetSkill()
	{
		return (SkillComponent*)decrypter::GetComponent(this->Common, eComp::COMP_SKILL);
	}

	bool IsEnemy();

	int GetBoneId(int CustomBone)
	{
		auto Identifier = this->GetIdentifier();
		auto Mesh = this->GetMesh();
		if (!Identifier || !Mesh || !Mesh->SkeletalMesh)
			return 0;
		bool IsMeka = (Identifier->HeroID == eHero::HERO_DVA) && (Mesh->SkeletalMesh->BoneCount > 240);
		bool IsBastion = (Identifier->HeroID == eHero::HERO_BASTION);
		bool IsBot = (Identifier->HeroID >= eHero::HERO_TRAININGBOT1 && Identifier->HeroID <= eHero::HERO_TRAININGBOT4);

		switch (CustomBone)
		{
		case eBone::BONE_HEAD:
			return 0x11;
		case eBone::BONE_NECK:
			return 0x10;
		case eBone::BONE_SPINE_01:
			return IsMeka ? 0x6 : IsBot ? 0x3 : IsBastion ? 0x4 : 0x5;
		case eBone::BONE_SPINE_02:
			return IsBot ? 0x92D : 0x4;
		case eBone::BONE_PELVIS:
			return IsBot ? 0 : 0x3;
		case eBone::BONE_LEFT_UPPER_ARM:
			return 0xD;
		case eBone::BONE_LEFT_LOWER_ARM:
			return IsBot ? 0 : 0xE;
		case eBone::BONE_LEFT_HAND:
			return (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x1C;
		case eBone::BONE_RIGHT_UPPER_ARM:
			return 0x36;
		case eBone::BONE_RIGHT_LOWER_ARM:
			return IsBot ? 0 : 0x37;
		case eBone::BONE_RIGHT_HAND:
			return (IsBot | IsBastion) ? 0x0 : IsBot ? 0 : 0x3A;
		case eBone::BONE_LEFT_THIGH:
			return IsBot ? 0 : 0x55;
		case eBone::BONE_LEFT_CALF:
			return IsBot ? 0 : 0x59;
		case eBone::BONE_LEFT_FOOT:
			return IsBot ? 0 : 0x5A;
		case eBone::BONE_RIGHT_THIGH:
			return IsBot ? 0 : 0x5F;
		case eBone::BONE_RIGHT_CALF:
			return IsBot ? 0 : 0x63;
		case eBone::BONE_RIGHT_FOOT:
			return IsBot ? 0 : 0x64;
		default:
			return 0;
		}
	}
};

class Triggers
{
private:
	uint64_t Common{};

public:

	Triggers() { Common = NULL; }
	Triggers(uint64_t a1) { Common = a1; }

	Player GetPlayer()
	{
		uint64_t Reaper = decrypter::GetReaperWithCommon(this->Common);
		if (!Reaper)
			return Player();
		return Player(this->Common, Reaper);
	}

	uint64_t GetTriggersId()
	{
		uint64_t info = mem::RPM<uint64_t>(this->Common + 0x30) & 0xFFFFFFFFFFFFFFC0;
		return mem::RPM<uint64_t>(info + 0x10);
	}

	MeshComponent* GetMesh()
	{
		return (MeshComponent*)decrypter::GetComponent(this->Common, eComp::COMP_MESH);
	}

	TeamComponent* GetTeam()
	{
		return (TeamComponent*)decrypter::GetComponent(this->Common, eComp::COMP_TEAM);
	}

	HealthComponent* GetHealth()
	{
		return (HealthComponent*)decrypter::GetComponent(this->Common, eComp::COMP_HEALTH);
	}

	bool IsEnemy();

};

class Projectile
{
private:
	uint64_t Common{};

public:

	Projectile() { Common = NULL; }
	Projectile(uint64_t a1) { Common = a1; }

	uint64_t GetProjectileId()
	{
		uint64_t info = mem::RPM<uint64_t>(this->Common + 0x30) & 0xFFFFFFFFFFFFFFC0;
		return mem::RPM<uint64_t>(info + 0x10);
	}

	ProjectileComponent* GetProjectile()
	{
		return (ProjectileComponent*)decrypter::GetComponent(this->Common, eComp::COMP_PROJECTILE);
	}

	TeamComponent* GetTeam()
	{
		return (TeamComponent*)decrypter::GetComponent(this->Common, eComp::COMP_TEAM);
	}

	bool IsEnemy();

	Player GetOwner();

};