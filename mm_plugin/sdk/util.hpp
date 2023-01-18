#pragma once

#include <mathlib.h>
#include <set>
#include <utility>

#include "util.h"

class CMetaModFactoryDictionary
{
public:
	void InstallFactory(IEntityFactory* factory, const char* className);
	void InstallFactories(void);
	void RemoveFactory(IEntityFactory* factory);

private:
	std::set<std::pair<IEntityFactory*, const char*>> _factories;
};

extern CMetaModFactoryDictionary s_MetamodFactory;

inline CMetaModFactoryDictionary* MetaModFactoryDictionary()
{
	return &s_MetamodFactory;
}

template <class T>
class CMetaModFactory : public IEntityFactory
{
public:
	CMetaModFactory(const char* pClassName)
	{
		MetaModFactoryDictionary()->InstallFactory(this, pClassName);
	}

	~CMetaModFactory()
	{
		MetaModFactoryDictionary()->RemoveFactory(this);
	}

	virtual IServerNetworkable *Create( const char *pClassName ) override
	{
		return T::FactoryCreate(pClassName);
	}

	virtual void Destroy( IServerNetworkable *pNetworkable ) override
	{
		T::FactoryDestroy(pNetworkable);
	}

	virtual size_t GetEntitySize() override
	{
		return T::FactorySizeOf();
	}
};

#undef LINK_ENTITY_TO_CLASS

#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	static CMetaModFactory<DLLClassName> mapClassName( #mapClassName );

extern void DispatchSpawn(CBaseEntity* entity);

extern FCall<void, const char*, Vector, QAngle, CBaseEntity*> fDispatchParticleEffect;
void DispatchParticleEffect(const char*pszParticleName, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pEntity = nullptr);

extern FCall<CBasePlayer*> fUTIL_GetCommandClient;