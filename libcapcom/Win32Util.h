#pragma once
#include "Winternl.h"
#include "Util.h"
#pragma once

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <Psapi.h>
#include <intrin.h>
#include <Shlwapi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <list>
#include <winternl.h>
#include <random>
#include <type_traits>

#include "Win32Kernel.h"

namespace Util
{
	namespace Win32
	{
		// Auto deleter for C++ smart pointers for Win32 Handles
		struct Win32HandleDeleter
		{
			void operator()(HANDLE handle)
			{
				if (handle != nullptr)
				{
					::CloseHandle(handle);
				}
			}
		};

		using unique_handle = std::unique_ptr<void, Win32HandleDeleter>;

		struct Win32ModuleDeleter
		{
			void operator()(void* handle)
			{
				if (handle != nullptr)
				{
					::FreeLibrary((HMODULE)handle);
				}
			}
		};

		using unique_module = std::unique_ptr<void, Win32ModuleDeleter>;

		struct VirtualFreeDeleter
		{
			void operator()(LPVOID mem)
			{
				if (mem != nullptr)
				{
					auto ret = ::VirtualFree(mem, 0, MEM_RELEASE);

					if (!ret) Util::Exception::ThrowLastError("VirtualFree");
				}
			}
		};

		template<typename PtrType = VOID>
		using unique_virtalloc = std::unique_ptr<PtrType, VirtualFreeDeleter>;

		template <class StrClass>
		class UnicodeStringWrapper : public UNICODE_STRING
		{
		public:
			StrClass innerStr;

			UnicodeStringWrapper(const StrClass& _innerStr)
			{
				innerStr = _innerStr;
				::RtlInitUnicodeString(this, innerStr.c_str());
			}
		};

		inline std::string NtNativeToWin32(const std::wstring& NtNativePath)
		{
			DWORD dwRet;
			OBJECT_ATTRIBUTES  objAttr;
			HANDLE handle;
			IO_STATUS_BLOCK    ioStatusBlock = { 0 };

			auto Path = std::string{};
			Path.resize(MAX_PATH);
			auto uniNativePath = UnicodeStringWrapper<std::wstring>{ NtNativePath };

			InitializeObjectAttributes(&objAttr, (PUNICODE_STRING)&uniNativePath,
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				NULL, NULL);

			auto ntstatus = NtCreateFile(&handle,
				GENERIC_READ,
				&objAttr,
				&ioStatusBlock,
				NULL,
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ,
				FILE_OPEN,
				FILE_NON_DIRECTORY_FILE,
				NULL,
				0);

			if (!NT_SUCCESS(ntstatus))
			{
				return Path;
			}

			dwRet = GetFinalPathNameByHandleA(handle, &Path[0], MAX_PATH, VOLUME_NAME_DOS);
			CloseHandle(handle);
			return Path;
		}
	}
}
