#ifndef asw_weapon_devastator_shared_h__
#define asw_weapon_devastator_shared_h__


#include "asw_weapon_assault_shotgun_shared.h"

#ifdef CLIENT_DLL
#define CASW_Weapon_Devastator C_ASW_Weapon_Devastator
#endif

class CASW_Weapon_Devastator : public CASW_Weapon_Assault_Shotgun
{
public:
	DECLARE_CLASS(CASW_Weapon_Devastator, CASW_Weapon_Assault_Shotgun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Devastator();
	virtual ~CASW_Weapon_Devastator();
	virtual void Precache();

	virtual void SecondaryAttack() {}	// don't inherit secondary attack
	virtual void FireShotgunPellet( CASW_Marine *pMarine, const FireBulletsInfo_t &info, int iSeed );	// shotgun specific, used to add piercing only for Devastator

	virtual float GetWeaponDamage();
	virtual float GetMovementScale();
	virtual const Vector& GetAngularBulletSpread();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual const char* GetPickupClass() { return "asw_pickup_devastator"; }
#else 
	virtual bool HasSecondaryExplosive(void) const { return false; }
#endif

	virtual bool ShouldMarineMoveSlow();
	virtual Class_T		Classify(void) { return (Class_T)CLASS_ASW_DEVASTATOR; }
};

#endif // asw_weapon_devastator_shared_h__
