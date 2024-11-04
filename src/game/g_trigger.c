#include "g_local.h"
#include "Vector.h"
#include "g_DefaultMessageHandler.h"
#include "fx.h"
#include "g_playstats.h"
#include "cl_strings.h"

#define TRIGGER_MONSTER		1
#define TRIGGER_NOT_PLAYER	2
#define TRIGGER_TRIGGERED	4
#define TRIGGER_ANY			8

#define PUZZLE_SHOWNO_INVENTORY 16
#define PUZZLE_DONT_REMOVE		32	

void Trigger_Deactivate(edict_t *self, G_Message_t *msg);
void Trigger_Activate(edict_t *self, G_Message_t *msg);

void trigger_enable(edict_t *self, edict_t *other, edict_t *activator);
void Use_Multi(edict_t *self, edict_t *other, edict_t *activator);

void TriggerStaticsInit()
{
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_SUSPEND] = Trigger_Deactivate;
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_UNSUSPEND] = Trigger_Activate;
}

// the wait time has passed, so set back up for another activation
void multi_wait(edict_t *self)
{
	self->think = NULL;
	if(self->activator)
	{
		self->activator->target_ent = NULL;
	}
}

// the trigger was just activated
// self->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void TriggerActivated(edict_t *self)
{
	if(self->think)
	{
		return;		// already been triggered
	}

	assert(self->TriggerActivated);

	self->TriggerActivated(self, self->activator);

	if(self->wait > 0)
	{
		self->think = multi_wait;
		self->nextthink = level.time + self->wait;
	}
	else
	{
		self->touch = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void Touch_Multi(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// Monsters or players can trigger it
	if ((self->spawnflags & TRIGGER_ANY) && ((strcmp(other->classname, "player") == 0) ||
		(other->svflags & SVF_MONSTER)))
		;
	// Player cannot trigger it
	else if(strcmp(other->classname, "player") == 0)
	{
		if(self->spawnflags & TRIGGER_NOT_PLAYER)
		{
			return;
		}
	}
	// Just monster will trigger it
	else if(other->svflags & SVF_MONSTER)
	{
		if(!(self->spawnflags & TRIGGER_MONSTER))
		{
			return;
		}
	}
	else
	{
		return;
	}

	if(!Vec3IsZero(self->movedir))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);

		if(DotProduct(forward, self->movedir) < 0)
		{
			return;
		}
	}

	self->activator = other;

	TriggerActivated(self);
}

void InitTrigger(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	if(!self->wait)
	{
		self->wait = 0.2;
	}

	// Triggers still use the touch function even with the new physics
	self->touch = Touch_Multi;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	if(self->spawnflags & TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = trigger_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = Use_Multi;
	}

	if(!Vec3IsZero(self->s.angles))
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

void Trigger_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->use = NULL;
}

void Trigger_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void Trigger_Sounds(edict_t *self)
{
	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 3)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else 
		self->noise_index = 0;
}

//----------------------------------------------------------------------
// One Time Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger.  Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS---------
delay   - Time to wait after activating before firing.
message - text string to display when activated
wait    - Seconds between triggerings. (.2 default)
sounds  - sound made when activating
1)	secret
2)	none
3)	large switch
*/
void SP_trigger_Multiple(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = G_UseTargets;

	Trigger_Sounds(self);

}

void trigger_enable(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void Use_Multi(edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	TriggerActivated(self);
}

//----------------------------------------------------------------------
// One Time Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_once (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it

sounds
 1)	secret
 2)	no sound
 3)	large switch

"message"	string to be displayed when triggered
*/

void SP_trigger_Once(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = G_UseTargets;

	self->wait = -1;

	Trigger_Sounds(self);

}

//----------------------------------------------------------------------
// Relay Trigger
//----------------------------------------------------------------------

void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator);

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void SP_trigger_Relay(edict_t *self)
{
	self->use = trigger_relay_use;
}

void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator)
{
	G_UseTargets (self, activator);
}

//----------------------------------------------------------------------
// Key Trigger
//----------------------------------------------------------------------

void trigger_key_use(edict_t *self, edict_t *other, edict_t *activator);

/*QUAKED trigger_puzzle (.5 .5 .5) (-8 -8 -8) (8 8 8)  NO_TEXT  NO_TAKE
A relay trigger that only fires it's targets if player has the proper puzzle item.
------KEYS--------------
NO_TEXT - won't generate the "You need..." text when triggered
NO_TAKE - don't take puzzle item from player inventory
------FIELDS------------
Use "item" to specify the required puzzle item, for example "key_data_cd"
*/
void SP_trigger_puzzle(edict_t *self)
{
	self->classID = CID_TRIGGER;

	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = P_FindItemByClassname (st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	self->use = trigger_key_use;
}

void trigger_key_use(edict_t *self, edict_t *other, edict_t *activator)
{
	int	index;
	edict_t *puzzle;

	if (!self->item)
		return;
	if (!activator->client)
		return;

	index = ITEM_INDEX(self->item);

	if (!activator->client->playerinfo.pers.inventory.Items[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		if (!(self->spawnflags & 1))
			gi.gamemsg_centerprintf (activator, self->item->msg_nouse);

		return;
	}

	// Clear out the puzzle piece from all clients.

	if (!(self->spawnflags & 2))
	{
		int		i;
		edict_t	*ent;

		if (coop->value)	// If COOP remove model from world if puzzle item is used.
		{
			puzzle = NULL;

			puzzle = G_Find(puzzle, FOFS(classname), (char *) self->item->classname);

			if (puzzle)
			{
				gi.sound(puzzle, CHAN_ITEM, gi.soundindex(self->item->pickup_sound), 1, ATTN_NORM, 0);

				gi.CreateEffect(NULL, FX_PICKUP, 0, puzzle->s.origin, "");

				puzzle->solid = SOLID_NOT;

				// Once picked up, the item is gone forever, so remove it's client effect(s).
				gi.RemoveEffects(&puzzle->s,0);

				// The persistent part is removed from the server here.
				G_SetToFree(puzzle);
			}
		}


		for (i=0 ; i<maxclients->value ; i++)
		{
			ent = g_edicts + 1 + i;
		
			if (!ent->inuse)
				continue;
			
			ent->client->playerinfo.pers.inventory.Items[index]=0;	
		}
	}

	gi.sound (self, CHAN_AUTO, gi.soundindex ("player/useobject.wav"), 2, ATTN_NORM, 0);

	G_UseTargets (self, activator);

	self->use = NULL;

	if (!(other->spawnflags & PUZZLE_DONT_REMOVE))	// Get rid of it.
	{
		G_SetToFree(other);
		activator->target_ent=activator->client->playerinfo.target_ent = 0;
	}
}

//----------------------------------------------------------------------
// Counter Trigger
//----------------------------------------------------------------------

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator);

#define TRIGGER_COUNTER_NOMESSAGE	1
/*QUAKED trigger_counter (.5 .5 .5) ? NOMESSAGE
Acts as an intermediary for an action that takes multiple inputs.

If NOMESSAGE is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/
void SP_trigger_Counter(edict_t *self)
{
	self->classID = CID_TRIGGER;

	self->wait = -1;

	if (!self->count)
	{
		self->count = 2;
	}

	self->use = trigger_counter_use;

	self->TriggerActivated = G_UseTargets;
}

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
	{
		return;
	}
	
	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & TRIGGER_COUNTER_NOMESSAGE))
		{
			gi.gamemsg_centerprintf(activator, (short)(self->count + GM_SEQCOMPLETE));
//			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	
	if (! (self->spawnflags & TRIGGER_COUNTER_NOMESSAGE))
	{
		gi.gamemsg_centerprintf(activator, GM_SEQCOMPLETE);
//		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

	self->activator = activator;

	TriggerActivated(self);
}

//----------------------------------------------------------------------
// Always Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_Always(edict_t *self)
{
	self->classID = CID_TRIGGER;

	// we must have some delay to make sure our use targets are present
	if (self->delay < 0.2)
	{
		self->delay = 0.2;
	}

	G_UseTargets(self, self);
}

//----------------------------------------------------------------------
// Player Use Item
//----------------------------------------------------------------------

void trigger_playerusepuzzle(edict_t *self, edict_t *activator)
{
	if (!(self->spawnflags & PUZZLE_SHOWNO_INVENTORY))
	{
		if(!strcmp(activator->classname, "player"))
		{
			activator->target_ent = self;
			self->activator = activator;
		}
	}
	else
		G_UseTargets(self,activator);

}

/*QUAKED trigger_playerusepuzzle (.5 .5 .5) ?  MONSTER NOT_PLAYER TRIGGERED ANY NO_INVENTORY DONT_REMOVE
Player can 'use' puzzle items within this entity.  Will remove itself after one use.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
NO_INVENTORY - don't show inventory bar, don't take puzzle piece
DONT_REMOVE - entity won't remove itself after one use
*/

void SP_trigger_PlayerUsePuzzle(edict_t *self)
{
	InitTrigger(self);

	self->wait = 1.0;
	self->TriggerActivated = trigger_playerusepuzzle;

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}

//----------------------------------------------------------------------
// Player Push Button Trigger
//----------------------------------------------------------------------

void trigger_playerpushbutton(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	if(!strcmp(other->classname, "player"))
	{
		other->target = self->target;
	}
}

/*QUAKED trigger_playerpushbutton (.5 .5 .5) ?
Triggers player to know he is near a button. 
*/
void SP_trigger_PlayerPushButton(edict_t *self)
{
	self->classID = CID_TRIGGER;

	self->wait = FRAMETIME;
	self->touch = trigger_playerpushbutton;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}

//----------------------------------------------------------------------
// Player Push Button Trigger
//----------------------------------------------------------------------

void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
void trigger_elevator_init (edict_t *self);

/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void SP_trigger_Elevator (edict_t *self)
{
	self->classID = CID_TRIGGER;

	self->think = trigger_elevator_init;
	self->nextthink = level.time + FRAMETIME;
}

void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator)
{
	void train_resume (edict_t *self);
	edict_t *target;

	if (self->movetarget->nextthink)
	{
//		gi.dprintf("elevator busy\n");
		return;
	}

	if (!other->pathtarget)
	{
#ifdef _DEVEL
		gi.dprintf("elevator used with no pathtarget\n");
#endif
		return;
	}

	target = G_PickTarget (other->pathtarget);
	if (!target)
	{
#ifdef _DEVEL
		gi.dprintf("elevator used with bad pathtarget: %s\n", other->pathtarget);
#endif
		return;
	}

	self->movetarget->target_ent = target;
	train_resume (self->movetarget);
}

void trigger_elevator_init (edict_t *self)
{
	if (!self->target)
	{
#ifdef _DEVEL
		gi.dprintf("trigger_elevator has no target\n");
#endif
		return;
	}
	self->movetarget = G_PickTarget (self->target);
	if (!self->movetarget)
	{
#ifdef _DEVEL
		gi.dprintf("trigger_elevator unable to find target %s\n", self->target);
#endif
		return;
	}
	if (strcmp(self->movetarget->classname, "func_train") != 0)
	{
#ifdef _DEVEL
		gi.dprintf("trigger_elevator target %s is not a train\n", self->target);
#endif
		return;
	}

	self->use = trigger_elevator_use;
	self->svflags = SVF_NOCLIENT;
}

//----------------------------------------------------------------------
// Suspend Trigger
//----------------------------------------------------------------------

void SuspendTrigger_Activated(edict_t *self, edict_t *activator);

/*QUAKED trigger_Deactivate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger, which posts a SUSPEND message to its target.
Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS-----------
delay - If set, the trigger waits this amount after activating before firing.
wait  - Seconds between triggerings. (.2 default)
message - text string displayed when triggered
*/
void SP_trigger_Deactivate(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = SuspendTrigger_Activated;
}

void SuspendTrigger_Activated(edict_t *self, edict_t *activator)
{
	edict_t		*t;

	assert(self->target);

//
// DeActivate all targets
//
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->msgHandler)
			QPostMessage(t, G_MSG_SUSPEND, PRI_ORDER, "f", self->time);
	}
}

//----------------------------------------------------------------------
// Unsuspend Trigger
//----------------------------------------------------------------------

void ActivateTrigger_Activated(edict_t *self, edict_t *activator);

/*QUAKED trigger_Activate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger, which posts a UNSUSPEND message to its target.
Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
------KEYS-----------
delay - If set, the trigger waits this amount after activating before firing.
wait  - Seconds between triggerings. (.2 default)
message - text string displayed when triggered
*/
void SP_trigger_Activate(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = ActivateTrigger_Activated;
}

void ActivateTrigger_Activated(edict_t *self, edict_t *activator)
{
	edict_t		*t;

	assert(self->target);

//
// Activate all targets
//
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->msgHandler)
			QPostMessage(t, G_MSG_UNSUSPEND, PRI_ORDER, "f", self->time);
	}
}

// make every active client out there change CD track.
void everyone_play_track(int track, int loop)
{
	int j;
	edict_t	*other;

	// play the track  - make sure everyone gets sent this over the next in the client messages
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		gi.changeCDtrack(other, track, loop);
	}
}

void choose_CDTrack_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// if we aren't a player, forget it
	if (!other->client)
		return;
	
	// make everyone play this track
	everyone_play_track(self->style, self->spawnflags);
	// kill this trigger
	G_SetToFree(self);
}

void choose_CDTrack_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// make everyone play this track
	everyone_play_track(self->style, self->spawnflags);
	// kill this trigger
	G_SetToFree(self);
}

/*QUAKED choose_CDTrack (.5 .5 .5) ? NO_LOOP
Variable sized repeatable trigger which chooses a CD track.
------KEYS-----------
style - # of CD track to play
NO_LOOP - allows you to set the track to play not to loop
*/
void SP_choose_CDTrack(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	self->use = choose_CDTrack_use;

	if (self->spawnflags & 1)
		self->spawnflags = FALSE;
	else
		self->spawnflags = TRUE;

	if(!self->wait)
	{
		self->wait = 0.2;
	}

	// Triggers still use the touch function even with the new physics
	self->touch = choose_CDTrack_touch;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel(self, self->model);
	gi.linkentity(self);

}
void M_Menu_Quit_f (void);

void trigger_quit_to_menu_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(!other->client)
		return;

	gi.AddCommandString ("menu_main\n");
}

void trigger_quit_to_menu_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(!activator->client)
		return;

	gi.AddCommandString ("menu_main\n");
}

/*QUAKED trigger_quit_to_menu (.5 .5 .5) ?
Player only, quits to menu
*/

void SP_trigger_quit_to_menu(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	self->touch = trigger_quit_to_menu_touch;
	self->use = trigger_quit_to_menu_use;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	self->solid = SOLID_TRIGGER;

	if(!Vec3IsZero(self->s.angles))
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

void mappercentage_use (edict_t *self, edict_t *other)
{
	if (!other->client)	// Only players use these
		return;

	other->client->ps.map_percentage = (byte) self->count;

	G_UseTargets(self, self);

#ifdef _DEVEL
	gi.dprintf("Map percentage updated to %d\n", (byte) self->count);
#endif
}

/*QUAKED trigger_mappercentage (0.3 0.1 0.6) ?  MONSTER NOT_PLAYER TRIGGERED ANY
When triggered it updates Player with the percentage of the level completed.
--------FLAGS----------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
----------KEYS---------------
count - amount of level completed
*/
void SP_trigger_mappercentage (edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = mappercentage_use;

	if (self->count > 100)
		self->count = 100;
}


void lightning_use (edict_t *self, edict_t *other)
{
	edict_t *target=NULL;
	byte	width, duration;

	width=self->style;
	if (width<1) width=6;
	duration=(byte)(self->delay*10);
	
	G_UseTargets(self, self);

	// Find the entities targeted by this entity.
	while ((target = G_Find (target, FOFS(targetname), self->target)) != NULL)
	{
		if (target->classname)
		{
			if (strcmp(target->classname, "info_notnull") == 0)
			{
				// Found another with this target.
				if (self->materialtype)	// Red lightning
					gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, self->s.origin, "vbb", target->s.origin, width, duration);
				else
					gi.CreateEffect(NULL, FX_LIGHTNING, 0, self->s.origin, "vbb", target->s.origin, width, duration);
			}
		}
	}

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 2;
		gi.sound (self, CHAN_AUTO, gi.soundindex ("world/lightningloop.wav"), 1, ATTN_NORM, 0);
	}
}
void lightning_go (edict_t *self, edict_t *other, edict_t *activator)
{
	lightning_use (self,other);
}

/*QUAKED trigger_lightning (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers a lightning bolt
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
-------KEYS--------------------
origin-- Starting point.
target-- Ending point entity.  
         There may be more than one with a given targetname.
delay-- (0-25.5) Sec. duration of lightning.  
         Leave this at zero for a normal strike
materialtype-- 0=blue, 1=red
style-- Width of bolt.  Red rain uses 6.
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_lightning (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = lightning_use;
	self->use = lightning_go;	// This is so a trigger_relay can use it.
}

void quake_quiet(edict_t *self)
{
	gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE,self->moveinfo.sound_end, 2, ATTN_NORM, 0);
	self->nextthink = level.time + FRAMETIME;
	self->think = G_FreeEdict;
}

void quake_use (edict_t *self, edict_t *other)
{
	edict_t *killsound;
	int count,time;

	if (self->touch_debounce_time > level.time)
		return;

	self->touch_debounce_time = level.time + self->wait;

	count = (byte)self->count;
	time = (byte) self->time * 10;

	gi.CreateEffect(&self->s, FX_QUAKE, CEF_BROADCAST, self->s.origin,"bbb",count,time,self->style);

	G_UseTargets(self, self);

	if (self->wait==-1)
	{
		self->touch = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}	 

	// Because nextthink is multi_use for a trigger I have to create a new entity with the sound
	// so I can then kill the sound at the right time
	killsound = G_Spawn();

	gi.sound (killsound, CHAN_NO_PHS_ADD+CHAN_VOICE,self->moveinfo.sound_middle, 2, ATTN_NORM, 0);
	VectorCopy(self->s.origin,killsound->s.origin);
	killsound->moveinfo.sound_end = self->moveinfo.sound_end;
	killsound->nextthink = level.time + self->time;
	killsound->think = quake_quiet;
}

/*QUAKED trigger_quake (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers an earth quake
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS---------------------
wait - amount of time until it will become active again (default 10). -1 makes it go away for ever.
count - max number of pixels to shake screen (default 20)
time - duration to the tenth of a second  (range 0 - 12.8) (default 2)
style - direction of shake
1 - SHAKE_LATERAL   
2 - SHAKE_VERTICAL  
4 - SHAKE_DEPTH     
7 - SHAKE_ALL_DIR  (default)
*/
void SP_trigger_quake (edict_t *self)
{
	if (!self->wait)
		self->wait = 10;

	self->moveinfo.sound_middle = gi.soundindex ("world/quake.wav");
	self->moveinfo.sound_end = gi.soundindex ("world/quakend.wav");

	InitTrigger(self);

	if (!self->count)	// Amount of shake
		self->count = 20;

	if (!self->time)	// Duration
		self->time = 2.0;

	if (!self->style)
		self->style = SHAKE_ALL_DIR;

	self->TriggerActivated = quake_use;
}

void trig_done(edict_t *self)
{
	self = self;
}

void mission_give_use (edict_t *self, edict_t *other)
{
	int				num, i;
	player_state_t	*ps;

	num = atoi(self->message);
	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		ps = &other->client->ps;
		if((ps->mission_num1 != num) && (ps->mission_num2 != num))
		{
			if (!ps->mission_num1)
			{
				ps->mission_num1 = num;
			}
			else
			{
				ps->mission_num2 = num;
			}
			gi.gamemsg_centerprintf(other, GM_NEWOBJ);
		}
	}
	G_UseTargets(self, self);
}

/*QUAKED trigger_mission_give (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Gives player(s) the current mission objectives
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
-------KEYS--------------------
message - number of line from strings.txt, put in objectives
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_mission_give (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = mission_give_use;
}

#define MISSION_TAKE1 16
#define MISSION_TAKE2 32

void mission_take_use (edict_t *self, edict_t *other)
{
	player_state_t		*ps;
	int					i;

	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		ps = &other->client->ps;

		if (self->spawnflags & MISSION_TAKE1)
			ps->mission_num1 = 0;

		if (self->spawnflags & MISSION_TAKE2)
			ps->mission_num2 = 0;
	}
	G_UseTargets(self, self);

}

/*QUAKED trigger_mission_take (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY TAKE1  TAKE2
Removes player(s) the current mission objectives
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
TAKE1 mission statement 1
TAKE2 mission statement 2
-------KEYS--------------------
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_mission_take (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = mission_take_use;
}

void ClipDistance_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	char temp[10];
	cvar_t *r_farclipdist;
	r_farclipdist = gi.cvar("r_farclipdist", FAR_CLIP_DIST, 0);

	// if we aren't a player, forget it
	if (!other->client)
		return;

	if (self->pain_debounce_time < level.time)
	{
		if (r_farclipdist->value == FAR_CLIP_DIST_VAL)
		{
			sprintf(temp, "%f", self->s.scale);
			gi.cvar_set("r_farclipdist", temp);

		}
		else
		{
			gi.cvar_set("r_farclipdist", FAR_CLIP_DIST);
		}
		self->pain_debounce_time = level.time + 0.5;
	}
}

/*QUAKED trigger_farclip (0.5 0.5 0.5) ? 
Allows the console var Farclip to be reset - this is a toggle function - if triggered
and far-clip is set to the default, it will be reset to the value passed in. If its the
value passed in, its reset to the default. Be aware that there must be no teleport
destinations within the area that has a reset far-clip.
-------SPAWN FLAGS-------------
-------KEYS--------------------
scale - distance to set far clip to. Default of farclip is 4096.0
*/
void SP_trigger_farclip (edict_t *self)
{
	InitTrigger(self);

	self->touch = ClipDistance_touch;
	self->solid = SOLID_TRIGGER;

}

void trigger_endgame_think(edict_t *self)
{
	gi.AddCommandString ("newcoopgame\n");

	G_SetToFree(self);
}

void Touch_endgame(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(self->count)
		return;

	self->count++;

	// If we aren't a player, forget it.

	if (!other->client)
		return;

	// Not valid on DM play.

	if (deathmatch->value)
		return;
	
	// Single player - just end, coop - restart if sv_loopcoop is set.

	if(gi.cvar_variablevalue("sv_loopcoop") && coop->value )
	{	
		int		i;
		edict_t	*ent;

		for(i=0;i<maxclients->value;i++)
		{
			if((ent=(&g_edicts[i+1]))->inuse)	
				gi.gamemsg_centerprintf(ent,GM_COOP_RESTARTING);
		}

		self->think=trigger_endgame_think;
		self->nextthink=level.time+1.0;
	}
	else
	{
		gi.AddCommandString ("endgame\n");
		
		G_SetToFree(self);
	}
}

void Use_endgame (edict_t *self, edict_t *other, edict_t *activator)
{
	if(self->count)
		return;

	self->count++;

	// Not valid on DM play.

	if (deathmatch->value)
		return;

	// Single player - just end, coop - restart if sv_loopcoop is set.

	if(gi.cvar_variablevalue("sv_loopcoop") && coop->value)
	{
		int		i;
		edict_t *ent;

		for(i=0;i<maxclients->value;i++)
		{
			if((ent=(&g_edicts[i+1]))->inuse)	
				gi.gamemsg_centerprintf(ent,GM_COOP_RESTARTING);
		}

		self->think=trigger_endgame_think;
		self->nextthink=level.time+1.0;
	}
	else
	{
		gi.AddCommandString ("endgame\n");

		G_SetToFree(self);
	}

}


/*QUAKED trigger_endgame (.5 .5 .5) ?
End game trigger. once used, game over
*/
void SP_trigger_endgame(edict_t *self)
{
	InitTrigger(self);
	self->touch = Touch_endgame;
	self->solid = SOLID_TRIGGER;
	self->use = Use_endgame;
	self->count=0;
}

//----------------------------------------------------------------------
// Player Push Lever Trigger
//----------------------------------------------------------------------

//void trigger_playerpushlever(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
void trigger_playerpushlever(edict_t *self, edict_t *other)
{
	if(!strcmp(other->classname, "player"))
	{
		other->target = self->target;
	}
}

/*QUAKED trigger_playerpushlever (.5 .5 .5) ?  x1 x2 TRIGGERED 
Triggers player to know he is near a lever.
*/
void SP_trigger_PlayerPushLever(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = trigger_playerpushlever;

}

//----------------------------------------------------------------------
// Player Push Lever Trigger
//----------------------------------------------------------------------
