#include "sdk.hpp"
#include "baseentity.hpp"
#include "baseanimating.hpp"
#include "baseentityoutput.hpp"
#include "baseanimatingoverlay.hpp"
#include "basecombatcharacter.hpp"
#include "basetoggle.hpp"
#include "basepropdoor.hpp"
#include "funcbrush.hpp"
#include "tracefilter_simple.hpp"
#include "nav_mesh.hpp"

#include "NextBot/NextBotManager.h"

#include "util.hpp"
#include "interfaces.hpp"

ConVar* phys_pushscale = nullptr;

bool CSDK::Init(CGameConfig* config, char* error, size_t maxlen)
{
	try
	{
		fDispatchParticleEffect.Init(config, "DispatchParticleEffect");
		fUTIL_GetCommandClient.Init(config, "UTIL_GetCommandClient");
	}
	catch (const std::exception & e)
	{
		snprintf(error, maxlen, "%s", e.what());
		return false;
	}

	phys_pushscale = icvar->FindVar("phys_pushscale");

	if (!(CBaseEntity::Init(config, error, maxlen)
	&& CBaseEntityOutput::Init(config, error, maxlen)
	&& CBaseAnimating::Init(config, error, maxlen)
	&& CBaseAnimatingOverlay::Init(config, error, maxlen)
	&& CBaseCombatCharacter::Init(config, error, maxlen)
	&& CBaseToggle::Init(config, error, maxlen)
	&& CFuncBrush::Init(config, error, maxlen)
	&& CNavMesh::Init(config, error, maxlen)
	&& CBasePropDoor::Init(config, error, maxlen)
	&& NextBotManager::Init(config, error, maxlen)
	//&& CHalloweenBaseBoss::Init(config, error, maxlen)
	//&& CMonsterResource::Init(config, error, maxlen)
	&& CTraceFilterSimple_Init(config, error, maxlen)))
	{
		return false;
	}

	MetaModFactoryDictionary()->InstallFactories();
	return true;
}
/*
		|| !CTFGameRules::Init(g_pGameConf, error, maxlength)
*/