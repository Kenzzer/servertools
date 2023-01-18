#include "baseplayer.hpp"
#include "interfaces.hpp"
#include "util.hpp"

#include <tier1/utldict.h>

class _CEntityFactoryDictionary : public IEntityFactoryDictionary
{
public:
	void InstallFactory(IEntityFactory* factory, const char* className)
	{
		this->m_Factories.Insert(className, factory);
	}

	void RemoveFactory(IEntityFactory* factory)
	{
		for (size_t i = 0; i < m_Factories.Count(); i++)
		{
			if (this->m_Factories[i] == factory)
			{
				const char* className = this->m_Factories.GetElementName(i);
				
				for (CBaseEntity* ent = servertools->FirstEntity(),*nexEnt = nullptr; ent; ent = nexEnt)
				{
					nexEnt = servertools->NextEntity(ent);
					if (FClassnameIs(ent, className))
					{
						servertools->RemoveEntityImmediate(ent);
					}
				}

				m_Factories.RemoveAt(i);
			}
		}
	}
private:
	CUtlDict<IEntityFactory *, unsigned short> m_Factories;
};

IEntityFactory* topFactory = nullptr;
CMetaModFactoryDictionary s_MetamodFactory;

void CMetaModFactoryDictionary::InstallFactory(IEntityFactory* factory, const char* className)
{
	_factories.emplace(std::make_pair(factory, className));
}

void CMetaModFactoryDictionary::InstallFactories(void)
{
	auto dict = ((_CEntityFactoryDictionary*)servertools->GetEntityFactoryDictionary());

	for (auto factory : _factories)
	{
		dict->InstallFactory(factory.first, factory.second);
		META_CONPRINTF("Installed custom factory (%s)!\n", factory.second);
	}
}

void CMetaModFactoryDictionary::RemoveFactory(IEntityFactory* factory)
{
	((_CEntityFactoryDictionary*)servertools->GetEntityFactoryDictionary())->RemoveFactory(factory);
}

void DispatchSpawn(CBaseEntity* entity)
{
	servertools->DispatchSpawn(entity);
}

FCall<void, const char*, Vector, QAngle, CBaseEntity*> fDispatchParticleEffect;
FCall<CBasePlayer*> fUTIL_GetCommandClient;

void DispatchParticleEffect(const char*pszParticleName, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pEntity)
{
	fDispatchParticleEffect(pszParticleName, vecOrigin, vecAngles, pEntity);
}

CBasePlayer* UTIL_GetCommandClient()
{
	return fUTIL_GetCommandClient();
}

CBasePlayer	*UTIL_PlayerByIndex(int playerIndex)
{
	CBasePlayer *pPlayer = NULL;

	if ( playerIndex > 0 && playerIndex <= gpGlobals->maxClients )
	{
		edict_t *pPlayerEdict = INDEXENT( playerIndex );
		if ( pPlayerEdict && !pPlayerEdict->IsFree() )
		{
			pPlayer = (CBasePlayer*)GetContainingEntity( pPlayerEdict );
		}
	}
	
	return pPlayer;
}

float UTIL_VecToYaw( const Vector &vec )
{
	if (vec.y == 0 && vec.x == 0)
		return 0;
	
	float yaw = atan2( vec.y, vec.x );

	yaw = RAD2DEG(yaw);

	if (yaw < 0)
		yaw += 360;

	return yaw;
}

void UTIL_LogPrintf(const char *fmt, ...)
{
	va_list		argptr;
	char		tempString[1024];
	
	va_start ( argptr, fmt );
	Q_vsnprintf( tempString, sizeof(tempString), fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	engine->LogPrint( tempString );
}