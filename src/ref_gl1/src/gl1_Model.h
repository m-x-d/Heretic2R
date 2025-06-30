//
// gl1_Model.h
//
// Copyright 1998 Raven Software
//

#pragma once

extern void Mod_Init(void);

extern void Mod_Modellist_f(void);

extern void R_BeginRegistration(const char* model);
extern struct model_s* R_RegisterModel(const char* name);
extern void R_EndRegistration(void);