//
// g_PhysicsLocal.h -- Necessary forward declarations for g_Physics.c
//
// Copyright 2025 mxd
//

#pragma once

static void Physics_NoclipMove(edict_t* self);
static void Physics_FlyMove(edict_t* self);
static void Physics_StepMove(edict_t* self);
static void Physics_Push(edict_t* self);
static void Physics_ScriptAngular(edict_t* self);
static void MoveEntity_Bounce(edict_t* self, FormMove_t* form);
static void MoveEntity_Slide(edict_t* self);
static void ActivateTriggers(edict_t* self);
static void HandleForcefulCollision(edict_t* forcer, edict_t* forcee, const vec3_t move, qboolean forceful);
static void ApplyRotationalFriction(edict_t* self);
static void SetGroundEntFromTrace(edict_t* self, const trace_t* trace);