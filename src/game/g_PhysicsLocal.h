//
// g_PhysicsLocal.h
//
// Copyright 2025 mxd
//

#pragma once

#define FRICTION_STOPSPEED			100.0f
#define FRICTION_SURFACE			6.0f

static void Physics_None(edict_t* self);
static void Physics_NoclipMove(edict_t* self);
static void Physics_FlyMove(edict_t* self);
static void Physics_StepMove(edict_t* self);
static void Physics_Push(edict_t* self);
static void Physics_ScriptAngular(edict_t* self);
static void MoveEntity_Bounce(edict_t* self, FormMove_t* form_move); //mxd
static void MoveEntity_Slide(edict_t* self); //mxd
static void ActivateTriggers(edict_t* self); //mxd
static void HandleForcefulCollision(edict_t* forcer, edict_t* forcee, const vec3_t move, qboolean forceful); //mxd
static void ApplyRotationalFriction(edict_t* self); //mxd
static void SetGroundEntFromTrace(edict_t* self, const trace_t* trace); //mxd