#include "PEFile.h"
#include "Win32Util.h"
#include "Util.h"
using namespace std;


PEFile::PEFile(const std::wstring& Filename)
{
	LoadFromFile(Filename);
	ParsePEHeaders();
}

PEFile::PEFile(PVOID PEFileMemoryBase, SIZE_T PEFileMemorySize)
{
	m_FileMemoryBase = PEFileMemoryBase;
	m_FileMemoryEnd = MakePointer<PVOID>(m_FileMemoryBase, PEFileMemorySize);
	ParsePEHeaders();
}

PEFile::PEFile(Util::Win32::unique_module Module)
{
	auto hModule = (HMODULE)Module.get();
	m_LoadedModule = move(Module);
	m_FileMemoryBase = hModule;

	ParsePEHeaders();

	m_FileMemoryEnd = MakePointer<PVOID>(m_FileMemoryBase, GetImageSize());
}

SIZE_T PEFile::GetTotalMappedSize()
{
	return m_SizeOfImage;
}

PEFile::~PEFile()
{
}

VOID PEFile::LoadFromFile(const wstring& Filename)
{
	auto hFile = Util::Win32::unique_handle
	{
		CreateFile(Filename.c_str(), FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, 0, NULL)
	};
	if (hFile.get() <= 0LL) Util::Exception::ThrowLastError("CreateFile");

	auto dwFileSizeHigh = DWORD{};
	auto dwFileSizeLow = GetFileSize(hFile.get(), &dwFileSizeHigh);
	if (dwFileSizeLow == INVALID_FILE_SIZE) Util::Exception::ThrowLastError("GetFileSize");

	auto dwFileSize = DWORD64{ dwFileSizeHigh | dwFileSizeLow };
	auto hMap = Util::Win32::unique_handle
	{
		CreateFileMapping(hFile.get(), NULL, PAGE_READONLY, 0, 0, NULL)
	};
	if (hMap.get() <= 0LL) Util::Exception::ThrowLastError("CreateFileMapping");

	auto PEFile = MapViewOfFile(hMap.get(), FILE_MAP_READ, 0, 0, 0);
	if (PEFile <= 0LL) Util::Exception::ThrowLastError("MapViewOfFile");

	m_FileHandle = move(hFile);
	m_FileMapping = move(hMap);
	m_FileMemoryBase = PEFile;
	m_FileMemoryEnd = MakePointer<PVOID>(PEFile, dwFileSize);

}

VOID PEFile::ParsePEHeaders()
{
	if (!m_FileMemoryBase) Util::Exception::Throw("No module loaded");

	auto DosHeader = MakePointer<PIMAGE_DOS_HEADER>(m_FileMemoryBase);
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		Util::Exception::Throw("Invalid image DOS signature");
	}

	auto NtHeaderOffset = DosHeader->e_lfanew;

	auto NtHeaders = MakePointer<PIMAGE_NT_HEADERS64>(DosHeader, NtHeaderOffset);
	if (NtHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		Util::Exception::Throw("Invalid image NT signature");
	}


	PIMAGE_SECTION_HEADER pSectionHeaders = nullptr;

	auto ParseHeaderFields = [this, &pSectionHeaders](auto _NtHeaders)
	{
		auto OptHdr = _NtHeaders->OptionalHeader;

		m_ImageBase = OptHdr.ImageBase;
		m_SizeOfImage = OptHdr.SizeOfImage;
		m_SizeOfHeaders = OptHdr.SizeOfHeaders;
		m_AddressOfEntryPointRVA = OptHdr.AddressOfEntryPoint;
		m_DllCharacteristics = OptHdr.DllCharacteristics;

		for (size_t i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
		{
			m_Directories.push_back(_NtHeaders->OptionalHeader.DataDirectory[i]);
		}

		pSectionHeaders = MakePointer<PIMAGE_SECTION_HEADER>(_NtHeaders, sizeof(*_NtHeaders));
	};

	if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		m_Is64 = TRUE;
		ParseHeaderFields(NtHeaders);
	}
	else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		auto NtHeaders32 = MakePointer<PIMAGE_NT_HEADERS32>(NtHeaders);
		m_Is64 = FALSE;
		ParseHeaderFields(NtHeaders32);
	}
	else
	{
		Util::Exception::Throw("Invalid OptionalHeader.Magic signature");
	}

	m_IsExe = !(NtHeaders->FileHeader.Characteristics == IMAGE_FILE_DLL);
	m_Characteristics = NtHeaders->FileHeader.Characteristics;

	auto numSections = NtHeaders->FileHeader.NumberOfSections;
	m_MemSections.reserve(numSections);

	for (size_t i = 0; i < numSections; i++, pSectionHeaders++)
	{
		if (pSectionHeaders >= m_FileMemoryEnd) Util::Exception::Throw("Sections extend past end of file");
		m_MemSections.push_back(*pSectionHeaders);
	}
}
