/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "g_headers.h"

#include "b_local.h"
#include "g_local.h"
#include "wp_saber.h"
#include "w_local.h"
#include "g_functions.h"

//---------------
//	Blaster
//---------------

//---------------------------------------------------------
static void WP_FireBlasterMissile( gentity_t *ent, vec3_t start, vec3_t dir, qboolean altFire )
//---------------------------------------------------------
{
	int velocity, size;

	if (altFire)
	{
		velocity = weaponData[WP_BLASTER].altVelocity;
		size = weaponData[WP_BLASTER].altMissileSize;
	}
	else
	{
		velocity = weaponData[WP_BLASTER].velocity;
		size = weaponData[WP_BLASTER].missileSize;
	}

	// Do the damages
	int damage = ent->NPC
		? NPC_Damage(weaponData[WP_BLASTER].npcDamages, g_spskill->integer)
		: !altFire ? weaponData[WP_BLASTER].damage : weaponData[WP_BLASTER].altDamage;

	// If an enemy is shooting at us, lower the velocity so you have a chance to evade
	if ( ent->client && ent->client->ps.clientNum != 0 )
	{
		if ( g_spskill->integer < 2 )
		{
			velocity *= BLASTER_NPC_VEL_CUT;
		}
		else
		{
			velocity *= BLASTER_NPC_HARD_VEL_CUT;
		}
	}

	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );//make sure our start point isn't on the other side of a wall

	gentity_t *missile = CreateMissile( start, dir, velocity, 10000, ent, altFire );

	missile->classname = "blaster_proj";
	missile->s.weapon = WP_BLASTER;

//	if ( ent->client )
//	{
//		if ( ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > 0 && ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > cg.time )
//		{
//			// in overcharge mode, so doing double damage
//			missile->flags |= FL_OVERCHARGED;
//			damage *= 2;
//		}
//	}

	if (size)
	{
		// Make it easier to hit things
		VectorSet( missile->maxs, size, size, size );
		VectorScale( missile->maxs, -1, missile->mins );
	}

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	if ( altFire )
	{
		missile->methodOfDeath = MOD_BLASTER_ALT;
	}
	else
	{
		missile->methodOfDeath = MOD_BLASTER;
	}
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}

//---------------------------------------------------------
void WP_FireBlaster( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	float	spread;
	vec3_t	dir, angs;

	vectoangles( wpFwd, angs );

	if ( alt_fire )
	{
		spread = weaponData[WP_BLASTER].altSpread;
		// add some slop to the alt-fire direction
		angs[PITCH] += Q_flrand(-1.0f, 1.0f) * spread;
		angs[YAW]	+= Q_flrand(-1.0f, 1.0f) * spread;
	}
	else
	{
		// Troopers use their aim values as well as the gun's inherent inaccuracy
		// so check for all classes of stormtroopers and anyone else that has aim error
		if ( ent->client && ent->NPC &&
			( ent->client->NPC_class == CLASS_STORMTROOPER ||
			ent->client->NPC_class == CLASS_SWAMPTROOPER ) )
		{
			angs[PITCH] += ( Q_flrand(-1.0f, 1.0f) * (BLASTER_NPC_SPREAD+(6-ent->NPC->currentAim)*0.25f));//was 0.5f
			angs[YAW]	+= ( Q_flrand(-1.0f, 1.0f) * (BLASTER_NPC_SPREAD+(6-ent->NPC->currentAim)*0.25f));//was 0.5f
		}
		else
		{
			spread = weaponData[WP_BLASTER].spread;
			// add some slop to the main-fire direction
			angs[PITCH] += Q_flrand(-1.0f, 1.0f) * spread;
			angs[YAW]	+= Q_flrand(-1.0f, 1.0f) * spread;
		}
	}

	AngleVectors( angs, dir, NULL, NULL );

	// FIXME: if temp_org does not have clear trace to inside the bbox, don't shoot!
	WP_FireBlasterMissile( ent, wpMuzzle, dir, alt_fire );
}

// vim: set noexpandtab tabstop=4 shiftwidth=4 :
