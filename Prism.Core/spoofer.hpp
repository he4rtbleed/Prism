#pragma once
#include <type_traits>

namespace spoofer
{
	extern "C" void* _spoofer_stub(...);
	extern "C" void _set_rax_stub(const void*);
	extern "C" void* _syscaller();

	template <typename Ret, typename... Args>
	static inline auto shellcode_stub_helper(
		const void* shell_param,
		Args... args
	) -> Ret
	{
		auto fn = (Ret(*)(Args...))(_spoofer_stub);
		_set_rax_stub(shell_param);
		return fn(args...);
	}

	template <std::size_t Argc>
	struct argument_remapper
	{
		template<
			typename Ret,
			typename... Pack
		>
			static Ret do_call(
				void* shell_param,
				Pack... pack
			)
		{
			return shellcode_stub_helper<Ret>(shell_param, pack...);
		}
	};
}

template <typename Ret, typename... Args>
static inline auto spoof_call(
	const void* trampoline,
	Ret(*fn)(Args...),
	Args... args
) -> Ret
{
	struct shell_params
	{
		const void* trampoline;			// 0x0
		void* function;			// 0x8
		void* backupRbx;
		void* backupRsi;
		void* backupRet;
	};

	shell_params p{ (const void*)(trampoline), reinterpret_cast<void*>(fn) };
	using mapper = spoofer::argument_remapper<sizeof...(Args)>;

	return mapper::template do_call<Ret, Args...>(&p, args...);
}

__forceinline uint32_t GetSyscallId(uint64_t func_addr)
{
	__try
	{
		if (*(uint32_t*)(func_addr) == 0xB8D18B4C)
			return *(uint32_t*)(func_addr + 0x4);
		return NULL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

template< typename ReturnType = void, typename... Args,
	typename T1 = void*, typename T2 = void*, typename T3 = void*, typename T4 = void* > __forceinline
	ReturnType Syscall(const uint64_t Index, T1 A1 = { }, T2 A2 = { }, T3 A3 = { }, T4 A4 = { }, Args... Arguments)
{
	ReturnType A{};
	if (!Index)
		return A;
	return reinterpret_cast<ReturnType(*)(T1, T2, T3, T4, uint64_t, uint64_t, Args...)>(::spoofer::_syscaller)(
		A1, A2, A3, A4, Index, 0, Arguments... // Stack must be aligned to 16 byte boundary.
		);
}