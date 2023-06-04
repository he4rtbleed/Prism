#pragma once
#include <Windows.h>
#include <emmintrin.h>
#include <cstdint>
#include "memory.hpp"
#include "spoofer.hpp"

#define OFFSET_PARENTPTR_UNIQUEID 0xB0 // 45 33 C9 45 33 C0 49 8B ? 49 8B CD E8 ? //inside below call	//41 0f b6 d8 4c 8b e2 4c 8b e9 //below mov edi,[r13+offset]
#define OFFSET_COMPONENTPTR_ENCRYPTEDPARENT 0x8
#define OFFSET_COMPONENTPTR_ID 0x10

struct TArray
{
	uint64_t base_address;
	uint32_t num_elements;
};

namespace decrypter
{
	static void DecryptData(uint64_t* encrypted_buf, size_t size, uint64_t* xor_key, uint64_t* decrypted_buf, uint64_t first_key = NULL);

	inline uint64_t global_key;
	inline uint64_t manager_key;
	static void GetGlobalKey(uint64_t* a0, uint64_t* a1)
	{
		if (global_key && global_key != -1)
			spoof_call((PVOID)(mem::dwRetAddr), (void(_fastcall*)(uint64_t*, uint64_t*))(mem::dwGameBase + global_key), a0, a1); //0xB56B8
	}

	static uint64_t GetGManagerXOR(uint64_t a1)
	{
		if (!manager_key && manager_key == -1)
			return 0;

		return spoof_call((PVOID)(mem::dwRetAddr), (uint64_t(_fastcall*)(uint64_t))(mem::dwGameBase + manager_key), a1); //0x3139F0
	}

	static uint64_t GetGManagerIndex(uint64_t* a1)
	{
		return spoof_call((PVOID)(mem::dwRetAddr), (uint64_t(_fastcall*)(uint64_t*))(mem::dwGameBase + 0x319C20), a1); //0x319C20
	}

	static uint64_t GetGManager()
	{
		__try
		{
			uint64_t result = 0;

			uint64_t v1 = 0x45AC7860419B9A55;
			uint64_t v2 = 0x953776696193DF3;
			GetGlobalKey(&v2, &v1);

			uint64_t qword_2CE5590 = *(uint64_t*)(mem::dwGameBase + 0x2CE5590);

			uint64_t v3 = (uint32_t)v2 ^ (uint32_t)qword_2CE5590;
			uint64_t v4 = v3 | ((*(int*)(mem::dwGameBase + 0x2C74A50 + (v1 >> 0x34)) ^ (uint32_t)v2 ^ (uint32_t)qword_2CE5590 ^ ((v2 ^ ((uint32_t)qword_2CE5590 | (((uint32_t)qword_2CE5590 ^ 0xF6AC8899 ^ (((uint32_t)qword_2CE5590 | (((uint32_t)~(uint32_t)qword_2CE5590 ^ _rotr(*(uint64_t*)(mem::dwGameBase + 0x2C74A50 + (v1 & 0xFFF)), 0xB) ^ ((unsigned __int64)qword_2CE5590 >> 0x20)) << 0x20)) >> 0x20)) << 0x20))) >> 0x20)) << 0x20);
			uint64_t v5 = (uint32_t)v3 | (((uint32_t)(v4 - (*(uint64_t*)(mem::dwGameBase + 0x2C74A50 + (v1 & 0xFFF)) >> 0x20)) ^ (v4 >> 0x20)) << 0x20);

			uint64_t* encrypted_buf = (uint64_t*)(v5);

			uint64_t decrypted_buf[20]{};

			static uint64_t xor_key[8] = {
				0xB7F4D29A6659B0EE,
				0x2E0E0E74C32610E4,
				0xE8557C4B03ED1580,
				0x564F3FDED63A2312,
				0x612A2EAA5532C164,
				0xD6754A6878829C9,
				0xA4779E85148316C5,
				0xFC34626FCFB2EB7A
			};

			size_t size = sizeof(decrypted_buf) / sizeof(uint64_t);

			decrypter::DecryptData(encrypted_buf, size, xor_key, decrypted_buf, GetGManagerXOR(encrypted_buf[20]));

			// 48 89 5C 24 18 57 48 83 EC 20 48 8B D9
			// Overwatch.exe + 55D220

			// 48 3b 15 ? ? ? ? 75 ? 48 8b 81 ? ? 00 00 48 85 c0
			// 48 8B 4C CC ? E8 ? ? ? ? 48 8B 7C 24 ? 84 //doesn't works
			// (2i64 * __ROR8__(0xA0297DABE6BBD067i64, 11) - qword_7FF672B269B9) ^ a1[6]
			uint64_t key = *(uint64_t*)(mem::dwGameBase + 0x2C74D23) - _rotr64(0xCCC1E009D3E84255, 0xB);

			result = *(uint64_t*)(decrypted_buf[GetGManagerIndex(decrypted_buf) + 4] + 0x30) ^ key;

			return result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return NULL;
		}
	}

	static uint64_t GetCameraMgr(uint64_t entity_mgr)
	{
		//40 53 48 83 EC 20 48 8B 59 20 48 85 DB 74 ? 48 8B 9B
		__try
		{
			if (!entity_mgr || IsBadReadPtr((void*)entity_mgr, sizeof(entity_mgr)))
				return NULL;
			uint64_t camera_mgr = reinterpret_cast<uint64_t(_fastcall*)(uint64_t)>(mem::dwGameBase + 0x5C02E0)((uint64_t)&entity_mgr - 0x20);
			return camera_mgr;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return NULL;
		}
	}

	static uint64_t GetEntityTArray(uint64_t pGManager)
	{
		//49 8B C8 E8 ? ? ? ? 48 8B 7C 24 ? 48
		__try
		{
			uint64_t xor_key = 0x591658E0EC47A3D2 - *(uint64_t*)(mem::dwGameBase + 0x2C754DE);
			return xor_key ^ *(uint64_t*)(pGManager + 0x18);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return NULL;
		}
	}

	static uint64_t GetEntityWithUID(uint64_t pGManager, uint32_t UID)
	{
		__try
		{
			uint64_t result = 0;

			uint64_t pEntityTArray = GetEntityTArray(pGManager);

			TArray CurrTArray = mem::RPM<TArray>(pEntityTArray + 0x10 * (UID & 0xFFF));

			if (CurrTArray.base_address && CurrTArray.num_elements && CurrTArray.num_elements < 100)
			{
				for (int i = 0; i < CurrTArray.num_elements; i++)
				{
					uint32_t CurrUID = mem::RPM<uint32_t>(CurrTArray.base_address + 0x10 * i);
					uint64_t CurrEntity = mem::RPM<uint64_t>(CurrTArray.base_address + 0x10 * i + 8);

					if (mem::RPM<uint32_t>(CurrEntity + OFFSET_PARENTPTR_UNIQUEID) == CurrUID && CurrUID == UID)
					{
						result = CurrEntity;
					}
				}
			}

			return result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return NULL;
		}
	}

	static uint64_t DecryptParent(uint64_t ComponentBase)
	{
		// 40 53 48 83 EC 20 48 8D 59 ? 48 8B CB E8 ? ? ? ? F6 43 ? ? 48
		uint64_t key = 0x4831531B8348B753 - *(uint64_t*)(mem::dwGameBase + 0x2C759E9);
		return key ^ *(uint64_t*)(ComponentBase + OFFSET_COMPONENTPTR_ENCRYPTEDPARENT);
	}

#define HIDWORD(l) ((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))
	static uint64_t fnDecryptChildInParent(uint64_t a1)
	{
		// 48 89 ? ? ? ? 00 48 8D 15 ? ? ? 00 E8 ? ? ? ? 48 //lea rcx,[fnDecryptChild]
		__try
		{
			uint64_t v29 = 0x8C6D64C470D6B748;
			uint64_t v30 = 0xC5F9D8163FAD2D3E;
			GetGlobalKey(&v30, &v29);
			uint64_t v25 = (uint32_t)a1 | ((((uint32_t)a1 - *(int*)(mem::dwGameBase + 0x2C74A50 + (v29 & 0xFFF))) ^ (a1 >> 0x20)) << 0x20);
			uint64_t v26 = (uint32_t)v30 ^ (uint32_t)v25 | (((uint32_t)v30 ^ (uint32_t)v25 ^ (uint32_t)*(uint64_t*)(mem::dwGameBase + 0x2C74A50 + (v29 >> 0x34)) ^ ((v30 ^ ((uint32_t)v25 | (((uint32_t)(v25 + 0x3A0627EA) ^ (v25 >> 0x20)) << 0x20))) >> 0x20)) << 0x20);
			uint64_t v6 = (uint32_t)v30 ^ (uint32_t)v25 | (((uint32_t)(v26 + _rotl(*(uint64_t*)(mem::dwGameBase + 0x2C74A50 + (v29 & 0xFFF)) >> 0x20, 1)) ^ (v26 >> 0x20)) << 0x20);

			return v6;
		} __except(EXCEPTION_EXECUTE_HANDLER) { }
		
		return NULL;
	}

	/*static uint64_t GetComponent_deprecated(uint64_t ComponentParent, uint8_t ComponentID)
	{
		uint64_t pPointerTable = ComponentParent + 0x60;					//HERE	//find in fnDecryptChild //upper call GlobalKey

		uint64_t PointerTable = mem::RPM<uint64_t>(pPointerTable + 0x0);
		uint32_t PointerCount = mem::RPM<uint32_t>(pPointerTable + 0x8);

		for (int i = 0; i < PointerCount; i++)
		{
			uint64_t CurrentComponent = fnDecryptChildInParent(*(uint64_t*)(PointerTable + i * 8));

			if (!CurrentComponent)
				continue;

			if (mem::RPM<uint8_t>(CurrentComponent + OFFSET_COMPONENTPTR_ID) == ComponentID)
				return CurrentComponent;
		}

		return NULL;
	}*/

	static uint64_t GetComponent(uint64_t ComponentParent, uint8_t ComponentID)
	{
		__try
		{
			uint64_t v1 = ComponentParent;

			uint64_t v2 = (uint64_t)1 << (uint64_t)(ComponentID & 0x3F), v3 = v2 - 1, v4 = ComponentID & 0x3F, v5 = ComponentID / 0x3F;

			uint64_t v6 = *(uint64_t*)(v1 + 8 * (uint32_t)v5 + 0x88);
			int64_t v7 = (v2 & v6) >> v4;
			uint64_t v8 = (v3 & v6) - (((v3 & v6) >> 1) & 0x5555555555555555);
			uint64_t* v9 = (uint64_t*)(*(uint64_t*)(v1 + 0x58)
				+ 8
				* (*(uint8_t*)((uint32_t)v5 + v1 + 0xA8)
					+ ((0x101010101010101
						* (((v8 & 0x3333333333333333)
							+ ((v8 >> 2) & 0x3333333333333333)
							+ (((v8 & 0x3333333333333333) + ((v8 >> 2) & 0x3333333333333333)) >> 4)) & 0xF0F0F0F0F0F0F0F)) >> 56)));

			uint64_t r = -v7 & fnDecryptChildInParent(*v9);

			return r;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return NULL;
		}
	}

	static uint64_t GetReaperWithCommon(uint64_t Common)
	{
		uint64_t ReaperLinker = GetComponent(Common, COMP_REAPERLINKER);
		uint64_t xor_key = spoof_call((PVOID)(mem::dwRetAddr), reinterpret_cast<uint64_t(__fastcall*)(uint64_t)>(mem::dwGameBase + 0xB63350), mem::RPM<uint64_t>(ReaperLinker + 0xE0));
		return GetEntityWithUID(GetGManager(), xor_key ^ mem::RPM<uint64_t>(ReaperLinker + 0xD8)); //48 8B 8E ? ? 00 00 E8 ? ? ? ? 48 8B C8 48 8B C3 48 33 8E ? ? 00 00
	}

	static uint64_t GetCommonWithReaper(uint64_t Reaper)
	{
		uint64_t CommonLinker = GetComponent(Reaper, COMP_COMMONLINKER);
		return GetEntityWithUID(GetGManager(), mem::RPM<uint64_t>(CommonLinker + 0xD0));
	}

	static void DecryptData(uint64_t* encrypted_buf, size_t size, uint64_t* xor_key, uint64_t* decrypted_buf, uint64_t first_key)
	{
		uint64_t decrypt_key = first_key ? first_key : encrypted_buf[size] + xor_key[min(encrypted_buf[size] & 7, 7)];

		for (int i = 0; i < size; i++)
		{
			decrypted_buf[i] = encrypted_buf[i] ^ decrypt_key;
			decrypt_key += xor_key[min(decrypt_key & 7, 7)];
		}
	}

	static uint64_t GetGlobalPtr(int index)
	{
		//48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ?
		uint64_t EncryptedGlobalPtrTable = mem::RPM<uint64_t>(mem::dwGameBase + 0x2CE93A8);
		uint64_t decrypt_key = mem::RPM<uint64_t>(mem::dwGameBase + 0x2C755CB) ^ ~_rotr64(0x99C8EE82DB2DF3F4, 0xB);
		decrypt_key ^= mem::RPM<uint64_t>(EncryptedGlobalPtrTable + 0x140);
		return mem::RPM<uint64_t>(decrypt_key + index * 8);
	}

}
