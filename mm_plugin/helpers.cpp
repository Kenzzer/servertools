#include "helpers.hpp"
#include "sdk/baseentity.hpp"
#include "engine_wrappers.h"
#include "interfaces.hpp"
#include <util_shared.h>

#ifndef __linux__
struct RTTIBaseClassArray;
struct TypeDescriptor;
// http://www.openrce.org/articles/full_view/23
struct RTTIClassHierarchyDescriptor
{
	DWORD signature;
	DWORD attributes;
	DWORD numBaseClasses;
	struct RTTIBaseClassArray* pBaseClassArray;
};
struct RTTICompleteObjectLocator
{
	DWORD signature;
	DWORD offset;
	DWORD cdOffset;
	struct TypeDescriptor* pTypeDescriptor;
	struct RTTIClassHierarchyDescriptor* pClassDescriptor;
};
#else
class __class_type_info;
// https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/tinfo.h
struct vtable_prefix
{
  ptrdiff_t whole_object;
#ifdef _GLIBCXX_VTABLE_PADDING
  ptrdiff_t padding1;               
#endif
  const __class_type_info *whole_type;  
#ifdef _GLIBCXX_VTABLE_PADDING
  ptrdiff_t padding2;               
#endif
  const void *origin;               
};
#endif

void** vtable_dup(void* thisPtr, std::size_t numFuncs)
{

#ifndef _LINUX
	std::size_t size = sizeof(RTTICompleteObjectLocator*) + (sizeof(void*) * numFuncs);
#else
	std::size_t size = sizeof(vtable_prefix) + (sizeof(void*) * numFuncs);
#endif

	std::uint8_t* newvtable = (uint8_t*)calloc(1, size);
	std::uint8_t* vtable = *(uint8_t**)thisPtr;

#ifndef _LINUX
	std::uint8_t* vtable_start = vtable - sizeof(RTTICompleteObjectLocator*);
#else
	std::uint8_t* vtable_start = vtable - sizeof(vtable_prefix);
#endif
	memcpy(newvtable, vtable_start, size);

#ifndef _LINUX
	newvtable += sizeof(RTTICompleteObjectLocator*);
#else
	newvtable += sizeof(vtable_prefix);
#endif

	return (void **)newvtable;
}

void vtable_replace(void* thisPtr, void** newVtable)
{
	*(void ***)thisPtr = newVtable;
}

int ClassPropertiesDictionary::CSendPropExtra_UtlVector_offset = -1;
ClassPropertiesDictionary g_classHelpers;

bool UTIL_FindInSendTable(SendTable* pTable, 
						  const char* name,
						  SendPropInfo& info,
						  unsigned int offset)
{
	int props = pTable->GetNumProps();
	for (int i = 0; i < props; i++)
	{
		SendProp *prop = pTable->GetProp(i);

		if (prop->IsInsideArray())
		{
			continue;
		}

		const char *pname = prop->GetName();
		SendTable *pInnerTable = prop->GetDataTable();

		if (pname && strcmp(name, pname) == 0)
		{
			if (ClassPropertiesDictionary::CSendPropExtra_UtlVector_offset != -1 && prop->GetOffset() == 0 && pInnerTable && pInnerTable->GetNumProps())
			{
				SendProp *pLengthProxy = pInnerTable->GetProp(0);
				const char *ipname = pLengthProxy->GetName();
				if (ipname && strcmp(ipname, "lengthproxy") == 0 && pLengthProxy->GetExtraData())
				{
					info.prop = prop;
					info.offset = offset + *reinterpret_cast<size_t *>(reinterpret_cast<intptr_t>(pLengthProxy->GetExtraData()) + ClassPropertiesDictionary::CSendPropExtra_UtlVector_offset);
					return true;
				}
			}
			info.prop = prop;
			info.offset = offset + info.prop->GetOffset();
			return true;
		}

		if (pInnerTable)
		{
			if (UTIL_FindInSendTable(pInnerTable, 
				name,
				info,
				offset + prop->GetOffset())
				)
			{
				return true;
			}
		}
	}

	return false;
}

bool UTIL_FindInDataMap(datamap_t* pMap, const char* name, DataPropInfo& info)
{
	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == nullptr)
			{
				continue;
			}
			if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
			{
				info.map = &(pMap->dataDesc[i]);
				info.offset = GetTypeDescOffs(info.map);
				return true;
			}
			if (pMap->dataDesc[i].td == nullptr || !UTIL_FindInDataMap(pMap->dataDesc[i].td, name, info))
			{
				continue;
			}
			
			info.offset += GetTypeDescOffs(&(pMap->dataDesc[i]));
			return true;
		}
		
		pMap = pMap->baseMap;
	}

	return false; 
}

bool ClassPropertiesDictionary::Init(CGameConfig* config)
{
	ClassPropertiesDictionary::CSendPropExtra_UtlVector_offset = config->GetOffset("CSendPropExtra_UtlVector::m_Offset");
	return true;
}

bool ClassPropertiesDictionary::FindSendPropInfo(CBaseEntity* entity, const std::string& name, SendPropInfo& info)
{
	ServerClass* svClass = entity->GetServerClass();
	auto& map = _sendprops[svClass];

	if (map.find(name) == map.end())
	{
		if (UTIL_FindInSendTable(svClass->m_pTable, name.c_str(), info, 0))
		{
			map[name] = info;
			return true;
		}
		return false;
	}
	info = map[name];
	return true;
}

bool ClassPropertiesDictionary::FindDataPropInfo(CBaseEntity* entity, const std::string& name, DataPropInfo& info)
{
	datamap_t* datamap = entity->GetDataDescMap();
	auto& map = _dataprops[datamap];

	if (map.find(name) == map.end())
	{
		if (UTIL_FindInDataMap(datamap, name.c_str(), info))
		{
			map[name] = info;
			return true;
		}
		return false;
	}
	info = map[name];
	return true;
}

string_t AllocPooledString(const char* pszValue)
{
	CBaseEntity* pEntity = ((IServerUnknown*)servertools->FirstEntity())->GetBaseEntity();

	static int offset = -1;
	if (offset == -1)
	{
		DataPropInfo info;
		bool found = g_classHelpers.FindDataPropInfo(pEntity, "m_iName", info);
		offset = info.offset;
	}

	string_t *pProp = (string_t*)((intp)pEntity + offset);
	string_t backup = *pProp;
	servertools->SetKeyValue(pEntity, "targetname", pszValue);
	string_t newString = *pProp;
	*pProp = backup;
	return newString;
}

void DebugDrawLine(const Vector& vecAbsStart, const Vector& vecAbsEnd, int r, int g, int b, bool test, float duration)
{
}

float IntervalTimer::Now(void) const
{
	return gpGlobals->curtime;
}

float CountdownTimer::Now(void) const
{
	return gpGlobals->curtime;
}

bool CGameTrace::DidHitWorld() const
{
	return m_pEnt == servertools->FirstEntity();
}

bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != nullptr && !DidHitWorld();
}