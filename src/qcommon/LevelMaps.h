#pragma once

// Level progression map only needs to work on the dmireswamp, kellcaves, canyon, oglemine1, and oglemine2.
// The other levels should simply highlight a city on the map.

// 1st unit
// SSdocks, SSwarehouse, SStown, SSpalace.
// The player will acquire the hellstaff, the array, and the ring of repulsion.

// 2nd unit
// Dmireswamp.
// No new weapon.

// 3rd unit
// Andhealer, Andplaza, Andacademic, Andslums.
// The player will acquire the Red Rain Bow, Polymorph, Sphere of Annihilation.

// 4th unit
// Kellcaves, Canyon.
// The player will acquire the Phoenix Bow.

// 5th unit
// Hive1, Hive2, Gauntlet, Hivetrialpit, Hivepriestess.
// The player will acquire the Meteor Barrier, the Firewall, and the Mace Ball.

// 6th unit
// Oglemine1, Oglemine2, Dungeon.
// The player will acquire the Teleport Spell.

// 7th unit
// Cloudhub, Cloudlabs, Cloudquarters, Cloudsanctum.
// All weapons have been acquired at this point.

#define LMI_PROGRESS	1
#define LMI_DRAW		2
#define LMI_NODRAW		4

typedef struct level_map_info_s
{
	int		hub;
	int		flags;
	char	*world_map;
	char	*city_map;
	short	message;
	short	pad;
	int		x, y;
	int		count;
	int		*dot_coords;
} level_map_info_t;

// end