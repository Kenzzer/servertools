
#include "baseentityoutput.hpp"
#include "baseentity.hpp"

CUtlMemoryPool * g_pEntityListPool = nullptr;
ISaveRestoreOps *eventFuncs = nullptr;

bool CBaseEntityOutput::Init(CGameConfig* config, char* error, size_t maxlength)
{
	// g_EntityListPool
	uint8_t* addr = (uint8_t*)config->GetMemSig("g_EntityListPool");
	if (addr)
	{
#if defined _LINUX
		g_pEntityListPool = reinterpret_cast<CUtlMemoryPool*>(addr);
#else
		int offset = config->GetOffset("g_EntityListPool");
		if (offset == -1)
		{
			snprintf(error, maxlength, "Couldn't find offset for g_EntityListPool ptr!");
			return false;
		}
		g_pEntityListPool = *reinterpret_cast<CUtlMemoryPool**>(addr + offset);
#endif
	}
	else
	{
		snprintf(error, maxlength, "Couldn't find g_EntityListPool!");
		return false;
	}

	// eventFuncs
	CBaseEntity* pOffsetEnt = servertools->CreateEntityByName("info_target");
	if (pOffsetEnt)
	{
		for (datamap_t* pDataMap = pOffsetEnt->GetDataDescMap(); pDataMap && !eventFuncs; pDataMap = pDataMap->baseMap)
		{
			for (int i = 0; i < pDataMap->dataNumFields; i++)
			{
				typedescription_t *pTypeDesc = &pDataMap->dataDesc[i];
				if (pTypeDesc->fieldType == FIELD_CUSTOM && ( pTypeDesc->flags & FTYPEDESC_OUTPUT ) )
				{
					if (pTypeDesc->pSaveRestoreOps)
					{
						// All outputs use eventFuncs as pSaveRestoreOps
						eventFuncs = pTypeDesc->pSaveRestoreOps;
						break;
					}
				}
			}
		}

		servertools->RemoveEntityImmediate(pOffsetEnt);
	}

	if (!eventFuncs)
	{
		snprintf(error, maxlength, "Couldn't find eventFuncs!");
		return false;
	}

    return true;
}

void CBaseEntityOutput::Init()
{
	m_ActionList = nullptr;

	m_Value.fieldType = FIELD_VOID;
	m_Value.iVal = 0;
}

void CBaseEntityOutput::Destroy()
{
    DeleteAllElements();
}

void CBaseEntityOutput::DeleteAllElements( void ) 
{
	CEventAction* pNext = m_ActionList;
	m_ActionList = NULL;
	while (pNext)
	{
		CEventAction* strikeThis = pNext;
		pNext = pNext->m_pNext;

		// This can be directly called like this because CUtlMemoryPool::Free() 
		// doesn't physically free memory, rather it marks the given memory 
		// block as free in its internal structure.
		g_pEntityListPool->Free(strikeThis);
	}
}

int CBaseEntityOutput::NumberOfElements( void )
{
	int count = 0;
	for (CEventAction* ev = m_ActionList; ev != NULL; ev = ev->m_pNext)
	{
		count++;
	}
	return count;
}

float CBaseEntityOutput::GetMaxDelay(void)
{
	float flMaxDelay = 0;
	CEventAction* ev = m_ActionList;

	while (ev != NULL)
	{
		if (ev->m_flDelay > flMaxDelay)
		{
			flMaxDelay = ev->m_flDelay;
		}
		ev = ev->m_pNext;
	}

	return flMaxDelay;
}