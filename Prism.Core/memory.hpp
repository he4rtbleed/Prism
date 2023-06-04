#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>
#include <sstream>
#include <vector>

namespace mem
{

	static uintptr_t dwGameBase = (uintptr_t)GetModuleHandle(NULL);
	inline uintptr_t dwRetAddr;

	template<class T>
	static T RPM(uintptr_t address)
	{
		__try
		{
			return *(T*)(address);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return T{};
		}
	}

	template<class T>
	static void WPM(uintptr_t address, T value)
	{
		__try
		{
			*(T*)(address) = value;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	template<typename type>
	static std::vector<uintptr_t> arrayscan(const std::string& arrays, type start_address, size_t size) {
		std::vector<uintptr_t> result;
		std::vector<std::pair<uint8_t, bool>> splits;

		//splits
		char delimiter = ' ';
		std::stringstream ss(arrays);

		std::string temp;
		while (std::getline(ss, temp, delimiter)) {
			uint8_t value = 0xCC;
			bool mask = temp == "?" || temp == "??";
			if (!mask) {
				value = (uint8_t)strtol(temp.c_str(), nullptr, 16);
			}
			splits.push_back({ value, mask });
		}

		std::vector<uint32_t> allows = {
			PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY,
			PAGE_READONLY, PAGE_READWRITE, PAGE_WRITECOPY
		};
		uintptr_t start = (uintptr_t)start_address;
		uintptr_t end = start + size;
		while (start < end) {
			MEMORY_BASIC_INFORMATION mbi = {};
			VirtualQuery((const void*)start, &mbi, sizeof(mbi));
			if ((mbi.BaseAddress) &&
				(mbi.RegionSize) &&
				(mbi.State == MEM_COMMIT) &&
				(std::find(allows.begin(), allows.end(), mbi.Protect) != allows.end())) {
				for (uintptr_t n = (uintptr_t)mbi.BaseAddress; n < (uintptr_t)mbi.BaseAddress + mbi.RegionSize - splits.size(); n++) {
					if (std::equal(splits.begin(), splits.end(), (uint8_t*)n, [](const auto& find, uint8_t original) {
						return find.second || find.first == original;
						})) {
						result.push_back(n);
					}
				}
			}
			uintptr_t next_address = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
			start = next_address > start ? next_address : end;
		}

		return result;
	}

	template<typename type>
	static std::vector<uintptr_t> arrayscan_module(const std::string& arrays, type module_address) {
		IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)module_address;
		IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)((uintptr_t)module_address + dos_header->e_lfanew);
		return arrayscan(arrays, module_address, nt_headers->OptionalHeader.SizeOfImage);
	}

	static uint64_t find_spoof_addr()
	{
		return mem::dwGameBase + 0x575C36;
		uint64_t allow_size = 0x023F9AE0 + 0x1000;
		//FF E3 jmp rbx
		//FF E5 jmp rbp
		//FF E6 jmp rsi
		//FF E7 jmp rdi
		//41 FF E4 jmp r12
		//41 FF E5 jmp r13
		//41 FF E6 jmp r14
		//41 FF E7 jmp r15
		//FF D6 call rsi
		std::vector<uint64_t> spoof_addrs = arrayscan_module("FF D6", mem::dwGameBase);

		for (uint64_t spoof_addr : spoof_addrs)
		{
			if (spoof_addr - dwGameBase > allow_size)
				continue;

			bool isCalled = mem::RPM<uint8_t>(spoof_addr - 5) == 0xE8;
			if (isCalled)
				return spoof_addr;

			bool flag_1 = (mem::RPM<uint8_t>(spoof_addr - 7) == 0xFF) && !(((mem::RPM<uint8_t>(spoof_addr - 6) & 0x38) - 0x10) & 0xF7);
			bool flag_2 = (mem::RPM<uint8_t>(spoof_addr - 6) == 0xFF) && !(((mem::RPM<uint8_t>(spoof_addr - 5) & 0x38) - 0x10) & 0xF7);
			bool flag_3 = (mem::RPM<uint8_t>(spoof_addr - 4) == 0xFF) && !(((mem::RPM<uint8_t>(spoof_addr - 3) & 0x38) - 0x10) & 0xF7);
			bool flag_4 = (mem::RPM<uint8_t>(spoof_addr - 3) == 0xFF) && !(((mem::RPM<uint8_t>(spoof_addr - 2) & 0x38) - 0x10) & 0xF7);
			bool flag_5 = (mem::RPM<uint8_t>(spoof_addr - 2) == 0xFF) && !(((mem::RPM<uint8_t>(spoof_addr - 1) & 0x38) - 0x10) & 0xF7);

#ifdef VMP
			printf("%I64x - %d, %d, %d, %d, %d\n", spoof_addr, flag_1, flag_2, flag_3, flag_4, flag_5);
#endif

			if (flag_1 || flag_2 || flag_3 || flag_4 || flag_5)
				return spoof_addr;
		}

		return NULL;
	}

}