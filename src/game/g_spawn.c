#include "g_local.h"

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
	int		CID;
} spawn_t;

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_coop (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);
void SP_info_buoy (edict_t *ent);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_train (edict_t *ent);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_monsterspawner (edict_t *ent);

void SP_trigger_Activate(edict_t *self);
void SP_trigger_Always (edict_t *ent);
void SP_trigger_Counter (edict_t *ent);
void SP_trigger_Damage(edict_t *self);
void SP_trigger_Deactivate(edict_t *self);
void SP_trigger_Elevator (edict_t *ent);
//void SP_trigger_flamethrower (edict_t *ent);
void SP_trigger_fogdensity(edict_t *self);
void SP_trigger_Gravity(edict_t *self);
void SP_trigger_mappercentage(edict_t *self);
void SP_trigger_quit_to_menu(edict_t *self);
void SP_trigger_mission_give(edict_t *self);
void SP_trigger_mission_take(edict_t *self);
void SP_trigger_MonsterJump(edict_t *self);
void SP_trigger_goto_buoy(edict_t *self);
void SP_trigger_Multiple (edict_t *ent);
void SP_trigger_Once (edict_t *ent);
void SP_trigger_PlayerPushButton (edict_t *ent);
void SP_trigger_PlayerPushLever (edict_t *ent);
void SP_trigger_PlayerUsePuzzle (edict_t *ent);
void SP_trigger_push(edict_t *self);
void SP_trigger_puzzle (edict_t *ent);
void SP_trigger_quake (edict_t *ent);
void SP_trigger_Relay (edict_t *ent);
void SP_trigger_lightning (edict_t *ent);
void SP_trigger_farclip (edict_t *ent);
void SP_trigger_endgame(edict_t *self);


void SP_choose_CDTrack(edict_t *self);

void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);

void SP_worldspawn (edict_t *ent);

void SP_light (edict_t *self);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);

void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void misc_update_spawner (edict_t *self);
void SP_misc_remote_camera (edict_t *self);
void SP_misc_magic_portal (edict_t *self);
void SP_misc_fire_sparker (edict_t *ent);

void SP_Monster_Gkrokon(edict_t *Self);
void SP_misc_flag (edict_t *ent);
void SP_monster_gorgon (edict_t *self);
void SP_monster_gorgon_leader (edict_t *self);
void SP_monster_rat (edict_t *self);
void SP_monster_rat_giant (edict_t *self);
void SP_monster_chicken (edict_t *self);
void SP_monster_plagueElf(edict_t *self);
void SP_monster_palace_plague_guard(edict_t *self);
void SP_monster_palace_plague_guard_invisible(edict_t *self);
void SP_monster_fish (edict_t *self);
void SP_monster_harpy (edict_t *self);
void SP_monster_spreader (edict_t *self);
void SP_monster_elflord (edict_t *self);
void SP_monster_plague_ssithra (edict_t *self);
void SP_monster_mssithra (edict_t *self);
void SP_monster_chkroktk (edict_t *self);
void SP_monster_tcheckrik_male (edict_t *self);
void SP_monster_tcheckrik_female (edict_t *self);
void SP_monster_tcheckrik_mothers (edict_t *self);
void SP_monster_high_priestess (edict_t *self);
void SP_monster_ogle (edict_t *self);
void SP_monster_seraph_overlord (edict_t *self);
void SP_monster_seraph_guard (edict_t *self);
void SP_monster_assassin (edict_t *self);
void SP_monster_morcalavin (edict_t *self);
void SP_monster_trial_beast (edict_t *self);
void SP_monster_imp (edict_t *self);
void SP_monster_bee (edict_t *self);

void SP_character_corvus1 (edict_t *self);
void SP_character_corvus2 (edict_t *self);
void SP_character_corvus3 (edict_t *self);
void SP_character_corvus4 (edict_t *self);
void SP_character_corvus5 (edict_t *self);
void SP_character_corvus6 (edict_t *self);
void SP_character_corvus7 (edict_t *self);
void SP_character_corvus8 (edict_t *self);
void SP_character_corvus9 (edict_t *self);
void SP_character_dranor (edict_t *self);
void SP_character_elflord (edict_t *self);
void SP_character_highpriestess (edict_t *self);
void SP_character_highpriestess2 (edict_t *self);
void SP_character_morcalavin (edict_t *self);
void SP_character_sidhe_guard (edict_t *self);
void SP_character_siernan1 (edict_t *self);
void SP_character_siernan2 (edict_t *self);
void SP_character_ssithra_scout (edict_t *self);
void SP_character_ssithra_victim (edict_t *self);
void SP_character_tome (edict_t *self);

void SP_breakable_brush (edict_t *ent);

void SP_light_walltorch (edict_t *ent);
//void SP_light_flame (edict_t *ent);
void SP_light_floortorch (edict_t *ent);
void SP_light_torch1(edict_t *ent);
void SP_light_gem2(edict_t *ent);
void SP_light_chandelier1 (edict_t *ent);
void SP_light_chandelier2 (edict_t *ent);
void SP_light_chandelier3 (edict_t *ent);
void SP_light_lantern1 (edict_t *ent);
void SP_light_lantern2 (edict_t *ent);
void SP_light_lantern3 (edict_t *ent);
void SP_light_lantern4 (edict_t *ent);
void SP_light_lantern5 (edict_t *ent);
void SP_light_buglight (edict_t *ent);

void SP_env_fire (edict_t *self);
void SP_env_dust (edict_t *self);
void SP_env_smoke (edict_t *self);
void SP_env_mist(edict_t *self);
void SP_env_bubbler(edict_t *self);
void SP_env_water_drip(edict_t *self);
void SP_env_water_fountain(edict_t *self);
void SP_env_waterfall_base(edict_t *self);
void SP_env_sun1(edict_t *ent);
void SP_env_muck(edict_t *ent);
//void SP_env_galaxy (edict_t *ent);

void SP_sound_ambient_silverspring (edict_t *ent);
void SP_sound_ambient_swampcanyon (edict_t *ent);
void SP_sound_ambient_andoria (edict_t *ent);
void SP_sound_ambient_hive (edict_t *ent);
void SP_sound_ambient_mine (edict_t *ent);
void SP_sound_ambient_cloudfortress (edict_t *ent);

// Object stuff
void SP_obj_andwallhanging(edict_t *ent);

void SP_obj_banner(edict_t *ent);
void SP_obj_banneronpole(edict_t *ent);
void SP_obj_barrel(edict_t *ent);
void SP_obj_barrel_explosive(edict_t *ent);
void SP_obj_barrel_metal(edict_t *ent);
void SP_obj_basket(edict_t *ent);
void SP_obj_bench(edict_t *ent);
void SP_obj_bigcrystal(edict_t *self);
void SP_obj_biotank (edict_t *self); 
void SP_obj_bloodsplat(edict_t *ent);
void SP_obj_bookclosed(edict_t *ent);
void SP_obj_bookopen(edict_t *ent);
void SP_obj_bottle1(edict_t *ent);
void SP_obj_broom (edict_t *ent);
void SP_obj_bucket (edict_t *ent);
void SP_obj_bush1 (edict_t *ent);
void SP_obj_bush2 (edict_t *ent);

void SP_obj_cactus (edict_t *ent);
void SP_obj_cactus3 (edict_t *ent);
void SP_obj_cactus4 (edict_t *ent);
void SP_obj_cauldron(edict_t *ent);
void SP_obj_chair1(edict_t *ent);
void SP_obj_chair2(edict_t *ent);
void SP_obj_chair3(edict_t *ent);
void SP_obj_chest1(edict_t *ent);
void SP_obj_chest2(edict_t *ent);
void SP_obj_chest3(edict_t *ent);
void SP_obj_choppeddude(edict_t *ent);
void SP_obj_claybowl (edict_t *ent);
void SP_obj_clayjar (edict_t *ent);
void SP_obj_cocoon(edict_t *ent);
void SP_obj_cocoonopen(edict_t *ent);
void SP_obj_cog1(edict_t *ent);
void SP_obj_corpse1(edict_t *self);
void SP_obj_corpse2(edict_t *self);
void SP_obj_corpse_ogle(edict_t *self);
void SP_obj_corpse_ssithra(edict_t *self);

void SP_obj_dying_elf(edict_t *self);

void SP_obj_eggpan(edict_t *ent);
void SP_obj_eyeball_jar(edict_t *ent);

void SP_obj_firepot(edict_t *ent);
void SP_obj_fishhead1(edict_t *ent);
void SP_obj_fishhead2(edict_t *ent);
void SP_obj_fishtrap (edict_t *ent);
void SP_obj_flagonpole (edict_t *ent);
void SP_obj_floor_candelabrum(edict_t *ent);
void SP_obj_fountain_fish(edict_t *ent);
void SP_obj_frypan(edict_t *ent);

void SP_obj_gascan(edict_t *ent);
void SP_obj_gorgonbones(edict_t *ent);
void SP_obj_grass(edict_t *ent);

void SP_obj_hangingdude(edict_t *ent);
void SP_obj_hanging_ogle(edict_t *ent);
void SP_obj_hivepriestessssymbol (edict_t *self);

void SP_obj_jawbone(edict_t *ent);
void SP_obj_jug1(edict_t *ent);

void SP_obj_kettle(edict_t *ent);

void SP_obj_lab_parts_container(edict_t *ent);
void SP_obj_lab_tray(edict_t *ent);
void SP_obj_larva(edict_t *ent);
void SP_obj_larvabrokenegg(edict_t *ent);
void SP_obj_larvaegg(edict_t *ent);
void SP_obj_lever1 (edict_t *ent);
void SP_obj_lever2 (edict_t *ent);
void SP_obj_lever3 (edict_t *ent);

void SP_obj_metalchunk1(edict_t *ent);
void SP_obj_metalchunk2(edict_t *ent);
void SP_obj_metalchunk3(edict_t *ent);
void SP_obj_minecart(edict_t *ent);
void SP_obj_minecart2(edict_t *ent);
void SP_obj_minecart3(edict_t *ent);
void SP_obj_moss1(edict_t *self);
void SP_obj_moss2(edict_t *self);
void SP_obj_moss3(edict_t *self);
void SP_obj_moss4(edict_t *self);
void SP_obj_moss5(edict_t *self);

void SP_obj_nest(edict_t *self);

void SP_obj_pick(edict_t *ent);
void SP_obj_pipe1(edict_t *ent);
void SP_obj_pipe2(edict_t *ent);
void SP_obj_pipewheel(edict_t *ent);
void SP_obj_plant1(edict_t *ent);
void SP_obj_plant2(edict_t *ent);
void SP_obj_plant3(edict_t *ent);
void SP_obj_pot1(edict_t *ent);
void SP_obj_pot2(edict_t *ent);
void SP_obj_pottedplant(edict_t *ent);
void SP_obj_pushcart(edict_t *ent);

void SP_obj_queenchair(edict_t *ent);
void SP_obj_queenthrone(edict_t *ent);

void SP_obj_ring_plaque2 (edict_t *ent);
void SP_obj_rocks1(edict_t *ent);
void SP_obj_rocks2(edict_t *ent);
void SP_obj_rope(edict_t *ent);
void SP_obj_ropechain (edict_t *ent);

void SP_obj_scroll(edict_t *ent);
void SP_obj_seasonglobe(edict_t *ent);
void SP_obj_shovel(edict_t *ent);
void SP_obj_shrine(edict_t *ent);
void SP_obj_sign1(edict_t *ent);
void SP_obj_sign4(edict_t *ent);
void SP_obj_skullpole(edict_t *ent);
void SP_obj_spellbook(edict_t *ent);
void SP_obj_stalactite1(edict_t *ent);
void SP_obj_stalactite2(edict_t *ent);
void SP_obj_stalactite3(edict_t *ent);
void SP_obj_stalagmite1(edict_t *ent);
void SP_obj_stalagmite2(edict_t *ent);
void SP_obj_stalagmite3(edict_t *ent);
void SP_obj_statue_corvus (edict_t *ent);
void SP_obj_statue_boulderfish (edict_t *ent);
void SP_obj_statue_dolphin1(edict_t *ent);
void SP_obj_statue_dolphin2(edict_t *ent);
void SP_obj_statue_dolphin3(edict_t *ent);
void SP_obj_statue_dolphin4(edict_t *ent);
void SP_obj_statue_dragon (edict_t *ent);
void SP_obj_statue_dragonhead (edict_t *ent);
void SP_obj_statue_duckbill1(edict_t *ent);
void SP_obj_statue_duckbill2(edict_t *ent);
void SP_obj_statue_guardian(edict_t *ent);
void SP_obj_statue_saraphbust(edict_t *ent);
void SP_obj_statue_sariph(edict_t *ent);
void SP_obj_statue_sithraguard(edict_t *ent);
void SP_obj_statue_tchecktrik_bust (edict_t *self);
void SP_obj_statue_techeckrikleft (edict_t *self);
void SP_obj_statue_techeckrikright (edict_t *self);
void SP_obj_statue_techeckriktomb (edict_t *self);
void SP_obj_stein(edict_t *ent);
void SP_obj_swampflat_top(edict_t *ent);
void SP_obj_swampflat_bottom(edict_t *ent);

void SP_obj_table1(edict_t *ent);
void SP_obj_table2(edict_t *ent);
void SP_obj_tapper(edict_t *ent);
void SP_obj_throne (edict_t *ent);
void SP_obj_torture_bed (edict_t *ent);
void SP_obj_torture_ironmaiden (edict_t *ent);
void SP_obj_torture_rack (edict_t *ent);
void SP_obj_torture_table (edict_t *ent);
void SP_obj_torture_wallring (edict_t *ent);
void SP_obj_tree(edict_t *ent);
void SP_obj_tree2(edict_t *ent);
void SP_obj_tree3(edict_t *ent);
void SP_obj_treefallen(edict_t *ent);
void SP_obj_treestump(edict_t *ent);
void SP_obj_treetall(edict_t *ent);
void SP_obj_treetop(edict_t *ent);

void SP_obj_urn (edict_t *ent);

void SP_obj_venusflytrap(edict_t *ent);

void SP_obj_wallringplaque(edict_t *ent);
void SP_obj_web(edict_t *ent);
void SP_obj_wheelbarrow(edict_t *ent);
void SP_obj_wheelbarrowdamaged(edict_t *ent);
void SP_obj_woodpile(edict_t *ent);

void SP_obj_morcalavin_barrier(edict_t *ent);

void SP_flamethrower(edict_t *ent);

void SP_item_spitter(edict_t *ent);

// shrine stuff

void shrine_heal (edict_t *ent);
void shrine_armor (edict_t *ent);
void shrine_armor_gold (edict_t *ent);
void shrine_staff (edict_t *ent);
void shrine_lung (edict_t *ent);
void shrine_light (edict_t *ent);
void shrine_mana (edict_t *ent);
void shrine_ghost (edict_t *ent);
void shrine_reflect (edict_t *ent);
void shrine_powerup (edict_t *ent);
void shrine_random (edict_t *ent);
void shrine_speed (edict_t *ent);

void SP_script_runner (edict_t *ent);

spawn_t	spawns[]= 
{
	// Quake2 specific spawns.
	{"info_player_start", SP_info_player_start,-1},
	{"info_player_deathmatch", SP_info_player_deathmatch,-1},
	{"info_player_coop", SP_info_player_coop,-1},
	{"info_player_intermission", SP_info_player_intermission,-1},

	{"func_plat", SP_func_plat,-1},
	{"func_button", SP_func_button,CID_BUTTON},
	{"func_door", SP_func_door,CID_FUNC_DOOR},
	{"func_door_secret", SP_func_door_secret,-1},
	{"func_door_rotating", SP_func_door_rotating,CID_FUNC_ROTATE},
	{"func_rotating", SP_func_rotating,CID_FUNC_ROTATE},
	{"func_train", SP_func_train,-1},
	{"func_water", SP_func_water,-1},
	{"func_areaportal", SP_func_areaportal,-1},
	{"func_monsterspawner", SP_func_monsterspawner,-1},
	{"func_wall", SP_func_wall,-1},
	{"func_object", SP_func_object,-1},
	{"func_timer", SP_func_timer,-1},

	{"trigger_Activate", SP_trigger_Activate,CID_TRIGGER},
	{"trigger_always", SP_trigger_Always,CID_TRIGGER},
	{"trigger_Damage", SP_trigger_Damage,CID_TRIG_DAMAGE},
	{"trigger_Deactivate", SP_trigger_Deactivate,CID_TRIGGER},
	{"trigger_counter", SP_trigger_Counter,CID_TRIGGER},
	{"trigger_elevator", SP_trigger_Elevator,CID_TRIGGER},
//	{"trigger_flamethrower", SP_trigger_flamethrower,CID_TRIGGER},
	{"trigger_fogdensity", SP_trigger_fogdensity,CID_TRIGGER},
	{"trigger_Gravity", SP_trigger_Gravity,-1},
	{"trigger_lightning", SP_trigger_lightning,CID_TRIGGER},
	{"trigger_mappercentage", SP_trigger_mappercentage,CID_TRIGGER},
	{"trigger_quit_to_menu", SP_trigger_quit_to_menu,CID_TRIGGER},
	{"trigger_mission_give", SP_trigger_mission_give,CID_TRIGGER},
	{"trigger_mission_take", SP_trigger_mission_take,CID_TRIGGER},
	{"trigger_MonsterJump", SP_trigger_MonsterJump,-1},
	{"trigger_goto_buoy", SP_trigger_goto_buoy,-1},
	{"trigger_multiple", SP_trigger_Multiple,CID_TRIGGER},
	{"trigger_playerpushbutton", SP_trigger_PlayerPushButton,CID_TRIGGER},
	{"trigger_playerpushlever", SP_trigger_PlayerPushLever,CID_TRIGGER},
	{"trigger_playerusepuzzle", SP_trigger_PlayerUsePuzzle,CID_TRIGGER},
	{"trigger_push", SP_trigger_push,CID_TRIG_PUSH},
	{"trigger_puzzle", SP_trigger_puzzle,CID_TRIGGER},
	{"trigger_once", SP_trigger_Once,CID_TRIGGER},
	{"trigger_quake", SP_trigger_quake,CID_TRIGGER},
	{"trigger_relay", SP_trigger_Relay,CID_TRIGGER},
	{"trigger_farclip", SP_trigger_farclip,CID_TRIGGER},
	{"trigger_endgame", SP_trigger_endgame,CID_TRIGGER},

	{"choose_CDTrack", SP_choose_CDTrack,-1},

	{"target_explosion", SP_target_explosion,-1},
	{"target_changelevel", SP_target_changelevel,-1},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger,-1},
	{"target_crosslevel_target", SP_target_crosslevel_target,-1},
	//{"target_actor", SP_target_actor,-1},
	{"target_lightramp", SP_target_lightramp,-1},
	{"target_earthquake", SP_target_earthquake,-1},

	{"worldspawn", SP_worldspawn,-1},

	{"light", SP_light,-1},
	{"info_null", SP_info_null,-1},
	{"func_group", SP_info_null,-1},
	{"info_notnull", SP_info_notnull,-1},
	{"path_corner", SP_path_corner,-1},
	{"point_combat", SP_point_combat,-1},

	//{"misc_actor", SP_misc_actor,-1},
	{"misc_teleporter", SP_misc_teleporter,CID_TELEPORTER},
	{"misc_teleporter_dest", SP_misc_teleporter_dest,-1},
	{"misc_update_spawner", misc_update_spawner, CID_TRIGGER},
	{"misc_remote_camera", SP_misc_remote_camera,-1},
	{"misc_magic_portal", SP_misc_magic_portal, -1},
	{"misc_fire_sparker", SP_misc_fire_sparker, -1},

	{"misc_flag", SP_misc_flag,-1},

	{"monster_gorgon",SP_monster_gorgon,CID_GORGON},
	{"monster_rat",SP_monster_rat,CID_RAT},
	{"monster_plagueElf", SP_monster_plagueElf,CID_PLAGUEELF}, 
	{"monster_fish", SP_monster_fish,CID_FISH}, 
	{"monster_harpy", SP_monster_harpy,CID_HARPY},
	{"monster_spreader", SP_monster_spreader,CID_SPREADER},
	{"monster_assassin",SP_monster_assassin,CID_ASSASSIN},
	{"monster_chicken",SP_monster_chicken,CID_CHICKEN},
	{"monster_tcheckrik_male",SP_monster_tcheckrik_male,CID_TCHECKRIK},
	{"monster_gkrokon", SP_Monster_Gkrokon,CID_GKROKON},

#if !DEMO_CODE
	{"monster_gorgon_leader",SP_monster_gorgon_leader,CID_GORGON},
	{"monster_rat_giant",SP_monster_rat_giant,CID_RAT},
	{"monster_palace_plague_guard", SP_monster_palace_plague_guard,CID_PLAGUEELF}, 
	{"monster_palace_plague_guard_invisible", SP_monster_palace_plague_guard_invisible,CID_PLAGUEELF}, 
	{"monster_elflord", SP_monster_elflord,CID_ELFLORD},
	{"monster_ssithra",SP_monster_plague_ssithra,CID_SSITHRA},
	{"monster_mssithra",SP_monster_mssithra,CID_MSSITHRA},
	{"monster_chkroktk",SP_monster_chkroktk,CID_RAT},
	{"monster_tcheckrik_female",SP_monster_tcheckrik_female,CID_TCHECKRIK},
	{"monster_tcheckrik_mothers",SP_monster_tcheckrik_mothers,CID_MOTHER},
	{"monster_high_priestess",SP_monster_high_priestess,CID_HIGHPRIESTESS},
	{"monster_ogle",SP_monster_ogle,CID_OGLE},
	{"monster_seraph_overlord",SP_monster_seraph_overlord,CID_SERAPH_OVERLORD},
	{"monster_seraph_guard",SP_monster_seraph_guard,CID_SERAPH_GUARD},
	{"monster_bee",SP_monster_bee,CID_BEE},
	{"monster_morcalavin",SP_monster_morcalavin,CID_MORK},
	{"monster_trial_beast",SP_monster_trial_beast,CID_TBEAST},
	{"monster_imp", SP_monster_imp,CID_IMP},
#endif
	{"character_corvus1",SP_character_corvus1,CID_CORVUS},
	{"character_corvus2",SP_character_corvus2,CID_CORVUS2},
	{"character_corvus3",SP_character_corvus3,CID_CORVUS3},
	{"character_corvus4",SP_character_corvus4,CID_CORVUS4},
	{"character_corvus5",SP_character_corvus5,CID_CORVUS5},
	{"character_corvus6",SP_character_corvus6,CID_CORVUS6},
	{"character_corvus7",SP_character_corvus7,CID_CORVUS7},
	{"character_corvus8",SP_character_corvus8,CID_CORVUS8},
	{"character_corvus9",SP_character_corvus9,CID_CORVUS9},
	{"character_dranor",SP_character_dranor,CID_DRANOR},
	{"character_elflord",SP_character_elflord,CID_C_ELFLORD},
	{"character_highpriestess",SP_character_highpriestess,CID_C_HIGHPRIESTESS},
	{"character_highpriestess2",SP_character_highpriestess2,CID_C_HIGHPRIESTESS2},
	{"character_morcalavin",SP_character_morcalavin,CID_C_MORCALAVIN},
	{"character_sidhe_guard",SP_character_sidhe_guard,CID_PLAGUEELF},
	{"character_siernan1",SP_character_siernan1,CID_C_SIERNAN1},
	{"character_siernan2",SP_character_siernan2,CID_C_SIERNAN2},
	{"character_ssithra_scout",SP_character_ssithra_scout,CID_SSITHRA_SCOUT},
	{"character_ssithra_victim",SP_character_ssithra_victim,CID_SSITHRA_VICTIM},
	{"character_tome",SP_character_tome,CID_C_TOME},

	// Heretic2 specific spawns.

	{"breakable_brush",SP_breakable_brush,CID_BBRUSH},


	{"light_walltorch",SP_light_walltorch,CID_LIGHT},
	{"light_floortorch",SP_light_floortorch,CID_LIGHT},
//	{"light_flame",SP_light_flame,CID_LIGHT},
	{"light_torch1",SP_light_torch1,CID_LIGHT},
	{"light_gem2",SP_light_gem2,CID_LIGHT},
	{"light_chandelier1",SP_light_chandelier1,CID_LIGHT},
	{"light_chandelier2",SP_light_chandelier2,CID_LIGHT},
	{"light_chandelier3",SP_light_chandelier3,CID_LIGHT},
	{"light_lantern1",SP_light_lantern1,CID_LIGHT},
	{"light_lantern2",SP_light_lantern2,CID_LIGHT},
	{"light_lantern3",SP_light_lantern3,CID_LIGHT},
	{"light_lantern4",SP_light_lantern4,CID_LIGHT},
	{"light_lantern5",SP_light_lantern5,CID_LIGHT},
	{"light_buglight",SP_light_buglight,CID_LIGHT},

	{"env_fire",SP_env_fire,CID_OBJECT},
	{"env_dust",SP_env_dust,CID_OBJECT},
	{"env_smoke",SP_env_smoke,CID_OBJECT},
	{"env_mist",SP_env_mist,CID_OBJECT},
	{"env_bubbler",SP_env_bubbler,CID_OBJECT},
	{"env_water_drip",SP_env_water_drip,CID_OBJECT},
	{"env_water_fountain",SP_env_water_fountain,CID_OBJECT},
	{"env_waterfall_base",SP_env_waterfall_base,CID_OBJECT},
	{"env_sun1",SP_env_sun1,CID_OBJECT},
	{"env_muck",SP_env_muck,CID_OBJECT},
//	{"env_galaxy",SP_env_galaxy,CID_OBJECT},

	{"sound_ambient_silverspring",SP_sound_ambient_silverspring,-1},
	{"sound_ambient_swampcanyon",SP_sound_ambient_swampcanyon,-1},
	{"sound_ambient_andoria",SP_sound_ambient_andoria,-1},
	{"sound_ambient_hive",SP_sound_ambient_hive,-1},
	{"sound_ambient_mine",SP_sound_ambient_mine,-1},
	{"sound_ambient_cloudfortress",SP_sound_ambient_cloudfortress,-1},

	{"obj_andwallhanging",SP_obj_andwallhanging,CID_OBJECT},
	{"obj_banner",SP_obj_banner,CID_OBJECT},
	{"obj_banneronpole",SP_obj_banneronpole,CID_OBJECT},
	{"obj_barrel",SP_obj_barrel,CID_OBJECT},
	{"obj_barrel_explosive",SP_obj_barrel_explosive,CID_OBJECT},
	{"obj_barrel_metal",SP_obj_barrel_metal,CID_OBJECT},
	{"obj_basket",SP_obj_basket,CID_OBJECT},
	{"obj_bench",SP_obj_bench,CID_OBJECT},
	{"obj_bigcrystal",SP_obj_bigcrystal,CID_OBJECT},
	{"obj_biotank",SP_obj_biotank,CID_OBJECT},
	{"obj_bloodsplat",SP_obj_bloodsplat,CID_OBJECT},
	{"obj_bookclosed",SP_obj_bookclosed,CID_OBJECT},
	{"obj_bookopen",SP_obj_bookopen,CID_OBJECT},
	{"obj_bottle1",SP_obj_bottle1,CID_OBJECT},
	{"obj_broom",SP_obj_broom,CID_OBJECT},
	{"obj_bucket",SP_obj_bucket,CID_OBJECT},
	{"obj_bush1",SP_obj_bush1,CID_OBJECT},
	{"obj_bush2",SP_obj_bush2,CID_OBJECT},
	{"obj_cactus",SP_obj_cactus,CID_OBJECT},
	{"obj_cactus3",SP_obj_cactus3,CID_OBJECT},
	{"obj_cactus4",SP_obj_cactus4,CID_OBJECT},
	{"obj_cauldron",SP_obj_cauldron,CID_OBJECT},
	{"obj_chair1",SP_obj_chair1,CID_OBJECT},
	{"obj_chair2",SP_obj_chair2,CID_OBJECT},
	{"obj_chair3",SP_obj_chair3,CID_OBJECT},
	{"obj_chest1",SP_obj_chest1,CID_OBJECT},
	{"obj_chest2",SP_obj_chest2,CID_OBJECT},
	{"obj_chest3",SP_obj_chest3,CID_OBJECT},
	{"obj_choppeddude",SP_obj_choppeddude,CID_OBJECT},
	{"obj_claybowl",SP_obj_claybowl,CID_OBJECT},
	{"obj_clayjar",SP_obj_clayjar,CID_OBJECT},
	{"obj_cocoon",SP_obj_cocoon,CID_OBJECT},
	{"obj_cocoonopen",SP_obj_cocoonopen,CID_OBJECT},
	{"obj_cog1",SP_obj_cog1,CID_OBJECT},
	{"obj_corpse1",SP_obj_corpse1,CID_OBJECT},
	{"obj_corpse2",SP_obj_corpse2,CID_OBJECT},
	{"obj_corpse_ogle",SP_obj_corpse_ogle,CID_OBJECT},
	{"obj_corpse_ssithra",SP_obj_corpse_ssithra,CID_OBJECT},
	{"obj_dying_elf",SP_obj_dying_elf,CID_OBJECT},
	{"obj_eggpan",SP_obj_eggpan,CID_OBJECT},
	{"obj_eyeball_jar",SP_obj_eyeball_jar,CID_OBJECT},
	{"obj_firepot",SP_obj_firepot,CID_OBJECT},
	{"obj_fishhead1",SP_obj_fishhead1,CID_OBJECT},
	{"obj_fishhead2",SP_obj_fishhead2,CID_OBJECT},
	{"obj_fishtrap",SP_obj_fishtrap,CID_OBJECT},
	{"obj_flagonpole",SP_obj_flagonpole,CID_OBJECT},
	{"obj_floor_candelabrum",SP_obj_floor_candelabrum,CID_OBJECT},
	{"obj_fountain_fish",SP_obj_fountain_fish,CID_OBJECT},
	{"obj_frypan",SP_obj_frypan,CID_OBJECT},
	{"obj_gascan",SP_obj_gascan,CID_OBJECT},
	{"obj_gorgonbones",SP_obj_gorgonbones,CID_OBJECT},
	{"obj_grass",SP_obj_grass,CID_OBJECT},
	{"obj_hangingdude",SP_obj_hangingdude,CID_OBJECT},
	{"obj_hanging_ogle",SP_obj_hanging_ogle,CID_OBJECT},
	{"obj_hivepriestessssymbol",SP_obj_hivepriestessssymbol,CID_OBJECT},
	{"obj_jawbone",SP_obj_jawbone,CID_OBJECT},
	{"obj_jug1",SP_obj_jug1,CID_OBJECT},
	{"obj_kettle",SP_obj_kettle,CID_OBJECT},
	{"obj_lab_parts_container",SP_obj_lab_parts_container,CID_OBJECT},
	{"obj_lab_tray",SP_obj_lab_tray,CID_OBJECT},
	{"obj_larva",SP_obj_larva,CID_OBJECT},
	{"obj_larvabrokenegg",SP_obj_larvabrokenegg,CID_OBJECT},
	{"obj_larvaegg",SP_obj_larvaegg,CID_OBJECT},
	{"obj_lever1",SP_obj_lever1,CID_LEVER},
	{"obj_lever2",SP_obj_lever2,CID_LEVER},
	{"obj_lever3",SP_obj_lever3,CID_LEVER},
	{"obj_metalchunk1",SP_obj_metalchunk1,CID_OBJECT},
	{"obj_metalchunk2",SP_obj_metalchunk2,CID_OBJECT},
	{"obj_metalchunk3",SP_obj_metalchunk3,CID_OBJECT},
	{"obj_minecart",SP_obj_minecart,CID_OBJECT},
	{"obj_minecart2",SP_obj_minecart2,CID_OBJECT},
	{"obj_minecart3",SP_obj_minecart3,CID_OBJECT},
	{"obj_moss1",SP_obj_moss1,CID_OBJECT},
	{"obj_moss2",SP_obj_moss2,CID_OBJECT},
	{"obj_moss3",SP_obj_moss3,CID_OBJECT},
	{"obj_moss4",SP_obj_moss4,CID_OBJECT},
	{"obj_moss5",SP_obj_moss5,CID_OBJECT},
	{"obj_nest",SP_obj_nest,CID_OBJECT},
	{"obj_pick",SP_obj_pick,CID_OBJECT},
	{"obj_pipe1",SP_obj_pipe1,CID_OBJECT},
	{"obj_pipe2",SP_obj_pipe2,CID_OBJECT},
	{"obj_pipewheel",SP_obj_pipewheel,CID_OBJECT},
	{"obj_plant1",SP_obj_plant1,CID_OBJECT},
	{"obj_plant2",SP_obj_plant2,CID_OBJECT},
	{"obj_plant3",SP_obj_plant3,CID_OBJECT},
	{"obj_pot1",SP_obj_pot1,CID_OBJECT},
	{"obj_pot2",SP_obj_pot2,CID_OBJECT},
	{"obj_pottedplant",SP_obj_pottedplant,CID_OBJECT},
	{"obj_pushcart",SP_obj_pushcart,CID_OBJECT},
	{"obj_queenthrone",SP_obj_queenthrone,CID_OBJECT},
	{"obj_queenchair",SP_obj_queenchair,CID_OBJECT},
	{"obj_ring_plaque2",SP_obj_ring_plaque2,CID_OBJECT},
	{"obj_rocks1",SP_obj_rocks1,CID_OBJECT},
	{"obj_rocks2",SP_obj_rocks2,CID_OBJECT},
	{"obj_rope",SP_obj_rope,CID_OBJECT},
	{"obj_ropechain",SP_obj_ropechain,CID_OBJECT},
	{"obj_scroll",SP_obj_scroll,CID_OBJECT},
	{"obj_seasonglobe",SP_obj_seasonglobe,CID_OBJECT},
	{"obj_shovel",SP_obj_shovel,CID_OBJECT},
	{"obj_shrine",SP_obj_shrine,CID_OBJECT},
	{"obj_sign1",SP_obj_sign1,CID_OBJECT},
	{"obj_sign4",SP_obj_sign4,CID_OBJECT},
	{"obj_skullpole",SP_obj_skullpole,CID_OBJECT},
	{"obj_spellbook",SP_obj_spellbook,CID_OBJECT},
	{"obj_stalactite1",SP_obj_stalactite1,CID_OBJECT},
	{"obj_stalactite2",SP_obj_stalactite2,CID_OBJECT},
	{"obj_stalactite3",SP_obj_stalactite3,CID_OBJECT},
	{"obj_stalagmite1",SP_obj_stalagmite1,CID_OBJECT},
	{"obj_stalagmite2",SP_obj_stalagmite2,CID_OBJECT},
	{"obj_stalagmite3",SP_obj_stalagmite3,CID_OBJECT},
	{"obj_statue_boulderfish",SP_obj_statue_boulderfish,CID_OBJECT},
	{"obj_statue_corvus",SP_obj_statue_corvus,CID_OBJECT},
	{"obj_statue_dolphin1",SP_obj_statue_dolphin1,CID_OBJECT},
	{"obj_statue_dolphin2",SP_obj_statue_dolphin2,CID_OBJECT},
	{"obj_statue_dolphin3",SP_obj_statue_dolphin3,CID_OBJECT},
	{"obj_statue_dolphin4",SP_obj_statue_dolphin4,CID_OBJECT},
	{"obj_statue_dragon",SP_obj_statue_dragon,CID_OBJECT},
	{"obj_statue_dragonhead",SP_obj_statue_dragonhead,CID_OBJECT},
	{"obj_statue_duckbill1",SP_obj_statue_duckbill1,CID_OBJECT},
	{"obj_statue_duckbill2",SP_obj_statue_duckbill2,CID_OBJECT},
	{"obj_statue_guardian",SP_obj_statue_guardian,CID_OBJECT},
	{"obj_statue_saraphbust",SP_obj_statue_saraphbust,CID_OBJECT},
	{"obj_statue_sariph",SP_obj_statue_sariph,CID_OBJECT},
	{"obj_statue_sithraguard",SP_obj_statue_sithraguard,CID_OBJECT},
	{"obj_statue_tchecktrik_bust",SP_obj_statue_tchecktrik_bust,CID_OBJECT},
	{"obj_statue_techeckrikleft",SP_obj_statue_techeckrikleft,CID_OBJECT},
	{"obj_statue_techeckrikright",SP_obj_statue_techeckrikright,CID_OBJECT},
	{"obj_statue_techeckriktomb",SP_obj_statue_techeckriktomb,CID_OBJECT},
	{"obj_stein",SP_obj_stein,CID_OBJECT},
	{"obj_swampflat_top",SP_obj_swampflat_top,CID_OBJECT},
	{"obj_swampflat_bottom",SP_obj_swampflat_bottom,CID_OBJECT},
	{"obj_table1",SP_obj_table1,CID_OBJECT},
	{"obj_table2",SP_obj_table2,CID_OBJECT},
	{"obj_tapper",SP_obj_tapper,CID_OBJECT},
	{"obj_throne",SP_obj_throne,CID_OBJECT},
	{"obj_torture_bed",SP_obj_torture_bed,CID_OBJECT},
	{"obj_torture_ironmaiden",SP_obj_torture_ironmaiden,CID_OBJECT},
	{"obj_torture_rack",SP_obj_torture_rack,CID_OBJECT},
	{"obj_torture_table",SP_obj_torture_table,CID_OBJECT},
	{"obj_torture_wallring",SP_obj_torture_wallring,CID_OBJECT},
	{"obj_tree",SP_obj_tree,CID_OBJECT},
	{"obj_tree2",SP_obj_tree2,CID_OBJECT},
	{"obj_tree3",SP_obj_tree3,CID_OBJECT},
	{"obj_treefallen",SP_obj_treefallen,CID_OBJECT},
	{"obj_treestump",SP_obj_treestump,CID_OBJECT},
	{"obj_treetall",SP_obj_treetall,CID_OBJECT},
	{"obj_treetop",SP_obj_treetop,CID_OBJECT},
	{"obj_urn",SP_obj_urn,CID_OBJECT},
	{"obj_venusflytrap",SP_obj_venusflytrap,CID_OBJECT},
	{"obj_wallringplaque",SP_obj_wallringplaque,CID_OBJECT},
	{"obj_web",SP_obj_web,CID_OBJECT},
	{"obj_wheelbarrow",SP_obj_wheelbarrow,CID_OBJECT},
	{"obj_wheelbarrowdamaged",SP_obj_wheelbarrowdamaged,CID_OBJECT},
	{"obj_woodpile",SP_obj_woodpile,CID_OBJECT},

	{"obj_morcalavin_barrier",SP_obj_morcalavin_barrier,CID_OBJECT},
	
	{"flamethrower",SP_flamethrower,CID_FLAMETHROWER},

	{"item_spitter", SP_item_spitter,-1},

	{"info_buoy", SP_info_buoy,-1},

	{"shrine_heal", shrine_heal, CID_TRIGGER},
	{"shrine_armor", shrine_armor, CID_TRIGGER},
	{"shrine_staff", shrine_staff, CID_TRIGGER},
	{"shrine_lung", shrine_lung, CID_TRIGGER},
	{"shrine_armor_gold", shrine_armor_gold, CID_TRIGGER},
	{"shrine_light", shrine_light, CID_TRIGGER},
	{"shrine_mana", shrine_mana, CID_TRIGGER},
	{"shrine_ghost", shrine_ghost, CID_TRIGGER},
	{"shrine_reflect", shrine_reflect, CID_TRIGGER},
	{"shrine_powerup", shrine_powerup, CID_TRIGGER},
	{"shrine_speed", shrine_speed, CID_TRIGGER},
	{"shrine_random", shrine_random, CID_TRIGGER},

	{"script_runner", SP_script_runner, CID_TRIGGER},
	{NULL, NULL,-1}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t *ent)
{
	extern qboolean loadingBaseEnts;

	spawn_t	*s;
	gitem_t	*item;

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		return;
	}

	if(item = IsItem(ent))
	{
		SpawnItem(ent, item);

		return;
	}

	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			if((s->CID != -1) && !Cid_init[s->CID])	 	// Need to call once per level that item is on
			{
				classStaticsInits[s->CID]();	
				Cid_init[s->CID] = -1;
				ent->classID = s->CID;						// Make sure classID is set
			}

			ent->classID = 0;
			if(s->CID != -1)
			{
				ent->classID = s->CID;
			}
			s->spawn (ent);								// Need to call for every item
			return;
		}
	}
	gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
}

// end