#pragma once
#include <TlHelp32.h>

#include <VMProtectSDK.h>

#define IOCTL_PROTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_UNLOAD CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _TProtect
{
	DWORD GamePID;
	UINT64 BaseAddress;
	UINT64 Size;
	UINT64 Access;
} TProtect, * PProtect;

namespace utils
{
	inline uint64_t hInstance;

	inline uint32_t GetAsyncKeyStateId, DeviceIoControlId, SendInputId;

	inline HANDLE hDevice;

	static void Protect()
	{
#ifndef VMP
		TProtect inp{};
		inp.GamePID = GetCurrentProcessId();
		inp.BaseAddress = hInstance;
		inp.Size = 0x7C0000;
		inp.Access = PAGE_NOACCESS;
		DeviceIoControl((HANDLE)REDACTED, IOCTL_PROTECT, &inp, sizeof(inp), nullptr, 0, nullptr, NULL);
		//Syscall<NTSTATUS>(DeviceIoControlId, REDACTED, NULL, nullptr, nullptr, nullptr, IOCTL_PROTECT, &inp, sizeof(inp), nullptr, 0);
#endif
	}

	static HANDLE GetValidHandle()
	{
		UINT nDevices = 0;
		GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST));

		PRAWINPUTDEVICELIST pRawInputDeviceList;
		pRawInputDeviceList = new RAWINPUTDEVICELIST[sizeof(RAWINPUTDEVICELIST) * nDevices];

		int nResult;
		nResult = GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST));

		for (UINT i = 0; i < nDevices; i++)
		{
			UINT nBufferSize = 0;
			nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice,
				RIDI_DEVICENAME,
				NULL,
				&nBufferSize);

			WCHAR* wcDeviceName = new WCHAR[nBufferSize + 1];


			nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice,
				RIDI_DEVICENAME,
				wcDeviceName,
				&nBufferSize);


			RID_DEVICE_INFO rdiDeviceInfo;
			rdiDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
			nBufferSize = rdiDeviceInfo.cbSize;

			nResult = GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice,
				RIDI_DEVICEINFO,
				&rdiDeviceInfo,
				&nBufferSize);

			if (nResult < 0)
			{
				continue;
			}
			if (rdiDeviceInfo.dwType == RIM_TYPEMOUSE)
			{
				return pRawInputDeviceList[i].hDevice;
			}
			delete[] wcDeviceName;
		}
		delete[] pRawInputDeviceList;
		return NULL;
	}

	static bool Init(uint64_t g_Instance)
	{
#ifndef VMP
		VMProtectBeginUltra("utils::Init");
#endif
		hInstance = g_Instance;
		DeviceIoControlId = GetSyscallId((uint64_t)GetProcAddress(GetModuleHandleA(xorstr("ntdll").crypt_get()), xorstr("NtDeviceIoControlFile").crypt_get()));

#ifndef VMP
		if (Syscall<NTSTATUS>(DeviceIoControlId, REDACTED, NULL, nullptr, nullptr, nullptr, IOCTL_UNLOAD, nullptr, 0, nullptr, 0))
		{
			if (DeviceIoControl((HANDLE)REDACTED, IOCTL_UNLOAD, nullptr, 0, nullptr, 0, nullptr, NULL))
				return false;
		}
#endif

		GetAsyncKeyStateId = GetSyscallId((uint64_t)GetProcAddress(GetModuleHandleA(xorstr("win32u").crypt_get()), xorstr("NtUserGetAsyncKeyState").crypt_get()));
		if (!GetAsyncKeyStateId)
		{
			GetAsyncKeyStateId = GetSyscallId((uint64_t)GetProcAddress(GetModuleHandleA(xorstr("user32").crypt_get()), xorstr("NtUserGetAsyncKeyState").crypt_get()));
			if (!GetAsyncKeyStateId)
			{
				GetAsyncKeyStateId = 0x1044;
			}
		}

		SendInputId = GetSyscallId((uint64_t)GetProcAddress(GetModuleHandleA(xorstr("win32u").crypt_get()), xorstr("NtUserSendInput").crypt_get()));
		if (!SendInputId)
		{
			SendInputId = GetSyscallId((uint64_t)GetProcAddress(GetModuleHandleA(xorstr("user32").crypt_get()), xorstr("NtUserSendInput").crypt_get()));
			if (!SendInputId)
			{
				SendInputId = 0x1082;
			}
		}

		hDevice = GetValidHandle();

		return true;
#ifndef VMP
		VMProtectEnd();
#endif
	}

	static bool IsKeyPressed(int key)
	{
		if (Syscall<SHORT>(GetAsyncKeyStateId, key) & 0x8000)
			return true;

		return false;
	}

	static UINT SendInput(LONG dx, LONG dy, DWORD mouseData, DWORD dwFlags)
	{
		INPUT input{};
		input.type = INPUT_MOUSE;
		input.mi.dx = dx;
		input.mi.dy = dy;
		input.mi.mouseData = mouseData;
		input.mi.dwFlags = dwFlags;
		return Syscall<UINT>(utils::SendInputId, 1, &input, sizeof(INPUT));
	}

	static void ThreadStuff(bool bSuspend)
	{
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		THREADENTRY32 threadEntry;
		threadEntry.dwSize = sizeof(THREADENTRY32);

		Thread32First(hThreadSnapshot, &threadEntry);
		do
		{
			if (threadEntry.th32OwnerProcessID == GetCurrentProcessId() && threadEntry.th32ThreadID != GetCurrentThreadId())
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
				bSuspend ? SuspendThread(hThread) : ResumeThread(hThread);
				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnapshot, &threadEntry));

		CloseHandle(hThreadSnapshot);
	}

}