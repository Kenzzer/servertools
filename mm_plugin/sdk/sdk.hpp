#pragma once

#include "gameconfig.hpp"

class CSDK
{
public:
	static bool Init(CGameConfig* config, char* error, size_t maxlen);
};