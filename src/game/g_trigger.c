//
// g_trigger.c
//
// Copyright 1998 Raven Software
//

#include "cl_strings.h"
#include "g_DefaultMessageHandler.h"
#include "g_func_Train.h" //mxd
#include "g_playstats.h"
#include "Vector.h"
#include "g_local.h"

#define SF_TRIGGER_MONSTER		1
#define SF_TRIGGER_NOT_PLAYER	2
#define SF_TRIGGER_TRIGGERED	4
#define SF_TRIGGER_ANY			8

#define SF_PUZZLE_SHOW_NO_INVENTORY	16
#define SF_PUZZLE_DONT_REMOVE		32

void TriggerMultipleUse(edict_t *self, edict_t *other, edict_t *activator); //TODO: add to header.

#pragma region ========================== TriggerStaticsInit ==========================

static void TriggerDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Trigger_Deactivate' in original logic.
{
	self->solid = SOLID_NOT;
	self->use = NULL;
}

static void TriggerActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Trigger_Activate' in original logic.
{
	self->solid = SOLID_TRIGGER;
	self->use = TriggerMultipleUse;
	gi.linkentity(self);
}

void TriggerStaticsInit(void)
{
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_SUSPEND] = TriggerDeactivate;
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_UNSUSPEND] = TriggerActivate;
}

#pragma endregion

#pragma region ========================== Utility functions ==========================

// The wait time has passed, so set back up for another activation.
static void TriggerMultipleWaitThink(edict_t* self) //mxd. Named 'multi_wait' in original logic.
{
	self->think = NULL;

	if (self->activator != NULL)
		self->activator->target_ent = NULL;
}

// The trigger was just activated.
// self->activator should be set to the activator so it can be held through a delay, so wait for the delay time before firing.
static void TriggerActivated(edict_t* self)
{
	if (self->think != NULL)
		return; // Already been triggered.

	assert(self->TriggerActivated);
	self->TriggerActivated(self, self->activator);

	if (self->wait > 0.0f)
	{
		self->think = TriggerMultipleWaitThink;
		self->nextthink = level.time + self->wait;
	}
	else
	{
		self->touch = NULL;
		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;
	}
}

static void TriggerMultipleTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_Multi' in original logic.
{
	const qboolean is_player = (strcmp(other->classname, "player") == 0); //mxd
	const qboolean is_monster = (other->svflags & SVF_MONSTER); //mxd

	// Monsters or players can trigger it.
	if ((self->spawnflags & SF_TRIGGER_ANY) && !is_player && !is_monster)
		return;

	// Player can't trigger it.
	if ((self->spawnflags & SF_TRIGGER_NOT_PLAYER) && is_player)
		return;

	// Monsters can trigger it.
	if ((self->spawnflags & SF_TRIGGER_MONSTER) && !is_monster)
		return;

	if (Vec3NotZero(self->movedir))
	{
		vec3_t forward;
		AngleVectors(other->s.angles, forward, NULL, NULL);

		if (DotProduct(forward, self->movedir) < 0.0f)
			return;
	}

	self->activator = other;
	TriggerActivated(self);
}

void TriggerMultipleUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'Use_Multi' in original logic.
{
	self->activator = activator;
	TriggerActivated(self);
}

static void TriggerEnable(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_enable' in original logic.
{
	self->solid = SOLID_TRIGGER;
	self->use = TriggerMultipleUse;
	gi.linkentity(self);
}

void TriggerInit(edict_t* self) //mxd. Named 'InitTrigger' in original logic. //TODO: add to header.
{
	self->classID = CID_TRIGGER;
	self->msgHandler = DefaultMsgHandler;

	if (self->wait == 0.0f)
		self->wait = 0.2f;

	// Triggers still use the touch function even with the new physics.
	self->touch = TriggerMultipleTouch;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	if (self->spawnflags & SF_TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = TriggerEnable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = TriggerMultipleUse;
	}

	if (Vec3NotZero(self->s.angles))
		G_SetMovedir(self->s.angles, self->movedir);

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

static void SetTriggerSound(edict_t* self) //mxd. Named 'Trigger_Sounds' in original logic.
{
	if (self->sounds == 1)
		self->noise_index = gi.soundindex("misc/secret.wav"); //TODO: missing sound!
	else if (self->sounds == 3)
		self->noise_index = gi.soundindex("misc/talk.wav");
	else
		self->noise_index = 0;
}

#pragma endregion

#pragma region ========================== trigger_multiple ==========================

// QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
// Variable sized repeatable trigger. Must be targeted at one or more entities.
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// Variables:
// delay   - Time to wait after activating before firing.
// message - Text string to display when activated.
// wait    - Seconds between re-triggering. (default 0.2).
// sounds  - Sound made when activating:
//		1)	Secret.
//		2)	None.
//		3)	Chat message.
void SP_trigger_Multiple(edict_t* self)
{
	TriggerInit(self);
	SetTriggerSound(self);

	self->TriggerActivated = G_UseTargets;
}

#pragma endregion

#pragma region ========================== trigger_once ==========================

// QUAKED trigger_once (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
// Triggers once, then removes itself.
// You must set the key "target" to the name of another object in the level that has a matching "targetname".
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// Variables:
// delay	- Time to wait after activating before firing.
// message	- Text string to display when activated.
// sounds	- Sound made when activating:
//		1)	Secret.
//		2)	None.
//		3)	Chat message.
void SP_trigger_Once(edict_t* self)
{
	TriggerInit(self);
	SetTriggerSound(self);

	self->TriggerActivated = G_UseTargets;
	self->wait = -1.0f;
}

#pragma endregion

#pragma region ========================== trigger_relay ==========================

static void TriggerRelayUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_relay_use' in original logic.
{
	G_UseTargets(self, activator);
}

// QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
// This fixed size trigger cannot be touched, it can only be fired by other events.
void SP_trigger_Relay(edict_t* self)
{
	self->use = TriggerRelayUse;
}

#pragma endregion

#pragma region ========================== trigger_puzzle ==========================

#define SF_NO_TEXT	1 //mxd
#define SF_NO_TAKE	2 //mxd

static void TriggerPuzzleUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_key_use' in original logic.
{
	if (self->item == NULL || activator->client == NULL)
		return;

	const int item_index = ITEM_INDEX(self->item);

	// Does activating player have required item?
	if (activator->client->playerinfo.pers.inventory.Items[item_index] == 0)
	{
		if (level.time >= self->touch_debounce_time)
		{
			self->touch_debounce_time = level.time + 5.0f;

			if (!(self->spawnflags & SF_NO_TEXT))
				gi.gamemsg_centerprintf(activator, self->item->msg_nouse);
		}

		return;
	}

	// Clear out the puzzle piece from all clients.
	if (!(self->spawnflags & SF_NO_TAKE))
	{
		if (COOP) // If COOP, remove model from world if puzzle item is used.
		{
			edict_t* puzzle = NULL;
			puzzle = G_Find(puzzle, FOFS(classname), self->item->classname);

			if (puzzle != NULL)
			{
				gi.sound(puzzle, CHAN_ITEM, gi.soundindex(self->item->pickup_sound), 1.0f, ATTN_NORM, 0.0f);
				gi.CreateEffect(NULL, FX_PICKUP, 0, puzzle->s.origin, "");
				puzzle->solid = SOLID_NOT;

				// Once picked up, the item is gone forever, so remove it's client effect(s).
				gi.RemoveEffects(&puzzle->s, 0);

				// The persistent part is removed from the server here.
				G_SetToFree(puzzle);
			}
		}

		for (int i = 0; i < MAXCLIENTS; i++)
		{
			const edict_t* ent = &g_edicts[i + 1];

			if (ent->inuse)
				ent->client->playerinfo.pers.inventory.Items[item_index] = 0;
		}
	}

	gi.sound(self, CHAN_AUTO, gi.soundindex("player/useobject.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

	G_UseTargets(self, activator);
	self->use = NULL;

	if (!(other->spawnflags & SF_PUZZLE_DONT_REMOVE)) // Get rid of it.
	{
		G_SetToFree(other);

		activator->target_ent = NULL;
		activator->client->playerinfo.target_ent = NULL;
	}
}

// QUAKED trigger_puzzle (.5 .5 .5) (-8 -8 -8) (8 8 8) NO_TEXT NO_TAKE
// A relay trigger that only fires it's targets if player has the proper puzzle item.
// Spawnflags:
// NO_TEXT - Won't generate the "You need..." text when triggered.
// NO_TAKE - Don't take puzzle item from player inventory.
// Variables:
// item - Classname of the required puzzle item, for example "key_data_cd".
void SP_trigger_puzzle(edict_t* self)
{
	self->classID = CID_TRIGGER;

	if (st.item == NULL)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}

	self->item = P_FindItemByClassname(st.item);

	if (self->item == NULL)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (self->target == NULL)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	self->use = TriggerPuzzleUse;
}

#pragma endregion

#pragma region ========================== trigger_counter ==========================

#define SF_NOMESSAGE	1

static void TriggerCounterUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_counter_use' in original logic.
{
	if (self->count == 0)
		return;

	self->count--;

	if (!(self->spawnflags & SF_NOMESSAGE))
		gi.gamemsg_centerprintf(activator, (short)(GM_SEQCOMPLETE + self->count));

	if (self->count == 0)
	{
		self->activator = activator;
		TriggerActivated(self);
	}
}

// QUAKED trigger_counter (.5 .5 .5) ? NOMESSAGE
// Acts as an intermediary for an action that takes multiple inputs.
// Spawnflags:
// NOMESSAGE - When not set, it will print "Find N more" when triggered and "You completed the sequence!" when finished.
// Variables:
// count - After the trigger_counter has been triggered this many times (default 2), it will fire all of it's targets and remove itself.
void SP_trigger_Counter(edict_t* self)
{
	self->classID = CID_TRIGGER;
	self->wait = -1.0f;

	if (self->count == 0) //TODO: will print unrelated messages when count > 11...
		self->count = 2;

	self->use = TriggerCounterUse;
	self->TriggerActivated = G_UseTargets;
}

#pragma endregion

#pragma region ========================== trigger_always ==========================

// QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
// This trigger will always fire. It is activated by the world.
void SP_trigger_Always(edict_t* self)
{
	self->classID = CID_TRIGGER;
	self->delay = max(0.2f, self->delay); // We must have some delay to make sure our use targets are present.

	G_UseTargets(self, self);
}

#pragma endregion

#pragma region ========================== trigger_playerusepuzzle ==========================

static void TriggerPlayerUsePuzzleActivated(edict_t* self, edict_t* activator) //mxd. Named 'trigger_playerusepuzzle' in original logic.
{
	if (self->spawnflags & SF_PUZZLE_SHOW_NO_INVENTORY)
	{
		G_UseTargets(self, activator);
	}
	else if (strcmp(activator->classname, "player") == 0)
	{
		activator->target_ent = self;
		self->activator = activator;
	}
}

// QUAKED trigger_playerusepuzzle (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY NO_INVENTORY DONT_REMOVE
// Player can 'use' puzzle items within this entity. Will remove itself after one use.
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// NO_INVENTORY	- Don't show inventory bar, don't take puzzle piece.
// DONT_REMOVE	- Entity won't remove itself after one use.
void SP_trigger_PlayerUsePuzzle(edict_t* self)
{
	TriggerInit(self);

	self->wait = 1.0f;
	self->TriggerActivated = TriggerPlayerUsePuzzleActivated;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== trigger_playerpushbutton ==========================

static void TriggerPlayerPushButtonTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'trigger_playerpushbutton' in original logic.
{
	if (strcmp(other->classname, "player") == 0)
		other->target = self->target;
}

// QUAKED trigger_playerpushbutton (.5 .5 .5) ?
// Triggers player to know he is near a button.
void SP_trigger_PlayerPushButton(edict_t* self)
{
	self->classID = CID_TRIGGER;

	self->wait = FRAMETIME;
	self->touch = TriggerPlayerPushButtonTouch;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== trigger_elevator ==========================

static void TriggerElevatorUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_elevator_use' in original logic.
{
	if (self->movetarget->nextthink > 0.0f) // Elevator busy...
		return;

	if (other->pathtarget == NULL)
	{
		gi.dprintf("trigger_elevator at %s used with no pathtarget\n", vtos(self->s.origin)); //mxd. Print trigger coords.
		return;
	}

	edict_t* target = G_PickTarget(other->pathtarget);

	if (target == NULL)
	{
		gi.dprintf("trigger_elevator at %s used with bad pathtarget: %s\n", vtos(self->s.origin), other->pathtarget); //mxd. Print trigger coords.
		return;
	}

	self->movetarget->target_ent = target;
	FuncTrainResume(self->movetarget);
}

static void TriggerElevatorInitThink(edict_t* self) //mxd. Named 'trigger_elevator_init' in original logic.
{
	if (self->target == NULL)
	{
		gi.dprintf("trigger_elevator at %s has no target\n", vtos(self->s.origin)); //mxd. Print trigger coords.
		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink().

		return;
	}

	self->movetarget = G_PickTarget(self->target);

	if (self->movetarget == NULL)
	{
		gi.dprintf("trigger_elevator at %s unable to find target %s\n", vtos(self->s.origin), self->target); //mxd. Print trigger coords.
		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink().

		return;
	}

	if (strcmp(self->movetarget->classname, "func_train") != 0)
	{
		gi.dprintf("trigger_elevator at %s: target %s is not a func_train\n", vtos(self->s.origin), self->target); //mxd. Print trigger coords.
		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink().

		return;
	}

	self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink().
	self->use = TriggerElevatorUse;
	self->svflags = SVF_NOCLIENT;
}

// QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
// Special func_train trigger (or not trigger, if func_train is moving). Not used in any H2 maps -- mxd.
void SP_trigger_Elevator(edict_t* self)
{
	self->classID = CID_TRIGGER;
	self->think = TriggerElevatorInitThink;
	self->nextthink = level.time + FRAMETIME;
}

#pragma endregion

#pragma region ========================== trigger_deactivate ==========================

static void TriggerDeactivateActivated(edict_t* self, edict_t* activator) //mxd. Named 'SuspendTrigger_Activated' in original logic.
{
	assert(self->target);

	// Deactivate all targets.
	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
		if (target->msgHandler != NULL)
			QPostMessage(target, G_MSG_SUSPEND, PRI_ORDER, "f", self->time);
}

// QUAKED trigger_Deactivate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
// Variable sized repeatable trigger, which posts a SUSPEND message to its target. Must be targeted at one or more entities.
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// Variables:
// delay	- Time to wait after activating before firing.
// message	- Text string to display when activated.
// wait		- Seconds between re-triggering. (default 0.2).
void SP_trigger_Deactivate(edict_t* self)
{
	TriggerInit(self);
	self->TriggerActivated = TriggerDeactivateActivated;
}

#pragma endregion

#pragma region ========================== trigger_activate ==========================

static void TriggerActivateActivated(edict_t* self, edict_t* activator) //mxd. Named 'ActivateTrigger_Activated' in original logic.
{
	assert(self->target);

	// Activate all targets.
	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
		if (target->msgHandler != NULL)
			QPostMessage(target, G_MSG_UNSUSPEND, PRI_ORDER, "f", self->time);
}

// QUAKED trigger_Activate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
// Variable sized repeatable trigger, which posts a UNSUSPEND message to its target. Must be targeted at one or more entities.
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// Variables:
// delay	- Time to wait after activating before firing.
// message	- Text string to display when activated.
// wait		- Seconds between re-triggering. (default 0.2).
void SP_trigger_Activate(edict_t* self)
{
	TriggerInit(self);
	self->TriggerActivated = TriggerActivateActivated;
}

#pragma endregion

#pragma region ========================== choose_CDTrack ==========================

#define SF_NO_LOOP	1 //mxd

// Make every active client out there change CD track.
static void ChooseCDTrackPlayTrack(const int track, const int loop) //mxd. Named 'everyone_play_track' in original logic.
{
	for (int i = 1; i <= game.maxclients; i++)
	{
		const edict_t* cl = &g_edicts[i];

		if (cl->inuse && cl->client != NULL)
			gi.changeCDtrack(cl, track, loop);
	}
}

static void ChooseCDTrackUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'choose_CDTrack_use' in original logic.
{
	ChooseCDTrackPlayTrack(self->style, self->spawnflags);
	G_SetToFree(self); // Kill this trigger. //TODO: don't kill this trigger? Advertised as repeatable in entity description.
}

static void ChooseCDTrackTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'choose_CDTrack_touch' in original logic.
{
	// If we aren't a player, forget it.
	if (other->client != NULL)
		ChooseCDTrackUse(self, other, other);
}

// QUAKED choose_CDTrack (.5 .5 .5) ? NO_LOOP
// Variable sized repeatable trigger which chooses a CD track.
// Spawnflags:
// NO_LOOP - Allows you to set the track to play not to loop.
// Variables:
// style - # of CD track to play.
void SP_choose_CDTrack(edict_t* self)
{
	self->classID = CID_TRIGGER;
	self->msgHandler = DefaultMsgHandler;
	self->spawnflags = ((self->spawnflags & SF_NO_LOOP) ? FALSE : TRUE);

	if (self->wait == 0.0f)
		self->wait = 0.2f;

	// Triggers still use the touch function even with the new physics.
	self->touch = ChooseCDTrackTouch;
	self->use = ChooseCDTrackUse;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== trigger_quit_to_menu ==========================

static void TriggerQuitToMenuTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'trigger_quit_to_menu_touch' in original logic.
{
	if (other->client != NULL)
		gi.AddCommandString("menu_main\n");
}

static void TriggerQuitToMenuUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_quit_to_menu_use' in original logic.
{
	if (activator->client != NULL)
		gi.AddCommandString("menu_main\n");
}

// QUAKED trigger_quit_to_menu (.5 .5 .5) ?
// Player only, quits to menu.
void SP_trigger_quit_to_menu(edict_t* self)
{
	self->classID = CID_TRIGGER;
	self->msgHandler = DefaultMsgHandler;

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->touch = TriggerQuitToMenuTouch;
	self->use = TriggerQuitToMenuUse;

	if (Vec3NotZero(self->s.angles))
		G_SetMovedir(self->s.angles, self->movedir);

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== trigger_mappercentage ==========================

static void TriggerMappercentageUse(edict_t* self, edict_t* other) //mxd. Named 'mappercentage_use' in original logic.
{
	if (other->client != NULL) // Only players use these.
	{
		other->client->ps.map_percentage = (byte)self->count;
		G_UseTargets(self, self);
	}
}

// QUAKED trigger_mappercentage (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
// When triggered it updates Player with the percentage of the level completed.
// Spawnflags:
// MONSTER		- Only a monster will trigger it.
// NOT_PLAYER	- Can't be triggered by player.
// TRIGGERED	- Starts trigger deactivated.
// ANY			- Anything can activate it.
// Variables:
// count - Amount of level completed.
void SP_trigger_mappercentage(edict_t* self)
{
	TriggerInit(self);

	self->count = ClampI(self->count, 0, 100); //mxd. Clamp lower bound too.
	self->TriggerActivated = TriggerMappercentageUse;
}

#pragma endregion

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
	TriggerInit(self);

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

	TriggerInit(self);

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
	TriggerInit(self);

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
	TriggerInit(self);

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
	TriggerInit(self);

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
	TriggerInit(self);
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
	TriggerInit(self);

	self->TriggerActivated = trigger_playerpushlever;

}

//----------------------------------------------------------------------
// Player Push Lever Trigger
//----------------------------------------------------------------------
