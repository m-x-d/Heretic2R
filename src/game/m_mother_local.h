typedef enum AnimID_e
{	
	ANIM_PAIN,
	ANIM_STAND,	
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_GROWL1,
	SND_GROWL2,
	SND_PAIN,
	SND_GIB,

	NUM_SOUNDS
} SoundID_t;

extern animmove_t mother_move_pain; 
extern animmove_t mother_move_stand;

void mother_pause (edict_t *self);

