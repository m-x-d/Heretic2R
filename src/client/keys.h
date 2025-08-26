//
// keys.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <stdio.h>
#include "q_Typedef.h"

// These are the key numbers that should be passed to Key_Event
#define K_TAB			9
#define K_ENTER			13
#define K_ESCAPE		27
#define K_SPACE			32
#define K_SLASH			47 //mxd

// Normal keys should be passed as lowercased ascii
#define K_BACKSPACE		127
#define K_UPARROW		128
#define K_DOWNARROW		129
#define K_LEFTARROW		130
#define K_RIGHTARROW	131

#define K_ALT			132
#define K_CTRL			133
#define K_SHIFT			134
#define K_F1			135
#define K_F2			136
#define K_F3			137
#define K_F4			138
#define K_F5			139
#define K_F6			140
#define K_F7			141
#define K_F8			142
#define K_F9			143
#define K_F10			144
#define K_F11			145
#define K_F12			146
#define K_INS			147
#define K_DEL			148
#define K_PGDN			149
#define K_PGUP			150
#define K_HOME			151
#define K_END			152

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define K_KP_ENTER		169
#define K_KP_INS		170
#define K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174
#define K_KP_NUMLOCK	175

#define K_CAPSLOCK		254
#define K_PAUSE			255

// Mouse buttons generate virtual keys
#define K_MOUSE1		200
#define K_MOUSE2		201
#define K_MOUSE3		202

// Joystick buttons
#define K_JOY1			203
#define K_JOY2			204
#define K_JOY3			205
#define K_JOY4			206

// Aux keys are for multi-buttoned joysticks to generate so they can use the normal binding process
#define K_AUX1			207
#define K_AUX2			208
#define K_AUX3			209
#define K_AUX4			210
#define K_AUX5			211
#define K_AUX6			212
#define K_AUX7			213
#define K_AUX8			214
#define K_AUX9			215
#define K_AUX10			216
#define K_AUX11			217
#define K_AUX12			218
#define K_AUX13			219
#define K_AUX14			220
#define K_AUX15			221
#define K_AUX16			222
#define K_AUX17			223
#define K_AUX18			224
#define K_AUX19			225
#define K_AUX20			226
#define K_AUX21			227
#define K_AUX22			228
#define K_AUX23			229
#define K_AUX24			230
#define K_AUX25			231
#define K_AUX26			232
#define K_AUX27			233
#define K_AUX28			234
#define K_AUX29			235
#define K_AUX30			236
#define K_AUX31			237
#define K_AUX32			238

#define K_MWHEELDOWN	239
#define K_MWHEELUP		240

extern char* keybindings[256];
extern char* keybindings_double[256]; // H2
extern int key_repeats[256];

extern qboolean keydown[256]; //mxd
extern int anykeydown;
extern char chat_buffer[];
extern int chat_bufferlen;
extern qboolean chat_team;

//mxd. Made global.
#define MAXCMDLINE		256

// Number of console command lines saved in history, must be a power of two, because we use & (NUM_KEY_LINES-1) instead of % so -1 wraps to NUM_KEY_LINES - 1.
#define NUM_KEY_LINES	32

extern char key_lines[NUM_KEY_LINES][MAXCMDLINE];
extern int edit_line;
extern int key_linepos;

extern void Key_Event(int key, qboolean down, uint time);
extern void Key_Init(void);
extern void Key_WriteBindings(FILE* f);
extern void Key_WriteBindings_Double(FILE* f); // H2
extern void Key_WriteConsoleHistory(void); // YQ2
extern void Key_ReadConsoleHistory(void); // YQ2
extern void Key_SetBinding(int keynum, const char* binding);
extern void Key_SetDoubleBinding(int keynum, const char* binding); // H2
extern void Key_ClearStates(void);