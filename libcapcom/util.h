#pragma once

#include "Win32Util.h"
#include <Windows.h>

VOID PrintErrorAndExit(wchar_t* Function, ULONG dwErrorCode);

template<typename TargetType>
TargetType MakePointer(void* anyptr)
{
	return reinterpret_cast<TargetType>(anyptr);
}

template<typename TargetType>
TargetType MakePointer(SIZE_T anyptr)
{
	return reinterpret_cast<TargetType>(anyptr);
}


template<typename TargetType>
TargetType MakePointer(void* anyptr, SIZE_T offset)
{
	return reinterpret_cast<TargetType>(reinterpret_cast<SIZE_T>(anyptr) + offset);
}

namespace Util
{
	namespace Debug
	{
		inline static void Print(const char* fmt, ...)
		{
#if _DEBUG
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);
			va_end(args);
#endif
		}
	};

	namespace String
	{
		inline static void ToLower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), [](UCHAR c) { return ::tolower(c); });
		}

		static inline std::string ToUTF8(const std::wstring& wstr)
		{
			if (wstr.empty()) return std::string();
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
			std::string strTo(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
			return strTo;
		}

		static inline std::wstring ToUnicode(const std::string& str)
		{
			if (str.empty()) return std::wstring();
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
			return wstrTo;
		}
	};

	namespace Random
	{
		inline static std::mt19937_64 __random_gen = std::mt19937_64{ std::random_device{}() };

		inline static ULONGLONG Generate64()
		{
			return Util::Random::__random_gen();
		}
	};

	namespace Path
	{
		inline static std::wstring RelativeToAbsolute(const std::wstring& RelativePath)
		{
			auto curdir = std::wstring{ };
			auto fpath = std::wstring{ };

			curdir.reserve(MAX_PATH);
			fpath.reserve(MAX_PATH);

			GetModuleFileName(NULL, &curdir[0], MAX_PATH);

			PathRemoveFileSpec(&curdir[0]);

			PathCombine(&fpath[0], &curdir[0], RelativePath.c_str());

			return fpath;
		}
	};

	namespace Exception
	{
		template<typename ... Args>
		static void Throw(const std::string& format, Args ... args)
		{
			SIZE_T size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
			auto buf = std::string{};
			buf.resize(size);
			snprintf(&buf[0], size, format.c_str(), args ...);
			throw std::runtime_error(buf);
		}

		static inline VOID ThrowWin32ErrorCode(const std::wstring& FunctionName, ULONG dwErrorCode)
		{
			LPSTR Buffer;
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwErrorCode,
				LANG_USER_DEFAULT,
				(LPSTR)&Buffer,
				0,
				NULL);

			Util::Exception::Throw("%ws: %s", FunctionName.c_str(), Buffer);
		}

		static inline VOID ThrowWin32ErrorCode(const std::string& FunctionName, ULONG dwErrorCode)
		{
			LPSTR Buffer;
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwErrorCode,
				LANG_USER_DEFAULT,
				(LPSTR)&Buffer,
				0,
				NULL);

			Util::Exception::Throw("%s: %s", FunctionName.c_str(), Buffer);
		}

		static inline VOID ThrowLastError(const std::wstring& FunctionName)
		{
			Util::Exception::ThrowWin32ErrorCode(Util::String::ToUTF8(FunctionName), GetLastError());
		}

		static inline VOID ThrowLastError(const std::string& FunctionName)
		{
			Util::Exception::ThrowWin32ErrorCode(FunctionName, GetLastError());
		}
	};

};

