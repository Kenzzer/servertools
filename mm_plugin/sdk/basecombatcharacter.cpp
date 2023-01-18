#include "basecombatcharacter.hpp"

VCall<void> CBaseCombatCharacter::vUpdateLastKnownArea;
VCall<CNavArea*> CBaseCombatCharacter::vGetLastKnownArea;
VCall<bool, const Vector&> CBaseCombatCharacter::vIsHiddenByFog_Vector;
VCall<bool, CBaseEntity*> CBaseCombatCharacter::vIsHiddenByFog_CBaseEntity;
VCall<bool, float> CBaseCombatCharacter::vIsHiddenByFog_float;
VCall<int, const CTakeDamageInfo&> CBaseCombatCharacter::vOnTakeDamage_Alive;

bool CBaseCombatCharacter::Init(CGameConfig* config, char* error, size_t maxlength)
{
	try
	{
		vUpdateLastKnownArea.Init(config, "CBaseCombatCharacter::UpdateLastKnownArea");
		vGetLastKnownArea.Init(config, "CBaseCombatCharacter::GetLastKnownArea");
		vIsHiddenByFog_Vector.Init(config, "CBaseCombatCharacter::IsHiddenByFog[Vector]");
		vIsHiddenByFog_CBaseEntity.Init(config, "CBaseCombatCharacter::IsHiddenByFog[CBaseEntity]");
		vIsHiddenByFog_float.Init(config, "CBaseCombatCharacter::IsHiddenByFog[float]");
		vOnTakeDamage_Alive.Init(config, "CBaseCombatCharacter::OnTakeDamage_Alive");
	}
	catch (const std::exception& e)
	{
		// Could use strncpy, but compiler complains
		snprintf(error, maxlength, "%s", e.what());
		return false;
	}
	return true;
}

void CBaseCombatCharacter::UpdateLastKnownArea(void)
{
	vUpdateLastKnownArea(this);
}

CNavArea* CBaseCombatCharacter::GetLastKnownArea(void)
{
	return vGetLastKnownArea(this);
}

bool CBaseCombatCharacter::IsHiddenByFog(const Vector& v)
{
	return vIsHiddenByFog_Vector(this, v);
}

bool CBaseCombatCharacter::IsHiddenByFog(CBaseEntity* ent)
{
	return vIsHiddenByFog_CBaseEntity(this, ent);
}

bool CBaseCombatCharacter::IsHiddenByFog(float v)
{
	return vIsHiddenByFog_float(this, v);
}

int CBaseCombatCharacter::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	return vOnTakeDamage_Alive(this, info);
}