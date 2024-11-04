//
// g_obj.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "fx.h"
#include "g_local.h"
#include "g_misc.h"
#include "g_DefaultMessageHandler.h"
#include "vector.h"
#include "m_plagueElf_anim.h"
//#include "m_plaguessithra_anim.h"		// Can't do this because NUM_MESH_NODES is in all tris headers
#include "Random.h"
#include "angles.h"
#include "g_playstats.h"
#include "m_stats.h"


void SpawnFlame(edict_t *self,vec3_t origin);
void SpawnClientAnim(edict_t *self, byte type, char *sound);


void ObjectStaticsInit(void) 
{
	classStatics[CID_OBJECT].msgReceivers[MSG_DEATH] = DefaultObjectDieHandler;
}

void objpush_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{//FIXME: make player push?
	float	ratio;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	if (M_walkmove (self, other->s.angles[YAW], 20 * ratio * FRAMETIME))
	{
		if (self->pain_debounce_time < level.time)
		{
			// there are going to be more sounds to choose from, dependant on the mass of the object
			gi.sound (self, CHAN_BODY, gi.soundindex("misc/barrelmove.wav"), 1, ATTN_STATIC, 0);
			self->pain_debounce_time = level.time + 1.2;
		}			
	}
}

/*--------------------------------------
 It is assumed all bounding boxes for objects were initially implimented as if the objects 
 yaw was 0
----------------------------------------*/
void BboxYawAndScale(edict_t *self)
{
	vec3_t holdmins,holdmaxs;
	vec3_t	forward, right;
	vec3_t point[4],minpoint,maxpoint;
	int i;
	float scale;

	VectorCopy(self->mins,holdmins);
	VectorCopy(self->maxs,holdmaxs);

	AngleVectors (self->s.angles,forward, right,NULL);

	VectorMA (self->s.origin, holdmins[0],forward, point[0]);
	VectorMA (point[0], holdmins[1],right, point[0]);

	VectorMA (self->s.origin, holdmaxs[0],forward, point[1]);
	VectorMA (point[1], holdmaxs[1],right, point[1]);

	VectorMA (self->s.origin, holdmins[0],forward, point[2]);
	VectorMA (point[2], holdmaxs[1],right, point[2]);

	VectorMA (self->s.origin, holdmaxs[0],forward, point[3]);
	VectorMA (point[3], holdmins[1],right, point[3]);

	minpoint[0] = point[0][0];
	minpoint[1] = point[0][1];

	maxpoint[0] = point[0][0];
	maxpoint[1] = point[0][1];

	// Find max and min point
	for (i=0;i<4;++i)
	{
		if (point[i][0] < minpoint[0])
			minpoint[0] = point[i][0];

		if (point[i][1] < minpoint[1])
			minpoint[1] = point[i][1];


		if (point[i][0] > maxpoint[0])
			maxpoint[0] = point[i][0];

		if (point[i][1] > maxpoint[1])
			maxpoint[1] = point[i][1];

	}

	scale = self->s.scale;
	if (scale == 0)
		scale = 1;

	self->mins[0] = (minpoint[0] - self->s.origin[0]) * scale;
	self->mins[1] = (minpoint[1] - self->s.origin[1]) * scale;
	self->mins[2] = self->mins[2] * scale;

	self->maxs[0] = (maxpoint[0] - self->s.origin[0]) * scale;
	self->maxs[1] = (maxpoint[1] - self->s.origin[1]) * scale;
	self->maxs[2] = self->maxs[2] * scale;

}

/*--------------------------------------
 It is assumed all bounding boxes for objects were initially implimented as if the objects 
 yaw was 0
----------------------------------------*/
void BboxYawAndScaleAndMoveUp(edict_t *self)
{
	vec3_t holdmins,holdmaxs;
	vec3_t	forward, right;
	vec3_t point[4],minpoint,maxpoint;
	int i;
	float scale;

	VectorCopy(self->mins,holdmins);
	VectorCopy(self->maxs,holdmaxs);

	AngleVectors (self->s.angles,forward, right,NULL);

	VectorMA (self->s.origin, holdmins[0],forward, point[0]);
	VectorMA (point[0], holdmins[1],right, point[0]);

	VectorMA (self->s.origin, holdmaxs[0],forward, point[1]);
	VectorMA (point[1], holdmaxs[1],right, point[1]);

	VectorMA (self->s.origin, holdmins[0],forward, point[2]);
	VectorMA (point[2], holdmaxs[1],right, point[2]);

	VectorMA (self->s.origin, holdmaxs[0],forward, point[3]);
	VectorMA (point[3], holdmins[1],right, point[3]);

	minpoint[0] = point[0][0];
	minpoint[1] = point[0][1];

	maxpoint[0] = point[0][0];
	maxpoint[1] = point[0][1];

	// Find max and min point
	for (i=0;i<4;++i)
	{
		if (point[i][0] < minpoint[0])
			minpoint[0] = point[i][0];

		if (point[i][1] < minpoint[1])
			minpoint[1] = point[i][1];


		if (point[i][0] > maxpoint[0])
			maxpoint[0] = point[i][0];

		if (point[i][1] > maxpoint[1])
			maxpoint[1] = point[i][1];

	}

	scale = self->s.scale;
	if (scale == 0)
		scale = 1;

	self->mins[0] = (minpoint[0] - self->s.origin[0]) * scale;
	self->mins[1] = (minpoint[1] - self->s.origin[1]) * scale;
	self->mins[2] = self->mins[2] * scale;

	self->maxs[0] = (maxpoint[0] - self->s.origin[0]) * scale;
	self->maxs[1] = (maxpoint[1] - self->s.origin[1]) * scale;
	self->maxs[2] = self->maxs[2] * scale;

}

void ObjectInit(edict_t *self,int health,int mass, int materialtype,int solid)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = solid;
	self->msgHandler = DefaultMsgHandler;
	self->takedamage = DAMAGE_YES;
	self->clipmask = MASK_MONSTERSOLID;

	if (!self->health)
		self->health = health;

	if (!self->mass)
	{
		self->mass = mass;
		if (!mass)	// Needs a mass if it breaks up
			self->mass = 10;
	}

	if (!self->materialtype)
		self->materialtype = materialtype;

	if (self->spawnflags & OBJ_INVULNERABLE)	// Invulnerable
	{
		self->takedamage = DAMAGE_NO;
	}
	else
	{
		self->takedamage = DAMAGE_YES;
	}

	BboxYawAndScale(self);

	if (!(self->spawnflags & OBJ_NOPUSH))	// PUSHABLE
	{
		self->movetype = PHYSICSTYPE_STOP;
		self->monsterinfo.aiflags = AI_NOSTEP;
		self->touch = objpush_touch;

		self->think = M_droptofloor;
		self->nextthink = level.time + (2 * FRAMETIME);
	}
	else
	{
		gi.linkentity (self);
	}
}



//============================================================================
//
//  OBJECTS
//
//============================================================================


/*QUAKED obj_banner (1 .5 0) (-8 -44 -296) (8 44 0) INVULNERABLE ANIMATE EXPLODING NOPUSH
A really big banner.  
-------  FIELDS  ------------------
INVULNERABLE - N/A (it can't ever be hurt)
ANIMATE - makes it flutter in the breeze, just like a...banner
EXPLODING - N/A
NOPUSH - N/A (banner can't be pushed)
-------  KEYS  ------------------
skinnum = 0 : blue
          1 : red
-----------------------------------
*/
void SP_obj_banner (edict_t *self)
{

//	G_FreeEdict(self);
//	return;

	VectorSet(self->mins, -8, -44, -296);
	VectorSet(self->maxs, 8, 44, 0);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	SpawnClientAnim(self, FX_ANIM_BANNER, "ambient/bannerflap.wav");
	ObjectInit(self,40,200,MAT_CLOTH,SOLID_BBOX);
}

/*QUAKED obj_banneronpole (1 .5 0) (-8 -28 -30) (8 28 30) INVULNERABLE ANIMATE EXPLODING NOPUSH
A banner on a pole sticking out of a wall.  
-------  FIELDS  ------------------
INVULNERABLE - can it be hurt
ANIMATE - makes it flutter in the breeze
EXPLODING - N/A
NOPUSH - N/A (banner can't be pushed)
-----------------------------------
*/
void SP_obj_banneronpole (edict_t *self)
{
	VectorSet(self->mins, -8, -28, -30);
	VectorSet(self->maxs, 8, 28, 30);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	SpawnClientAnim(self, FX_ANIM_BANNERONPOLE, "ambient/bannerflap.wav");
	ObjectInit(self,40,200,MAT_WOOD,SOLID_BBOX);
}



/*-----------------------------------------------
	exploding barrel
-----------------------------------------------*/
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
void barrel_explode_think(edict_t *self)
{
	vec3_t loc;

	VectorCopy(self->s.origin, loc);
	AlertMonsters (self, self->owner, 3, false);

	self->fire_damage_time = level.time + 1.0;
	self->svflags |= SVF_ONFIRE;
	BecomeDebris(self);

	T_DamageRadiusFromLoc(loc, self->owner, self->owner, NULL, BARREL_EXPLODE_RADIUS, 
					BARREL_EXPLODE_DMG_MAX, BARREL_EXPLODE_DMG_MIN, 
					DAMAGE_NORMAL|DAMAGE_FIRE|DAMAGE_EXTRA_KNOCKBACK,MOD_BARREL);
	
	// Start the explosion
	gi.CreateEffect(NULL, FX_BARREL_EXPLODE, CEF_BROADCAST, loc, "");

	G_SetToFree(self);
}



int barrel_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->think = barrel_explode_think;
	self->nextthink = level.time + FRAMETIME;
	self->owner = attacker;					// The one to get credit for this should be the one destroying the barrel.

	self->takedamage = DAMAGE_NO;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->blocked = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;

	gi.linkentity(self);

	return 1;
}



/*QUAKED obj_barrel (1 .5 0) (-12 -12 -19) (12 12 19) INVULNERABLE ANIMATE EXPLODING NOPUSH
A barrel.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - the barrel will explode and cause radius damage
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_barrel (edict_t *self)
{

	self->s.modelindex = gi.modelindex("models/objects/barrel/normal/tris.fm");

	VectorSet(self->mins, -12, -12, -19);
	VectorSet(self->maxs, 12, 12, 19);

	self->dmg = 10;

	ObjectInit(self,10,60,MAT_WOOD,SOLID_BBOX);

	if (self->spawnflags & OBJ_EXPLODING)
	{
		// Set this up so we go through die rather than the message.
		self->classID = CID_NONE;
		self->die = barrel_die;
		self->s.skinnum = 1;
	}
}

/*QUAKED obj_broom (1 .5 0) (-2 -2 -25) (2 2 25) INVULNERABLE ANIMATE EXPLODING NOPUSH
A broom.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_broom (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/broom/tris.fm");

	VectorSet(self->mins, -2, -2, -25);
	VectorSet(self->maxs, 2, 2, 25);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,10,40,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_chair1 (1 .5 0) (-12 -8 -26) (12 8 26)  INVULNERABLE ANIMATE EXPLODING NOPUSH  
A highback wooden chair with a triangle at the top.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chair1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/chair1/tris.fm");

	VectorSet(self->mins, -12, -8, -26);
	VectorSet(self->maxs, 12, 8, 26);

	ObjectInit(self,20,50,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_chair2 (1 .5 0) (-18 -29 -30) (18 29 30)  INVULNERABLE ANIMATE EXPLODING NOPUSH 
A thick chair with slanted sides
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chair2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/chair2/tris.fm");

	VectorSet(self->mins, -18, -29, -30);
	VectorSet(self->maxs, 18, 29, 30);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,20,50,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_chair3 (1 .5 0) (-14 -21 -28) (14 21 28)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A big stone throne.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chair3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/chair3/tris.fm");

	VectorSet(self->mins, -14, -21, -28);
	VectorSet(self->maxs, 14, 21, 28);

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,20,50,MAT_GREYSTONE,SOLID_BBOX);
}


void chest1_anim (edict_t *self)
{
	if (self->s.frame < 10)
	{
		++self->s.frame;
		self->think = chest1_anim;
		self->nextthink = level.time + FRAMETIME;
	}
	else
		self->think = NULL;
}

void chest1_use (edict_t *self, edict_t *other, edict_t *activator)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex("objects/chest.wav"), 1, ATTN_NORM, 0);

	chest1_anim(self);
}

/*QUAKED obj_chest1 (1 .5 0) (-10 -18 -19) (10 18 19) INVULNERABLE ANIMATE EXPLODING NOPUSH
A large chest with a snake carving on top. When used it opens its lid.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chest1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chests/chest1/tris.fm");

	VectorSet(self->mins, -10, -18, -19);
	VectorSet(self->maxs, 10, 18, 19);

	ObjectInit(self,60,150,MAT_WOOD,SOLID_BBOX);

	self->use = chest1_use;
}

/*QUAKED obj_chest2 (1 .5 0) (-14 -17 -9) (14 17 9) INVULNERABLE ANIMATE EXPLODING NOPUSH
A medium sized chest with the top open - for use in the mines
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chest2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chests/chest2/tris.fm");

	VectorSet(self->mins, -14, -17, -9);
	VectorSet(self->maxs, 14, 17, 9);

	self->spawnflags &= ~OBJ_NOPUSH;

	ObjectInit(self,60,150,MAT_WOOD,SOLID_BBOX);

}

/*QUAKED obj_chest3 (1 .5 0) (-10 -17 -6) (10 17 6) INVULNERABLE ANIMATE EXPLODING NOPUSH
A medium sized chest with the top closed - for use in the mines
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_chest3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chests/chest3/tris.fm");

	VectorSet(self->mins, -10, -17, -6);
	VectorSet(self->maxs, 10, 17, 6);

	self->spawnflags &= ~OBJ_NOPUSH;

	ObjectInit(self,60,150,MAT_WOOD,SOLID_BBOX);

}

void cog1_anim (edict_t *self)
{

	if (self->s.frame < 11)
		++self->s.frame;
	else
		self->s.frame=1;

	if (self->touch_debounce_time > level.time)	// First time through reach anim
	{
		self->think = cog1_anim;
		self->nextthink = level.time + FRAMETIME * 2;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1, ATTN_NORM, 0);
		self->think = NULL;
	}
}

void cog1_use (edict_t *self, edict_t *other, edict_t *activator)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex("items/cogsturn.wav"), 1, ATTN_NORM, 0);

	self->touch_debounce_time = level.time + (FRAMETIME * 30);

	self->s.frame=1;
	cog1_anim(self);
}

/*QUAKED obj_cog1 (1 .5 0) (-8 -4 0) (8 4 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A cog with spokes coming out the front of it.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (cog can't be pushed)
-----------------------------------
*/
void SP_obj_cog1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/cogs/cog1/tris.fm");

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	VectorSet(self->mins, -8, -4, 0);
	VectorSet(self->maxs, 8, 4, 20);

	ObjectInit(self,40,20,MAT_WOOD,SOLID_BBOX);

	self->use = cog1_use;


}


void SpawnCorpse(edict_t *self)
{
	int chance;

	self->s.modelindex = gi.modelindex("models/monsters/plaguelf/tris.fm");

	if ((self->style > 4) || (self->style < 0))
		self->style = 0;

	if (self->style == 0)
		self->s.frame = FRAME_death13end;
	else if (self->style == 1)
		self->s.frame = FRAME_deathb13end;
	else if (self->style == 2)
		self->s.frame = FRAME_deathc13end;
	else if (self->style == 3)
		self->s.frame = FRAME_deathd13end;
	else if (self->style == 4)
	{
		self->s.frame = FRAME_skewered;
		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
	}

	chance = irand(0, 3);
	if(chance < 1)
	{
		//show the hammer
		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	}
	else if(chance < 2)
	{
		//show the hoe
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	}
	else	
	{
		//show the gaff (that hook thingie)
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	}

	VectorSet(self->mins,-30,-12,-2);
	VectorSet(self->maxs,30,12,2);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->svflags |= SVF_DEADMONSTER;//doesn't block walking

	ObjectInit(self,40,60,MAT_FLESH,SOLID_BBOX);
}

/*QUAKED obj_corpse1 (1 .5 0) (-30 -12 0) (30 12 5) INVULNERABLE ANIMATE EXPLODING NOPUSH
Plague elf dead.
---------- KEYS -----------------  
style - (default 0)
   0 - both arms above head	
   1 - on side	
   2 - arm over face
   3 - arms out to side
   4 - skewered
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (corpse can't be pushed)
-----------------------------------
*/
void SP_obj_corpse1(edict_t *self)
{
	SpawnCorpse(self);

	self->s.skinnum = 0;
}


/*QUAKED obj_corpse2 (1 .5 0) (-30 -12 0) (30 12 5)  INVULNERABLE ANIMATE EXPLODING NOPUSH
Plague elf dead with a different skin
---------- KEYS -----------------  
style - (default 0)
   0 - both arms above head	
   1 - on side	
   2 - arm over face
   3 - arms out to side
   4 - skewered
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (corpse can't be pushed)
-----------------------------------
*/
void SP_obj_corpse2(edict_t *self)
{
	SpawnCorpse(self);

	self->s.skinnum = 1;
}

#define PELF_NUM_PAIN_VOICES	4
#define PELF_NUM_IDLE_VOICES	4
#define PELF_NUM_TOUCH_VOICES	7

static const char *dying_pelf_touch_voices[PELF_NUM_TOUCH_VOICES] =
{
	"voices/helpe.wav",
	"voices/helpk.wav",
	"voices/helpm.wav",
	"voices/getawayb.wav",
	"voices/leavemeb.wav",
	"voices/mercyp.wav",
	"voices/nomrj.wav",
};

static const char *dying_pelf_pain_voices[PELF_NUM_PAIN_VOICES] =
{
	"voices/getawayb.wav",
	"voices/leavemeb.wav",
	"voices/mercyp.wav",
	"voices/nomrj.wav",
};

static const char *dying_pelf_idle_voices[PELF_NUM_IDLE_VOICES] =
{
	"pelfgasp.wav",
	"pelfpant.wav",
	"pelfshiv.wav",
	"pelfsigh.wav",
};

void dying_elf_sounds (edict_t *self, int type)
{
	char	sound_string[1024];

	strcpy(sound_string, "monsters/plagueElf/");

	switch(type)
	{
	case DYING_ELF_PAIN_VOICE:
		strcat(sound_string, dying_pelf_pain_voices[irand(0, PELF_NUM_PAIN_VOICES - 1)]);
		gi.sound (self, CHAN_VOICE, gi.soundindex(sound_string), 1, ATTN_NORM, 0);
		break;
	case DYING_ELF_IDLE_VOICE:
		strcat(sound_string, dying_pelf_idle_voices[irand(0, PELF_NUM_IDLE_VOICES - 1)]);
		gi.sound (self, CHAN_VOICE, gi.soundindex(sound_string), 1, ATTN_IDLE, 0);
		break;
	case DYING_ELF_TOUCH_VOICE:
		strcat(sound_string, dying_pelf_touch_voices[irand(0, PELF_NUM_TOUCH_VOICES - 1)]);
		gi.sound (self, CHAN_VOICE, gi.soundindex(sound_string), 1, ATTN_NORM, 0);
		break;
	}
}

void dying_elf_idle(edict_t *self)
{
	++self->s.frame;
	if (self->s.frame > FRAME_fetal26)
	{
		self->s.frame=FRAME_fetal1;
		self->nextthink = level.time + FRAMETIME;
	}

	if(!irand(0, 50))
		dying_elf_sounds(self, DYING_ELF_IDLE_VOICE);

	self->think = dying_elf_idle;
	self->nextthink = level.time + FRAMETIME;
}

void dying_elf_reach_anim(edict_t *self)
{
	if (self->touch_debounce_time < level.time)	// First time through reach anim
	{
		self->s.frame = FRAME_reach1;
		self->touch_debounce_time = level.time + (FRAMETIME * 60);
	}
	else if (!self->count)	// Reaching
	{
		++self->s.frame;

		self->think = dying_elf_idle;
		self->nextthink = level.time + FRAMETIME;
	}

	if (self->s.frame > FRAME_reach38)	// All done, stay down for a bit
	{
		self->s.frame = FRAME_fetal1;
		self->think = dying_elf_idle;
	}
	else
	{
		self->think = dying_elf_reach_anim;
	}

	self->nextthink = level.time + FRAMETIME;
}

void dying_elf_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->touch_debounce_time < level.time)	// First time through reach anim
	{
		if (irand(1,3) == 1 || self->touch_debounce_time == -1)
		{
			self->enemy = other;
			dying_elf_reach_anim(self);
			if (self->enemy->client || self->enemy->svflags & SVF_MONSTER)
				dying_elf_sounds(self, DYING_ELF_TOUCH_VOICE);
		}
		else
			self->touch_debounce_time = level.time + (FRAMETIME * 20);
	}
}

int dying_elf_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->enemy = other;
	dying_elf_reach_anim(self);
	if (self->enemy->client || self->enemy->svflags & SVF_MONSTER)
		dying_elf_sounds(self, DYING_ELF_PAIN_VOICE);

	return true;
}

int dying_elf_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		chance;

	chance = irand(0, 2);
	switch(chance)
	{
	case 0:
		gi.sound(self, CHAN_VOICE, gi.soundindex("monsters/plagueElf/death1.wav"), 1, ATTN_NORM, 0);
		break;

	case 1:
		gi.sound(self, CHAN_VOICE, gi.soundindex("monsters/plagueElf/death2.wav"), 1, ATTN_NORM, 0);
		break;

	case 2:
		gi.sound(self, CHAN_VOICE, gi.soundindex("monsters/plagueElf/death3.wav"), 1, ATTN_NORM, 0);
		break;
	}

	BecomeDebris(self);
	return (false);
}

/*QUAKED obj_dying_elf (1 .5 0) (-30 -12 0) (30 12 5) INVULNERABLE ANIMATE EXPLODING NOPUSH
Plague elf lying on the ground shaking.
---------- KEYS -----------------  
style - skin to use
0 - 
1 - 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (corpse can't be pushed)
-----------------------------------
*/
void SP_obj_dying_elf(edict_t *self)
{

	VectorSet(self->mins,-32,-32,-2);
	VectorSet(self->maxs,32,32,6);

	self->s.modelindex = gi.modelindex("models/monsters/plaguelf/tris.fm");


	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->movetype = PHYSICSTYPE_STEP;

	self->touch_debounce_time = -1;
	self->touch = dying_elf_touch;
	self->pain = dying_elf_pain;
	self->die = dying_elf_die;

	ObjectInit(self, 40, 60, MAT_FLESH, SOLID_BBOX);

	// No weapons
	self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

	self->s.frame = FRAME_fetal1;
	self->think = dying_elf_idle;
	self->nextthink = level.time + FRAMETIME;
}


/*QUAKED obj_sign1 (1 .5 0) (-29 -4 -16) (29 4 16)   INVULNERABLE ANIMATE EXPLODING NOPUSH
A square sign coming out of a wall.
--------KEYS-------------------
style -
0 - sign with a dragon
1 - sign with two steins
2 - sign with a fish
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (sign1 can't be pushed)
-----------------------------------
*/
void SP_obj_sign1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/signs/sign1/tris.fm");

	VectorSet(self->mins, -29, -4,-16);
	VectorSet(self->maxs, 29,  4, 16);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,40,150,MAT_WOOD,SOLID_BBOX);

	if (self->style==0)
		self->s.skinnum = 0;
	else if (self->style==1)
		self->s.skinnum = 1;
	else if (self->style==2)
		self->s.skinnum = 2;
	else
		self->s.skinnum = 0;
	
	self->s.frame = 3;

}


/*QUAKED obj_sign4 (1 .5 0) (-8 -18 -29) (8 18 29)   INVULNERABLE ANIMATE EXPLODING NOPUSH
A square sign that is on top of a post. It is leaning badly.
For the MINE or DESERT areas.
style - 
0 for the Andorian skin
1 for the Tchecktrik skin

-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (sign4 can't be pushed)
-----------------------------------
*/
void SP_obj_sign4 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/signs/sign4/tris.fm");

	VectorSet(self->mins, -8, -18, -29);
	VectorSet(self->maxs,  8,  18,  29);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	if (self->style==0)
		self->s.skinnum = 0;
	else if (self->style==1)
		self->s.skinnum = 1;


	ObjectInit(self,40,150,MAT_WOOD,SOLID_BBOX);
}

/*QUAK-ED obj_stalagmite1 (1 .5 0) (-32 -32 -200) (32 32 0) INVULNERABLE ANIMATE EXPLODING DARKSKIN
A big long thick stalagmite.These point up.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
DARKSKIN - if checked it uses the dark skin
-----------------------------------
*/
void SP_obj_stalagmite1(edict_t *self) 
{
	self->s.modelindex=gi.modelindex("models/objects/stalagmite/smite1/tris.fm");
	
	VectorSet(self->mins,-32,-32,-200);
	VectorSet(self->maxs,32,32,0);
	
	if (self->spawnflags & 8)
		self->s.skinnum = 1;

	ObjectInit(self,200,300,MAT_BROWNSTONE,SOLID_BBOX);
	
}


/*QUAK-ED obj_stalagmite2 (1 .5 0) (-32 -32 -128) (32 32 0) DARKSKIN
A big squat stalagmite.These point up.	
-------  FIELDS  ------------------
DARKSKIN - if checked it uses the dark skin
-----------------------------------
*/
void SP_obj_stalagmite2(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/stalagmite/smite2/tris.fm");
	
	VectorSet(self->mins,-32,-32,-128);
	VectorSet(self->maxs,32,32,0);
		
	if (self->spawnflags & 1)
		self->s.skinnum = 1;

	ObjectInit(self,200,300,MAT_BROWNSTONE,SOLID_BBOX);
}

/*QUAK-ED obj_stalagmite3 (1 .5 0) (-32 -32 -200) (32 32 0)  DARKSKIN
A long pointy stalagmite. These point up
-------  FIELDS  ------------------
DARKSKIN - if checked it uses the dark skin
-----------------------------------
-*/
void SP_obj_stalagmite3(edict_t *self)
{	
	self->s.modelindex=gi.modelindex("models/objects/stalagmite/smite3/tris.fm");

	VectorSet(self->mins,-16,-16,-200);
	VectorSet(self->maxs,16,16,0);
		
	if (self->spawnflags & 1)
		self->s.skinnum = 1;

	ObjectInit(self,200,200,MAT_BROWNSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_corvus (1 .5 0) (-16 -16 0) (16 16 32) INVULNERABLE ANIMATE EXPLODING NOPUSH
A statue of Corvus.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_statue_corvus (edict_t *self) 
{
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 128);

	self->s.modelindex = gi.modelindex("models/objects/statue/corvus/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_dolphin1 (1 .5 0) (-68 -22 -30) (68 22 30) VULNERABLE ANIMATE EXPLODING NOPUSH
The dolphin on all fours
-------  FIELDS  ------------------
VULNERABLE - it can be hurt - default it can't be
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dolphin1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dolphin/tris.fm");

	VectorSet(self->mins, -68, -22, -30);
	VectorSet(self->maxs, 68, 22, 30);

	self->spawnflags |= OBJ_NOPUSH;
	if (self->spawnflags & OBJ_INVULNERABLE)
		self->spawnflags &= ~OBJ_INVULNERABLE; // can be destroyed
	else
		self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,300,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_dolphin2 (1 .5 0) (-17 -20 -70) (17 20 70) INVULNERABLE ANIMATE EXPLODING NOPUSH
The dolphin on a wall. Head turned to the right
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dolphin2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dolphin/tris.fm");

	VectorSet(self->mins, -17, -20, -70);
	VectorSet(self->maxs, 17, 20, 70);

	self->s.frame = 1;

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,300,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_dolphin3 (1 .5 0) (-17 -20 -70) (17 20 70) INVULNERABLE ANIMATE EXPLODING NOPUSH
The dolphin on a wall. Head turned to the left
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dolphin3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dolphin/tris.fm");

	VectorSet(self->mins, -17, -20, -70);
	VectorSet(self->maxs, 17, 20, 70);

	self->s.frame = 3;

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,300,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_dolphin4 (1 .5 0) (-63 -22 -37) (63 22 37) INVULNERABLE ANIMATE EXPLODING NOPUSH
The dolphin up on two legs
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dolphin4 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dolphin/tris.fm");

	VectorSet(self->mins, -63, -22, -37);
	VectorSet(self->maxs, 63, 22, 37);
	self->s.frame = 2;

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,300,200,MAT_GREYSTONE,SOLID_BBOX);
}


/*QUAKED obj_statue_guardian (1 .5 0) (-100 -64 0) (64 64 128) INVULNERABLE ANIMATE EXPLODING NOPUSH
A big statue of a fish guy on his haunches holding a spear.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_guardian (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/guardian/tris.fm");

	VectorSet(self->mins, -100, -64, 0);
	VectorSet(self->maxs, 64, 64, 128);

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,400,300,MAT_METAL,SOLID_BBOX);
}

/*QUAKED obj_table1 (1 .5 0) (-28 -54 -18) (28 54 18) INVULNERABLE ANIMATE EXPLODING NOPUSH
A large wooden dining table with two legs.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_table1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/tables/table1/tris.fm");

	VectorSet(self->mins, -28, -54, -18);
	VectorSet(self->maxs, 28, 54, 18);

	ObjectInit(self,40,100,MAT_WOOD,SOLID_BBOX);

}

/*QUAKED obj_table2 (1 .5 0) (-28 -54 -17) (28 54 17) INVULNERABLE ANIMATE EXPLODING NOPUSH
A grey stone table.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_table2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/tables/table2/tris.fm");

	VectorSet(self->mins, -28, -54, -17);
	VectorSet(self->maxs, 28, 54, 17);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,80,150,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_throne (1 .5 0) (-20 -22 -44) (20 22 44) INVULNERABLE ANIMATE EXPLODING NOPUSH
A highbacked throne.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_throne (edict_t *self)
{
	VectorSet(self->mins, -20, -22, -44);
	VectorSet(self->maxs, 20,22, 44);

	self->s.modelindex = gi.modelindex("models/objects/chairs/throne/tris.fm");

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,150,200,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_kettle (1 .5 0) (-8 -8 0) (8 8 10) INVULNERABLE ANIMATE EXPLODING NOPUSH
A kettle.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_kettle (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/kettle/tris.fm");

	VectorSet(self->mins, -8, -8, 0);
	VectorSet(self->maxs, 8, 8, 10);

	ObjectInit(self,40,100,MAT_METAL,SOLID_BBOX);
}

/*QUAKED obj_cauldron (1 .5 0) (-22 -22 -10) (22 22 10) INVULNERABLE ANIMATE EXPLODING NOPUSH
A metal cauldron filled with green liquid
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_cauldron (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/caldrn/tris.fm");

	VectorSet(self->mins, -22, -22, -10);
	VectorSet(self->maxs, 22, 22, 10);

	if (self->spawnflags & OBJ_ANIMATE)	// Animate it
	{
		self->s.sound = gi.soundindex("ambient/cauldronbubble.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,60,100,MAT_METAL,SOLID_BBOX);
}


/*QUAKED obj_firepot (1 .5 0) (-18 -18 -12) (18 18 12) INVULNERABLE ANIMATE EXPLODING NOPUSH
A grey stone firepot
-------  FIELDS  ------------------
INVULNERABLE - always invulnerable
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't ever be moved by player)
-----------------------------------
*/
void SP_obj_firepot (edict_t *self)
{
	vec3_t holdorigin;

	self->s.modelindex = gi.modelindex("models/objects/pots/firepot/tris.fm");
	self->s.sound = gi.soundindex("ambient/fireplace.wav");
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;

	VectorSet(self->mins, -18, -18, -12);
	VectorSet(self->maxs, 18, 18, 12);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Cant' be moved

	ObjectInit(self,140,100,MAT_GREYSTONE,SOLID_BBOX);

	if (self->spawnflags & OBJ_ANIMATE)	// Animate it
	{
		VectorCopy(self->s.origin,holdorigin);
		holdorigin[2] += 30;
		SpawnFlame(self,holdorigin);		
	}
}

/*QUAKED obj_statue_duckbill1 (1 .5 0) (-67 -24 -51) (67 24 51) INVULNERABLE ANIMATE EXPLODING NOPUSH
The duckbilled thing - tail to the right
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_duckbill1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/duckbill/tris.fm");

	VectorSet(self->mins, -67, -24, -51);
	VectorSet(self->maxs, 67, 24, 51);

	self->s.frame = 0;

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible

	ObjectInit(self,150,100,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_duckbill2 (1 .5 0) (-67 -24 -50) (67 24 50) INVULNERABLE ANIMATE EXPLODING NOPUSH
The duckbilled thing - tail to the left
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_duckbill2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/duckbill/tris.fm");

	VectorSet(self->mins, -67, -24, -50);
	VectorSet(self->maxs, 67, 24, 50);

	self->s.frame = 1;

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible

	ObjectInit(self,150,100,MAT_GREYSTONE,SOLID_BBOX);
}


void globebottom_turn (edict_t *self)
{
	float current, ideal, move;

	M_ChangeYaw(self);
	
	current = anglemod(self->s.angles[YAW]);
	ideal = self->ideal_yaw;
	move = ideal - current;

	if (current == ideal || abs(move) < 0.1)
	{
		self->s.angles[YAW] = ceil(ideal);
	
		if (self->s.angles[YAW] == 225 && self->enemy->s.angles[YAW] == 45)
		{
			G_UseTargets(self, self);
		}
		self->monsterinfo.idle_time = 0;
		self->think = NULL;
	}
	else
	{
		self->think = globebottom_turn;
		self->nextthink = level.time + FRAMETIME;
	}
}

void globetop_turn (edict_t *self)
{
	float current, ideal, move;

	M_ChangeYaw(self);
	
	current = anglemod(self->s.angles[YAW]);
	ideal = self->ideal_yaw;
	move = ideal - current;

	if (current == ideal || abs(move) < 0.1)
	{
		self->s.angles[YAW] = ceil(ideal);

		if (self->s.angles[YAW] == 270)	// Because they want it to swing all the way around
		{
			self->ideal_yaw = 45;
			self->think = globetop_turn;
			self->nextthink = level.time + FRAMETIME;
		}
		else
		{
			if (self->s.angles[YAW] == 45 && self->enemy->s.angles[YAW] == 225)
			{
				G_UseTargets(self, self);
			}
			self->monsterinfo.idle_time = 0;
			self->think = NULL;
		}
	}
	else
	{
		self->think = globetop_turn;
		self->nextthink = level.time + FRAMETIME;
	}
}

void globetop_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->monsterinfo.idle_time)
		return;

	gi.sound (self, CHAN_BODY, gi.soundindex ("objects/globetop.wav"), 1, ATTN_NORM, 0);

	self->monsterinfo.idle_time = 1;

	self->ideal_yaw = 270;

	self->think = globetop_turn;
	self->nextthink = level.time + FRAMETIME;
}

void globebottom_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->monsterinfo.idle_time)
		return;

	gi.sound (self, CHAN_BODY, gi.soundindex ("objects/globebottomstart.wav"), 1, ATTN_NORM, 0);

	self->monsterinfo.idle_time = 1;

	self->ideal_yaw = 225;

	self->think = globebottom_turn;
	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED obj_seasonglobe (1 .5 0) (-80 -80 0) (80 80 100) INVULNERABLE ANIMATE EXPLODING NOPUSH
The globe thingy
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (globe can't be moved)
-----------------------------------
*/
void SP_obj_seasonglobe (edict_t *bottom)
{
	edict_t *top;

	bottom->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	bottom->movetype = PHYSICSTYPE_NONE;
	bottom->solid = SOLID_BBOX;
	VectorSet(bottom->mins, -80, -80, 0);
	VectorSet(bottom->maxs, 80, 80, 320);
	bottom->s.modelindex = gi.modelindex("models/objects/globe/globebottom/tris.fm");
	bottom->s.frame = 1;
	bottom->s.scale = 1.75;
	BboxYawAndScale(bottom);
	bottom->targetname = "globebottom";
	bottom->use = globebottom_use;
	bottom->yaw_speed = 2.5;
	VectorSet(bottom->s.angles, 0, 90, 0);
	gi.linkentity (bottom);

	top = G_Spawn();

	VectorCopy(bottom->s.origin,top->s.origin);
	top->s.origin[2] += 36;
	top->movetype = PHYSICSTYPE_NONE;
	top->solid = SOLID_BBOX;
	VectorSet(top->mins, -80, -80, 0);
	VectorSet(top->maxs, 80, 80, 100);
	top->s.modelindex = gi.modelindex("models/objects/globe/globetop/tris.fm");
	top->s.frame = 1;
	top->s.scale = 1.75;
	BboxYawAndScale(top);

	bottom->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	top->targetname = "globetop";
	top->use = globetop_use;
	top->yaw_speed = 2.5;
	top->target = bottom->target;
	VectorSet(top->s.angles, 0, 120, 0);
	gi.linkentity (top);

	bottom->enemy = top;
	top->enemy = bottom;
}



/*QUAKED obj_stein (1 .5 0) (-2 -2 -3) (2 2 3)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A beer stein.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_stein (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/stein/tris.fm");

	VectorSet(self->mins, -2, -2, -3);
	VectorSet(self->maxs, 2, 2, 3);

	ObjectInit(self,15,10,MAT_METAL,SOLID_BBOX);
}

/*QUAKED obj_scroll (1 .5 0) (-2 -18 -3) (2 18 3)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A paper scroll
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_scroll (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/scroll/tris.fm");

	VectorSet(self->mins, -2, -18, -3);
	VectorSet(self->maxs, 2, 18, 3);

	ObjectInit(self,10,50,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_fountain_fish (1 .5 0) (-52 -34 -48) (52 34 48) INVULNERABLE ANIMATE EXPLODING NOPUSH 
A two headed fish fountain
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_fountain_fish (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/fountainfish/tris.fm");

	VectorSet(self->mins, -52, -34, -48);
	VectorSet(self->maxs, 52, 34, 48);

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_statue_boulderfish (1 .5 0) (-26 -16 -27) (26 16 27) INVULNERABLE ANIMATE EXPLODING NOPUSH
A statue of a fish. The one which raises up a boulder.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_boulderfish (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/boulderfish/tris.fm");

	VectorSet(self->mins, -26, -16, -27);
	VectorSet(self->maxs, 26, 16, 27);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,200,150,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_pottedplant (1 .5 0) (-20 -20 -30) (20 20 30)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A potted plant with ferns
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pottedplant (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/plant/tris.fm");

	VectorSet(self->mins, -20, -20, -30);
	VectorSet(self->maxs, 20, 20, 30);

	ObjectInit(self,20,50,MAT_POTTERY,SOLID_BBOX);
}

/*QUAKED obj_plant1 (1 .5 0) (-8 -8 -24) (8 8 24) INVULNERABLE ANIMATE EXPLODING NOPUSH
A clump of tall, thin, plants
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (plant1 can't be moved)
-----------------------------------
*/
void SP_obj_plant1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/plant1/tris.fm");

	// The bbox in QUAKED comment is bigger than code bbox to show designers how big model is
	VectorSet(self->mins, -8, -8, -24);
	VectorSet(self->maxs, 8, 8, 24);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.effects|=EF_CAMERA_NO_CLIP;

	ObjectInit(self,20,50,MAT_LEAF,SOLID_NOT);

}

/*QUAKED obj_plant2 (1 .5 0) (-20 -20 -10) (20 20 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A plant with broad leaves.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (plant2 can't be moved)
-----------------------------------
*/
void SP_obj_plant2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/plant2/tris.fm");

	// The bbox in QUAKED comment is bigger than code bbox to show designers how big model is
	VectorSet(self->mins, -20, -20, -10);
	VectorSet(self->maxs, 20, 20, 20);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.effects|=EF_CAMERA_NO_CLIP;

	ObjectInit(self,20,50,MAT_LEAF,SOLID_NOT);

}

/*QUAKED obj_plant3 (1 .5 0) (-8 -8 -12) (8 8 12) INVULNERABLE ANIMATE EXPLODING NOPUSH
A group of ferns
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (plant3 can't be moved)
-----------------------------------
*/
void SP_obj_plant3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/plant3/tris.fm");

	VectorSet(self->mins, -8, -8, -12);
	VectorSet(self->maxs, 8, 8, 12);

	if (self->style==0)
		self->s.skinnum = 0;
	else if (self->style==1)
		self->s.skinnum = 1;
	else if (self->style==2)
		self->s.skinnum = 2;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.effects|=EF_CAMERA_NO_CLIP;

	ObjectInit(self,20,50,MAT_LEAF,SOLID_NOT);

}

/*QUAKED obj_treetop (1 .5 0) (-176 -176 -125) (176 176 125) INVULNERABLE ANIMATE EXPLODING NOPUSH
A canopy for a tree. 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (treetop can't be moved)
-----------------------------------
*/
void SP_obj_treetop (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/treetop/tris.fm");

	VectorSet(self->mins, -176, -176, -125);
	VectorSet(self->maxs,  176,  176,  125);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_NOT);
}

/*QUAKED obj_tree (1 .5 0) (-100 -100 -120) (100 100 120) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tree for Matt's level
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (tree can't be moved)
-----------------------------------
*/
void SP_obj_tree (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/tree/tris.fm");

	VectorSet(self->mins, -100, -100, -120);
	VectorSet(self->maxs, 100, 100, 120);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_tree2 (1 .5 0) (-50 -50 -286) (50 50 286) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tall spikey tree for the swamps
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (tree2 can't be moved)
-----------------------------------
*/
void SP_obj_tree2 (edict_t *self)
{
	edict_t *moss;

	self->s.modelindex = gi.modelindex("models/objects/plants/tree2/trunk2/tris.fm");

	VectorSet(self->mins, -50, -50, -286);
	VectorSet(self->maxs, 50, 50, 286);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);


	moss = G_Spawn();
	VectorCopy(self->s.origin,moss->s.origin);
	VectorCopy(self->s.angles,moss->s.angles);
	moss->s.modelindex = gi.modelindex("models/objects/plants/tree2/moss2/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.scale = self->s.scale;
	moss->s.renderfx |= RF_TRANSLUCENT;


	BboxYawAndScale(moss);
	gi.linkentity (moss);

	self->target_ent = moss;
}

/*QUAKED obj_tree3 (1 .5 0) (-50 -50 -286) (50 50 286) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tall spikey tree with big roots on the bottom.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (tree3 can't be moved)
-----------------------------------
*/
void SP_obj_tree3 (edict_t *self)
{
	edict_t *moss;

	self->s.modelindex = gi.modelindex("models/objects/plants/tree3/trunk3/tris.fm");

	VectorSet(self->mins, -50, -50, -286);
	VectorSet(self->maxs, 50, 50, 286);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);

	moss = G_Spawn();
	VectorCopy(self->s.origin,moss->s.origin);
	VectorCopy(self->s.angles,moss->s.angles);
	moss->s.modelindex = gi.modelindex("models/objects/plants/tree3/moss3/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.scale = self->s.scale;
	moss->s.renderfx |= RF_TRANSLUCENT;

	BboxYawAndScale(moss);
	gi.linkentity (moss);

	self->target_ent = moss;
}

/*QUAKED obj_treetall (1 .5 0) (-46 -46 -340) (46 46 340) INVULNERABLE ANIMATE EXPLODING NOPUSH
A very tall tree
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (tree3 can't be moved)
-----------------------------------
*/
void SP_obj_treetall (edict_t *self)
{
	edict_t *moss;

	self->s.modelindex = gi.modelindex("models/objects/plants/talltree/trunk1/tris.fm");

	VectorSet(self->mins, -46, -46, -340);
	VectorSet(self->maxs, 46, 46, 340);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);

	moss = G_Spawn();
	VectorCopy(self->s.origin,moss->s.origin);
	VectorCopy(self->s.angles,moss->s.angles);
	moss->s.modelindex = gi.modelindex("models/objects/plants/talltree/moss1/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.renderfx |= RF_TRANSLUCENT;
	moss->s.scale = self->s.scale;
	BboxYawAndScale(moss);
	gi.linkentity (moss);

	self->target_ent = moss;
}

/*QUAKED obj_treefallen (1 .5 0) (-24 -62 -35) (24 62 35) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tree that is leaning as if it had over. Meant to be partially submerged in water or muck.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (treefallen can't be moved)
-----------------------------------
*/
void SP_obj_treefallen (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/cactus/tris.fm");

	VectorSet(self->mins, -24, -62, -35);
	VectorSet(self->maxs, 24, 62, 35);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,40,50,MAT_WOOD,SOLID_BBOX);

	self->s.frame = 1;
}


/*QUAKED obj_shovel (1 .5 0) (-8 -8 -20) (8 8 20)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A shovel
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_shovel (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/shovel/tris.fm");

	VectorSet(self->mins, -8, -8, -20);
	VectorSet(self->maxs, 8, 8, 20);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,20,40,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_woodpile (1 .5 0) (-12 -20 -7) (12 20 7) INVULNERABLE ANIMATE EXPLODING NOPUSH
A pile of chopped  wood.
-------  FIELDS  ------------------
INVULNERABLE - Can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_woodpile (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/wood/tris.fm");

	self->spawnflags |= OBJ_NOPUSH;	// Can't move

	VectorSet(self->mins, -12, -20, -7);
	VectorSet(self->maxs, 12, 20, 7);
	self->s.scale = 2;

	ObjectInit(self,100,150,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_fishtrap (1 .5 0) (-14 -28 -13) (14 28 13) INVULNERABLE ANIMATE EXPLODING NOPUSH
A fishtrap
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_fishtrap (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/fishtrap/tris.fm");

	VectorSet(self->mins, -14, -28, -13);
	VectorSet(self->maxs, 14, 28, 13);

	ObjectInit(self,30,100,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_bench (1 .5 0) (-10 -21 -10) (10 21 10)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A stone bench to sit on
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_bench (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/bench/tris.fm");

	VectorSet(self->mins, -10, -22, -10);
	VectorSet(self->maxs, 10, 22, 10);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,3,4,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_bucket (1 .5 0) (-8 -8 -9) (8 8 10) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bucket
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_bucket (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/bucket/tris.fm");

	VectorSet(self->mins, -8, -8, -9);
	VectorSet(self->maxs, 8, 8, 10);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,3,4,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_ropechain (1 .5 0) (-20 -20 -14) (20 20 14) INVULNERABLE ANIMATE EXPLODING NOPUSH
A pile of rope or chain
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (ropechain can't be moved)

Variables:
skinnum = 0 : rope
          1 : chain
-----------------------------------
*/
void SP_obj_ropechain (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/rope/tris.fm");

	VectorSet(self->mins, -20, -20, -14);
	VectorSet(self->maxs, 20, 20, 14);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	if (self->s.skinnum == 0)
		ObjectInit(self,80,100,MAT_WOOD,SOLID_BBOX);
	else
		ObjectInit(self,160,100,MAT_METAL,SOLID_BBOX);
}


/*QUAKED obj_wheelbarrow (1 .5 0) (-37 -20 -21) (37 20 21) INVULNERABLE ANIMATE EXPLODING NOPUSH
A wheelbarrow  
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_wheelbarrow (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/wheelbarrow/tris.fm");

	VectorSet(self->mins, -37, -20, -21);
	VectorSet(self->maxs, 37, 20, 21);

	ObjectInit(self,60,100,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_wheelbarrowdamaged (1 .5 0) (-38 -26 -20) (38 26 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A wheelbarrow  on it's side missing a wheel
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (wheelbarrowdamaged can't be moved)
-----------------------------------
*/
void SP_obj_wheelbarrowdamaged (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/wheelbarrow/tris.fm");

	VectorSet(self->mins, -38, -26, -20);
	VectorSet(self->maxs,  38,  26,  20);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->s.frame = 1;

	ObjectInit(self,60,100,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_urn (1 .5 0) (-8 -8 -27) (8 8 30) INVULNERABLE ANIMATE EXPLODING NOPUSH
An urn 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_urn (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/urn/tris.fm");

	VectorSet(self->mins, -8, -8, -27);
	VectorSet(self->maxs, 8, 8, 30);

	ObjectInit(self,50,100,MAT_POTTERY,SOLID_BBOX);
}



/*QUAKED obj_bigcrystal (1 .5 0) (-35 -35 -50) (35 35 50) INVULNERABLE ANIMATE EXPLODING NOPUSH
A big circular crystal which rotates.
-------  KEYS  ------------------
speed - rate of rotation
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_bigcrystal(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/crystals/bigcrystal/tris.fm");

	VectorSet(self->mins,-35,-35,-50);
	VectorSet(self->maxs,35,35,50);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	ObjectInit(self,350,200,MAT_GREYSTONE,SOLID_BBOX);

	self->avelocity[1] = self->speed;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0;
}


/*QUAKED obj_moss1 (1 .5 0) (-4 -10 -40) (4 10 40) INVULNERABLE ANIMATE EXPLODING NOPUSH

-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (moss1 can't be moved)
-----------------------------------
*/
void SP_obj_moss1(edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/moss/tris.fm");

	VectorSet(self->mins, -1, -10, -40);
	VectorSet(self->maxs, 1, 10, 40);
	self->s.skinnum = 0;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->s.renderfx |= RF_TRANSLUCENT;
	ObjectInit(self,10,10,MAT_LEAF,SOLID_NOT);
}

/*QUAKED obj_moss2 (1 .5 0) (-4 -9 -40) (4 9 40)  INVULNERABLE ANIMATE EXPLODING NOPUSH
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (moss2 can't be moved)
-----------------------------------
*/
void SP_obj_moss2(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/moss/tris.fm");

	VectorSet(self->mins,-1,-9,-40);
	VectorSet(self->maxs,1,9,40);
	self->s.skinnum = 1;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->s.renderfx |= RF_TRANSLUCENT;
	ObjectInit(self,10,10,MAT_LEAF,SOLID_NOT);
}

/*QUAKED obj_moss3 (1 .5 0) (-4 -15 -40) (4 15 40) INVULNERABLE ANIMATE EXPLODING NOPUSH
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (moss3 can't be moved)
-----------------------------------
*/
void SP_obj_moss3(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/moss/tris.fm");

	VectorSet(self->mins,-1,-15,-40);
	VectorSet(self->maxs,1,15,40);
	self->s.skinnum = 2;
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->s.renderfx |= RF_TRANSLUCENT;

	ObjectInit(self,10,10,MAT_LEAF,SOLID_NOT);
}

/*QUAKED obj_moss4 (1 .5 0) (-4 -12 -40) (4 12 40)  INVULNERABLE ANIMATE EXPLODING NOPUSH
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (moss4 can't be moved)
-----------------------------------
*/
void SP_obj_moss4(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/moss/tris.fm");

	VectorSet(self->mins,-1,-12,-40);
	VectorSet(self->maxs,1,12,40);
	self->s.skinnum = 3;
	self->s.renderfx |= RF_TRANSLUCENT;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	ObjectInit(self,10,10,MAT_LEAF,SOLID_NOT);
}

/*QUAKED obj_moss5 (1 .5 0) (-4 -10 -40) (4 10 40) INVULNERABLE ANIMATE EXPLODING NOPUSH
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (moss5 can't be moved)
-----------------------------------
*/
void SP_obj_moss5(edict_t *self)
{
	self->s.modelindex=gi.modelindex("models/objects/moss/tris.fm");

	VectorSet(self->mins,-1,-10,-40);
	VectorSet(self->maxs,1,10,40);
	self->s.skinnum = 4;
	self->s.renderfx |= RF_TRANSLUCENT;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	ObjectInit(self,10,10,MAT_LEAF,SOLID_NOT);
}

/*QUAKED obj_floor_candelabrum (1 .5 0) (-8 -8 -35) (8 8 35) INVULNERABLE ANIMATE EXPLODING NOPUSH
A floor candelabrum.
-------  FIELDS  ------------------
INVULNERABLE - it can't be destroyed
ANIMATE - makes flickering flames appear
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_floor_candelabrum (edict_t *self)
{
	VectorSet(self->mins, -8, -8, -35);
	VectorSet(self->maxs, 8, 8, 35);

	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	SpawnClientAnim(self, FX_ANIM_CANDELABRUM, NULL);
	ObjectInit(self,40,60,MAT_METAL,SOLID_BBOX);
}

/*QUAKED obj_statue_dragonhead (1 .5 0) (-76 -28 -46) (76 28 46) INVULNERABLE ANIMATE EXPLODING NOPUSH 
A statue of a dragon head
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dragonhead (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dragonhead/tris.fm");

	VectorSet(self->mins, -76, -28, -46);
	VectorSet(self->maxs, 76, 28, 46);

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,200,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_statue_dragon (1 .5 0) (-53 -33 -72) (53 33 72)  INVULNERABLE  ANIMATE  EXPLODING  NOPUSH
A statue of a dragon
---------- KEYS -----------------  
style - (default 0)
   0 - dragon looking left	
   1 - dragon looking right
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_statue_dragon (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/dragon/tris.fm");

	VectorSet(self->mins, -53, -33, -72);
	VectorSet(self->maxs, 53, 33, 72);

	self->spawnflags |= OBJ_NOPUSH;
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,200,400,MAT_GREYSTONE,SOLID_BBOX);

	if (self->style == 0)
		self->s.frame = 0;
	else if (self->style == 1)
		self->s.frame = 1;
	else 
		self->s.frame = 0;

}

/*QUAKED obj_flagonpole (1 .5 0) (-8 -8 0) (8 8 60)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH 
A flag on a pole
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (flagonpole can't be moved)
-----------------------------------
*/
void SP_obj_flagonpole (edict_t *self)
{
	VectorSet(self->mins, -8, -28, -30);
	VectorSet(self->maxs, 8, 28, 30);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	SpawnClientAnim(self, FX_ANIM_FLAGONPOLE, "ambient/bannerflap.wav");
	ObjectInit(self,40,200,MAT_WOOD,SOLID_BBOX);
}

void lever1downthink(edict_t *self)
{
	if (self->s.frame < 5)
	{
		++self->s.frame;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever1upthink(edict_t *self)
{
	if (self->s.frame > 0)
	{
		--self->s.frame;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever1_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->s.frame)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever1.wav"), 1, ATTN_NORM, 0);
		self->think = lever1downthink;
		self->nextthink = level.time + FRAMETIME;
	}
	else if (self->s.frame == 5)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever1.wav"), 1, ATTN_NORM, 0);
		self->think = lever1upthink;
		self->nextthink = level.time + FRAMETIME;
	}
	G_UseTargets(self, activator);

}

/*QUAKED obj_lever1 (1 .5 0) (-6 -14 -17) (6 14 17)  INVULNERABLE  ANIMATE   EXPLODING   NOPUSH
A wooden floor lever that is triggerable.
-------  FIELDS  ------------------
INVULNERABLE - always invulnerable
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_lever1 can't be moved)
-----------------------------------
*/
void SP_obj_lever1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/levers/lever1/tris.fm");

	VectorSet(self->mins, -6, -14, -17);
	VectorSet(self->maxs, 6, 14, 17);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,150,125,MAT_WOOD,SOLID_BBOX);

	self->use = lever1_use;
}



void lever2downthink(edict_t *self)
{
	if (self->s.frame < 4)
	{
		++self->s.frame;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever2upthink(edict_t *self)
{
	if (self->s.frame > 0)
	{
		--self->s.frame;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever2_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->s.frame)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever2.wav"), 1, ATTN_NORM, 0);
		self->think = lever2downthink;
	}
	else if (self->s.frame == 4)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever2.wav"), 1, ATTN_NORM, 0);
		self->think = lever2upthink;
	}
 	self->nextthink = level.time + FRAMETIME;
	G_UseTargets(self, activator);
}

/*QUAKED obj_lever2 (1 .5 0) (-14 -14 -9) (14 14 9)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH 
A wooden wheel lever that is triggerable.
-------  FIELDS  ------------------
INVULNERABLE - always invulnerable
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_lever2 can't be moved)
-----------------------------------
*/
void SP_obj_lever2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/levers/lever2/tris.fm");

	VectorSet(self->mins, -14, -14, -9);
	VectorSet(self->maxs, 14, 14, 9);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,150,125,MAT_WOOD,SOLID_BBOX);

	self->use = lever2_use;
}

void lever3downthink(edict_t *self)
{
	if (self->s.frame < 5)
	{
		++self->s.frame;
	 	self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever3upthink(edict_t *self)
{
	if (self->s.frame > 0)
	{
		--self->s.frame;
 		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void lever3_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->s.frame)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever3.wav"), 1, ATTN_NORM, 0);
		self->think = lever3downthink;
	}
	else if (self->s.frame == 5)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever3.wav"), 1, ATTN_NORM, 0);
		self->think = lever3upthink;
	}
 	self->nextthink = level.time + FRAMETIME;
	G_UseTargets(self, activator);
}

/*QUAKED obj_lever3 (1 .5 0) (-4 -6 -16) (4 6 16)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A lever for the wall...and it's triggerable.
-------  FIELDS  ------------------
INVULNERABLE - always invulnerable
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_lever3 can't be moved)
-----------------------------------
*/
void SP_obj_lever3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/levers/lever3/tris.fm");

	VectorSet(self->mins, -6, -4, -16);
	VectorSet(self->maxs, 6, 4, 16);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,150,125,MAT_WOOD,SOLID_BBOX);

	self->use = lever3_use;
}
void lever_used(edict_t *self, G_Message_t *msg)
{
	self->activator = self->enemy;
//	button_fire (self);
}

void LeverStaticsInit()
{
	classStatics[CID_LEVER].msgReceivers[MSG_DEATH] = lever_used;
}





void bush_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->touch_debounce_time > level.time)
		return;

	//gi.sound (self, CHAN_BODY, gi.soundindex ("objects/bush.wav"), 1, ATTN_NORM, 0);
	self->touch_debounce_time = level.time + 1;
}

/*QUAKED obj_bush1 (1 .5 0) (-34 -34 -19) (34 34 19)  INVULNERABLE  ANIMATE  EXPLODING  NOPUSH
A dome shaped bush
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_bush1 can't be moved)
-----------------------------------
*/
void SP_obj_bush1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/bush1/tris.fm");

	// The bounding box in the QUAKED comment is bigger to show the designers the true size of the objects
	VectorSet(self->mins, -16, -16, -19);
	VectorSet(self->maxs, 16, 16, 19);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,25,25,MAT_WOOD,SOLID_NOT);

	self->touch_debounce_time = level.time;
	self->touch = bush_touch;

}

/*QUAKED obj_bush2 (1 .5 0) (-56 -56 -40) (56 56 40)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A larger dome shaped bush
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_bush2 can't be moved)
-----------------------------------
*/
void SP_obj_bush2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/bush2/tris.fm");

	// The bounding box in the QUAKED comment is bigger to show the designers the true size of the objects
	VectorSet(self->mins, -16, -16, -40);
	VectorSet(self->maxs, 16, 16, 40);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,25,50,MAT_WOOD,SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->touch = bush_touch;

}

// Cactus will hurt player 
void cactus_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (self->touch_debounce_time > level.time)
		return;

	self->touch_debounce_time = level.time + 5;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 1, 0, DAMAGE_AVOID_ARMOR,MOD_DIED);
}

/*QUAKED obj_cactus (1 .5 0) (-18 -18 -44) (18 18 44)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A cactus.  Hurts the player 1 pt every five seconds he is pushes against it.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_cactus can't be moved)
-----------------------------------
*/
void SP_obj_cactus (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/cactus/tris.fm");

	VectorSet(self->mins, -18, -18, -44);
	VectorSet(self->maxs, 18, 18, 44);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,50,125,MAT_NONE,SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->touch = cactus_touch;
}

/*QUAKED obj_cactus3 (1 .5 0) (-14 -14 -32) (14 14 32)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A cactus.  Hurts the player 1 pt every five seconds he is pushes against it.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_cactus3 can't be moved)
-----------------------------------
*/
void SP_obj_cactus3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/cactus3/tris.fm");

	VectorSet(self->mins, -14, -14, -32);
	VectorSet(self->maxs, 14, 14, 32);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,50,125,MAT_NONE,SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->touch = cactus_touch;
}

void cactus_close(edict_t *self)
{
	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->think=cactus_close;
	}
	self->nextthink=level.time + FRAMETIME;
}

void cactus_open(edict_t *self)
{
	self->s.frame+=4;
	if (self->s.frame < 16)
	{
		self->think=cactus_open;
	}
	else
		self->s.frame = 16;

	self->nextthink=level.time + FRAMETIME;
}

void cactus4_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->s.frame)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever1.wav"), 1, ATTN_NORM, 0);
		self->think = cactus_open;
	}
	else if (self->s.frame == 16)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("objects/lever1.wav"), 1, ATTN_NORM, 0);
		self->think = cactus_close;
	}
	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED obj_cactus4 (1 .5 0) (-11 -11 -11) (11 11 11)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A small cactus that open up or closes. Is triggerable.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_cactus4 can't be moved)
-----------------------------------
*/
void SP_obj_cactus4 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/cactus4/tris.fm");

	VectorSet(self->mins, -11, -11, -11);
	VectorSet(self->maxs, 11, 11, 11);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_NONE,SOLID_BBOX);
	self->use = cactus4_use;

	self->touch_debounce_time = level.time;
	self->touch = cactus_touch;
}

/*QUAKED obj_basket (1 .5 0) (-13 -13 -21) (13 13 21)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A tall basket with a lid on it.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_basket (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/basket/tris.fm");

	VectorSet(self->mins, -13, -13, -21);
	VectorSet(self->maxs, 13, 13, 21);

	ObjectInit(self,50,70,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_claybowl (1 .5 0) (-6 -6 -2) (6 6 2)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A bowl made of clay
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_claybowl (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/claybowl/tris.fm");

	VectorSet(self->mins, -6, -6, -2);
	VectorSet(self->maxs, 6, 6, 2);

	ObjectInit(self,5,5,MAT_POTTERY,SOLID_BBOX);
}

/*QUAKED obj_clayjar (1 .5 0) (-15 -15 -24) (15 15 24)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A big honking urn made of clay
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_clayjar (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/clayjar/tris.fm");

	VectorSet(self->mins, -15, -15, -24);
	VectorSet(self->maxs, 15, 15, 24);

	ObjectInit(self,25,125,MAT_POTTERY,SOLID_BBOX);
}

/*QUAKED obj_gorgonbones (1 .5 0) (-18 -38 -9) (18 38 1)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
The bones of a dead gorgon.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_gorgonbones can't be moved)
-----------------------------------
*/
void SP_obj_gorgonbones (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/bones/gorgon/tris.fm");

	VectorSet(self->mins, -18, -38, -9);
	VectorSet(self->maxs,  18,  38, 1);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,50,125,MAT_NONE,SOLID_BBOX);
}

/*QUAKED obj_grass (1 .5 0) (-8 -8 -10) (8 8 10)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A clump of grass
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (obj_grass can't be moved)
-----------------------------------
*/
void SP_obj_grass (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/grass/tris.fm");

	VectorSet(self->mins,  -8, -8, -10);
	VectorSet(self->maxs,  8,  8, 10);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,50,125,MAT_WOOD,SOLID_NOT);
	
	self->s.effects|=EF_CAMERA_NO_CLIP;
}

/*QUAKED obj_swampflat_top (1 .5 0) (0 -100 -48) (2 100 48)  INVULNERABLE  ANIMATE EXPLODING NOPUSH
A flat poly to be used on the outer edge of swamp levels. Vegetation growing up.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_swampflat_top (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/swampflat/tris.fm");

	VectorSet(self->mins,  0, -100, -48);
	VectorSet(self->maxs,   2, 100,  48);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_swampflat_bottom (1 .5 0) (0 -100 -48) (2 100 48)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A flat poly to be used on the outer edge of swamp levels. Vegetation hanging down.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_swampflat_bottom (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/swampflat/tris.fm");

	VectorSet(self->mins,  0, -100, -48);
	VectorSet(self->maxs,   2, 100,  48);

	self->s.skinnum = 1;
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);

}

/*QUAKED obj_treestump (1 .5 0) (-18 -18 -16) (18 18 16)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A short tree stump  
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_treestump (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/plants/treestump/tris.fm");

	VectorSet(self->mins,  -18, -18, -16);
	VectorSet(self->maxs,   18,  18,  16);

	self->s.skinnum = 1;
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_jawbone (1 .5 0) (-11 -11 -12) (11 11 12)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
The jaws of a fish. 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_jawbone (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/bones/jaws/tris.fm");

	VectorSet(self->mins,  -11, -11, -12);
	VectorSet(self->maxs,   11,  11,  12);

	self->s.effects|=EF_CAMERA_NO_CLIP;

	ObjectInit(self,25,125,MAT_NONE,SOLID_BBOX);
	
	self->s.skinnum = 1;

}

/*QUAKED obj_barrel_metal (1 .5 0) (-11 -12 -18) (11 12 18)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A metal barrel. 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_barrel_metal (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/barrel/metal/tris.fm");

	VectorSet(self->mins,  -11, -12, -18);
	VectorSet(self->maxs,   11,  12,  18);

	ObjectInit(self,75,125,MAT_METAL,SOLID_BBOX);
	self->s.skinnum = 1;
}

/*QUAKED obj_barrel_explosive (1 .5 0) (-11 -12 -18) (11 12 18)  INVULNERABLE  ANIMATE   EXPLODING  NOPUSH
A barrel that explodes and does damage. 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_barrel_explosive (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/barrel/normal/tris.fm");

	VectorSet(self->mins,  -11, -12, -18);
	VectorSet(self->maxs,   11,  12,  18);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
	self->s.skinnum = 1;

}
/*QUAKED obj_gascan (1 .5 0) (-8 -9 -13) (8 9 13)  INVULNERABLE ANIMATE EXPLODING NOPUSH
A metal gas can. 
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_gascan (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/barrel/gascan/tris.fm");

	VectorSet(self->mins,  -8, -9, -13);
	VectorSet(self->maxs,   8,  9,  13);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
	self->s.skinnum = 1;

}

/*QUAKED obj_pipe1 (1 .5 0) (-11 -24 -7) (11 24 7)  INVULNERABLE  ANIMATE   EXPLODING NOPUSH
A section of pipe with 90 degree bend in it.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pipe1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/pipe1/tris.fm");

	VectorSet(self->mins,  -11, -24, -7);
	VectorSet(self->maxs,   11,  24,  7);

	ObjectInit(self,50,125,MAT_WOOD,SOLID_BBOX);
	self->s.skinnum = 1;

}

/*QUAKED obj_pipe2 (1 .5 0) (-6 -25 -4) (6 25 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
A straight section of pipe.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pipe2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/pipe2/tris.fm");

	VectorSet(self->mins,  -6, -25, -4);
	VectorSet(self->maxs,   6,  25,  4);

	ObjectInit(self,50,125,MAT_WOOD,SOLID_BBOX);
	self->s.skinnum = 1;

}

/*QUAKED obj_pipewheel (1 .5 0) (-14 -14 -12) (14 14 12) INVULNERABLE ANIMATE EXPLODING NOPUSH
A shutoff valve for pipe.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pipewheel (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/pipewheel/tris.fm");

	VectorSet(self->mins,  -14, -14, -12);
	VectorSet(self->maxs,   14,  14,  12);

	self->spawnflags |= OBJ_NOPUSH;

	ObjectInit(self,50,125,MAT_WOOD,SOLID_BBOX);
	self->s.skinnum = 1;

}

/*QUAKED obj_minecart (1 .5 0) (-18 -29 -20) (18 29 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A full mine cart used on the mine levels.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_minecart (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/carts/mine/tris.fm");

	VectorSet(self->mins,  -18, -29, -20);
	VectorSet(self->maxs,   18,  29,  20);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
	self->s.frame = 0;

}

/*QUAKED obj_minecart2 (1 .5 0) (-18 -29 -20) (18 29 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
An empty mine cart used on the mine levels.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_minecart2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/carts/mine/tris.fm");

	VectorSet(self->mins,  -18, -29, -20);
	VectorSet(self->maxs,   18,  29,  20);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
	self->s.frame = 20;

}


/*QUAKED obj_minecart3 (1 .5 0) (-18 -29 -15) (18 29 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A busted mine cart used on the mine levels.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_minecart3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/carts/mine/tris.fm");

	VectorSet(self->mins,  -18, -29, -15);
	VectorSet(self->maxs,   18,  29,  20);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
	self->s.frame = 40;

}

/*QUAKED obj_andwallhanging (1 .5 0) ( 0 -19 -24) (4 19 24) INVULNERABLE ANIMATE EXPLODING NOPUSH
A circular Andorian wall hanging.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_andwallhanging (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/andwallhang/tris.fm");

	VectorSet(self->mins,   0, -19, -24);
	VectorSet(self->maxs,   4,  19,  24);

	ObjectInit(self,75,100,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_pick (1 .5 0) ( -12 -13 -2) (12 13 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tool that is a pick.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pick (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/tools/pick/tris.fm");

	VectorSet(self->mins,   -12, -13, -2);
	VectorSet(self->maxs,    12,  13,  2);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);
}

/*QUAKED obj_metalchunk1 (1 .5 0) ( -10 -26 -4) (10 26 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
A chunk of twisted metal.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_metalchunk1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/metalchunks/tris.fm");

	VectorSet(self->mins,   -10, -26, -4);
	VectorSet(self->maxs,   10,   26,  4);

	ObjectInit(self,75,125,MAT_METAL,SOLID_BBOX);
}

/*QUAKED obj_metalchunk2 (1 .5 0) ( -10 -26 -7) (10 26 7) INVULNERABLE ANIMATE EXPLODING NOPUSH
Another chunk of twisted metal.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_metalchunk2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/metalchunks/tris.fm");

	VectorSet(self->mins,   -10, -26, -7);
	VectorSet(self->maxs,   10,   26,  7);

	ObjectInit(self,75,125,MAT_METAL,SOLID_BBOX);

	self->s.frame = 1;
}


/*QUAKED obj_metalchunk3 (1 .5 0) ( -9 -30 -4) (9 30 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
Another chunk of twisted metal.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_metalchunk3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pipes/metalchunks/tris.fm");

	VectorSet(self->mins,   -9, -30, -4);
	VectorSet(self->maxs,    9,   30, 4);

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);

	self->s.frame = 2;
}

/*QUAKED obj_rocks1 (1 .5 0) ( -12 -13 -4) (12 13 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bunch of rocks together
For the MINE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_rocks1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/rocks/rock1/tris.fm");

	VectorSet(self->mins,   -12, -13, -4);
	VectorSet(self->maxs,    12,  13,  4);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

}

/*QUAKED obj_rocks2 (1 .5 0) ( -9 -30 -4) (9 30 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
A big rock
For the MINE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_rocks2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/rocks/rock2/tris.fm");

	VectorSet(self->mins,   -34, -40, -19);
	VectorSet(self->maxs,    34,  40,  19);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

}

void symbolthink(edict_t *self)
{
	M_droptofloor(self);

	self->think = symbolthink;
	self->nextthink = level.time + FRAMETIME;

	if (self->touch_debounce_time < level.time)
	{
		self->think = NULL;
		self->nextthink = -1;		
	}
}

void hivepriestesssymbol_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.modelindex = gi.modelindex("models/items/puzzles/hiveidol/tris.fm");

	VectorSet(self->mins,   -4, -4, -13);
	VectorSet(self->maxs,    4,  4,  13);

	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

	self->velocity[2] = -1;

	self->touch_debounce_time = level.time + 5;

	self->think = symbolthink;
	self->nextthink = level.time + FRAMETIME;

}

/*QUAKED obj_hivepriestessssymbol (1 .5 0) ( -4 -4 -13) (4 4 13) INVULNERABLE ANIMATE EXPLODING NOPUSH
The Hive Priestess Symbol was created originally by T'jektaluck back in 14567 AH (After Hive).  It was stolen by Matt Pinkston in 20054.  He was promptly captured and fed Zots until he died from sugar overload.
It starts off invisible.  When used it appears.
For the HIVE level
-------  FIELDS  ------------------
INVULNERABLE - N/A it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_hivepriestessssymbol (edict_t *self)
{
	self->s.modelindex = 0;

	VectorSet(self->mins,   -4, -4, -13);
	VectorSet(self->maxs,    4,  4,  13);
	self->use = hivepriestesssymbol_use;

	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_NOT);

}

/*QUAKED obj_queenthrone (1 .5 0) ( -40 -56 -49) (40 56 49) INVULNERABLE ANIMATE EXPLODING NOPUSH
A throne for the queen
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_queenthrone (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/queen/tris.fm");

	VectorSet(self->mins,   -40, -56, -49);
	VectorSet(self->maxs,    40,  56,  49);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

}

/*QUAKED obj_queenchair (1 .5 0) ( -30 -28 -31) (30 28 31) INVULNERABLE ANIMATE EXPLODING NOPUSH
A chair to go by the queen throne
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_queenchair (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chairs/smallchair/tris.fm");

	VectorSet(self->mins,   -30, -28, -31);
	VectorSet(self->maxs,    30,  28,  31);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

}

/*QUAKED obj_shrine (1 .5 0) ( -26 -38 -38) (26 38 38) INVULNERABLE ANIMATE EXPLODING NOPUSH
Its a shrine
For all levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
---------- KEYS -----------------  
style - (default 0) type of shrine
   0 - heal
   1 - mana
   2 - lungs
   3 - light
   4 - power up
   5 - armor
   6 - armor gold
   7 - random
   8 - reflection
   9 - staff
  10 - ghost
  11 - speed
-----------------------------------

*/
void SP_obj_shrine (edict_t *self)
{
	vec3_t	offset2;
	vec3_t	offset;

	if (deathmatch->value && ((int)dmflags->value & DF_NO_SHRINE))
	{
		G_SetToFree(self);
		return;
	}

	if (self->style == 11 && no_runshrine->value)
	{
		G_SetToFree(self);
		return;
	}

	self->s.modelindex = gi.modelindex("models/objects/shrine/tris.fm");

	VectorSet(self->mins,   -26, -38, -38);
	VectorSet(self->maxs,    26,  38,  38);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

	if (deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS))
		self->style = 7;
	

	if ((self->style >= 0)  && (self->style <= 12))
	{
		++self->style;
	}

	// make sure we always send the model
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	// make the ball appear in the middle
	VectorScale(self->s.angles, ANGLE_TO_RAD, offset);
	DirFromAngles(offset, offset2);
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_SHRINE_BALL, CEF_BROADCAST, self->s.origin, 
														"db", offset2, (byte)(self->style-1));

}

/*QUAKED obj_larvaegg (1 .5 0) ( -6 -14 -6) (6 14 6) INVULNERABLE ANIMATE EXPLODING NOPUSH
An egg for the larva
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_larvaegg (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/eggs/eggs/tris.fm");

	VectorSet(self->mins,   -6, -14, -6);
	VectorSet(self->maxs,    6,  14,  6);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GLASS,SOLID_BBOX);

}

/*QUAKED obj_larvabrokenegg (1 .5 0) ( -6 -7 -5) (6 7 5) INVULNERABLE ANIMATE EXPLODING NOPUSH
A broken egg for the larva
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_larvabrokenegg (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/eggs/brokenegg/tris.fm");

	VectorSet(self->mins,   -6, -7, -5);
	VectorSet(self->maxs,    6,  7,  5);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GLASS,SOLID_BBOX);

}

/*QUAKED obj_cocoon (1 .5 0) ( -20 -20 -36) (20 20 36) INVULNERABLE ANIMATE EXPLODING NOPUSH
A cocoon which hangs from the ceiling
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A (always animates)
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_cocoon (edict_t *self)
{

	VectorSet(self->mins,   -20, -20, -36);
	VectorSet(self->maxs,    20,  20,  36);

	// Always animate and can't be pushed
	self->spawnflags |= OBJ_NOPUSH | OBJ_ANIMATE;
	SpawnClientAnim(self, FX_ANIM_COCOON, NULL);

	ObjectInit(self,75,125,MAT_INSECT,SOLID_BBOX);
}

/*QUAKED obj_cocoonopen (1 .5 0) ( -20 -20 -34) (20 20 34) INVULNERABLE ANIMATE EXPLODING NOPUSH
An open cocoon which hangs from the ceiling
For the HIVE levels
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_cocoonopen (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/eggs/cocoon/tris.fm");

	VectorSet(self->mins,   -20, -20, -34);
	VectorSet(self->maxs,    20,  20,  34);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.frame = 20;

	ObjectInit(self,75,125,MAT_INSECT,SOLID_BBOX);

}

/*QUAKED obj_venusflytrap (1 .5 0) ( -20 -20 -24) (20 20 24) INVULNERABLE ANIMATE EXPLODING NOPUSH
The venus flytrap - a viscous beast of a plant
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_venusflytrap (edict_t *self)
{
	edict_t *leaves;

	self->s.modelindex = gi.modelindex("models/objects/plants/venus/tris.fm");

	VectorSet(self->mins,   -20, -20, -24);
	VectorSet(self->maxs,    20,  20,  24);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_LEAF,SOLID_BBOX);

	leaves = G_Spawn();

	VectorCopy(self->s.origin,leaves->s.origin);
	leaves->s.origin[2] -= 16;
	leaves->movetype = PHYSICSTYPE_NONE;
	leaves->solid = SOLID_NOT;
	leaves->s.modelindex = gi.modelindex("models/objects/plants/v-plant/tris.fm");
	BboxYawAndScale(leaves);
	leaves->s.angles[YAW] = self->s.angles[YAW];
	gi.linkentity (leaves);

	self->target_ent = leaves;


}

void tomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
	--self->s.frame;
	if (self->s.frame < 0)
		self->s.frame=1;
}
/*QUAKED obj_statue_techeckriktomb (1 .5 0) ( -41 -11 -14) (41 11 14) INVULNERABLE ANIMATE EXPLODING NOPUSH
The statue of the techeckrik laying down.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_statue_techeckriktomb (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/tomb/tris.fm");

	VectorSet(self->mins,   -41, -11, -14);
	VectorSet(self->maxs,    41,  11,  14);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

	self->s.frame = 1;

	self->use = tomb_use;
}


void tcheckrik_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->s.frame > 0)
		--self->s.frame;
}

/*QUAKED obj_statue_techeckrikright (1 .5 0) ( -26 -40 -50) (26 40 50) INVULNERABLE ANIMATE EXPLODING NOPUSH NOGEM
The statue of the techeckrik with his arms raised up to the right.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
NOGEM - No gem in the chest  (puzzle)
-----------------------------------
*/
void SP_obj_statue_techeckrikright (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/tcheckrik/tris.fm");

	VectorSet(self->mins,   -26, -40, -50);
	VectorSet(self->maxs,    26,  40,  50);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

	self->s.frame = 2;
	if (self->spawnflags & 16)	// NOGEM
		self->s.frame = 3;

	self->use = tcheckrik_use;

}



/*QUAKED obj_statue_techeckrikleft (1 .5 0) ( -26 -40 -50) (26 40 50) INVULNERABLE ANIMATE EXPLODING NOPUSH NOGEM
The statue of the techeckrik with his arms raised up to the left.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
NOGEM - No gem in the chest (puzzle)
-----------------------------------
*/
void SP_obj_statue_techeckrikleft (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/statue/tcheckrik/tris.fm");

	VectorSet(self->mins,   -26, -40, -50);
	VectorSet(self->maxs,    26,  40,  50);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_GREYSTONE,SOLID_BBOX);

	self->s.frame = 0;

	if (self->spawnflags & 16)	// NOGEM
		self->s.frame = 1;

	self->use = tcheckrik_use;

}


void spellbook_anim (edict_t *self)
{
	if (self->s.frame > 0)
	{
		--self->s.frame;

		self->think = spellbook_anim;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/spbkcls.wav"), 1, ATTN_NORM, 0);
		self->think = NULL;
	}

}

void spellbook_use (edict_t *self, edict_t *other, edict_t *activator)
{
	spellbook_anim(self);

	G_FreeEdict(self->target_ent);

	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/spbook.wav"), 1, ATTN_NORM, 0);

}

/*QUAKED obj_spellbook (1 .5 0) ( -14 -14 -35) (14 14 40) INVULNERABLE ANIMATE EXPLODING NOPUSH
A spell book that closes when triggered.
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_spellbook (edict_t *self)
{
	edict_t *beam;

	self->s.modelindex = gi.modelindex("models/objects/spellbook/book/tris.fm");

	VectorSet(self->mins,   -14, -14, -35);
	VectorSet(self->maxs,    14,  14,  40);

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->use = spellbook_use;
	ObjectInit(self,75,125,MAT_NONE,SOLID_BBOX);
	self->s.frame = 20;

	beam = G_Spawn();

	VectorCopy(self->s.origin,beam->s.origin);
	beam->s.origin[2] += 2;
	beam->movetype = PHYSICSTYPE_NONE;
	beam->solid = SOLID_NOT;
	beam->s.modelindex = gi.modelindex("models/objects/spellbook/beam/tris.fm");
	BboxYawAndScale(beam);
	beam->s.angles[YAW] = self->s.angles[YAW];
	gi.linkentity (beam);

	self->target_ent = beam;
}

/*QUAKED obj_skullpole (1 .5 0) ( -10 -10 -47) (10 10 47) INVULNERABLE ANIMATE EXPLODING NOPUSH
A pole with skulls on it
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_skullpole (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/flags/totempole/tris.fm");

	VectorSet(self->mins,   -10, -10, -47);
	VectorSet(self->maxs,    10,  10,  47);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,75,125,MAT_POTTERY,SOLID_BBOX);

}

/*QUAKED obj_pot1 (1 .5 0) ( -3 -8 -8) (3 8 8) INVULNERABLE ANIMATE EXPLODING NOPUSH
A cooking pot which hangs from a wall
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_pot1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/pot/tris.fm");

	VectorSet(self->mins,   -3, -8, -8);
	VectorSet(self->maxs,    3,  8,  8);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_POTTERY,SOLID_BBOX);

}

/*QUAKED obj_pot2 (1 .5 0) ( -7 -7 -3) (7 7 3) INVULNERABLE ANIMATE EXPLODING NOPUSH
A cooking pot which lays flat on a table
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_pot2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/pots/pot2/tris.fm");

	VectorSet(self->mins,   -7, -7, -3);
	VectorSet(self->maxs,    7,  7,  3);

	ObjectInit(self,75,125,MAT_POTTERY,SOLID_BBOX);

}

/*QUAKED obj_bottle1 (1 .5 0) ( -3 -3 -7) (3 3 7) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bottle that looks like the one Jennie came out of in I Dream of Jennie
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_bottle1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/jars/bottle/tris.fm");

	VectorSet(self->mins,   -3, -3, -7);
	VectorSet(self->maxs,    3,  3,  7);

	ObjectInit(self,75,125,MAT_POTTERY,SOLID_BBOX);

}

/*QUAKED obj_jug1 (1 .5 0) ( -6 -6 -6) (6 6 6) INVULNERABLE ANIMATE EXPLODING NOPUSH
A wine jug
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - can't be moved by player
-----------------------------------
*/
void SP_obj_jug1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/jars/jug/tris.fm");

	VectorSet(self->mins,   -6, -6, -6);
	VectorSet(self->maxs,    6,  6,  6);

	ObjectInit(self,75,125,MAT_POTTERY,SOLID_BBOX);

}


/*QUAKED obj_torture_table (1 .5 0) ( -46 -14 -14) (46 14 14) INVULNERABLE ANIMATE EXPLODING NOPUSH
A table useful for wringing confessions from your broken and pitiful enemies.
style - the frame of animation for model
 0 -table is down
 1 - table is in upright position
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't ever be moved by player)
-----------------------------------
*/
void SP_obj_torture_table (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/torture/table/tris.fm");

	VectorSet(self->mins,   -46, -14, -14);
	VectorSet(self->maxs,    46,  14,  14);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	if (self->style < 2)
		self->s.frame = self->style;
	else
		self->s.frame = 0;

	ObjectInit(self,75,125,MAT_WOOD,SOLID_BBOX);

}


/*QUAKED obj_torture_wallring (1 .5 0) ( -2 -4 -6) (2 4 6) INVULNERABLE ANIMATE EXPLODING NOPUSH
A ring that hangs from a wall
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't ever be moved by player)
-----------------------------------
*/
void SP_obj_torture_wallring (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/torture/wallring/tris.fm");

	VectorSet(self->mins,   -2, -4, -6);
	VectorSet(self->maxs,    2,  4,  6);

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,75,125,MAT_METAL,SOLID_BBOX);

}

void statue_tchecktrik_bust_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->s.frame)
		self->s.frame = 0;
	else
		self->s.frame = 1;
}

/*QUAKED obj_statue_tchecktrik_bust (1 .5 0) (-8 -12 -15) (8 12 15) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bust of a tchecktrik.  When used a necklace appears aroundit's neck.
style - 
0 - no necklace until used then necklace appears
1 - necklace until used then necklace disappears
-------  FIELDS  ------------------
INVULNERABLE - N/A  (can't be hurt)
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_statue_tchecktrik_bust (edict_t *self) 
{
	VectorSet(self->mins, -8, -12, -15);
	VectorSet(self->maxs, 8, 12, 15);

	self->s.modelindex = gi.modelindex("models/items/puzzles/tchecktrikbust/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	if (!self->style)
		self->s.frame = 1;
	else
		self->s.frame = 0;

	self->use = statue_tchecktrik_bust_use;

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);

}

void statue_sithraguard_think (edict_t *self)
{
	if (self->s.frame < 20)
	{
		++self->s.frame;
		self->think = statue_sithraguard_think;
		self->nextthink = level.time + FRAMETIME;
	}
	else
		self->think = NULL;
}

void statue_sithraguard_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *shield;

	statue_sithraguard_think(self);
	gi.sound (self, CHAN_BODY, gi.soundindex ("items/statuearm.wav"), 1, ATTN_NORM, 0);

	shield = G_Spawn();
	VectorCopy(self->s.origin,shield->s.origin);
	VectorCopy(self->s.angles,shield->s.angles);
	shield->s.modelindex = gi.modelindex("models/objects/statue/sithshield/tris.fm");
	shield->s.scale = self->s.scale;
	shield->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	shield->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	shield->movetype = PHYSICSTYPE_NONE;
	shield->solid = SOLID_NOT;

	BboxYawAndScale(shield);
	gi.linkentity (shield);


	
}

/*QUAKED obj_statue_sithraguard (1 .5 0) (-22 -20 -57) (22 20 57) INVULNERABLE ANIMATE EXPLODING NOPUSH
A statue of a sithra guard with spear extended.  When used the guard pulls his arm back.
style - type of statue
-------  FIELDS  ------------------
INVULNERABLE - N/A (can't be hurt)
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_statue_sithraguard (edict_t *self) 
{
	VectorSet(self->mins, -22, -20, -57);
	VectorSet(self->maxs, 22, 20, 57);

	self->s.modelindex = gi.modelindex("models/objects/statue/sithraguard/tris.fm");
	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.frame = 0;

	self->use = statue_sithraguard_use;

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);

}

void ironmaiden_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void ironmaiden_open (edict_t *self)
{

	if (self->s.frame == 9)
		gi.sound (self, CHAN_BODY, gi.soundindex ("items/ironmaiden.wav"), 1, ATTN_NORM, 0);

	if (self->s.frame > 0)
	{
		--self->s.frame;
		self->think = ironmaiden_open;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->s.frame = 0;
		self->think = NULL;
		self->touch = ironmaiden_touch;
	}
}

void ironmaiden_close (edict_t *self)
{
	if (self->s.frame < 10)
	{
		++self->s.frame;
		self->think = ironmaiden_close;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = ironmaiden_open;
		self->nextthink = level.time + FRAMETIME * 30;	// Open up in 30 seconds
	}
}

void ironmaiden_use (edict_t *self, edict_t *other, edict_t *activator)
{
	VectorSet(self->mins, -18, -18, -49);
	VectorSet(self->maxs, 18, 18, 49);

	self->touch  = NULL;

	ironmaiden_close(self);
	gi.sound (self, CHAN_BODY, gi.soundindex ("items/ironmaiden.wav"), 1, ATTN_NORM, 0);

}


// Cactus will hurt player 
void ironmaiden_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		source, vf;
	trace_t			trace;

	if (!other->client)
		return;

	if (self->touch_debounce_time > level.time)
		return;

	self->touch_debounce_time = level.time + 5;

	VectorCopy(self->s.origin, source);
	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(source, 44, vf, source);

	gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_PLAYERSOLID,&trace);
	if (trace.ent == other)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 5, 0, DAMAGE_AVOID_ARMOR,MOD_DIED);

	ironmaiden_use(self,NULL,NULL);
}


/*QUAKED obj_torture_ironmaiden (1 .5 0) (-18 -18 -49) (18 18 49) INVULNERABLE ANIMATE EXPLODING NOPUSH
An iron maiden that closes when used
-------  FIELDS  ------------------
INVULNERABLE - N/A (can't be hurt)
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_torture_ironmaiden (edict_t *self) 
{
	VectorSet(self->mins, -28, -48, -49);
	VectorSet(self->maxs, 28, 48, 49);

	self->s.modelindex = gi.modelindex("models/objects/torture/ironmaiden/tris.fm");
	self->spawnflags |= OBJ_INVULNERABLE; 
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	self->s.frame = 0;
	self->use = ironmaiden_use;
	self->touch = ironmaiden_touch;

	ObjectInit(self,250,200,MAT_METAL,SOLID_BBOX);

}


/*QUAKED obj_torture_rack (1 .5 0) (-22 -46 -19) (22 46 19) INVULNERABLE ANIMATE EXPLODING NOPUSH
A rack 
-------  FIELDS  ------------------
INVULNERABLE - N/A (can't be hurt)
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_torture_rack (edict_t *self) 
{
	VectorSet(self->mins, -22, -46, -19);
	VectorSet(self->maxs, 22, 46, 19);

	self->s.modelindex = gi.modelindex("models/objects/torture/rack/tris.fm");
	self->spawnflags |= OBJ_INVULNERABLE; 
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_WOOD,SOLID_BBOX);

}

/*QUAKED obj_torture_bed (1 .5 0) (-21 -43 -94) (21 43 94) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bed of spikes that falls from the ceiling.
-------  FIELDS  ------------------
INVULNERABLE - N/A (can't be hurt)
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_torture_bed (edict_t *self) 
{
	VectorSet(self->mins, -21, -43, -94);
	VectorSet(self->maxs, 21, 43, 94);

	self->s.modelindex = gi.modelindex("models/objects/torture/bed/tris.fm");
	self->spawnflags |= OBJ_INVULNERABLE; 
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_WOOD,SOLID_BBOX);

}


/*QUAKED obj_statue_saraphbust (1 .5 0) (-10 -20 -24) (10 20 24) INVULNERABLE ANIMATE EXPLODING NOPUSH
A bust of a saraph
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_statue_saraphbust (edict_t *self) 
{
	VectorSet(self->mins, -10, -20, -24);
	VectorSet(self->maxs,  10, 20, 24);

	self->s.modelindex = gi.modelindex("models/objects/statue/saraphbust/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);

}

#define FISHBOB_HEIGHT		.1
#define FISHBOB_SPEED		ANGLE_10

void fish_anim (edict_t *self)
{

  	M_ChangeYaw(self);

	self->s.origin[2] += (cos(self->moveinfo.current_speed) * FISHBOB_HEIGHT); 
	self->moveinfo.current_speed += FISHBOB_SPEED;

	if (self->s.frame < self->count)
	{
		++self->s.frame;
	}
	else
		self->s.frame=0;

	self->think = fish_anim;
	self->nextthink = level.time + FRAMETIME;
}

void biotank_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	self->target_ent->ideal_yaw = anglemod(other->s.angles[YAW] + 180);
}

/*QUAKED obj_biotank (1 .5 0) (-20 -33 -52) (20 33 52) INVULNERABLE ANIMATE EXPLODING NOPUSH
A biotank
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-------  KEYS  ------------------
style - what's in the biotank
0 - empty
1 - ET's head
2 - hairless Critter
3 - three fish
4 - wasp
*/
void SP_obj_biotank (edict_t *self) 
{
	edict_t *fish,*glass;
	vec3_t forward,right;

	VectorSet(self->mins, -21, -34, -52);
	VectorSet(self->maxs,  21, 34, 52);

	self->s.modelindex = gi.modelindex("models/objects/labs/biotank/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
				 
	glass = G_Spawn();
	VectorSet(glass->mins, -1, -1, -1);
	VectorSet(glass->maxs,  1, 1, 1);

	VectorCopy(self->s.origin,glass->s.origin);
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(glass->s.origin, (-8 * self->s.scale), right, glass->s.origin);
	VectorCopy(self->s.angles,glass->s.angles);
	glass->s.scale = self->s.scale;
	glass->s.angles[1] += 90.0f;
	glass->s.renderfx |= RF_TRANSLUCENT;
	glass->s.modelindex = gi.modelindex("models/objects/labs/bioglass2/tris.fm");
	glass->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	glass->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(glass,250,200,MAT_GLASS,SOLID_NOT);
	

	glass = G_Spawn();
	VectorSet(glass->mins, -1, -1, -1);
	VectorSet(glass->maxs,  1, 1, 1);

	VectorCopy(self->s.origin,glass->s.origin);
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(glass->s.origin, (26 * self->s.scale), right, glass->s.origin);
	VectorCopy(self->s.angles,glass->s.angles);
	glass->s.scale = self->s.scale;
	glass->s.angles[1] += 270.0f;
	glass->s.renderfx |= RF_TRANSLUCENT;
	glass->s.modelindex = gi.modelindex("models/objects/labs/bioglass2/tris.fm");
	glass->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	glass->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(glass,250,200,MAT_GLASS,SOLID_NOT);

	glass = G_Spawn();
	VectorSet(glass->mins, -1, -1, -1);
	VectorSet(glass->maxs,  1, 1, 1);

	VectorCopy(self->s.origin,glass->s.origin);
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(glass->s.origin, (9 * self->s.scale), right, glass->s.origin);
	VectorMA(glass->s.origin, (-17 * self->s.scale), forward, glass->s.origin);
	VectorCopy(self->s.angles,glass->s.angles);
	glass->s.scale = self->s.scale;
	glass->s.renderfx |= RF_TRANSLUCENT;
	glass->s.modelindex = gi.modelindex("models/objects/labs/bioglass2/tris.fm");
	glass->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	glass->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(glass,250,200,MAT_GLASS,SOLID_NOT);

	glass = G_Spawn();
	VectorSet(glass->mins, -1, -1, -1);
	VectorSet(glass->maxs,  1, 1, 1);

	VectorCopy(self->s.origin,glass->s.origin);
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(glass->s.origin, (9 * self->s.scale), right, glass->s.origin);
	VectorMA(glass->s.origin, (17 * self->s.scale), forward, glass->s.origin);
	glass->s.scale = self->s.scale;
	VectorCopy(self->s.angles,glass->s.angles);
	glass->s.angles[1] += 180.0f;
	glass->s.renderfx |= RF_TRANSLUCENT;
	glass->s.modelindex = gi.modelindex("models/objects/labs/bioglass2/tris.fm");
	glass->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	glass->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	
	ObjectInit(glass,250,200,MAT_GLASS,SOLID_NOT);

	if (!self->style)
		return;

	fish = G_Spawn();

	VectorCopy(self->s.origin,fish->s.origin);
	fish->s.origin[2] -= 8;
	fish->movetype = PHYSICSTYPE_FLY;
	fish->solid = SOLID_NOT;
	fish->gravity = 0;

	fish->yaw_speed = 4;
	fish->ideal_yaw = self->s.angles[YAW];

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(fish->s.origin, 9, right, fish->s.origin);

	if (self->style==1)
	{
		fish->s.modelindex = gi.modelindex("models/objects/labs/labfish/tris.fm");
		fish->count = 30;
		self->s.sound = gi.soundindex("objects/slowbubbles.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}
	else if (self->style==2)
	{
		fish->s.modelindex = gi.modelindex("models/objects/labs/labfish2/tris.fm");
		fish->count = 60;
		self->s.sound = gi.soundindex("objects/slowbubbles.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}
	else if (self->style==3)
	{
		fish->s.modelindex = gi.modelindex("models/objects/labs/labfish3/tris.fm");
		fish->count = 100;
		self->s.sound = gi.soundindex("objects/fastbubbles.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}
	else 
	{
		fish->s.modelindex = gi.modelindex("models/monsters/bee/tris.fm");
		fish->count = 60;
		fish->s.scale = 2;
		self->s.sound = gi.soundindex("objects/fastbubbles.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}

	BboxYawAndScale(fish);
	fish->s.angles[YAW] = self->s.angles[YAW];

	gi.linkentity (fish);

	fish->think = fish_anim;
	fish->nextthink = level.time + FRAMETIME;

	self->target_ent = fish;
	self->touch = biotank_touch;
}


/*QUAKED obj_tapper (1 .5 0) (-2 -5 -2) (2 5 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tapper for a keg 
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_tapper (edict_t *self) 
{
	VectorSet(self->mins, -2, -5, -2);
	VectorSet(self->maxs,  2, 5, 2);

	self->s.modelindex = gi.modelindex("models/objects/tapper/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_NOT);

}


/*QUAKED obj_wallringplaque (1 .5 0) (-3 -20 -55) (3 20 55) INVULNERABLE ANIMATE EXPLODING NOPUSH
An iron plaque with rings hanging from it.  Great for hanging half corpses from 
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_wallringplaque (edict_t *self) 
{
	VectorSet(self->mins, -3, -20, -55);
	VectorSet(self->maxs,  3,  20,  55);

	self->s.modelindex = gi.modelindex("models/objects/torture/plaque/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_hangingdude (1 .5 0) (-3 -20 -55) (3 20 55) INVULNERABLE ANIMATE EXPLODING NOPUSH
A half a dude hanging from a wall ring plaque
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_hangingdude (edict_t *self) 
{
	edict_t *ring;


	VectorSet(self->mins, -3, -20, -55);
	VectorSet(self->maxs,  3,  20,  55);

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/objects/torture/guy1/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->s.frame = 0;
	BboxYawAndScale(self);
	self->targetname = "guy1";
	gi.linkentity (self);


	ring = G_Spawn();
	VectorCopy(self->s.origin,ring->s.origin);
	VectorCopy(self->s.angles,ring->s.angles);
	ring->s.modelindex = gi.modelindex("models/objects/torture/plaque/tris.fm");
	ring->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	ring->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed
	ring->movetype = PHYSICSTYPE_NONE;
	ring->solid = SOLID_BBOX;
	VectorSet(ring->mins, -3, -20, -55);
	VectorSet(ring->maxs,  3,  20,  55);

	BboxYawAndScale(ring);
	gi.linkentity (ring);

}

/*QUAKED obj_frypan (1 .5 0) (-1 -3 -10) (1 3 10) INVULNERABLE ANIMATE EXPLODING NOPUSH
A pan which is hanging on a nail
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_frypan (edict_t *self) 
{
	VectorSet(self->mins, -1, -3, -10);
	VectorSet(self->maxs,  1,  3, 10);

	self->s.modelindex = gi.modelindex("models/objects/pots/frypan/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_eggpan (1 .5 0) (-4 -10 -1) (4 10 1) INVULNERABLE ANIMATE EXPLODING NOPUSH
A pan which is laying flat like it was on a stove
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_eggpan (edict_t *self) 
{
	VectorSet(self->mins, -4, -10, -1);
	VectorSet(self->maxs,  4,  10, 1);

	self->s.modelindex = gi.modelindex("models/objects/pots/eggpan/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_nest (1 .5 0) (-25 -25 -4) (25 25 4) INVULNERABLE ANIMATE EXPLODING NOPUSH
A big bird nest
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_nest (edict_t *self) 
{
	VectorSet(self->mins, -25, -25, -4);
	VectorSet(self->maxs,  25,  25, 4);

	self->s.modelindex = gi.modelindex("models/objects/nest/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_GREYSTONE,SOLID_BBOX);
}


/*QUAKED obj_choppeddude (1 .5 0) (-15 -40 -8) (15 40 8) INVULNERABLE ANIMATE EXPLODING NOPUSH
A guy who's all chopped up and laying on a table
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (can't be moved)
-----------------------------------
*/
void SP_obj_choppeddude (edict_t *self) 
{
	VectorSet(self->mins, -15, -40, -8);
	VectorSet(self->maxs,  15,  40, 8);

	self->s.modelindex = gi.modelindex("models/objects/torture/guy2/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_FLESH,SOLID_BBOX);
}

/*QUAKED obj_lab_parts_container (1 .5 0) (-8 -8 -11) (8 8 11) INVULNERABLE ANIMATE EXPLODING NOPUSH
A container of moving body parts
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A (thing always animates)
EXPLODING - N/A
NOPUSH - can't be pushed
-----------------------------------
*/
void SP_obj_lab_parts_container (edict_t *self)
{
	VectorSet(self->mins, -8, -8, -11);
	VectorSet(self->maxs, 8, 8, 11);

	self->spawnflags |= OBJ_ANIMATE;
	SpawnClientAnim(self, FX_ANIM_LABPARTSCONTAINER, NULL);
	ObjectInit(self,40,200,MAT_GLASS,SOLID_BBOX);
}

/*QUAKED obj_eyeball_jar (1 .5 0) (-13 -13 -18) (13 13 18) INVULNERABLE ANIMATE EXPLODING NOPUSH
A jar full of eyeballs
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A (thing always animates)
EXPLODING - N/A
NOPUSH - can't be moved
-----------------------------------
*/
void SP_obj_eyeball_jar (edict_t *self) 
{
	VectorSet(self->mins, -13, -13, -18);
	VectorSet(self->maxs,  13,  13, 18);

	self->spawnflags |= OBJ_NOPUSH | OBJ_ANIMATE;

	SpawnClientAnim(self, FX_ANIM_EYEBALLJAR, NULL);
	ObjectInit(self,50,200,MAT_GLASS,SOLID_BBOX);
}

/*QUAKED obj_lab_tray (1 .5 0) (-8 -8 -5) (8 8 5) INVULNERABLE ANIMATE EXPLODING NOPUSH
A tray with a beating heart and some tools on it.
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A (thing always animates)
EXPLODING - N/A
NOPUSH - N/A  (can't be pushed)
-----------------------------------
*/
void SP_obj_lab_tray (edict_t *self)
{
	VectorSet(self->mins, -8, -8, -5);
	VectorSet(self->maxs, 8, 8, 5);

	self->spawnflags |= OBJ_NOPUSH | OBJ_ANIMATE;
	SpawnClientAnim(self, FX_ANIM_LABTRAY, NULL);
	ObjectInit(self,40,200,MAT_FLESH,SOLID_BBOX);
}


void ogle_moan (edict_t *self)
{
	gi.sound(self, CHAN_VOICE, gi.soundindex(va("monsters/ogle/oglemoan%c.wav", irand('1', '2'))), 1, ATTN_IDLE, 0);
	self->think = ogle_moan;
	self->nextthink = level.time + irand (3, 10);
}

/*QUAKED obj_hanging_ogle (1 .5 0) (-8 -16 -34) (8 16 34) INVULNERABLE ANIMATE EXPLODING NOPUSH
Poor little hanging ogle
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A (thing always animates)
EXPLODING - N/A
NOPUSH - N/A  (can't be pushed)
-----------------------------------
*/
void SP_obj_hanging_ogle (edict_t *self)
{
	edict_t *ring;
	vec3_t forward;

	VectorSet(self->mins, -8, -16, -34);
	VectorSet(self->maxs,  8, 16, 34);

	self->spawnflags |= OBJ_NOPUSH | OBJ_ANIMATE;

	SpawnClientAnim(self, FX_ANIM_HANGING_OGLE, NULL);
	ObjectInit(self,100,200,MAT_FLESH,SOLID_BBOX);


	ring = G_Spawn();
	VectorCopy(self->s.origin,ring->s.origin);
	ring->s.origin[2] += 27;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(ring->s.origin, -9, forward, ring->s.origin);

	VectorCopy(self->s.angles,ring->s.angles);
	ring->s.modelindex = gi.modelindex("models/objects/torture/plaque2/tris.fm");
	ring->movetype = PHYSICSTYPE_NONE;
	ring->solid = SOLID_NOT;
	ring->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	ring->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	BboxYawAndScale(ring);
	gi.linkentity (ring);

	self->target_ent = ring;

	self->think = ogle_moan;
	self->nextthink = level.time + irand (3, 10);
}


/*QUAKED obj_ring_plaque2 (1 .5 0) (-2 -24 -20) (2 24 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
More rings mounted into a wall plate
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - N/A  (can't be pushed)
-----------------------------------
*/
void SP_obj_ring_plaque2 (edict_t *self)
{
	VectorSet(self->mins, -2, -24, -20);
	VectorSet(self->maxs,  2, 24, 20);

	self->s.modelindex = gi.modelindex("models/objects/torture/plaque2/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->spawnflags |= OBJ_INVULNERABLE; // can't be destroyed

	ObjectInit(self,250,200,MAT_FLESH,SOLID_BBOX);
}


/*QUAKED obj_statue_sariph (1 .5 0) (-13 -16 -41) (13 16 41) INVULNERABLE ANIMATE EXPLODING NOPUSH
Big statue in armor carrying an axe
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - N/A  (can't be pushed)
-----------------------------------
*/
void SP_obj_statue_sariph (edict_t *self)
{
	VectorSet(self->mins, -13, -16, -41);
	VectorSet(self->maxs,  13, 16, 41);

	self->s.modelindex = gi.modelindex("models/objects/statue/sariph/tris.fm");
	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed

	ObjectInit(self,250,200,MAT_STONE,SOLID_BBOX);
}

/*QUAKED obj_pushcart (1 .5 0) (-13 -16 -41) (13 16 41) INVULNERABLE ANIMATE EXPLODING NOPUSH
A push cart for the ogles to push around
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_pushcart (edict_t *self)
{
	VectorSet(self->mins, -14, -30, -17);
	VectorSet(self->maxs,  14, 30, 17);

	self->s.modelindex = gi.modelindex("models/objects/carts/pushcart/tris.fm");

	ObjectInit(self,250,200,MAT_WOOD,SOLID_BBOX);
}



/*QUAKED obj_bookopen (1 .5 0) (-8 -16 -2) (8 16 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
An open book
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_bookopen (edict_t *self)
{
	VectorSet(self->mins, -8, -16, -2);
	VectorSet(self->maxs,  8, 16, 2);

	self->s.modelindex = gi.modelindex("models/objects/books/bookopen/tris.fm");

	ObjectInit(self,250,200,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_bookclosed (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A closed book standing up
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_bookclosed (edict_t *self)
{
	VectorSet(self->mins, -8, -8, -2);
	VectorSet(self->maxs,  8, 8, 2);

	self->s.modelindex = gi.modelindex("models/objects/books/bookclosed/tris.fm");

	ObjectInit(self,250,200,MAT_WOOD,SOLID_BBOX);
}


/*QUAKED obj_web (1 .5 0) (-2 -18 -20) (2 18 20) INVULNERABLE ANIMATE EXPLODING NOPUSH
A cobweb
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_web (edict_t *self)
{
	VectorSet(self->mins, -2, -18, -20);
	VectorSet(self->maxs,  2,  18,  20);

	self->s.modelindex = gi.modelindex("models/objects/web/web/tris.fm");
	self->s.renderfx |= RF_TRANSLUCENT;

	ObjectInit(self,250,200,MAT_WOOD,SOLID_NOT);
}

#define FAST_TWITCH 16

void larva_anim (edict_t *self)
{

	if (self->s.frame < self->count)
	{
		++self->s.frame;

		if (self->spawnflags & FAST_TWITCH)	// Some squirm faster
			++self->s.frame;
	}
	else
	{
		self->s.frame=0;
	}

	self->think = larva_anim;
	self->nextthink = level.time + FRAMETIME;
}

void larva_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	T_Damage (self, other, other, vec3_origin, self->s.origin, vec3_origin, 10, 0, DAMAGE_AVOID_ARMOR,MOD_DIED);
}

/*QUAKED obj_larva (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A squirming larva
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_larva (edict_t *self)
{
	VectorSet(self->mins, -8, -12, -2);
	VectorSet(self->maxs,  8,  12,  2);

	self->s.modelindex = gi.modelindex("models/objects/eggs/larva/tris.fm");

	ObjectInit(self,2,100,MAT_INSECT,SOLID_BBOX);

	self->movetype = PHYSICSTYPE_STEP;
	self->count = 19;

	self->think = larva_anim;
	self->nextthink = level.time + FRAMETIME;
	self->nextthink += FRAMETIME * flrand(1, 6);	// Don't all start at the same time

	self->s.sound = gi.soundindex("misc/larva.wav");
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;

	self->svflags = SVF_ALLOW_AUTO_TARGET;

	if (irand(0, 1))
		self->spawnflags |= FAST_TWITCH;

	self->touch = larva_touch;


}

/*QUAKED obj_bloodsplat (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A red blood splat
-------  FIELDS  ------------------
INVULNERABLE - can't be hurt
ANIMATE - N/A 
EXPLODING - N/A
NOPUSH - can be pushed
-----------------------------------
*/
void SP_obj_bloodsplat (edict_t *self)
{
	VectorSet(self->mins, -8, -8, -2);
	VectorSet(self->maxs,  8,  8,  2);

	self->s.modelindex = gi.modelindex("sprites/fx/bsplat.sp2");
	self->flags |= RF_FIXED | RF_ALPHA_TEXTURE;

	ObjectInit(self,2,100,MAT_FLESH,SOLID_NOT);

}
