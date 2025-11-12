//
// g_obj.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "FX.h"
#include "q_shared.h"
#include "q_Typedef.h"

// Spawnflags for object entities.
#define SF_OBJ_INVULNERABLE		1
#define SF_OBJ_ANIMATE			2
#define SF_OBJ_EXPLODING		4
#define SF_OBJ_NOPUSH			8

extern void BboxYawAndScale(edict_t* self);

extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);
extern void ObjectInit(edict_t* self, int health, int mass, MaterialID_t material_type, int solid);
extern void ObjectStaticsInit(void);
extern void LeverStaticsInit(void);

extern void SP_obj_banner(edict_t* self);
extern void SP_obj_banneronpole(edict_t* self);
extern void SP_obj_barrel(edict_t* self);
extern void SP_obj_broom(edict_t* self);
extern void SP_obj_chair1(edict_t* self);
extern void SP_obj_chair2(edict_t* self);
extern void SP_obj_chair3(edict_t* self);
extern void SP_obj_chest1(edict_t* self);
extern void SP_obj_chest2(edict_t* self);
extern void SP_obj_chest3(edict_t* self);
extern void SP_obj_cog1(edict_t* self);
extern void SP_obj_corpse1(edict_t* self);
extern void SP_obj_corpse2(edict_t* self);
extern void SP_obj_dying_elf(edict_t* self);
extern void SP_obj_sign1(edict_t* self);
extern void SP_obj_sign4(edict_t* self);
extern void SP_obj_stalactite1(edict_t* self);
extern void SP_obj_stalactite2(edict_t* self);
extern void SP_obj_stalactite3(edict_t* self);
extern void SP_obj_stalagmite1(edict_t* self);
extern void SP_obj_stalagmite2(edict_t* self);
extern void SP_obj_stalagmite3(edict_t* self);
extern void SP_obj_statue_corvus(edict_t* self);
extern void SP_obj_statue_dolphin1(edict_t* self);
extern void SP_obj_statue_dolphin2(edict_t* self);
extern void SP_obj_statue_dolphin3(edict_t* self);
extern void SP_obj_statue_dolphin4(edict_t* self);
extern void SP_obj_statue_guardian(edict_t* self);
extern void SP_obj_table1(edict_t* self);
extern void SP_obj_table2(edict_t* self);
extern void SP_obj_throne(edict_t* self);
extern void SP_obj_kettle(edict_t* self);
extern void SP_obj_cauldron(edict_t* self);
extern void SP_obj_firepot(edict_t* self);
extern void SP_obj_statue_duckbill1(edict_t* self);
extern void SP_obj_statue_duckbill2(edict_t* self);
extern void SP_obj_seasonglobe(edict_t* bottom);
extern void SP_obj_stein(edict_t* self);
extern void SP_obj_scroll(edict_t* self);
extern void SP_obj_fountain_fish(edict_t* self);
extern void SP_obj_statue_boulderfish(edict_t* self);
extern void SP_obj_pottedplant(edict_t* self);
extern void SP_obj_plant1(edict_t* self);
extern void SP_obj_plant2(edict_t* self);
extern void SP_obj_plant3(edict_t* self);
extern void SP_obj_treetop(edict_t* self);
extern void SP_obj_tree(edict_t* self);
extern void SP_obj_tree2(edict_t* self);
extern void SP_obj_tree3(edict_t* self);
extern void SP_obj_treetall(edict_t* self);
extern void SP_obj_treefallen(edict_t* self);
extern void SP_obj_shovel(edict_t* self);
extern void SP_obj_woodpile(edict_t* self);
extern void SP_obj_fishtrap(edict_t* self);
extern void SP_obj_bench(edict_t* self);
extern void SP_obj_bucket(edict_t* self);
extern void SP_obj_ropechain(edict_t* self);
extern void SP_obj_wheelbarrow(edict_t* self);
extern void SP_obj_wheelbarrowdamaged(edict_t* self);
extern void SP_obj_urn(edict_t* self);
extern void SP_obj_bigcrystal(edict_t* self);
extern void SP_obj_moss1(edict_t* self);
extern void SP_obj_moss2(edict_t* self);
extern void SP_obj_moss3(edict_t* self);
extern void SP_obj_moss4(edict_t* self);
extern void SP_obj_moss5(edict_t* self);
extern void SP_obj_floor_candelabrum(edict_t* self);
extern void SP_obj_statue_dragonhead(edict_t* self);
extern void SP_obj_statue_dragon(edict_t* self);
extern void SP_obj_flagonpole(edict_t* self);
extern void SP_obj_lever1(edict_t* self);
extern void SP_obj_lever2(edict_t* self);
extern void SP_obj_lever3(edict_t* self);
extern void SP_obj_bush1(edict_t* self);
extern void SP_obj_bush2(edict_t* self);
extern void SP_obj_cactus(edict_t* self);
extern void SP_obj_cactus3(edict_t* self);
extern void SP_obj_cactus4(edict_t* self);
extern void SP_obj_basket(edict_t* self);
extern void SP_obj_claybowl(edict_t* self);
extern void SP_obj_clayjar(edict_t* self);
extern void SP_obj_gorgonbones(edict_t* self);
extern void SP_obj_grass(edict_t* self);
extern void SP_obj_swampflat_top(edict_t* self);
extern void SP_obj_swampflat_bottom(edict_t* self);
extern void SP_obj_treestump(edict_t* self);
extern void SP_obj_jawbone(edict_t* self);
extern void SP_obj_barrel_metal(edict_t* self);
extern void SP_obj_barrel_explosive(edict_t* self);
extern void SP_obj_gascan(edict_t* self);
extern void SP_obj_pipe1(edict_t* self);
extern void SP_obj_pipe2(edict_t* self);
extern void SP_obj_pipewheel(edict_t* self);
extern void SP_obj_minecart(edict_t* self);
extern void SP_obj_minecart2(edict_t* self);
extern void SP_obj_minecart3(edict_t* self);
extern void SP_obj_andwallhanging(edict_t* self);
extern void SP_obj_pick(edict_t* self);
extern void SP_obj_metalchunk1(edict_t* self);
extern void SP_obj_metalchunk2(edict_t* self);
extern void SP_obj_metalchunk3(edict_t* self);
extern void SP_obj_rocks1(edict_t* self);
extern void SP_obj_rocks2(edict_t* self);
extern void SP_obj_hivepriestessssymbol(edict_t* self);
extern void SP_obj_queenthrone(edict_t* self);
extern void SP_obj_queenchair(edict_t* self);
extern void SP_obj_larvaegg(edict_t* self);
extern void SP_obj_larvabrokenegg(edict_t* self);
extern void SP_obj_cocoon(edict_t* self);
extern void SP_obj_cocoonopen(edict_t* self);
extern void SP_obj_venusflytrap(edict_t* self);
extern void SP_obj_statue_techeckriktomb(edict_t* self);
extern void SP_obj_statue_techeckrikright(edict_t* self);
extern void SP_obj_statue_techeckrikleft(edict_t* self);
extern void SP_obj_spellbook(edict_t* self);
extern void SP_obj_skullpole(edict_t* self);
extern void SP_obj_pot1(edict_t* self);
extern void SP_obj_pot2(edict_t* self);
extern void SP_obj_bottle1(edict_t* self);
extern void SP_obj_jug1(edict_t* self);
extern void SP_obj_torture_table(edict_t* self);
extern void SP_obj_torture_wallring(edict_t* self);
extern void SP_obj_statue_tchecktrik_bust(edict_t* self);
extern void SP_obj_statue_sithraguard(edict_t* self);
extern void SP_obj_torture_ironmaiden(edict_t* self);
extern void SP_obj_torture_rack(edict_t* self);
extern void SP_obj_torture_bed(edict_t* self);
extern void SP_obj_statue_saraphbust(edict_t* self);
extern void SP_obj_biotank(edict_t* self);
extern void SP_obj_tapper(edict_t* self);
extern void SP_obj_wallringplaque(edict_t* self);
extern void SP_obj_hangingdude(edict_t* self);
extern void SP_obj_frypan(edict_t* self);
extern void SP_obj_eggpan(edict_t* self);
extern void SP_obj_nest(edict_t* self);
extern void SP_obj_choppeddude(edict_t* self);
extern void SP_obj_lab_parts_container(edict_t* self);
extern void SP_obj_eyeball_jar(edict_t* self);
extern void SP_obj_lab_tray(edict_t* self);
extern void SP_obj_hanging_ogle(edict_t* self);
extern void SP_obj_ring_plaque2(edict_t* self);
extern void SP_obj_statue_sariph(edict_t* self);
extern void SP_obj_pushcart(edict_t* self);
extern void SP_obj_bookopen(edict_t* self);
extern void SP_obj_bookclosed(edict_t* self);
extern void SP_obj_web(edict_t* self);
extern void SP_obj_larva(edict_t* self);
extern void SP_obj_bloodsplat(edict_t* self);

//mxd. Required by save system...
extern void ObjBarrelDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void ObjBarrelExplodeThink(edict_t* self);
extern void ObjBiotankContentsAnimThink(edict_t* self);
extern void ObjBiotankTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ObjCactus4CloseThink(edict_t* self);
extern void ObjCactus4OpenThink(edict_t* self);
extern void ObjCactus4Use(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjCactusTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ObjChest1AnimThink(edict_t* self);
extern void ObjChest1Use(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjCog1AnimThink(edict_t* self);
extern void ObjCog1Use(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjDyingElfDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void ObjDyingElfIdle(edict_t* self);
extern void ObjDyingElfPain(edict_t* self, edict_t* other, float kick, int damage);
extern void ObjDyingElfReachAnim(edict_t* self);
extern void ObjDyingElfTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ObjHangingOgleMoanThink(edict_t* self);
extern void ObjHivePriestessSymbolThink(edict_t* self);
extern void ObjHivePriestessSymbolUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjLarvaAnimThink(edict_t* self);
extern void ObjLarvaTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ObjLeverDownThink(edict_t* self);
extern void ObjLeverUpThink(edict_t* self);
extern void ObjLeverUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjSeasonglobeBottomThink(edict_t* self);
extern void ObjSeasonglobeBottomUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjSeasonglobeTopThink(edict_t* self);
extern void ObjSeasonglobeTopUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjSpellbookAnimThink(edict_t* self);
extern void ObjSpellbookUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjStatueSsithraGuardThink(edict_t* self);
extern void ObjStatueSsithraGuardUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjStatueTchecktrikBustUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjStatueTecheckrikTombUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjStatueTecheckrikUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ObjTortureIronmaidenClose(edict_t* self);
extern void ObjTortureIronmaidenOpen(edict_t* self);
extern void ObjTortureIronmaidenTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ObjTortureIronmaidenUse(edict_t* self, edict_t* other, edict_t* activator);
extern void PushableObjectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);