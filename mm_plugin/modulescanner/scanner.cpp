#include "scanner.hpp"
#include "../interfaces.hpp"

#include <link.h>
#include <stdio.h>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <filesystem>

ScannerModule::ScannerModule(void* module) : _module(module)
{
#if defined _LINUX
	struct link_map* lmap = (struct link_map*)module;

	std::uintmax_t size = std::filesystem::file_size(lmap->l_name);
	std::ifstream moduleFile(lmap->l_name, std::ios_base::binary);
	if (!moduleFile)
	{
		throw std::runtime_error("Couldn't open module file \"" + std::string(lmap->l_name) + "\"!");
	}

	std::int8_t* data = new std::int8_t[size];
	moduleFile.read((char*)data, size);

	Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)data;
	if (elfHeader->e_shoff == 0 || elfHeader->e_shstrndx == SHN_UNDEF)
	{
		// Missing section header
		delete[] data;
		return;
	}

	Elf32_Shdr* sectionsHeader = (Elf32_Shdr*)&data[elfHeader->e_shoff];
	auto sectionEntries = elfHeader->e_shnum;

	Elf32_Shdr* headerStringTableHeader = &sectionsHeader[elfHeader->e_shstrndx];
	const char* headerStringTable = (const char *)&data[headerStringTableHeader->sh_offset];

	Elf32_Shdr* stringTableHeader = nullptr;
	Elf32_Shdr* symbolTableHeader = nullptr;
	for (std::size_t i = 0; i < sectionEntries && (!stringTableHeader || !symbolTableHeader); i++)
	{
		Elf32_Shdr* section = &sectionsHeader[i];
		const char* name = headerStringTable + section->sh_name;

		if (strcmp(name, ".symtab") == 0)
		{
			symbolTableHeader = section;
		}
		else if (strcmp(name, ".strtab") == 0)
		{
			stringTableHeader = section;
		}
	}

	if (symbolTableHeader == nullptr || stringTableHeader == nullptr)
	{
		delete[] data;
		return;
	}

	Elf32_Sym* symbols = (Elf32_Sym*)&data[symbolTableHeader->sh_offset];
	const char* stringTable = (const char*)&data[stringTableHeader->sh_offset];
	auto symbol_count = symbolTableHeader->sh_size / symbolTableHeader->sh_entsize;

	for (std::size_t i = 0; i < symbol_count; i++)
	{
		Elf32_Sym* symbol = &symbols[i];
		const char* symbol_name = stringTable + symbol->st_name;
		unsigned char symbol_type = ELF32_ST_TYPE(symbol->st_info);

		if (symbol->st_shndx == SHN_UNDEF || (symbol_type != STT_FUNC && symbol_type != STT_OBJECT))
		{
			continue;
		}

		_symbols[symbol_name] = (void*)(lmap->l_addr + symbol->st_value);
	}
	
	delete[] data;
#elif
#endif
}

void* ScannerModule::FindSymbol(const std::string& name)
{
	auto it = _symbols.find(name);
	if (it == _symbols.end())
	{
		return nullptr;
	}
	return it->second;
}

void* ScannerModule::FindSignature(const std::string& signature)
{
	return nullptr;
}