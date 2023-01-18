#include "basepropdoor.hpp"

DEFINEVAR(CBasePropDoor, m_eDoorState);

bool CBasePropDoor::Init(CGameConfig* config, char* error, size_t maxlength)
{
	BEGIN_VAR("prop_door_rotating");
	OFFSETVAR_DATA(CBasePropDoor, m_eDoorState);
	END_VAR;
	return true;
}