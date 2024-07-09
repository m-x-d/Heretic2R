#ifndef Q_SPRITE_H
#define Q_SPRITE_H

typedef enum SpriteType_s
{
	SPRITE_EDICT = -1,	// Comes from the server.  Note this is a bit hacky, because the spritetype is unioned with the RootJoint (-1)
	SPRITE_STANDARD,	// standard square sprite
	SPRITE_DYNAMIC,		// sprite with 4 variable verts (x,y scale and s,t); texture must be square
	SPRITE_VARIABLE,	// sprite with n variable verts (x,y scale and s,t); texture must be square
	SPRITE_LINE,		// Long linear semi-oriented sprite with two verts (xyz start and end) and a width
	NUM_SPRITE_TYPES,
};


#endif