//
// ParticleFlags.h -- split from clfx/Particle.h, because some of those are also used by renderer --mxd.
//
// Copyright 1998 Raven Software
//

#pragma once

// Particle render flags, < 127 is the type.
#define PFL_FLAG_MASK	0x0000007f	// Mask out any flags.

#define PFL_LM_COLOR	0x00000100	//mxd. Use lightmap color instead of entity color.

// Move flags... Note, these are pretty much mutually exclusive.
#define PFL_MOVE_MASK	0x0f000000	// Check how the particles move.

#define PFL_MOVE_CYL_X	0x01000000	// Cylindrical on the X axis.					Yaw, Radius, Z.
#define PFL_MOVE_CYL_Y	0x02000000	// Cylindrical on the Y axis.
#define PFL_MOVE_CYL_Z	0x04000000	// Cylindrical on the Z axis.
#define PFL_MOVE_SPHERE	0x08000000	// Move using spherical coordinates.			Pitch, Yaw, Radius.
#define PFL_MOVE_NORM	0x00000000	// Move using normal cartesian coordinates.		X, Y, Z.

// Additional particle flags.
#define PFL_PULSE_ALPHA	0x10000000	// If the alpha delta's to above 255, it "bounces" back down towards zero.
#define PFL_SOFT_MASK	0x20000000	// For defining single point particles in software (unused in gl - here to stop people using this bit).
#define PFL_ADDITIVE	0x40000000	// Particle is added to additive particle list.
#define PFL_NEARCULL	0x80000000	// Force near culling.