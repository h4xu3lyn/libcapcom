#pragma once
#include "Win32Util.h"
#include "Util.h"

class PEFile
{
	friend class PEFileSection;
public:
	PEFile(const std::wstring& Filename);
	PEFile(PVOID PEFileMemoryBase, SIZE_T PEFileMemorySize);
	PEFile(Util::Win32::unique_module Module);
	SIZE_T GetTotalMappedSize();

	const auto& GetSections() const
	{
		return m_MemSections;
	}

	~PEFile();

	template<typename TargetPtr>
	TargetPtr FromOffset(SIZE_T Offset) const
	{
		auto ptr = MakePointer<TargetPtr>(m_FileMemoryBase, Offset);
		if (ptr >= m_FileMemoryEnd || ptr < m_FileMemoryBase)
		{
			Util::Exception::Throw("OffsetFromBase: Invalid file offset");
		}
		return ptr;
	}

	auto GetHeadersBase() const
	{
		return reinterpret_cast<const PIMAGE_DOS_HEADER&>(m_FileMemoryBase);
	}

	auto GetHeadersSize() const
	{
		return m_SizeOfHeaders;
	}

	auto HasFileCharacteristic(WORD Flag) const
	{
		return (Flag & m_Characteristics);
	}

	const auto& GetDirectoryEntry(WORD DirectoryIndex) const
	{
		return m_Directories.at(DirectoryIndex);
	}

	auto GetImageBase() const
	{
		return m_ImageBase;
	}

	auto GetImageSize() const
	{
		return m_SizeOfImage;
	}

	auto GetEntryPointRVA() const
	{
		return m_AddressOfEntryPointRVA;
	}

	auto GetSizeOfImage() const
	{
		return m_SizeOfImage;
	}

private:
	VOID LoadFromFile(const std::wstring& Filename);
	VOID ParsePEHeaders();

private:
	Util::Win32::unique_handle m_FileHandle;
	Util::Win32::unique_handle m_FileMapping;
	Util::Win32::unique_module m_LoadedModule;
	PVOID m_FileMemoryBase;
	PVOID m_FileMemoryEnd;

	std::vector<IMAGE_SECTION_HEADER> m_MemSections;
	std::vector<IMAGE_DATA_DIRECTORY> m_Directories;

private:
	BOOL m_Is64;
	BOOL m_IsExe;
	ULONGLONG m_ImageBase;
	DWORD m_SizeOfImage;
	DWORD m_SizeOfHeaders;
	DWORD m_AddressOfEntryPointRVA;
	WORD m_DllCharacteristics;
	WORD m_Characteristics;
};

