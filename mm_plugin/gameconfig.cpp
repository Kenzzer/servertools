#include "gameconfig.hpp"
#include "interfaces.hpp"

#include "modulescanner/scanner.hpp"

extern ScannerModule* g_ServerModule;
extern ScannerModule* g_EngineModule;

CGameConfig::CGameConfig(const std::string& path) : _kv(new KeyValues("Games"))
{
	if (!_kv->LoadFromFile(basefilesystem, path.c_str(), nullptr))
	{
		META_CONPRINTF("Failed to load gamedata file! \"%s\"\n", path.c_str());
	}

	KeyValues* game = _kv->FindKey("tf");
	if (game)
	{
#if defined _LINUX
		const char* platform = "linux";
#else
		const char* platform = "windows";
#endif

		KeyValues* offsets = game->FindKey("Offsets");
		if (offsets)
		{
			FOR_EACH_SUBKEY(offsets, it)
			{
				_offsets[it->GetName()] = it->GetInt(platform, -1);
			}
		}

		KeyValues* signatures = game->FindKey("Signatures");
		if (signatures)
		{
			FOR_EACH_SUBKEY(signatures, it)
			{
				ScannerModule* module = nullptr;

				const char* library = it->GetString("library");
				if (strcmp(library, "server") == 0)
				{
					module = g_ServerModule;
				}
				else if (strcmp(library, "engine") == 0)
				{
					module = g_EngineModule;
				}

				if (module == nullptr)
				{
					META_CONPRINTF("Failed to retrieve library \"%s\"!\n", library);
					continue;
				}

				const char* signature = it->GetString(platform);
				void* addr = nullptr;

				if (signature[0])
				{
					if (signature[0] == '@')
					{
						addr = module->FindSymbol(&signature[1]);
					}

					if (addr == nullptr)
					{
						addr = module->FindSignature(signature);
					}
				}
				_signatures[it->GetName()] = addr;
			}
		}
	}
}

CGameConfig::~CGameConfig()
{
	_kv->deleteThis();
}

void* CGameConfig::GetMemSig(const std::string& name)
{
	auto it = _signatures.find(name);
	if (it == _signatures.end())
	{
		return nullptr;
	}
	return it->second;
}

int CGameConfig::GetOffset(const std::string& name)
{
	auto it = _offsets.find(name);
	if (it == _offsets.end())
	{
		return -1;
	}
	return it->second;
}