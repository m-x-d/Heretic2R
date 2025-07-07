//
// Hunk.h -- Large block stack allocation routines.
//
// Copyright 1998 Raven Software
//

#pragma once

extern void* Hunk_Begin(int maxsize);
extern void* Hunk_Alloc(int size);
extern void Hunk_Free(void* buf);
extern int Hunk_End(void);