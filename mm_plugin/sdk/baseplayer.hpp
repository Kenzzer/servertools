#pragma once

#include "basecombatcharacter.hpp"

class CBasePlayer : public CBaseCombatCharacter
{
public:
	void EyeVectors(Vector*, Vector* = nullptr, Vector* = nullptr);
};

inline void CBasePlayer::EyeVectors(Vector* pForward, Vector* pRight, Vector* pUp)
{
	AngleVectors( EyeAngles(), pForward, pRight, pUp );
}