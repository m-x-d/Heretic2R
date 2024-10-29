#include "client.h"
#include "cl_strings.h"

#define MAX_MAPS		26

int coords_swamp[] = 
{
	196, 193,
	202, 193, 
	209, 197,
	213, 209,
	216, 216,
	222, 219
};

int coords_caves[] =
{
	243, 220,
	249, 220,
	256, 219,
	264, 224,
	268, 226,
	275, 228,
	281, 231,
	287, 234,
	293, 234
};

int coords_canyon[] =
{ 
	299, 238,
	300, 244,
	303, 250,
	300, 257,
	299, 266,
	300, 273,
	300, 281,
	306, 286,
	312, 292,
	317, 297,
	319, 305
};

int coords_mine1[] =
{
	387, 323,
	390, 317,
	393, 312,
	388, 306,
	383, 300,
	381, 293
};

int coords_mine2[] =
{
	387, 287,
	380, 280,
	383, 275,
	384, 268,
	389, 266,
	396, 269
};

level_map_info_t LevelMaps[MAX_MAPS] =
{
	0, 0,			NULL, NULL, 0, 0, 0, 0, 0, 0,												// level 0 - undefined
	0, 0,			NULL, NULL, 0, 0, 0, 0, 0, 0,												// level 0 - undefined
			        
	1, LMI_DRAW,	"book/back/b_worldmap.bk", "book/back/b_ssmap.bk",	0, 0,187, 191, 0, NULL,		// SSDocks
	1, 0,			"book/back/b_worldmap.bk", "book/back/b_ssmap.bk",	0, 0,0, 0, 0, NULL,			// SSWarehouse
	1, 0,			"book/back/b_worldmap.bk", "book/back/b_ssmap.bk",	0, 0,0, 0, 0, NULL,			// SSTown
	1, 0,			"book/back/b_worldmap.bk", "book/back/b_ssmap.bk",	0, 0,0, 0, 0, NULL,			// SSPalace

	2, LMI_PROGRESS, "book/back/b_worldmap.bk", NULL,	GM_M_DARKMIRE, 0,0, 0, 6, coords_swamp,	// Dmireswamp

	3, LMI_DRAW,	"book/back/b_worldmap.bk", "book/back/b_andmap.bk",	0, 0,232, 218, 0, NULL,		// Andhealer	
	3, 0,			"book/back/b_worldmap.bk", "book/back/b_andmap.bk",	0, 0,0, 0, 0, NULL,			// AndSlums
	3, 0,			"book/back/b_worldmap.bk", "book/back/b_andmap.bk",	0, 0,0, 0, 0, NULL,			// Andplaza
	3, 0,			"book/back/b_worldmap.bk", "book/back/b_andmap.bk",	0, 0,0, 0, 0, NULL,			// AndAcademic

	4, LMI_PROGRESS, "book/back/b_worldmap.bk", NULL,	GM_M_KELLCAVES, 0, 0, 0, 9, coords_caves,	// Kellcaves
	4, LMI_PROGRESS, "book/back/b_worldmap.bk", NULL,	GM_M_KATLITK, 0, 0, 0, 11, coords_canyon,	// Canyon

	5, LMI_DRAW,	"book/back/b_worldmap.bk", "book/back/b_hivemap.bk",	0, 0, 332, 307, 0, NULL,			// Hive1
	5, 0,			"book/back/b_worldmap.bk", "book/back/b_hivemap.bk",	0, 0, 0, 0, 0, NULL,				// Hive2
	5, 0,			"book/back/b_worldmap.bk", "book/back/b_hivemap.bk",	0, 0, 0, 0, 0, NULL,				// Gauntlet
	5, 0,			"book/back/b_worldmap.bk", "book/back/b_hivemap.bk",	0, 0, 0, 0, 0, NULL,				// Trialpit
	5, 0,			"book/back/b_worldmap.bk", "book/back/b_hivemap.bk",	0, 0, 0, 0, 0, NULL,				// Hivepriestess

	6, LMI_PROGRESS, "book/back/b_worldmap.bk", NULL, GM_M_MINES, 0,0, 0, 6, coords_mine1,	// OgleMine1
	6, LMI_PROGRESS, "book/back/b_worldmap.bk", NULL, GM_M_MINES, 0,	0, 0, 6, coords_mine2,			// OgleMine2
	6, 0,			"book/back/b_worldmap.bk", NULL,  GM_M_DUNGEON, 0,	0, 0, 0, NULL,					// Dungeon

	7, LMI_DRAW,	"book/back/b_worldmap.bk", NULL,  GM_M_CLOUD, 0,	405, 268, 0, NULL,		// Cloudhub
	7, 0,			"book/back/b_worldmap.bk", NULL,  GM_M_CLOUD, 0,	0, 0, 0, NULL,		// Cloudlabs
	7, 0,			"book/back/b_worldmap.bk", NULL,  GM_M_CLOUD, 0,	0, 0, 0, NULL,		// Cloudquarters
	7, 0,			"book/back/b_worldmap.bk", NULL,  GM_M_CLOUD, 0,	0, 0, 0, NULL,		// Cloudsanctum

	0, LMI_NODRAW,	"book/back/b_worldmap.bk", NULL, 0, 0,0, 0, 0, 0,												// Tutorial level
};

level_map_info_t *GetLMI()
{
	return(LevelMaps);
}

int GetLMIMax()
{
	return(MAX_MAPS);
}

// end
