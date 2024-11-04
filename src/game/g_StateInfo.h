#ifndef G_ACTIONINFO_H
#define G_ACTIONINFO_H

// hmm, this is pretty small now that MoveInfo has been created. . .
// Hopefully there will be more stuff to add to this
typedef struct ActionInfo_s
{
	int			state;
	int			readiness;
	int			curWeaponID;
} ActionInfo_t;

typedef enum WeaponID_s
{
	WEAPON_MELEE_0,
	WEAPON_MISSILE_0,
	WEAPON_SPELL_0,
	NUM_WEAPON_IDS
} WeaponID_t;

extern ActionInfo_t *ActionInfo_new();
extern void ActionInfo_delete(ActionInfo_t *toDelete);

#endif // G_ACTIONINFO_H
