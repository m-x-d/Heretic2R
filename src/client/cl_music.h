//
// cl_music.h
// 
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

void CDAudio_Init(void);
void CDAudio_Shutdown(void);
void CDAudio_Play(int track, qboolean looping);
void CDAudio_Stop(void);
void CDAudio_Activate(qboolean active);
void CDAudio_Update(void);
void CDAudio_RestartTrackIfNecessary(void);