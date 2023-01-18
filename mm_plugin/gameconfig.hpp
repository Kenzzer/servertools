#pragma once

#include "KeyValues.h"

#include <string>
#include <unordered_map>

class CGameConfig
{
public:
	CGameConfig(const std::string& path);
	~CGameConfig();
	
	int GetOffset(const std::string& name);
	void* GetMemSig(const std::string& name);
	
private:
	KeyValues* _kv;
	std::unordered_map<std::string, int> _offsets;
	std::unordered_map<std::string, void*> _signatures;
};