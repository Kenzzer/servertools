#pragma once

#include <vector.h>
#include <iserverentity.h>
#include <itoolentity.h>
#include <shareddefs.h>
#include <eiface.h>

extern IVEngineServer *engine;

#include "helpers.hpp"
#include "gameconfig.hpp"
#include "servernetworkproperty.hpp"
#include "collisionproperty.hpp"

class INextBot;

enum InvalidatePhysicsBits_t
{
	POSITION_CHANGED	= 0x1,
	ANGLES_CHANGED		= 0x2,
	VELOCITY_CHANGED	= 0x4,
	ANIMATION_CHANGED	= 0x8,
};

extern float k_flMaxEntityEulerAngle;
extern float k_flMaxEntityPosCoord;
extern CGlobalVars* gpGlobals;
extern IServerTools* servertools;

#define DECLAREVAR(type, var) \
	static int32_t offset_##var; \
	inline type* var() const \
	{ \
		return (type*)((uint8_t*)this + offset_##var); \
	} 

#define DEFINEVAR(classname, var) \
int32_t (classname::classname::offset_##var) = 0

#define BEGIN_VAR(classentity) \
SendPropInfo offset_send_info; \
DataPropInfo offset_data_info; \
CBaseEntity* offsetEntity = servertools->CreateEntityByName(classentity);

#define END_VAR \
	servertools->RemoveEntityImmediate(offsetEntity)

#define OFFSETVAR_SEND(classname, var) \
if (!g_classHelpers.FindSendPropInfo(offsetEntity, #var, offset_send_info)) \
{ \
	snprintf(error, maxlength, "Failed to retrieve "  #classname "::"  #var  "!"); \
	return false; \
} \
offset_##var = offset_send_info.offset

#define OFFSETVAR_DATA(classname, var) \
if (!g_classHelpers.FindDataPropInfo(offsetEntity, #var, offset_data_info)) \
{ \
	snprintf(error, maxlength, "Failed to retrieve "  #classname "::"  #var  "!"); \
	return false; \
} \
offset_##var = offset_data_info.offset

#define VAR_OFFSET(var) \
	offset_##var

#define VAR_OFFSET_SET(var, val) \
	offset_##var = val


#define NETWORKVAR_UPDATE(var, value) \
	auto var##_toUpdate = var(); \
	*var##_toUpdate = value; \
	this->NetworkStateChanged(var##_toUpdate);

class CBaseEntity;

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), 0, nullptr )
#define SetContextThink( a, b, context ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), (b), context )

typedef void(CBaseEntity::*HBASEPTR)(void);

class CBaseCombatCharacter;
class CBaseAnimating;
class IPhysicsObject;

struct thinkfunc_t
{
	HBASEPTR	m_pfnThink;
	string_t	m_iszContext;
	int			m_nNextThinkTick;
	int			m_nLastThinkTick;

	DECLARE_SIMPLE_DATADESC();
};

class CBaseEntity : public IServerEntity
{
public:
	static bool Init(CGameConfig* config, char* error, size_t maxlength);

	static const trace_t& GetTouchTrace(void);

	static MCall<void, bool> CBaseEntity_Ctor;

	static VCall<void, const char*> vPostConstructor;
	void PostConstructor(const char*);

	static int offset_GetDataDescMap;

	static int offset_MyNextBotPointer;

	static VCall<datamap_t*> vGetDataDescMap;
	datamap_t* GetDataDescMap(void);

	static VCall<void> vUpdateOnRemove;
	void UpdateOnRemove(void);

	static VCall<void> vSpawn;
	void Spawn(void);

	static VCall<void> vPrecache;
	void Precache(void);

	static VCall<void, Vector*, Vector*, Vector*> vGetVectors;
	void GetVectors(Vector*, Vector*, Vector*);

	static VCall<void, const Vector*, const QAngle*, const Vector*> vTeleport;
	void Teleport(const Vector*, const QAngle*, const Vector*);

	static VCall<void, char const*> vSetModel;
	void SetModel(char const*);

	static VCall<Vector> vEyePosition;
	Vector EyePosition(void);

	static VCall<const QAngle &> vEyeAngles;
	const QAngle &EyeAngles(void);

	static VCall<CBaseCombatCharacter*> vMyCombatCharacterPointer;
	CBaseCombatCharacter* MyCombatCharacterPointer(void);
	
	static VCall<CBaseAnimating*> vGetBaseAnimating;
	CBaseAnimating* GetBaseAnimating(void);

	static VCall<INextBot*> vMyNextBotPointer;
	INextBot* MyNextBotPointer(void);

	static VCall<bool> vIsPlayer;
	bool IsPlayer(void);

	static MCall<void, int> mInvalidatePhysicsRecursive;
	void InvalidatePhysicsRecursive(int nChangeFlags);

	static VCall<const Vector&> vWorldSpaceCenter;
	const Vector& WorldSpaceCenter(void);

	static MCall<void> mCalcAbsolutePosition;
	void CalcAbsolutePosition(void);
	void CalcAbsoluteVelocity(void);

	static VCall<int, const CTakeDamageInfo&> vOnTakeDamage;
	int OnTakeDamage(const CTakeDamageInfo& info);

	static VCall<bool> vIsAlive;
	bool IsAlive(void);

	//static MCall<void, const Vector&> mSetAbsVelocity;
	//void SetAbsVelocity(const Vector&);

	static MCall<void, CBaseEntity*> mSetGroundEntity;
	void SetGroundEntity(CBaseEntity* ground);

	static VCall<void, CBaseEntity*> vTouch;
	void Touch(CBaseEntity*);
	
	static MCall<int, const CTakeDamageInfo&> mTakeDamage;
	int TakeDamage(const CTakeDamageInfo&);

	static bool IsPrecacheAllowed();
	static void SetAllowPrecache(bool);

public:

	static int	PrecacheModel( const char *name, bool bPreload = true );
	static bool PrecacheScriptSound( const char * );

	friend class CServerNetworkProperty;
	CServerNetworkProperty* NetworkProp();
	const CServerNetworkProperty* NetworkProp() const;
	CCollisionPropertyHack* CollisionProp();
	const CCollisionPropertyHack* CollisionProp() const;

	ServerClass* GetServerClass();

	void DispatchUpdateTransmitState(void);

	const char* GetClassname() const;
	string_t GetStringClassname() const;

	int GetSpawnFlags(void) const;
	void AddSpawnFlags(int nFlags);
	void RemoveSpawnFlags(int nFlags);
	void ClearSpawnFlags(void);
	bool HasSpawnFlags(int nFlags) const;

	edict_t* edict(void);
	const edict_t* edict(void) const;

	int	entindex(void) const;
	bool IsWorld(void) const;

	IPhysicsObject* VPhysicsGetObject(void) const;

	int				GetFlags(void) const;

	int				GetEFlags(void) const;
	bool			IsEFlagSet(int nEFlagMask) const;
	void			SetEFlags(int iEFlags);
	void			AddEFlags(int nEFlagMask);
	void			RemoveEFlags(int nEFlagMask);

	void SetBlocksLOS(bool bBlocksLOS);
	bool BlocksLOS(void);

	string_t GetModelName(void) const;
	void	SetModelName(string_t name);

	char GetLifeState(void) const;
	char GetTakeDamage(void) const;

	int		GetHealth() const;
	void	SetHealth(int amt);

	int		 RegisterThinkContext(const char* szContext);
	HBASEPTR ThinkSet(HBASEPTR func, float flNextThinkTime = 0, const char* szContext = nullptr);
	void	 SetNextThink(float nextThinkTime, const char* szContext = nullptr);
	float	 GetNextThink(const char* szContext = nullptr);
	float	 GetLastThink(const char* szContext = nullptr);
	int		 GetNextThinkTick(const char* szContext = nullptr);
	int		 GetLastThinkTick(const char* szContext = nullptr);
	bool	 WillThink(void);
	void	 CheckHasThinkFunction(bool isThinking);

	const Vector& GetAbsOrigin(void) const;
	const QAngle& GetAbsAngles(void) const;
	const Vector& GetAbsVelocity(void) const;

	void SetAbsOrigin(const Vector& absOrigin);
	void SetAbsAngles(const QAngle& absAngles);
	void SetAbsVelocity(const Vector& vecAbsVelocity);
	
	matrix3x4_t&		EntityToWorldTransform(void);
	const matrix3x4_t&	EntityToWorldTransform(void) const;
	matrix3x4_t&		GetParentToWorldTransform(matrix3x4_t &tempMatrix);

	CBaseEntity	*GetGroundEntity(void);

	void	NetworkStateChanged(void);
	void	NetworkStateChanged(void* pVar);

	float GetSimulationTime(void) const;
	void  SetSimulationTime(float st);

	void SetLocalAngles(const QAngle& angles);
	const QAngle& GetLocalAngles(void) const;

	void SetLocalOrigin(const Vector& origin);
	const Vector& GetLocalOrigin(void) const;

	int		   GetParentAttachment(void);
	MoveType_t GetMoveType(void) const;
	CBaseEntity* GetMoveParent(void);
	CBaseEntity* FirstMoveChild(void);
	CBaseEntity* NextMovePeer(void);

	int GetTeamNumber(void) const;

	const Vector& WorldAlignMins() const;
	const Vector& WorldAlignMaxs() const;

	static int offset_UpdateOnRemove;

protected:
	enum thinkmethods_t
	{
		THINK_FIRE_ALL_FUNCTIONS,
		THINK_FIRE_BASE_ONLY,
		THINK_FIRE_ALL_BUT_BASE,
	};
	int	GetIndexForThinkContext(const char*);

private:
	// Members
	using think_ptr = void (CBaseEntity::*)(void);
	DECLAREVAR(think_ptr, m_pfnThink);
	DECLAREVAR(string_t, m_iClassname);
	DECLAREVAR(short, m_nModelIndex);
	DECLAREVAR(float, m_flSimulationTime);

	DECLAREVAR(int, m_iMaxHealth);
	DECLAREVAR(int, m_iHealth);
	DECLAREVAR(char, m_lifeState);
	DECLAREVAR(char , m_takedamage);

	DECLAREVAR(int, m_spawnflags);

	DECLAREVAR(int, m_fFlags);
	DECLAREVAR(int, m_iEFlags);

	DECLAREVAR(unsigned char, m_iParentAttachment);
	DECLAREVAR(CUtlVector<thinkfunc_t>, m_aThinkFunctions);
	DECLAREVAR(int, m_nLastThinkTick);
	DECLAREVAR(int, m_nNextThinkTick);
	DECLAREVAR(unsigned char, m_MoveType);
	DECLAREVAR(EHANDLE, m_hMoveParent);
	DECLAREVAR(EHANDLE, m_hMoveChild);
	DECLAREVAR(EHANDLE, m_hMovePeer);
	DECLAREVAR(CCollisionPropertyHack, m_Collision);

	DECLAREVAR(int, m_CollisionGroup);
	DECLAREVAR(IPhysicsObject*, m_pPhysicsObject);

	DECLAREVAR(Vector, m_vecAbsOrigin);
	DECLAREVAR(QAngle, m_angAbsRotation);
	DECLAREVAR(Vector, m_vecAbsVelocity);

	DECLAREVAR(Vector, m_vecOrigin);
	DECLAREVAR(QAngle, m_angRotation);
	DECLAREVAR(Vector, m_vecVelocity);

	DECLAREVAR(matrix3x4_t, m_rgflCoordinateFrame);

	DECLAREVAR(int, m_iTeamNum);
	
	DECLAREVAR(EHANDLE, m_hGroundEntity);
	
	DECLAREVAR(string_t, m_ModelName);
};

inline CServerNetworkProperty* CBaseEntity::NetworkProp()
{
	return (CServerNetworkProperty*)GetNetworkable();
}

inline const CServerNetworkProperty* CBaseEntity::NetworkProp() const
{
	return (CServerNetworkProperty*)((CBaseEntity*)this)->GetNetworkable();
}

inline CCollisionPropertyHack* CBaseEntity::CollisionProp()
{
	return m_Collision();
}

inline const CCollisionPropertyHack* CBaseEntity::CollisionProp() const
{
	return m_Collision();
}

inline ServerClass* CBaseEntity::GetServerClass()
{
	return NetworkProp()->GetServerClass();
}

inline void	CBaseEntity::NetworkStateChanged()
{
	NetworkProp()->NetworkStateChanged();
}

inline void	CBaseEntity::NetworkStateChanged(void* pVar)
{
	// Good, they passed an offset so we can track this variable's change
	// and avoid sending the whole entity.
	NetworkProp()->NetworkStateChanged((char*)pVar - (char*)this);
}

inline int CBaseEntity::entindex(void) const
{
	return NetworkProp()->entindex();
}

inline edict_t* CBaseEntity::edict( void )
{
	return NetworkProp()->edict();
}

inline const edict_t* CBaseEntity::edict(void) const
{
	return NetworkProp()->edict();
}

inline IPhysicsObject* CBaseEntity::VPhysicsGetObject(void) const
{
	return *m_pPhysicsObject();
}

inline const char* CBaseEntity::GetClassname() const
{
	if (*m_iClassname() == NULL_STRING)
	{
		return nullptr;
	}
	return m_iClassname()->ToCStr();
}

inline string_t CBaseEntity::GetStringClassname() const
{
	return *m_iClassname();
}

inline int CBaseEntity::GetSpawnFlags(void) const
{ 
	return *m_spawnflags();
}

inline void CBaseEntity::AddSpawnFlags(int nFlags)
{ 
	*m_spawnflags() |= nFlags;
}
inline void CBaseEntity::RemoveSpawnFlags(int nFlags)
{ 
	*m_spawnflags() &= ~nFlags;
}

inline void CBaseEntity::ClearSpawnFlags(void)
{ 
	*m_spawnflags() = 0;
}

inline bool CBaseEntity::HasSpawnFlags(int nFlags) const
{ 
	return (*m_spawnflags() & nFlags) != 0; 
}

inline bool CBaseEntity::IsWorld(void) const 
{
	return entindex() == 0;
}

inline int CBaseEntity::GetFlags(void) const
{
	return *m_fFlags();
}

inline int CBaseEntity::GetEFlags(void) const
{
	return *m_iEFlags();
}

inline void CBaseEntity::SetEFlags(int iEFlags)
{
	*m_iEFlags() = iEFlags;

	if (iEFlags & (EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX))
	{
		this->DispatchUpdateTransmitState();
	}
}

inline void CBaseEntity::AddEFlags(int nEFlagMask)
{
	*m_iEFlags() |= nEFlagMask;

	if (nEFlagMask & (EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX))
	{
		this->DispatchUpdateTransmitState();
	}
}

inline void CBaseEntity::RemoveEFlags(int nEFlagMask)
{
	*m_iEFlags() &= ~nEFlagMask;
	
	if (nEFlagMask & (EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX))
	{
		this->DispatchUpdateTransmitState();
	}
}

inline bool CBaseEntity::IsEFlagSet(int nEFlagMask) const
{
	return (*m_iEFlags() & nEFlagMask) != 0;
}

inline void CBaseEntity::SetBlocksLOS(bool bBlocksLOS)
{
	if (bBlocksLOS)
	{
		RemoveEFlags(EFL_DONTBLOCKLOS);
	}
	else
	{
		AddEFlags(EFL_DONTBLOCKLOS);
	}
}

inline bool CBaseEntity::BlocksLOS(void) 
{ 
	return !IsEFlagSet(EFL_DONTBLOCKLOS); 
}

inline void CBaseEntity::SetModelName(string_t name)
{
	*m_ModelName() = name;
	DispatchUpdateTransmitState();
}

inline string_t CBaseEntity::GetModelName(void) const
{
	return *m_ModelName();
}

inline char CBaseEntity::GetLifeState(void) const
{
	return *m_lifeState();
}

inline char CBaseEntity::GetTakeDamage(void) const
{
	return *m_takedamage();
}

inline int CBaseEntity::GetHealth() const
{
	return *m_iHealth();
}

inline void CBaseEntity::SetHealth(int amt)
{
	*m_iHealth() = amt;
}

inline CBaseEntity* CBaseEntity::GetGroundEntity(void)
{
	return *(m_hGroundEntity());
}

inline const Vector& CBaseEntity::GetAbsOrigin(void) const
{
	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsolutePosition();
	}
	return *m_vecAbsOrigin();
}

inline const QAngle& CBaseEntity::GetAbsAngles(void) const
{
	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsolutePosition();
	}
	return *m_angAbsRotation();
}

inline const Vector& CBaseEntity::GetAbsVelocity() const
{
	if (IsEFlagSet(EFL_DIRTY_ABSVELOCITY))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsoluteVelocity();
	}
	return *m_vecAbsVelocity();
}

inline matrix3x4_t &CBaseEntity::EntityToWorldTransform() 
{
	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		CalcAbsolutePosition();
	}
	return *m_rgflCoordinateFrame(); 
}

inline const matrix3x4_t &CBaseEntity::EntityToWorldTransform() const
{
	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsolutePosition();
	}
	return *m_rgflCoordinateFrame(); 
}

inline float CBaseEntity::GetSimulationTime() const
{
	return *m_flSimulationTime();
}

inline void CBaseEntity::SetSimulationTime(float st)
{
	*m_flSimulationTime() = st;
}

inline const QAngle& CBaseEntity::GetLocalAngles(void) const
{
	return *m_angRotation();
}

inline const Vector& CBaseEntity::GetLocalOrigin(void) const
{
	return *m_vecOrigin();
}

inline CBaseEntity* CBaseEntity::GetMoveParent(void)
{
	return (CBaseEntity*)m_hMoveParent()->Get();
}

inline CBaseEntity* CBaseEntity::FirstMoveChild(void)
{
	return (CBaseEntity*)m_hMoveChild()->Get();
}

inline CBaseEntity* CBaseEntity::NextMovePeer(void)
{
	return (CBaseEntity*)m_hMovePeer()->Get();
}

inline int CBaseEntity::GetParentAttachment()
{
	return *m_iParentAttachment();
}

inline MoveType_t CBaseEntity::GetMoveType() const
{
	return (MoveType_t)(unsigned char)*m_MoveType();
}

inline int CBaseEntity::GetTeamNumber(void) const
{
	return *m_iTeamNum();
}

inline const Vector& CBaseEntity::WorldAlignMins() const
{
	return CollisionProp()->OBBMins();
}

inline const Vector& CBaseEntity::WorldAlignMaxs() const
{
	return CollisionProp()->OBBMaxs();
}

inline bool IsEntityQAngleReasonable(const QAngle& q)
{
	float r = k_flMaxEntityEulerAngle;
	return
		q.x > -r && q.x < r &&
		q.y > -r && q.y < r &&
		q.z > -r && q.z < r;
}

inline bool IsEntityPositionReasonable(const Vector& v)
{
	float r = k_flMaxEntityPosCoord;
	return
		v.x > -r && v.x < r &&
		v.y > -r && v.y < r &&
		v.z > -r && v.z < r;
}

inline bool NamesMatch(const char *pszQuery, string_t nameToMatch)
{
	if (nameToMatch == NULL_STRING)
		return (!pszQuery || *pszQuery == 0 || *pszQuery == '*');

	const char *pszNameToMatch = STRING(nameToMatch);

	// If the pointers are identical, we're identical
	if ( pszNameToMatch == pszQuery )
		return true;

	while ( *pszNameToMatch && *pszQuery )
	{
		unsigned char cName = *pszNameToMatch;
		unsigned char cQuery = *pszQuery;
		// simple ascii case conversion
		if ( cName == cQuery )
			;
		else if ( cName - 'A' <= (unsigned char)'Z' - 'A' && cName - 'A' + 'a' == cQuery )
			;
		else if ( cName - 'a' <= (unsigned char)'z' - 'a' && cName - 'a' + 'A' == cQuery )
			;
		else
			break;
		++pszNameToMatch;
		++pszQuery;
	}

	if ( *pszQuery == 0 && *pszNameToMatch == 0 )
		return true;

	// @TODO (toml 03-18-03): Perhaps support real wildcards. Right now, only thing supported is trailing *
	if ( *pszQuery == '*' )
		return true;

	return false;
}

inline bool FClassnameIs(const CBaseEntity* entity, const char* classname)
{
	return (entity->GetClassname() != nullptr && strcmp(entity->GetClassname(), classname) == 0)
	|| (NamesMatch(classname, entity->GetStringClassname()));
}

inline CBaseEntity* GetContainingEntity(edict_t* pent)
{
	if (pent && pent->GetUnknown())
	{
		return pent->GetUnknown()->GetBaseEntity();
	}

	return nullptr;
}

void PrecacheParticleSystem(const char* name);