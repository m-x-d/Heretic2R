//
// q_Sprite.h
//
// Copyright 1998 Raven Software
//

#pragma once

enum SpriteType_s
{
	SPRITE_EDICT = -1,	// Comes from the server. Note this is a bit hacky, because the spritetype is unioned with the RootJoint (-1).
	SPRITE_STANDARD,	// Standard square sprite.
	SPRITE_DYNAMIC,		// Sprite with 4 variable verts (x,y scale and s,t); texture must be square.
	SPRITE_VARIABLE,	// Sprite with n variable verts (x,y scale and s,t); texture must be square.
	SPRITE_LINE,		// Long linear semi-oriented sprite with two verts (xyz start and end) and a width.
	NUM_SPRITE_TYPES,
};