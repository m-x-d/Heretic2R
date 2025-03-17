//
// g_spawn.c
//
// Copyright 1998 Raven Software
//

#include "c_actors.h" //mxd
#include "g_breakable.h" //mxd
#include "g_env.h" //mxd
#include "g_field.h"
#include "g_flamethrower.h"
#include "g_func_Button.h" //mxd
#include "g_func_Door.h" //mxd
#include "g_func_MonsterSpawner.h" //mxd
#include "g_func_Plat.h" //mxd
#include "g_func_Rotating.h" //mxd
#include "g_func_Timer.h" //mxd
#include "g_func_Train.h" //mxd
#include "g_items.h" //mxd
#include "g_light.h" //mxd
#include "g_misc.h" //mxd
#include "g_obj.h" //mxd
#include "g_rope.h" //mxd
#include "g_Shrine.h" //mxd
#include "m_monsters.h" //mxd
#include "p_client.h" //mxd
#include "g_local.h"

typedef struct
{
	char* name;
	void (*spawn)(edict_t* ent);
	ClassID_t CID; //mxd. int in original version.
} spawn_t;

void SP_trigger_Activate(edict_t* self);
void SP_trigger_Always(edict_t* ent);
void SP_trigger_Counter(edict_t* ent);
void SP_trigger_Deactivate(edict_t* self);
void SP_trigger_Elevator(edict_t* ent);
void SP_trigger_mappercentage(edict_t* self);
void SP_trigger_quit_to_menu(edict_t* self);
void SP_trigger_mission_give(edict_t* self);
void SP_trigger_mission_take(edict_t* self);
void SP_trigger_Multiple(edict_t* ent);
void SP_trigger_Once(edict_t* ent);
void SP_trigger_PlayerPushButton(edict_t* ent);
void SP_trigger_PlayerPushLever(edict_t* ent);
void SP_trigger_PlayerUsePuzzle(edict_t* ent);
void SP_trigger_puzzle(edict_t* ent);
void SP_trigger_quake(edict_t* ent);
void SP_trigger_Relay(edict_t* ent);
void SP_trigger_lightning(edict_t* ent);
void SP_trigger_farclip(edict_t* ent);
void SP_trigger_endgame(edict_t* self);

void SP_choose_CDTrack(edict_t* self);

void SP_target_explosion(edict_t* ent);
void SP_target_changelevel(edict_t* ent);
void SP_target_crosslevel_trigger(edict_t* ent);
void SP_target_crosslevel_target(edict_t* ent);
void SP_target_lightramp(edict_t* self);
void SP_target_earthquake(edict_t* ent);

void SP_worldspawn(edict_t* ent);

void SP_misc_flag(edict_t* ent);

void SP_env_mist(edict_t* self);
void SP_env_bubbler(edict_t* self);
void SP_env_water_drip(edict_t* self);
void SP_env_water_fountain(edict_t* self);
void SP_env_waterfall_base(edict_t* self);

// Object stuff
void SP_obj_fishhead1(edict_t* ent);
void SP_obj_fishhead2(edict_t* ent);

void SP_obj_stalactite1(edict_t* ent);
void SP_obj_stalactite2(edict_t* ent);
void SP_obj_stalactite3(edict_t* ent);

void SP_script_runner(edict_t* ent);

#pragma region ========================== spawns array ==========================

static spawn_t spawns[] =
{
	// Quake2 specific spawns.
	{ "info_player_start", SP_info_player_start, -1 }, //TODO: change -1 to CID_NONE.
	{ "info_player_deathmatch", SP_info_player_deathmatch, -1 },
	{ "info_player_coop", SP_info_player_coop, -1 },
	{ "info_player_intermission", SP_info_player_intermission, -1 },

	{ "func_plat", SP_func_plat, -1 },
	{ "func_button", SP_func_button, CID_BUTTON },
	{ "func_door", SP_func_door, CID_FUNC_DOOR },
	{ "func_door_secret", SP_func_door_secret, -1 },
	{ "func_door_rotating", SP_func_door_rotating, CID_FUNC_ROTATE },
	{ "func_rotating", SP_func_rotating, CID_FUNC_ROTATE },
	{ "func_train", SP_func_train, -1 },
	{ "func_water", SP_func_water, -1 },
	{ "func_areaportal", SP_func_areaportal, -1 },
	{ "func_monsterspawner", SP_func_monsterspawner, -1 },
	{ "func_wall", SP_func_wall, -1 },
	{ "func_object", SP_func_object, -1 },
	{ "func_timer", SP_func_timer, -1 },

	{ "trigger_Activate", SP_trigger_Activate, CID_TRIGGER },
	{ "trigger_always", SP_trigger_Always, CID_TRIGGER },
	{ "trigger_Damage", SP_trigger_damage, CID_TRIG_DAMAGE },
	{ "trigger_Deactivate", SP_trigger_Deactivate, CID_TRIGGER },
	{ "trigger_counter", SP_trigger_Counter, CID_TRIGGER },
	{ "trigger_elevator", SP_trigger_Elevator, CID_TRIGGER },
	{ "trigger_fogdensity", SP_trigger_fogdensity, CID_TRIGGER },
	{ "trigger_Gravity", SP_trigger_gravity, -1 },
	{ "trigger_lightning", SP_trigger_lightning, CID_TRIGGER },
	{ "trigger_mappercentage", SP_trigger_mappercentage, CID_TRIGGER },
	{ "trigger_quit_to_menu", SP_trigger_quit_to_menu, CID_TRIGGER },
	{ "trigger_mission_give", SP_trigger_mission_give, CID_TRIGGER },
	{ "trigger_mission_take", SP_trigger_mission_take, CID_TRIGGER },
	{ "trigger_MonsterJump", SP_trigger_monsterjump, -1 },
	{ "trigger_goto_buoy", SP_trigger_goto_buoy, -1 },
	{ "trigger_multiple", SP_trigger_Multiple, CID_TRIGGER },
	{ "trigger_playerpushbutton", SP_trigger_PlayerPushButton, CID_TRIGGER },
	{ "trigger_playerpushlever", SP_trigger_PlayerPushLever, CID_TRIGGER },
	{ "trigger_playerusepuzzle", SP_trigger_PlayerUsePuzzle, CID_TRIGGER },
	{ "trigger_push", SP_trigger_push, CID_TRIG_PUSH },
	{ "trigger_puzzle", SP_trigger_puzzle, CID_TRIGGER },
	{ "trigger_once", SP_trigger_Once, CID_TRIGGER },
	{ "trigger_quake", SP_trigger_quake, CID_TRIGGER },
	{ "trigger_relay", SP_trigger_Relay, CID_TRIGGER },
	{ "trigger_farclip", SP_trigger_farclip, CID_TRIGGER },
	{ "trigger_endgame", SP_trigger_endgame, CID_TRIGGER },

	{ "choose_CDTrack", SP_choose_CDTrack, -1 },

	{ "target_explosion", SP_target_explosion, -1 },
	{ "target_changelevel", SP_target_changelevel, -1 },
	{ "target_crosslevel_trigger", SP_target_crosslevel_trigger, -1 },
	{ "target_crosslevel_target", SP_target_crosslevel_target, -1 },
	{ "target_lightramp", SP_target_lightramp, -1 },
	{ "target_earthquake", SP_target_earthquake, -1 },

	{ "worldspawn", SP_worldspawn, -1 },

	{ "light", SP_light, -1 },
	{ "info_null", SP_info_null, -1 },
	{ "func_group", SP_info_null, -1 },
	{ "info_notnull", SP_info_notnull, -1 },
	{ "path_corner", SP_path_corner, -1 },
	{ "point_combat", SP_point_combat, -1 },

	{ "misc_teleporter", SP_misc_teleporter, CID_TELEPORTER },
	{ "misc_teleporter_dest", SP_misc_teleporter_dest, -1 },
	{ "misc_update_spawner", SP_misc_update_spawner, CID_TRIGGER },
	{ "misc_remote_camera", SP_misc_remote_camera, -1 },
	{ "misc_magic_portal", SP_misc_magic_portal, -1 },
	{ "misc_fire_sparker", SP_misc_fire_sparker, -1 },

	{ "misc_flag", SP_misc_flag, -1 },

	{ "monster_gorgon",SP_monster_gorgon, CID_GORGON },
	{ "monster_rat",SP_monster_rat, CID_RAT },
	{ "monster_plagueElf", SP_monster_plagueElf, CID_PLAGUEELF },
	{ "monster_fish", SP_monster_fish, CID_FISH },
	{ "monster_harpy", SP_monster_harpy, CID_HARPY },
	{ "monster_spreader", SP_monster_spreader, CID_SPREADER },
	{ "monster_assassin",SP_monster_assassin, CID_ASSASSIN },
	{ "monster_chicken",SP_monster_chicken, CID_CHICKEN },
	{ "monster_tcheckrik_male",SP_monster_tcheckrik_male, CID_TCHECKRIK },
	{ "monster_gkrokon", SP_Monster_Gkrokon, CID_GKROKON },

	{ "monster_gorgon_leader",SP_monster_gorgon_leader, CID_GORGON },
	{ "monster_rat_giant",SP_monster_rat_giant, CID_RAT },
	{ "monster_palace_plague_guard", SP_monster_palace_plague_guard, CID_PLAGUEELF },
	{ "monster_palace_plague_guard_invisible", SP_monster_palace_plague_guard_invisible, CID_PLAGUEELF },
	{ "monster_elflord", SP_monster_elflord, CID_ELFLORD },
	{ "monster_ssithra",SP_monster_plague_ssithra, CID_SSITHRA },
	{ "monster_mssithra",SP_monster_mssithra, CID_MSSITHRA },
	{ "monster_chkroktk",SP_monster_chkroktk, CID_RAT },
	{ "monster_tcheckrik_female",SP_monster_tcheckrik_female, CID_TCHECKRIK },
	{ "monster_tcheckrik_mothers",SP_monster_tcheckrik_mothers, CID_MOTHER },
	{ "monster_high_priestess",SP_monster_high_priestess, CID_HIGHPRIESTESS },
	{ "monster_ogle",SP_monster_ogle, CID_OGLE },
	{ "monster_seraph_overlord",SP_monster_seraph_overlord, CID_SERAPH_OVERLORD },
	{ "monster_seraph_guard",SP_monster_seraph_guard, CID_SERAPH_GUARD },
	{ "monster_bee",SP_monster_bee, CID_BEE },
	{ "monster_morcalavin",SP_monster_morcalavin, CID_MORK },
	{ "monster_trial_beast",SP_monster_trial_beast, CID_TBEAST },
	{ "monster_imp", SP_monster_imp, CID_IMP },

	{ "character_corvus1",SP_character_corvus1, CID_CORVUS },
	{ "character_corvus2",SP_character_corvus2, CID_CORVUS2 },
	{ "character_corvus3",SP_character_corvus3, CID_CORVUS3 },
	{ "character_corvus4",SP_character_corvus4, CID_CORVUS4 },
	{ "character_corvus5",SP_character_corvus5, CID_CORVUS5 },
	{ "character_corvus6",SP_character_corvus6, CID_CORVUS6 },
	{ "character_corvus7",SP_character_corvus7, CID_CORVUS7 },
	{ "character_corvus8",SP_character_corvus8, CID_CORVUS8 },
	{ "character_corvus9",SP_character_corvus9, CID_CORVUS9 },
	{ "character_dranor",SP_character_dranor, CID_DRANOR },
	{ "character_elflord",SP_character_elflord, CID_C_ELFLORD },
	{ "character_highpriestess",SP_character_highpriestess, CID_C_HIGHPRIESTESS },
	{ "character_highpriestess2",SP_character_highpriestess2, CID_C_HIGHPRIESTESS2 },
	{ "character_morcalavin",SP_character_morcalavin, CID_C_MORCALAVIN },
	{ "character_sidhe_guard",SP_character_sidhe_guard, CID_PLAGUEELF },
	{ "character_siernan1",SP_character_siernan1, CID_C_SIERNAN1 },
	{ "character_siernan2",SP_character_siernan2, CID_C_SIERNAN2 },
	{ "character_ssithra_scout",SP_character_ssithra_scout, CID_SSITHRA_SCOUT },
	{ "character_ssithra_victim",SP_character_ssithra_victim, CID_SSITHRA_VICTIM },
	{ "character_tome",SP_character_tome, CID_C_TOME },

	// Heretic2 specific spawns.
	{ "breakable_brush",SP_breakable_brush, CID_BBRUSH },


	{ "light_walltorch",SP_light_walltorch, CID_LIGHT },
	{ "light_floortorch",SP_light_floortorch, CID_LIGHT },
	{ "light_torch1",SP_light_torch1, CID_LIGHT },
	{ "light_gem2",SP_light_gem2, CID_LIGHT },
	{ "light_chandelier1",SP_light_chandelier1, CID_LIGHT },
	{ "light_chandelier2",SP_light_chandelier2, CID_LIGHT },
	{ "light_chandelier3",SP_light_chandelier3, CID_LIGHT },
	{ "light_lantern1",SP_light_lantern1, CID_LIGHT },
	{ "light_lantern2",SP_light_lantern2, CID_LIGHT },
	{ "light_lantern3",SP_light_lantern3, CID_LIGHT },
	{ "light_lantern4",SP_light_lantern4, CID_LIGHT },
	{ "light_lantern5",SP_light_lantern5, CID_LIGHT },
	{ "light_buglight",SP_light_buglight, CID_LIGHT },

	{ "env_fire",SP_env_fire, CID_OBJECT },
	{ "env_dust",SP_env_dust, CID_OBJECT },
	{ "env_smoke",SP_env_smoke, CID_OBJECT },
	{ "env_mist",SP_env_mist, CID_OBJECT },
	{ "env_bubbler",SP_env_bubbler, CID_OBJECT },
	{ "env_water_drip",SP_env_water_drip, CID_OBJECT },
	{ "env_water_fountain",SP_env_water_fountain, CID_OBJECT },
	{ "env_waterfall_base",SP_env_waterfall_base, CID_OBJECT },
	{ "env_sun1",SP_env_sun1, CID_OBJECT },
	{ "env_muck",SP_env_muck, CID_OBJECT },

	{ "sound_ambient_silverspring",SP_sound_ambient_silverspring, -1 },
	{ "sound_ambient_swampcanyon",SP_sound_ambient_swampcanyon, -1 },
	{ "sound_ambient_andoria",SP_sound_ambient_andoria, -1 },
	{ "sound_ambient_hive",SP_sound_ambient_hive, -1 },
	{ "sound_ambient_mine",SP_sound_ambient_mine, -1 },
	{ "sound_ambient_cloudfortress",SP_sound_ambient_cloudfortress, -1 },

	{ "obj_andwallhanging",SP_obj_andwallhanging, CID_OBJECT },
	{ "obj_banner",SP_obj_banner, CID_OBJECT },
	{ "obj_banneronpole",SP_obj_banneronpole, CID_OBJECT },
	{ "obj_barrel",SP_obj_barrel, CID_OBJECT },
	{ "obj_barrel_explosive",SP_obj_barrel_explosive, CID_OBJECT },
	{ "obj_barrel_metal",SP_obj_barrel_metal, CID_OBJECT },
	{ "obj_basket",SP_obj_basket, CID_OBJECT },
	{ "obj_bench",SP_obj_bench, CID_OBJECT },
	{ "obj_bigcrystal",SP_obj_bigcrystal, CID_OBJECT },
	{ "obj_biotank",SP_obj_biotank, CID_OBJECT },
	{ "obj_bloodsplat",SP_obj_bloodsplat, CID_OBJECT },
	{ "obj_bookclosed",SP_obj_bookclosed, CID_OBJECT },
	{ "obj_bookopen",SP_obj_bookopen, CID_OBJECT },
	{ "obj_bottle1",SP_obj_bottle1, CID_OBJECT },
	{ "obj_broom",SP_obj_broom, CID_OBJECT },
	{ "obj_bucket",SP_obj_bucket, CID_OBJECT },
	{ "obj_bush1",SP_obj_bush1, CID_OBJECT },
	{ "obj_bush2",SP_obj_bush2, CID_OBJECT },
	{ "obj_cactus",SP_obj_cactus, CID_OBJECT },
	{ "obj_cactus3",SP_obj_cactus3, CID_OBJECT },
	{ "obj_cactus4",SP_obj_cactus4, CID_OBJECT },
	{ "obj_cauldron",SP_obj_cauldron, CID_OBJECT },
	{ "obj_chair1",SP_obj_chair1, CID_OBJECT },
	{ "obj_chair2",SP_obj_chair2, CID_OBJECT },
	{ "obj_chair3",SP_obj_chair3, CID_OBJECT },
	{ "obj_chest1",SP_obj_chest1, CID_OBJECT },
	{ "obj_chest2",SP_obj_chest2, CID_OBJECT },
	{ "obj_chest3",SP_obj_chest3, CID_OBJECT },
	{ "obj_choppeddude",SP_obj_choppeddude, CID_OBJECT },
	{ "obj_claybowl",SP_obj_claybowl, CID_OBJECT },
	{ "obj_clayjar",SP_obj_clayjar, CID_OBJECT },
	{ "obj_cocoon",SP_obj_cocoon, CID_OBJECT },
	{ "obj_cocoonopen",SP_obj_cocoonopen, CID_OBJECT },
	{ "obj_cog1",SP_obj_cog1, CID_OBJECT },
	{ "obj_corpse1",SP_obj_corpse1, CID_OBJECT },
	{ "obj_corpse2",SP_obj_corpse2, CID_OBJECT },
	{ "obj_corpse_ogle",SP_obj_corpse_ogle, CID_OBJECT },
	{ "obj_corpse_ssithra",SP_obj_corpse_ssithra, CID_OBJECT },
	{ "obj_dying_elf",SP_obj_dying_elf, CID_OBJECT },
	{ "obj_eggpan",SP_obj_eggpan, CID_OBJECT },
	{ "obj_eyeball_jar",SP_obj_eyeball_jar, CID_OBJECT },
	{ "obj_firepot",SP_obj_firepot, CID_OBJECT },
	{ "obj_fishhead1",SP_obj_fishhead1, CID_OBJECT },
	{ "obj_fishhead2",SP_obj_fishhead2, CID_OBJECT },
	{ "obj_fishtrap",SP_obj_fishtrap, CID_OBJECT },
	{ "obj_flagonpole",SP_obj_flagonpole, CID_OBJECT },
	{ "obj_floor_candelabrum",SP_obj_floor_candelabrum, CID_OBJECT },
	{ "obj_fountain_fish",SP_obj_fountain_fish, CID_OBJECT },
	{ "obj_frypan",SP_obj_frypan, CID_OBJECT },
	{ "obj_gascan",SP_obj_gascan, CID_OBJECT },
	{ "obj_gorgonbones",SP_obj_gorgonbones, CID_OBJECT },
	{ "obj_grass",SP_obj_grass, CID_OBJECT },
	{ "obj_hangingdude",SP_obj_hangingdude, CID_OBJECT },
	{ "obj_hanging_ogle",SP_obj_hanging_ogle, CID_OBJECT },
	{ "obj_hivepriestessssymbol",SP_obj_hivepriestessssymbol, CID_OBJECT },
	{ "obj_jawbone",SP_obj_jawbone, CID_OBJECT },
	{ "obj_jug1",SP_obj_jug1, CID_OBJECT },
	{ "obj_kettle",SP_obj_kettle, CID_OBJECT },
	{ "obj_lab_parts_container",SP_obj_lab_parts_container, CID_OBJECT },
	{ "obj_lab_tray",SP_obj_lab_tray, CID_OBJECT },
	{ "obj_larva",SP_obj_larva, CID_OBJECT },
	{ "obj_larvabrokenegg",SP_obj_larvabrokenegg, CID_OBJECT },
	{ "obj_larvaegg",SP_obj_larvaegg, CID_OBJECT },
	{ "obj_lever1",SP_obj_lever1, CID_LEVER },
	{ "obj_lever2",SP_obj_lever2, CID_LEVER },
	{ "obj_lever3",SP_obj_lever3, CID_LEVER },
	{ "obj_metalchunk1",SP_obj_metalchunk1, CID_OBJECT },
	{ "obj_metalchunk2",SP_obj_metalchunk2, CID_OBJECT },
	{ "obj_metalchunk3",SP_obj_metalchunk3, CID_OBJECT },
	{ "obj_minecart",SP_obj_minecart, CID_OBJECT },
	{ "obj_minecart2",SP_obj_minecart2, CID_OBJECT },
	{ "obj_minecart3",SP_obj_minecart3, CID_OBJECT },
	{ "obj_moss1",SP_obj_moss1, CID_OBJECT },
	{ "obj_moss2",SP_obj_moss2, CID_OBJECT },
	{ "obj_moss3",SP_obj_moss3, CID_OBJECT },
	{ "obj_moss4",SP_obj_moss4, CID_OBJECT },
	{ "obj_moss5",SP_obj_moss5, CID_OBJECT },
	{ "obj_nest",SP_obj_nest, CID_OBJECT },
	{ "obj_pick",SP_obj_pick, CID_OBJECT },
	{ "obj_pipe1",SP_obj_pipe1, CID_OBJECT },
	{ "obj_pipe2",SP_obj_pipe2, CID_OBJECT },
	{ "obj_pipewheel",SP_obj_pipewheel, CID_OBJECT },
	{ "obj_plant1",SP_obj_plant1, CID_OBJECT },
	{ "obj_plant2",SP_obj_plant2, CID_OBJECT },
	{ "obj_plant3",SP_obj_plant3, CID_OBJECT },
	{ "obj_pot1",SP_obj_pot1, CID_OBJECT },
	{ "obj_pot2",SP_obj_pot2, CID_OBJECT },
	{ "obj_pottedplant",SP_obj_pottedplant, CID_OBJECT },
	{ "obj_pushcart",SP_obj_pushcart, CID_OBJECT },
	{ "obj_queenthrone",SP_obj_queenthrone, CID_OBJECT },
	{ "obj_queenchair",SP_obj_queenchair, CID_OBJECT },
	{ "obj_ring_plaque2",SP_obj_ring_plaque2, CID_OBJECT },
	{ "obj_rocks1",SP_obj_rocks1, CID_OBJECT },
	{ "obj_rocks2",SP_obj_rocks2, CID_OBJECT },
	{ "obj_rope",SP_obj_rope, CID_OBJECT },
	{ "obj_ropechain",SP_obj_ropechain, CID_OBJECT },
	{ "obj_scroll",SP_obj_scroll, CID_OBJECT },
	{ "obj_seasonglobe",SP_obj_seasonglobe, CID_OBJECT },
	{ "obj_shovel",SP_obj_shovel, CID_OBJECT },
	{ "obj_shrine",SP_obj_shrine, CID_OBJECT },
	{ "obj_sign1",SP_obj_sign1, CID_OBJECT },
	{ "obj_sign4",SP_obj_sign4, CID_OBJECT },
	{ "obj_skullpole",SP_obj_skullpole, CID_OBJECT },
	{ "obj_spellbook",SP_obj_spellbook, CID_OBJECT },
	{ "obj_stalactite1",SP_obj_stalactite1, CID_OBJECT },
	{ "obj_stalactite2",SP_obj_stalactite2, CID_OBJECT },
	{ "obj_stalactite3",SP_obj_stalactite3, CID_OBJECT },
	{ "obj_stalagmite1",SP_obj_stalagmite1, CID_OBJECT },
	{ "obj_stalagmite2",SP_obj_stalagmite2, CID_OBJECT },
	{ "obj_stalagmite3",SP_obj_stalagmite3, CID_OBJECT },
	{ "obj_statue_boulderfish",SP_obj_statue_boulderfish, CID_OBJECT },
	{ "obj_statue_corvus",SP_obj_statue_corvus, CID_OBJECT },
	{ "obj_statue_dolphin1",SP_obj_statue_dolphin1, CID_OBJECT },
	{ "obj_statue_dolphin2",SP_obj_statue_dolphin2, CID_OBJECT },
	{ "obj_statue_dolphin3",SP_obj_statue_dolphin3, CID_OBJECT },
	{ "obj_statue_dolphin4",SP_obj_statue_dolphin4, CID_OBJECT },
	{ "obj_statue_dragon",SP_obj_statue_dragon, CID_OBJECT },
	{ "obj_statue_dragonhead",SP_obj_statue_dragonhead, CID_OBJECT },
	{ "obj_statue_duckbill1",SP_obj_statue_duckbill1, CID_OBJECT },
	{ "obj_statue_duckbill2",SP_obj_statue_duckbill2, CID_OBJECT },
	{ "obj_statue_guardian",SP_obj_statue_guardian, CID_OBJECT },
	{ "obj_statue_saraphbust",SP_obj_statue_saraphbust, CID_OBJECT },
	{ "obj_statue_sariph",SP_obj_statue_sariph, CID_OBJECT },
	{ "obj_statue_sithraguard",SP_obj_statue_sithraguard, CID_OBJECT },
	{ "obj_statue_tchecktrik_bust",SP_obj_statue_tchecktrik_bust, CID_OBJECT },
	{ "obj_statue_techeckrikleft",SP_obj_statue_techeckrikleft, CID_OBJECT },
	{ "obj_statue_techeckrikright",SP_obj_statue_techeckrikright, CID_OBJECT },
	{ "obj_statue_techeckriktomb",SP_obj_statue_techeckriktomb, CID_OBJECT },
	{ "obj_stein",SP_obj_stein, CID_OBJECT },
	{ "obj_swampflat_top",SP_obj_swampflat_top, CID_OBJECT },
	{ "obj_swampflat_bottom",SP_obj_swampflat_bottom, CID_OBJECT },
	{ "obj_table1",SP_obj_table1, CID_OBJECT },
	{ "obj_table2",SP_obj_table2, CID_OBJECT },
	{ "obj_tapper",SP_obj_tapper, CID_OBJECT },
	{ "obj_throne",SP_obj_throne, CID_OBJECT },
	{ "obj_torture_bed",SP_obj_torture_bed, CID_OBJECT },
	{ "obj_torture_ironmaiden",SP_obj_torture_ironmaiden, CID_OBJECT },
	{ "obj_torture_rack",SP_obj_torture_rack, CID_OBJECT },
	{ "obj_torture_table",SP_obj_torture_table, CID_OBJECT },
	{ "obj_torture_wallring",SP_obj_torture_wallring, CID_OBJECT },
	{ "obj_tree",SP_obj_tree, CID_OBJECT },
	{ "obj_tree2",SP_obj_tree2, CID_OBJECT },
	{ "obj_tree3",SP_obj_tree3, CID_OBJECT },
	{ "obj_treefallen",SP_obj_treefallen, CID_OBJECT },
	{ "obj_treestump",SP_obj_treestump, CID_OBJECT },
	{ "obj_treetall",SP_obj_treetall, CID_OBJECT },
	{ "obj_treetop",SP_obj_treetop, CID_OBJECT },
	{ "obj_urn",SP_obj_urn, CID_OBJECT },
	{ "obj_venusflytrap",SP_obj_venusflytrap, CID_OBJECT },
	{ "obj_wallringplaque",SP_obj_wallringplaque, CID_OBJECT },
	{ "obj_web",SP_obj_web, CID_OBJECT },
	{ "obj_wheelbarrow",SP_obj_wheelbarrow, CID_OBJECT },
	{ "obj_wheelbarrowdamaged",SP_obj_wheelbarrowdamaged, CID_OBJECT },
	{ "obj_woodpile",SP_obj_woodpile, CID_OBJECT },

	{ "obj_morcalavin_barrier",SP_obj_morcalavin_barrier, CID_OBJECT },

	{ "flamethrower",SP_flamethrower, CID_FLAMETHROWER },

	{ "item_spitter", SP_item_spitter, -1 },

	{ "info_buoy", SP_info_buoy, -1 },

	{ "shrine_heal", SP_shrine_heal_trigger, CID_TRIGGER },
	{ "shrine_armor", SP_shrine_armor_silver_trigger, CID_TRIGGER },
	{ "shrine_staff", SP_shrine_staff_trigger, CID_TRIGGER },
	{ "shrine_lung", SP_shrine_lungs_trigger, CID_TRIGGER },
	{ "shrine_armor_gold", SP_shrine_armor_gold_trigger, CID_TRIGGER },
	{ "shrine_light", SP_shrine_light_trigger, CID_TRIGGER },
	{ "shrine_mana", SP_shrine_mana_trigger, CID_TRIGGER },
	{ "shrine_ghost", SP_shrine_ghost_trigger, CID_TRIGGER },
	{ "shrine_reflect", SP_shrine_reflect_trigger, CID_TRIGGER },
	{ "shrine_powerup", SP_shrine_powerup_trigger, CID_TRIGGER },
	{ "shrine_speed", SP_shrine_speed_trigger, CID_TRIGGER },
	{ "shrine_random", SP_shrine_random_trigger, CID_TRIGGER },

	{ "script_runner", SP_script_runner, CID_TRIGGER },

	{ NULL, NULL, -1 }
};

#pragma endregion

// Finds the spawn function for the entity and calls it.
void ED_CallSpawn(edict_t* ent)
{
	if (ent->classname == NULL)
	{
		gi.dprintf("ED_CallSpawn: NULL classname\n");
		return;
	}

	gitem_t* item = IsItem(ent);
	if (item != NULL)
	{
		SpawnItem(ent, item);
		return;
	}

	// Check normal spawn functions.
	for (const spawn_t* s = &spawns[0]; s->name != NULL; s++)
	{
		if (strcmp(s->name, ent->classname) != 0)
			continue;

		// Found it.
		if (s->CID != -1 && !classStaticsInitialized[s->CID]) // Need to call once per level that item is on.
		{
			classStaticsInits[s->CID]();
			classStaticsInitialized[s->CID] = true;
		}

		ent->classID = ((s->CID != -1) ? s->CID : CID_NONE); // Make sure classID is set.
		s->spawn(ent); // Need to call for every item.

		return;
	}

	gi.dprintf("%s doesn't have a spawn function\n", ent->classname);
}