#pragma once

#include "baseanimatingoverlay.hpp"
#include <ehandle.h>
#include <isaverestore.h>
#include <takedamageinfo.h>

class CNavArea;
class CBaseCombatCharacter : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS_NOBASE(CBaseCombatCharacter);
	static bool Init(CGameConfig* config, char* error, size_t maxlength);

	static VCall<void> vUpdateLastKnownArea;
	void UpdateLastKnownArea(void);

	static VCall<CNavArea*> vGetLastKnownArea;
	CNavArea* GetLastKnownArea(void);

	static VCall<bool, const Vector&> vIsHiddenByFog_Vector;
	bool IsHiddenByFog(const Vector&);

	static VCall<bool, CBaseEntity*> vIsHiddenByFog_CBaseEntity;
	bool IsHiddenByFog(CBaseEntity*);

	static VCall<bool, float> vIsHiddenByFog_float;
	bool IsHiddenByFog(float);

	static VCall<int, const CTakeDamageInfo&> vOnTakeDamage_Alive;
	int OnTakeDamage_Alive(const CTakeDamageInfo& info);
};