// vid.h -- video driver defs

#define	MIN_GAMMA	0.1			// These also need to be defined in gl_local.h
#define MAX_GAMMA	4.0

#define DEF_WIDTH	640
#define DEF_HEIGHT	480

typedef struct vrect_s
{
	int				x,y,width,height;
} vrect_t;

typedef struct
{
	int			 	width;		
	int				height;
	byte			*pixels;
} viddef_t;

extern	viddef_t	viddef;				// global video state

// Video module initialisation etc
void	VID_Init (void);
void	VID_Shutdown (void);
void	VID_CheckChanges (void);

void	VID_MenuInit( void );
void	VID_PreMenuInit( void );
void	__stdcall VID_MenuDraw();
char	* __stdcall VID_MenuKey( int );

// end