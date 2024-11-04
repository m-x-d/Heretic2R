#ifndef _G_VOLUME_EFFECT_H_
#define _G_VOLUME_EFFECT_H_

#include "q_Typedef.h"
#include "g_local.h"

#define VE_FLAG1 			0x00000001	 	// place holder

/*-------------------------------------------------------------------------
	volume_effect_s
-------------------------------------------------------------------------*/
typedef struct volume_effect_s
{
	// fill these fields in when you pass this to AddVolumeEffect
	int flags;
	vec3_t origin; 					//center of the volume
	vec3_t velocity;				//direction that area is moving
	float growdelta;				//how much it grows/shrinks per think
	float radius;					//for spherical volumes				
	float expiration;				//time that volume ceases to effect
	float think_increment;			//frequency of effect in seconds
	float nextthinktime;

	void (*effect)(edict_t *ent, struct volume_effect_s *vol); 	//callback func for effect 

	edict_t *owner;					//the edict used as "inflictor" or "attacker"
									//for damage effects

	struct volume_effect_s *next;			// next effect in global list, don't set this

} volume_effect_t; 


/*-------------------------------------------------------------------------
	Prototypes
-------------------------------------------------------------------------*/
void VolumeEffectThink();
volume_effect_t *AddVolumeEffect(edict_t *owner, int flags, float thinkIncrement);

#endif
