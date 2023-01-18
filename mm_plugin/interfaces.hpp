#include <ISmmPlugin.h>
PLUGIN_GLOBALVARS();

#include <filesystem.h>
#include <itoolentity.h>
#include <entitylist_base.h>
#include <IEngineTrace.h>
#include <IEngineSound.h>
#include <IStaticPropMgr.h>
#include <ISmmAPI.h>
#include <ISmmPlugin.h>
#include <networkstringtabledefs.h>
#include <SoundEmitterSystem/isoundemittersystembase.h>
#include "engine_wrappers.h"

class ScannerModule;

extern IBaseFileSystem* basefilesystem;
extern IVEngineServer* engine;
extern IEngineTrace *enginetrace;
extern IServerTools* servertools;
extern CBaseEntityList* g_pEntityList;
extern IMDLCache* mdlcache;
extern CSharedEdictChangeInfo* g_pSharedChangeInfo;
extern IStaticPropMgrServer* staticpropmgr;
extern ICvar* icvar;
extern ISmmAPI* g_SMAPI;
extern INetworkStringTableContainer* netstringtables;
extern ISoundEmitterSystemBase* soundemitterbase;
extern IEngineSound* engsound;