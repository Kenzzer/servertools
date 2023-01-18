#pragma once

#include "gameconfig.hpp"
#include "sourcehook.h"
#if defined DEBUG
#include "interfaces.hpp"
#endif

#include <string_t.h>
#include <datamap.h>
#include <dt_send.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <unordered_map>

class CBaseEntity;
class ServerClass;

template<typename ReturnType, typename... Args>
class FCall
{
public:
	FCall() : func(nullptr) {};

	void Init(CGameConfig* config, const char* name)
	{
#if defined DEBUG
		META_CONPRINTF("Initialising FCall(%s)!\n", name);
#endif
		func = (ReturnType(*)(Args...))config->GetMemSig(name);
		if (!((void*)func))
		{
			std::string error("Failed to find signature: ");
			throw std::runtime_error(error + name);
		}
	}

	ReturnType operator()(Args... args)
	{
		return (*func)(args...);
	}

private:
	ReturnType (*func)(Args...);
};

template<typename ReturnType, typename... Args>
class CCall
{
protected:
	CCall() {};
	class funcEmptyClass {};

	// https://wiki.alliedmods.net/Virtual_Offsets_(Source_Mods)
	ReturnType Call(const void* thisPtr, void* func, Args... args) const
	{
#if defined DEBUG
		META_CONPRINTF("CCall this: 0x%08X addr: 0x%08X\n", thisPtr, func);
#endif
		void **this_ptr = *(void ***)&thisPtr;
		union
		{
			ReturnType (funcEmptyClass::*mfpnew)(Args...);
#ifndef _LINUX
			void *addr;
		} u;
		u.addr = func;
#else /* GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 */
			struct
			{
				void *addr;
				intptr_t adjustor;
			} s;
		} u;
		u.s.addr = func;
		u.s.adjustor = 0;
#endif
		return (ReturnType)(reinterpret_cast<funcEmptyClass*>(this_ptr)->*u.mfpnew)(args...);
	}
};

template<typename ReturnType, typename... Args>
class MCall;

template<typename ReturnType, typename... Args>
class VCall : public CCall<ReturnType, Args...>
{
public:
	VCall() : offset(0) {};

	void Init(CGameConfig* config, const char* name)
	{
#if defined DEBUG
		META_CONPRINTF("Initialising VCall(%s)!\n", name);
#endif
		offset = config->GetOffset(name);
		if (offset == -1)
		{
			std::string error("Failed to find offset: ");
			throw std::runtime_error(error + name);
		}
	}

	template<typename Function>
	void Init(Function f)
	{
		SourceHook::MemFuncInfo mfi = {true, -1, 0, 0};
		SourceHook::GetFuncInfo(f, mfi);
		if (mfi.thisptroffs < 0 || !mfi.isVirtual)
		{
			throw new std::runtime_error("Given function instance is not virtual!");
		}
		offset = mfi.vtblindex;
	}

	template<typename hackClass>
	void* Replace(void** vtable, ReturnType (hackClass::*infecfunc)(Args...))
	{
		void* func = nullptr;
		union
		{
			ReturnType (hackClass::*mfpnew)(Args...);
#ifndef _LINUX
			void *addr;
		} u;
		u.mfpnew = infecfunc;
		func = u.addr;
#else
			struct
			{
				void *addr;
				intptr_t adjustor;
			} s;
		} u;
		u.mfpnew = infecfunc;
		func = u.s.addr;
#endif
		void* oldFunc = vtable[offset];
		vtable[offset] = func;
		return oldFunc;
	}

	template<typename hackClass>
	void* Replace(void** vtable, ReturnType (hackClass::*infecfunc)(Args...) const)
	{
		void* func = nullptr;
		union
		{
			ReturnType (hackClass::*mfpnew)(Args...) const;
#ifndef _LINUX
			void *addr;
		} u;
		u.mfpnew = infecfunc;
		func = u.addr;
#else
			struct
			{
				void *addr;
				intptr_t adjustor;
			} s;
		} u;
		u.mfpnew = infecfunc;
		func = u.s.addr;
#endif
		void* oldFunc = vtable[offset];
		vtable[offset] = func;
		return oldFunc;
	}

	template<typename hackClass>
	void* Replace(void** vtable, ReturnType (hackClass::*infecfunc)(Args...), MCall<ReturnType, Args...>& pOriginalCall)
	{
		void* pOldFunc = Replace(vtable, infecfunc);
		pOriginalCall.Init(pOldFunc);
		return pOldFunc;
	}

	ReturnType operator()(const void* thisPtr, Args... args) const
	{
		void **vtable = *(void ***)thisPtr;
		void *func = vtable[offset];

#if defined DEBUG
		META_CONPRINTF("VCall vtable: 0x%08X offset: %d\n", vtable, offset);
#endif
		return this->Call(thisPtr, func, args...);
	}

	int GetOffset()
	{
		return offset;
	}

private:
	int offset;
};

template<typename ReturnType, typename... Args>
class MCall : public CCall<ReturnType, Args...>
{
public:
	MCall() : func(nullptr) {};

	void Init(CGameConfig* config, const char* name)
	{
#if defined DEBUG
		META_CONPRINTF("Initialising MCall(%s)!\n", name);
#endif
		func = config->GetMemSig(name);
		if (!func)
		{
			std::string error("Failed to find signature: ");
			throw std::runtime_error(error + name);
		}
	}

	void Init(void* addr)
	{
		func = addr;
	}

	ReturnType operator()(const void* thisPtr, Args... args) const
	{
		return this->Call(thisPtr, func, args...);
	}

private:
	void* func;
};

void** vtable_dup(void* thisPtr, size_t numFuncs);
void vtable_replace(void* thisPtr, void** newVtable);

struct SendPropInfo
{
	SendProp* prop;
	int offset;
};

struct DataPropInfo
{
	typedescription_t* map;
	int offset;
};

class ClassPropertiesDictionary
{
public:
	bool Init(CGameConfig* config);

	bool FindSendPropInfo(CBaseEntity* entity, const std::string& name, SendPropInfo& info);
	bool FindDataPropInfo(CBaseEntity* entity, const std::string& name, DataPropInfo& info);

private:
	std::unordered_map<ServerClass*, std::unordered_map<std::string, SendPropInfo>> _sendprops;
	std::unordered_map<datamap_t*, std::unordered_map<std::string, DataPropInfo>> _dataprops;
public:
	static int CSendPropExtra_UtlVector_offset;
};

extern ClassPropertiesDictionary g_classHelpers;
string_t AllocPooledString(const char* pszValue);