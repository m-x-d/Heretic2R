//
// p_items.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_types.h"
#include "p_weapon.h"
#include "p_item.h"
#include "g_items.h"
#include "p_anims.h"
#include "p_anim_data.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "items.h"
#include "g_local.h"
#include "g_itemstats.h"
#include "cl_strings.h"

PLAYER_API int		p_num_items=0;
PLAYER_API gitem_t	*p_itemlist=NULL;

// ************************************************************************************************
// itemlist
// --------
// The complete list of all items that may be picked up / dropped / used etc. by players.
// ************************************************************************************************

static gitem_t itemlist[]=
{
	// Leave index 0 empty.

	{
		NULL
	},

	// =============================================================================================

	// Weapon items.

	{
		"Weapon_SwordStaff",					// Spawnname (char *)
		"staff",								// Pickup name (char *)
		0,										// pickup message
		0,										// can`t use message
		NULL,	 								// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when used
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_WEAPON,								// flags
		NULL,									// void * ?
		ITEM_WEAPON_SWORDSTAFF,					// tag ?
		"icons/i_staff.m8",				// Icon name (char *)
	},

	{
		"Weapon_FlyingFist", 					// Spawnname
		"fball",								// Pickup name (char *)       
		0,										// pickup message
		GM_NOFLYINGFIST,						// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREBALL,							// Player animation sequence to engage when used
		ASEQ_WFIREBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		AMMO_USE_FIREBALL,						// Ammo/ammo use per shot     
		"Off-mana",								// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_WEAPON_FLYINGFIST,					// tag ?                      
 		"icons/i_fball.m8",   			// Icon name (char *)         
	},

	{
		"item_weapon_hellstaff",				// Spawnname
		"hell",									// Pickup name (char *)       
		GM_HELLSTAFF,							// pickup message
		GM_NOHELLORBS,										// can`t use message
		NULL,	 								// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WHELL_GO,							// Player animation sequence to engage when used
		ASEQ_WHELL_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = infinite)  
		2,										// Number of digits to display
		AMMO_USE_HELLSTAFF,						// Ammo/ammo use per shot     
		"Hell-staff-ammo",						// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_WEAPON_HELLSTAFF,					// tag ?                      
		"icons/i_hell.m8",				// Icon name (char *)         
	},

	{	
		"item_weapon_magicmissile",				// Spawnname
		"array",								// Pickup name (char *)       
		GM_FORCEBLAST, 							// pickup message
		GM_NOFORCE,										// can`t use message
		NULL,		   							// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WBLAST,							// Player animation sequence to engage when used
		ASEQ_WARRAY,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		AMMO_USE_MAGICMISSILE,					// Ammo/ammo use per shot     
		"Off-mana",								// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_WEAPON_MAGICMISSILE,				// tag ?                      
		"icons/i_array.m8",				// Icon name (char *)         
	},

	{
		"item_weapon_redrain_bow", 				// Spawnname
		"rain",									// Pickup name (char *)       
		GM_STORMBOW,										// pickup message
		GM_NOSTORMBOW,										// can`t use message
		NULL,	 								// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		2,										// Number of digits to display
		AMMO_USE_REDRAIN,						// Ammo/ammo use per shot     
		"Red-Rain-Arrows",						// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
		NULL,									// void * ? 
		ITEM_WEAPON_REDRAINBOW,					// tag ?                      
		"icons/i_rain.m8",				// Icon name (char *)         
	},

	{
		"item_weapon_firewall",					// Spawnname
		"fwall",								// Pickup name (char *)       
		GM_FIREWALL,										// pickup message
		GM_NOFIREWALL,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		EF_ROTATE,								// world model flags          
		ASEQ_WFIREWALL,							// Player animation sequence to engage when used
		ASEQ_WFIREWALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		AMMO_USE_FIREWALL,						// Ammo/ammo use per shot     
		"Off-mana",								// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_WEAPON_FIREWALL,					// tag ?                      
 		"icons/i_fwall.m8",					// Icon name (char *)         
	},

	{
		"item_weapon_phoenixbow", 				// Spawnname
		"phoen",							// Pickup name (char *)       
		GM_PHOENIX,										// pickup message
		GM_NOPHOENIX,										// can`t use message
		NULL,	 								// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		0, 0, 0,								// Bounding box mins
		0, 0, 0,								// Bounding box maxs
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		2,										// Number of digits to display
		AMMO_USE_PHOENIX,						// Ammo/ammo use per shot     
		"Phoenix-Arrows",						// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
		NULL,									// void * ? 
		ITEM_WEAPON_PHOENIXBOW,					// tag ?                      
		"icons/i_phoen.m8", 				// Icon name (char *)         
	},

	{
		"item_weapon_sphereofannihilation",		// Spawnname
		"sphere",								// Pickup name (char *)       
		GM_SPHERE,										// pickup message
		GM_NOSPHERE,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when used
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		AMMO_USE_SPHERE,						// Ammo/ammo use per shot     
		"Off-mana",								// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_WEAPON_SPHEREOFANNIHILATION,		// tag ?                      
 		"icons/i_sphere.m8",			// Icon name (char *)         
	},

	{
		"item_weapon_maceballs",				// Spawnname
		"mace",									// Pickup name (char *)       
		GM_IRONDOOM,										// pickup message
		GM_NOIRONDOOM,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRIPPER,							// Player animation sequence to engage when used
		ASEQ_WBIGBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		AMMO_USE_MACEBALL,						// Ammo/ammo use per shot     
		"Off-mana",								// Ammo (char *)              
		IT_WEAPON | IT_OFFENSE,					// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_WEAPON_MACEBALLS,					// tag ?                      
 		"icons/i_mace.m8",				// Icon name (char *)         
	},

	{
		"item_defense_powerup",					// Spawnname
		"powerup",								// Pickup name (char *)       
		GM_TOME, 								// pickup message
		GM_NOTOME,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_POWERUP,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
		NULL,									// void * ?                   
		ITEM_DEFENSE_POWERUP,					// tag ?                      
 		"icons/i_tome.m8",				// Icon name (char *)         
	},

	{
		"item_defense_ringofrepulsion",			// Spawnname
		"ring",									// Pickup name (char *)       
		GM_RING,										// pickup message
		GM_NORING,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_RING,							// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
		NULL,									// void * ?                   
		ITEM_DEFENSE_REPULSION,					// tag ?                      
 		"icons/i_ring.m8",			// Icon name (char *)         
	},

	{
		"item_defense_shield",					// Spawnname
		"lshield",								// Pickup name (char *)       
		GM_SHIELD,										// pickup message
		GM_NOSHIELD,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,	 								// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_SHIELD,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_DEFENSE_SHIELD,					// tag ?                      
 		"icons/i_shield.m8",			// Icon name (char *)         
	},

	{
		"item_defense_teleport",				// Spawnname
		"tele",									// Pickup name (char *)       
		GM_TELEPORT,										// pickup message
		GM_NOTELEPORT,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,  									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_TELEPORT,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
		NULL,									// void * ?                   
		ITEM_DEFENSE_TELEPORT,					// tag ?                      
 		"icons/i_tele.m8",					// Icon name (char *)         
	},

	{
		"item_defense_polymorph",			    // Spawnname
		"morph",						    // Pickup name (char *)       
		GM_MORPH,										// pickup message
		GM_NOMORPH,										// can`t use message
		NULL,								    // Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_POLYMORPH,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
		NULL,									// void * ?                   
		ITEM_DEFENSE_POLYMORPH,					// tag ?                      
 		"icons/i_morph.m8",					// Icon name (char *)         
	},

	{
		"item_defense_meteorbarrier",			// Spawnname
		"meteor",								// Pickup name (char *)       
		GM_METEOR,										// pickup message
		GM_NOMETEOR,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_METEORS,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_DEFENSE_METEORBARRIER,				// tag ?                      
 		"icons/i_meteor.m8",			// Icon name (char *)         
	},


	// =============================================================================================

	// Ammo items.

	{
		"item_mana_offensive_half",				// Spawnname
		"Off-mana",								// Pickup name (char *)       
		GM_OFFMANAS,										// pickup message
		0,
		NULL, 									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/picup.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		HALF_OFF_MANA,							// Ammo/ammo use per shot     
		NULL,									// Ammo (char *)              
		IT_AMMO | IT_OFFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_AMMO_MANA_OFFENSIVE_HALF,			// tag ?                      
		NULL,									// Icon name (char *)         
	},

	{
		"item_mana_offensive_full",				// Spawnname
		"Off-mana",								// Pickup name (char *)       
		GM_OFFMANAB,										// pickup message
		0,
		NULL, 									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/picup.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		FULL_OFF_MANA,							// Ammo/ammo use per shot     
		NULL,									// Ammo (char *)              
		IT_AMMO | IT_OFFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_AMMO_MANA_OFFENSIVE_FULL,			// tag ?                      
		NULL,									// Icon name (char *)         
	},

	{
		"item_mana_defensive_half",				// Spawnname
		"Def-mana",								// Pickup name (char *)       
		GM_DEFMANAS,							// pickup message
		0,
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/picup.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		HALF_DEF_MANA,							// Ammo/ammo use per shot     
		NULL,									// Ammo (char *)              
		IT_AMMO | IT_DEFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_AMMO_MANA_DEFENSIVE_HALF,			// tag ?                      
		NULL,									// Icon name (char *)         
	},

	{
		"item_mana_defensive_full",				// Spawnname
		"Def-mana",								// Pickup name (char *)       
		GM_DEFMANAB,							// pickup message
		0,
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/picup.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		0,										// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		FULL_DEF_MANA,							// Ammo/ammo use per shot     
		NULL,									// Ammo (char *)              
		IT_AMMO | IT_DEFENSE,					// flags                      
		NULL,									// void * ?                   
		ITEM_AMMO_MANA_DEFENSIVE_FULL,			// tag ?                      
		NULL,									// Icon name (char *)         
	},


	{
		"item_mana_combo_quarter",					  // Spawnname
		"Def-mana",									  // Pickup name (char *)       
		GM_COMBMANAS,										// pickup message
		0,
		NULL,								  // Pickup (f)                 
		NULL,										  // Use (f)                    
		NULL,										  // Drop	(f)                   
		NULL,										  // Think (f)                  
		"player/picup.wav",							  // Pickup sound (char *)      
		NULL,										  // world model (char *)       
		0,											  // world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)  
		0,											  // Number of digits to display
		HALF_COMBO_MANA,							  // Ammo/ammo use per shot     
		NULL,										  // Ammo (char *)              
		IT_AMMO,									  // flags                      
		NULL,										  // void * ?                   
		ITEM_AMMO_MANA_COMBO_QUARTER,				  // tag ?                      
		NULL,										  // Icon name (char *)         
	},

	{
		"item_mana_combo_half",					  // Spawnname
		"Def-mana",									  // Pickup name (char *)       
		GM_COMBMANAB,										// pickup message
		0,
		NULL,								  // Pickup (f)                 
		NULL,										  // Use (f)                    
		NULL,										  // Drop	(f)                   
		NULL,										  // Think (f)                  
		"player/picup.wav",							  // Pickup sound (char *)      
		NULL,										  // world model (char *)       
		0,											  // world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)  
		0,											  // Number of digits to display
		FULL_COMBO_MANA,							  // Ammo/ammo use per shot     
		NULL,										  // Ammo (char *)              
		IT_AMMO,									  // flags                      
		NULL,										  // void * ?                   
		ITEM_AMMO_MANA_COMBO_HALF,					  // tag ?                      
		NULL,										// Icon name (char *)         
	},

	{
		"item_ammo_redrain",						  // Spawnname
		"Red-Rain-Arrows",							  // Pickup name (char *)       
		GM_STORMARROWS,										// pickup message
		0,
		NULL,								  // Pickup (f)                 
		NULL,										  // Use (f)                    
		NULL,										  // Drop	(f)                   
		NULL,										  // Think (f)                  
		"player/picup.wav",							  // Pickup sound (char *)      
		NULL,										  // world model (char *)       
		0,											  // world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)  
		0,											  // Number of digits to display
		AMMO_COUNT_REDRAINBOW,						  // Ammo/ammo use per shot     
		NULL,										  // Ammo (char *)              
		IT_AMMO | IT_OFFENSE,						  // flags                      
		NULL,										  // void * ?                   
		ITEM_AMMO_REDRAIN,								  // tag ?                      
		"icons/i_ammo-redrain.m8",								  // Icon name (char *)         
	},

	{
		"item_ammo_phoenix",							  // Spawnname
		"Phoenix-Arrows",							  // Pickup name (char *)       
		GM_PHOENARROWS,										// pickup message
		0,
		NULL,										  // Pickup (f)                 
		NULL,										  // Use (f)                    
		NULL,										  // Drop	(f)                   
		NULL,										  // Think (f)                  
		"player/picup.wav",							  // Pickup sound (char *)      
		NULL,										  // world model (char *)       
		0,											  // world model flags          
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,										// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)  
		0,											  // Number of digits to display
		AMMO_COUNT_PHOENIXBOW,						  // Ammo/ammo use per shot     
		NULL,										  // Ammo (char *)              
		IT_AMMO | IT_OFFENSE,						  // flags                      
		NULL,										// void * ? 
		ITEM_AMMO_PHOENIX,							  // tag ?                      
		"icons/i_ammo-phoen.m8",								  // Icon name (char *)         
	},

	{
		"item_ammo_hellstaff",						  // Spawnname
		"Hell-staff-ammo",							  // Pickup name (char *)       
		GM_HELLORB,										// pickup message
		0,
		NULL,										  // Pickup (f)                 
		NULL,										  // Use (f)                    
		NULL,										  // Drop	(f)                   
		NULL,										  // Think (f)                  
		"player/picup.wav",							  // Pickup sound (char *)      
		NULL,										  // world model (char *)       
		0,											  // world model flags          
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)  
		0,											  // Number of digits to display
		AMMO_COUNT_HELLSTAFF,						  // Ammo/ammo use per shot     
		NULL,										  // Ammo (char *)              
		IT_AMMO | IT_OFFENSE,						  // flags                      
		NULL,										  // void * ?                   
		ITEM_AMMO_HELLSTAFF,						  // tag ?                      
		"icons/i_ammo-hellstaff.m8",				  // Icon name (char *)         
	},

	// ============================================================================================

	// Other items.

	{
		"item_health_half",							// Spawnname
		"Minor health",								// Pickup name (char *)       
		GM_HEALTHVIAL,										// pickup message
		0,
		NULL, 								// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"*gethealth.wav",							// Pickup sound (char *)      
		"models/items/health/healthsmall/tris.fm",	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		10,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags                      
		NULL,										// void * ?                   
		MODEL_HEALTH1,								// tag ?
		NULL,										// Icon name (char *)         
	},

	{
		"item_health_full",							// Spawnname
		"Major health",								// Pickup name (char *)       
		GM_HEALTHPOTION,										// pickup message
		0,
		NULL, 								// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"*gethealth.wav",							// Pickup sound (char *)      
		"models/items/health/healthbig/tris.fm",	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		30,										// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags                      
		NULL,										// void * ?                   
		MODEL_HEALTH2,								// tag ?                      
		NULL,								// Icon name (char *)         
	},

	// ============================================================================================

	// Puzzle Pieces

	{
		"item_puzzle_townkey",						// Spawnname
		"Town Key",									// Pickup name (char *)       
		GM_F_TOWNKEY,										// pickup message
		GM_NEED_TOWNKEY,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,		// world model (char *)       
		0,											// world model flags          
		-8, -8, -4,									// Bounding box mins
		 8,  8,  4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_TOWNKEY,							 				// tag ?                      
		"icons/p_townkey.m8",					// Icon name (char *)         
	},

	{
		"item_puzzle_cog",							// Spawnname
		"Cog",										// Pickup name (char *)       
		GM_F_COG,										// pickup message
		GM_NEED_COG,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,			// world model (char *)       
		0,											// world model flags          
		-10, -10, -24,								// Bounding box mins
		 10,  10,  20,								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_COG,							 				// tag ?                      
		"icons/p_cog.m8",							// Icon name (char *)         
	},

	{
		"item_puzzle_shield",					// Spawnname
		"Defensive Shield",						// Pickup name (char *)       
		GM_F_SHIELD,										// pickup message
		GM_NEED_SHIELD,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-2, -6, -12,							// Bounding box mins
		 2,  6,  12,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_SHIELD,				 							// tag ?                      
		"icons/p_shield.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_potion",						// Spawnname
		"Potion",									// Pickup name (char *)       
		GM_F_POTION,										// pickup message
		GM_NEED_POTION,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,		// world model (char *)       
		0,											// world model flags          
		-3, -3, -10,									// Bounding box mins
		 3,  3,  10,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_POTION,				 							// tag ?                      
		"icons/p_potion.m8",						// Icon name (char *)         
	},


	{
		"item_puzzle_plazacontainer",				// Spawnname
		"Container",								// Pickup name (char *)       
		GM_F_CONT,										// pickup message
		GM_NEED_CONT,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-6, -6, -8,									// Bounding box mins
		 6,  6,  6,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_CONT,				 							// tag ?                      
		"icons/p_plazajug.m8",						// Icon name (char *)         
	},


	{
		"item_puzzle_slumcontainer",				// Spawnname
		"Full Container",							// Pickup name (char *)       
		GM_F_CONTFULL,										// pickup message
		GM_NEED_CONTFULL,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-6, -6, -8,							// Bounding box mins
		 6,  6,  6,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_SLUMCONT,								// tag ?                      
		"icons/p_jugfill.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_crystal",						// Spawnname
		"Crystal",									// Pickup name (char *)       
		GM_F_CRYSTAL,										// pickup message
		GM_NEED_CRYSTAL,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_CRYSTAL,				 							// tag ?                      
		"icons/p_crystal.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_canyonkey",					// Spawnname
		"Canyon Key",								// Pickup name (char *)       
		GM_F_CANYONKEY,										// pickup message
		GM_NEED_CANYONKEY,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_CANKEY,				 							// tag ?                      
		"icons/p_canyonkey.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_hive2amulet",					// Spawnname
		"Hive 2 Amulet",							// Pickup name (char *)       
		GM_F_AMULET,										// pickup message
		GM_NEED_AMULET,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_AMULET,				 							// tag ?                      
		"icons/p_tcheckrikbust.m8",					// Icon name (char *)         
	},

	{
		"item_puzzle_hive2spear",					// Spawnname
		"Hive 2 Spear",								// Pickup name (char *)       
		GM_F_SPEAR,										// pickup message
		GM_NEED_SPEAR,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_SPEAR,				 							// tag ?                      
		"icons/p_spear.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_hive2gem",						// Spawnname
		"Hive 2 Gem",								// Pickup name (char *)       
		GM_F_GEM,										// pickup message
		GM_NEED_GEM,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_GEM,				 							// tag ?                      
		"icons/p_tcheckrikgem.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_minecartwheel",				// Spawnname
		"Minecart Wheel",							// Pickup name (char *)       
		GM_F_CARTWHEEL,										// pickup message
		GM_NEED_CARTWHEEL,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,		// world model (char *)       
		0,											// world model flags          
		-1,-6,-6,							// Bounding box mins
		 1, 6, 6,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_WHEEL,				 							// tag ?                      
		"icons/p_wheel.m8",							// Icon name (char *)         
	},

	{
		"item_puzzle_ore",							// Spawnname
		"Ore",										// Pickup name (char *)       
		GM_F_UNREFORE,										// pickup message
		GM_NEED_UNREFORE,										// can`t use message
		NULL,		 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-10,-10,-8,							// Bounding box mins
		 10, 10, 8,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_ORE	,				 							// tag ?                      
		"icons/p_oreunrefined.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_refinedore",					// Spawnname
		"Refined Ore",								// Pickup name (char *)       
		GM_F_REFORE,										// pickup message
		GM_NEED_REFORE,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-3,-12,-2,							// Bounding box mins
		 3, 12, 2,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_REF_ORE,	 							// tag ?                      
		"icons/p_orerefined.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_dungeonkey",					// Spawnname
		"Dungeon Key",								// Pickup name (char *)       
		GM_F_DUNGEONKEY,										// pickup message
		GM_NEED_DUNGEONKEY,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-1,-18,-9,									// Bounding box mins
		 1, 18, 9,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_DUNKEY,	 							// tag ?                      
		"icons/p_dungeonkey.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_cloudkey",						// Spawnname
		"Cloud Key",								// Pickup name (char *)       
		GM_F_CLOUDKEY,										// pickup message
		GM_NEED_CLOUDKEY,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-8, -8, -3,									// Bounding box mins
		 8,  8,  3,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_CLOUDKEY,				 							// tag ?                      
		"icons/p_cloudkey.m8",						// Icon name (char *)         
	},


	{
		"item_puzzle_highpriestesskey",				// Spawnname
		"Key",										// Pickup name (char *)       
		GM_F_HIGHKEY,										// pickup message
		GM_NEED_HIGHKEY,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-12,-12, -6,								// Bounding box mins
		 12, 12,  6,								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_HIVEKEY,				 							// tag ?                      
		"icons/p_hivekey.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_highpriestesssymbol",			// Spawnname
		"Symbol",									// Pickup name (char *)       
		GM_F_SYMBOL,										// pickup message
		GM_NEED_SYMBOL,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_HPSYM,				 							// tag ?                      
		"icons/p_queenkey.m8",						// Icon name (char *)         
	},

	{
		"item_puzzle_tome",							// Spawnname
		"Tome",										// Pickup name (char *)       
		GM_F_TOME,										// pickup message
		GM_NEED_TOME,										// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_TOME,				 							// tag ?                      
		"icons/p_tomepower.m8",						// Icon name (char *)         
	},
	{
		"item_puzzle_tavernkey",					// Spawnname
		"Tavern Key",								// Pickup name (char *)       
		GM_F_TAVERNKEY,								// pickup message
		GM_NEED_TAVERNKEY,							// can`t use message
		NULL,				 	  					// Pickup (f)                 
		NULL,										// Use (f)                    
		NULL,										// Drop	(f)                   
		NULL,										// Think (f)                  
		"player/picup.wav",							// Pickup sound (char *)      
		NULL,	// world model (char *)       
		0,											// world model flags          
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)  
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot     
		NULL,										// Ammo (char *)              
		IT_PUZZLE,									// flags                      
		NULL,										// void * ?                   
		ITEM_TAVERNKEY,				 				// tag ?                      
		"icons/p_tavernkey.m8",					// Icon name (char *)         
	},

	{
		"item_defense_tornado",					// Spawnname
		"tornado",								// Pickup name (char *)       
		GM_TORNADO,										// pickup message
		GM_NOTORNADO,										// can`t use message
		NULL,									// Pickup (f)                 
		NULL,									// Use (f)                    
		NULL,									// Drop	(f)                   
		NULL,									// Think (f)                  
		"player/getweapon.wav",						// Pickup sound (char *)      
		NULL,									// world model (char *)       
		EF_ROTATE,								// world model flags          
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)  
		0,										// Number of digits to display
		MANA_USE_TORNADO,						// Ammo/ammo use per shot     
		"Def-mana",								// Ammo (char *)              
		IT_DEFENSE,								// flags                      
#if DEMO_CODE
		(void*)1,								// void * ? 
#else
		NULL,									// void * ? 
#endif
		ITEM_DEFENSE_TORNADO,					// tag ?                      
 		"icons/i_tornado.m8",					// Icon name (char *)         
	},


	// End of list marker.
	
	{NULL}
};

// ************************************************************************************************
// InitItems
// ---------
// ************************************************************************************************

PLAYER_API void InitItems(void)
{
	p_itemlist=itemlist;
	p_num_items=(sizeof(itemlist)/sizeof(itemlist[0]))-1;
}


PLAYER_API	int GetItemIndex(gitem_t* x)
{
	if(!x) return(0);

	assert(x>=p_itemlist&&x<p_itemlist+p_num_items);
	
	return(x-p_itemlist);
}

// ************************************************************************************************
// GetItemByIndex
// --------------
// ************************************************************************************************

PLAYER_API gitem_t *GetItemByIndex(int Index)
{
	if((Index==0)||(Index>=p_num_items))
		return(NULL);

	return(&p_itemlist[Index]);
}

// ************************************************************************************************
// FindItemByClassname
// -------------------
// ************************************************************************************************

PLAYER_API gitem_t *FindItemByClassname(char *Classname)
{
	int		I;
	gitem_t	*Item;

	Item=p_itemlist;

	for(I=0;I<p_num_items;I++,Item++)
	{
		if(!Item->classname)
			continue;

		if(!Q_stricmp(Item->classname,Classname))
			return(Item);
	}

	return(NULL);
}

// ************************************************************************************************
// FindItem
// --------
// ************************************************************************************************

PLAYER_API gitem_t *FindItem(char *PickupName)
{
	int		I;
	gitem_t	*Item;

	Item=p_itemlist;

	for(I=0;I<p_num_items;I++,Item++)
	{
		if(!Item->pickup_name)
			continue;

		if(!Q_stricmp(Item->pickup_name,PickupName))
			return(Item);
	}

	return(NULL);
}
