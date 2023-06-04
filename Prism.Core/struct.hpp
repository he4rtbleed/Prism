#pragma once
#include <DirectXMath.h>

using namespace DirectX;

static XMFLOAT3 NormalizeVec3(XMFLOAT3 vec)
{
	XMFLOAT3 normalized{};
	XMStoreFloat3(&normalized, XMVector3Normalize(XMLoadFloat3(&vec)));
	return normalized;
}

static XMFLOAT3 LerpVec3(XMFLOAT3 vec, XMFLOAT3 vec2, float lerp)
{
	XMFLOAT3 lerped{};
	XMStoreFloat3(&lerped, XMVectorLerp(XMLoadFloat3(&vec), XMLoadFloat3(&vec2), lerp));
	return lerped;
}

static XMFLOAT3 Predict(XMFLOAT3 Local, XMFLOAT3 Target, XMFLOAT3 TargetVelocity, float ProjSpeed, float ProjGravity)
{
	XMFLOAT3 result = Target;

	float flDist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&Local) - XMLoadFloat3(&Target)));

	float flTime = flDist / ProjSpeed;
	XMStoreFloat3(&result, XMLoadFloat3(&Target) + XMLoadFloat3(&TargetVelocity) * flTime);

	if (ProjGravity != 0.f)
		result.y += ProjGravity * 0.5f * powf(flTime, 2.f);

	return result;
}

static float Clamp(float v, float lo, float hi)
{
	return (v < lo) ? lo : (hi < v) ? hi : v;
}

#pragma pack(push, 1)
struct Matrix {
	float m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44;

	XMFLOAT3 GetCameraVec()
	{
		float	A = m22 * m33 - m32 * m23,
			B = m32 * m13 - m12 * m33,
			C = m12 * m23 - m22 * m13,
			Z = m11 * A + m21 * B + m31 * C;

		if (abs(Z) < 0.0001) return XMFLOAT3();

		float	D = m31 * m23 - m21 * m33,
			E = m11 * m33 - m31 * m13,
			F = m21 * m13 - m11 * m23,
			G = m21 * m32 - m31 * m22,
			H = m31 * m12 - m11 * m32,
			K = m11 * m22 - m21 * m12;

		return XMFLOAT3(-(A * m41 + D * m42 + G * m43) / Z, -(B * m41 + E * m42 + H * m43) / Z, -(C * m41 + F * m42 + K * m43) / Z);
	}

	XMFLOAT3 GetCameraAngle()
	{
		return XMFLOAT3(m13, m23, m33);
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Color
{
	std::uint8_t R{ 255 };
	std::uint8_t G{ 255 };
	std::uint8_t B{ 255 };
	std::uint8_t A{ 255 };

	DWORD ApplyAlpha(uint8_t Alpha)
	{
		return ((Alpha & 0xff) << 24) | ((R & 0xff)) | ((G & 0xff) << 8) | ((B & 0xff) << 16);
	}

	DWORD RGBA2ARGB(uint8_t Alpha)
	{
		return ((Alpha & 0xff) << 24) | ((B & 0xff)) | ((G & 0xff) << 8) | ((R & 0xff) << 16);
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct RayCastIn
{
	XMFLOAT3 from;			// 0x0
	float pad_01;			// 0xC
	XMFLOAT3 to;			// 0x10
	float pad_02;			// 0x1C
	char pad_03[0x38];		// 0x20
	uint64_t mask;			// 0x58
	uint64_t offsetIn;		// 0x60
	char pad_04[0x1024];	// 0x68
};
#pragma pack(pop)

#pragma pack(push, 1)
struct RayCastOut
{
	XMFLOAT3 hittedVec;		// 0x0
	float pad_01;			// 0xC
	XMFLOAT3 hittedRot;		// 0x10
	float pad_02;			// 0x1C
	uint64_t hittedEntity;	// 0x20
	char pad_03[0x10];		// 0x28
	uint16_t hittedBone;	// 0x38
	char pad_04[0x1024];	// 0x3A
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BestBoneResult
{
	bool bFound;
	int foundBone;
	float lastFov;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SkillInfo
{
	char pad_01[0x48];		// 0x0
	uint32_t isUsing;		// 0x48
	char pad_02[0x4];		// 0x4C
	float flUltGauge;		// 0x50
	char pad_03[0xC];		// 0x54
	float flCoolTime_Curr;	// 0x60
	char pad_04[0xB4];		// 0x64
	uint32_t isBlocked;		// 0x118

	XMFLOAT2 GetCoolTime()
	{
		return XMFLOAT2{ flUltGauge, flCoolTime_Curr };
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct InputInfo
{
	char pad_01[0x2AE];		// 0x0
	bool key_forward;		// 0x2AE
	bool key_back;			// 0x2AF
	bool key_left;			// 0x2B0
	bool key_right;			// 0x2B1
	bool key_jump;			// 0x2B2
	bool key_duck;			// 0x2B3
	char pad_02;			// 0x2B4
	bool key_reload;		// 0x2B5
	char pad_03[0x5];		// 0x2B6
	bool key_mouse_left;	// 0x2BB
	char pad_04[0x8];		// 0x2BC
	bool key_mouse_right;	// 0x2C4
	bool key_interact;		// 0x2C5
	bool key_melee;			// 0x2C6
	char pad_05[0x3];		// 0x2C7
	bool key_ability_1;		// 0x2CA
	bool key_ability_2;		// 0x2CB
	bool key_ability_3;		// 0x2CC
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MouseInput
{
	int dx;
	int dy;
	int btn;
};
#pragma pack(pop)