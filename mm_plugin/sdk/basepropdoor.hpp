#pragma once

#include "baseentity.hpp"

class CBasePropDoor : public CBaseEntity
{
public:
	static bool Init(CGameConfig* config, char* error, size_t maxlength);

	bool IsDoorOpen();

protected:

	enum DoorState_t
	{
		DOOR_STATE_CLOSED = 0,
		DOOR_STATE_OPENING,
		DOOR_STATE_OPEN,
		DOOR_STATE_CLOSING,
		DOOR_STATE_AJAR,
	};

private:
	DECLAREVAR(DoorState_t, m_eDoorState);
};

inline bool CBasePropDoor::IsDoorOpen()
{
	return *m_eDoorState() == DOOR_STATE_OPEN;
}