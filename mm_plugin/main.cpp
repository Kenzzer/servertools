/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#include <stdio.h>
#include "main.h"

#include "interfaces.hpp"
#include "gameconfig.hpp"
#include "helpers.hpp"
#include "modulescanner/scanner.hpp"
#include "sdk/sdk.hpp"
#include "tools_navmesh.hpp"

ServerTools g_Plugin;
IServerGameDLL* server = nullptr;
IServerGameClients* gameclients = nullptr;
IVEngineServer* engine = nullptr;
IServerPluginHelpers* helpers = nullptr;
IGameEventManager2* gameeventmanager = nullptr;
IServerPluginCallbacks* vsp_callbacks = nullptr;
IPlayerInfoManager* playerinfomanager = nullptr;
ICvar *icvar = nullptr;
CGlobalVars *gpGlobals = nullptr;
IBaseFileSystem* basefilesystem = nullptr;
IServerTools* servertools = nullptr;
CBaseEntityList* g_pEntityList = nullptr;
IMDLCache* mdlcache = nullptr;
CSharedEdictChangeInfo* g_pSharedChangeInfo = nullptr;
IStaticPropMgrServer* staticpropmgr = nullptr;
IEngineTrace* enginetrace = nullptr;
INetworkStringTableContainer* netstringtables = nullptr;
ISoundEmitterSystemBase* soundemitterbase = nullptr;
IEngineSound* engsound = nullptr;

ConVar servertools_version("servertools_version", "0.0.1", 0);
ConVar r_visualizetraces("servertools_visualizetraces", "0", FCVAR_CHEAT);

CGameConfig* g_GameConfig = nullptr;
ScannerModule* g_ServerModule = nullptr;
ScannerModule* g_EngineModule = nullptr;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);

/** 
 * Something like this is needed to register cvars/CON_COMMANDs.
 */
class BaseAccessor : public IConCommandBaseAccessor
{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase)
	{
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

PLUGIN_EXPOSE(ServerTools, g_Plugin);
bool ServerTools::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	SetupScanner((void*)g_SMAPI->GetServerFactory(false), &g_ServerModule);
	SetupScanner((void*)g_SMAPI->GetEngineFactory(false), &g_EngineModule);

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameeventmanager, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, basefilesystem, IBaseFileSystem, BASEFILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, mdlcache, IMDLCache, MDLCACHE_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, staticpropmgr, IStaticPropMgrServer, INTERFACEVERSION_STATICPROPMGR_SERVER);
	GET_V_IFACE_ANY(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_ANY(GetEngineFactory, netstringtables, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);
	GET_V_IFACE_ANY(GetEngineFactory, soundemitterbase, ISoundEmitterSystemBase, SOUNDEMITTERSYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, engsound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);

	g_pEntityList = (CBaseEntityList*)servertools->GetEntityList();
	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();
	gpGlobals = ismm->GetCGlobals();

	/* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
	if ((vsp_callbacks = ismm->GetVSPInfo(nullptr)) == nullptr)
	{
		ismm->AddListener(this, this);
		ismm->EnableVSPListener();
	}

	META_CONPRINTF("Plugin loaded!\n");

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &ServerTools::Hook_LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &ServerTools::Hook_ServerActivate, true);

	g_GameConfig = new CGameConfig("addons/servertools/gamedata.txt");
	g_classHelpers.Init(g_GameConfig);

	g_pCVar = icvar;
	ConVar_Register(0, &s_BaseAccessor);

	nav_solid_props = g_pCVar->FindVar("nav_solid_props");

	if (late)
	{
		SDK_Init();
	}

	return true;
}

bool ServerTools::SetupScanner(void* factory, ScannerModule** scanner)
{
	if (!factory)
	{
		return false;
	}
#if defined POSIX
	Dl_info info;
	if (dladdr(factory, &info) != 0)
	{
		void* handle = dlopen(info.dli_fname, RTLD_NOW);
		if (handle)
		{
			*scanner = new ScannerModule(handle);
			return true;
		}
	}
#endif
	return false;
}

bool ServerTools::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &ServerTools::Hook_LevelInit, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &ServerTools::Hook_ServerActivate, true);
	return true;
}

bool ServerTools::Hook_LevelInit(const char *pMapName,
								char const *pMapEntities,
								char const *pOldLevel,
								char const *pLandmarkName,
								bool loadGame,
								bool background)
{
	return SDK_Init();
}

void ServerTools::Hook_ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
	ToolsNavMesh->Load();

	//g_pMonsterResource = (CMonsterResource*)servertools->FindEntityByClassname(servertools->FirstEntity(), "monster_resource");
}

bool ServerTools::SDK_Init()
{
	static bool once = true;
	static bool init = false;
	if (once)
	{
		once = false;
		char error[2048];
		if (!CSDK::Init(g_GameConfig, error, sizeof(error)))
		{
			META_CONPRINTF("Failed loading \"%s\"\n", error);
			return false;
		}
		META_CONPRINTF("Custom SDK initialised!\n");
		init = true;
	}
	return init;
}

void ServerTools::OnVSPListening(IServerPluginCallbacks *iface)
{
	vsp_callbacks = iface;
}

void ServerTools::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

bool ServerTools::Pause(char *error, size_t maxlen)
{
	return true;
}

bool ServerTools::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *ServerTools::GetLicense()
{
	return "PRIVATE";
}

const char *ServerTools::GetVersion()
{
	return "0.0.1";
}

const char *ServerTools::GetDate()
{
	return __DATE__;
}

const char *ServerTools::GetLogTag()
{
	return "SAMPLE";
}

const char *ServerTools::GetAuthor()
{
	return "Axiem";
}

const char *ServerTools::GetDescription()
{
	return "SM Servertools extension, but ported to Metamod";
}

const char *ServerTools::GetName()
{
	return "Servertools Metamod Plugin";
}

const char *ServerTools::GetURL()
{
	return "http://steamcommunity.com/profiles/76561198059675572";
}