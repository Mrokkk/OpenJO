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

#ifndef __G_SHARED_H__
#define __G_SHARED_H__

#include "bg_public.h"
#include "g_public.h"
#include "b_public.h"
#include "icarus/icarus.h"
#include "rd-common/tr_types.h"
#include "cgame/cg_public.h"
#include "bset.h"

#define	FOFS(x) offsetof(gentity_t, x)

typedef struct centity_s centity_t;
class CSequencer;
class CTaskManager;

enum
{
	HL_NONE = 0,
	HL_FOOT_RT,
	HL_FOOT_LT,
	HL_LEG_RT,
	HL_LEG_LT,
	HL_WAIST,
	HL_BACK_RT,
	HL_BACK_LT,
	HL_BACK,
	HL_CHEST_RT,
	HL_CHEST_LT,
	HL_CHEST,
	HL_ARM_RT,
	HL_ARM_LT,
	HL_HAND_RT,
	HL_HAND_LT,
	HL_HEAD,
	HL_GENERIC1,
	HL_GENERIC2,
	HL_GENERIC3,
	HL_GENERIC4,
	HL_GENERIC5,
	HL_GENERIC6,
	HL_MAX
};

typedef enum //# taskID_e
{
	TID_CHAN_VOICE = 0,	// Waiting for a voice sound to complete
	TID_ANIM_UPPER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_LOWER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_BOTH,		// Waiting to finish lower and upper anim holdtimes or normal md3 animating
	TID_MOVE_NAV,		// Trying to get to a navgoal or For ET_MOVERS
	TID_ANGLE_FACE,		// Turning to an angle or facing
	TID_BSTATE,			// Waiting for a certain bState to finish
	TID_LOCATION,		// Waiting for ent to enter a specific trigger_location
//	TID_MISSIONSTATUS,	// Waiting for player to finish reading MISSION STATUS SCREEN
	TID_RESIZE,			// Waiting for clear bbox to inflate size
	TID_SHOOT,			// Waiting for fire event
	NUM_TIDS,			// for def of taskID array
} taskID_t;


typedef enum //# material_e
{
	MAT_METAL = 0,	// scorched blue-grey metal
	MAT_GLASS,		// not a real chunk type, just plays an effect with glass sprites
	MAT_ELECTRICAL,	// sparks only
	MAT_ELEC_METAL,	// sparks/electrical type metal
	MAT_DRK_STONE,	// brown
	MAT_LT_STONE,	// tan
	MAT_GLASS_METAL,// glass sprites and METAl chunk
	MAT_METAL2,		// electrical metal type
	MAT_NONE,		// no chunks
	MAT_GREY_STONE,	// grey
	MAT_METAL3,		// METAL and METAL2 chunks
	MAT_CRATE1,		// yellow multi-colored crate chunks
	MAT_GRATE1,		// grate chunks
	MAT_ROPE,		// for yavin trial...no chunks, just wispy bits
	MAT_CRATE2,		// read multi-colored crate chunks
	MAT_WHITE_METAL,// white angular chunks

	NUM_MATERIALS

} material_t;

//===From cg_local.h================================================
#define	DEFAULT_HEADMODEL	""
#define	DEFAULT_TORSOMODEL	""
#define	DEFAULT_LEGSMODEL	"mouse"

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a userinfo configstring changes

#define	MAX_CUSTOM_BASIC_SOUNDS	14
#define	MAX_CUSTOM_COMBAT_SOUNDS	17
#define	MAX_CUSTOM_EXTRA_SOUNDS	36
#define	MAX_CUSTOM_JEDI_SOUNDS	22
#define	MAX_CUSTOM_SOUNDS	(MAX_CUSTOM_JEDI_SOUNDS + MAX_CUSTOM_EXTRA_SOUNDS + MAX_CUSTOM_COMBAT_SOUNDS + MAX_CUSTOM_BASIC_SOUNDS)
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class clientInfo_t
{
public:
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				score;			// updated by score servercmds

	int				handicap;

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;
	qboolean		extensions;		// do we have extra face skins ?

	int				animFileIndex;

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];

	char			*customBasicSoundDir;
	char			*customCombatSoundDir;
	char			*customExtraSoundDir;
	char			*customJediSoundDir;


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(infoValid);
		saved_game.write<int8_t>(name);
		saved_game.write<int32_t>(team);
		saved_game.write<int32_t>(score);
		saved_game.write<int32_t>(handicap);
		saved_game.write<int32_t>(legsModel);
		saved_game.write<int32_t>(legsSkin);
		saved_game.write<int32_t>(torsoModel);
		saved_game.write<int32_t>(torsoSkin);
		saved_game.write<int32_t>(headModel);
		saved_game.write<int32_t>(headSkin);
		saved_game.write<int32_t>(extensions);
		saved_game.write<int32_t>(animFileIndex);
		saved_game.write<int32_t>(sounds);
		saved_game.write<int32_t>(customBasicSoundDir);
		saved_game.write<int32_t>(customCombatSoundDir);
		saved_game.write<int32_t>(customExtraSoundDir);
		saved_game.write<int32_t>(customJediSoundDir);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(infoValid);
		saved_game.read<int8_t>(name);
		saved_game.read<int32_t>(team);
		saved_game.read<int32_t>(score);
		saved_game.read<int32_t>(handicap);
		saved_game.read<int32_t>(legsModel);
		saved_game.read<int32_t>(legsSkin);
		saved_game.read<int32_t>(torsoModel);
		saved_game.read<int32_t>(torsoSkin);
		saved_game.read<int32_t>(headModel);
		saved_game.read<int32_t>(headSkin);
		saved_game.read<int32_t>(extensions);
		saved_game.read<int32_t>(animFileIndex);
		saved_game.read<int32_t>(sounds);
		saved_game.read<int32_t>(customBasicSoundDir);
		saved_game.read<int32_t>(customCombatSoundDir);
		saved_game.read<int32_t>(customExtraSoundDir);
		saved_game.read<int32_t>(customJediSoundDir);
	}
}; // clientInfo_t


//==================================================================
typedef enum
{
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

// Rendering information structure

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
typedef struct modelInfo_s
{
	int		modelIndex;
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(modelIndex);
		saved_game.write<float>(customRGB);
		saved_game.write<int32_t>(customAlpha);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(modelIndex);
		saved_game.read<float>(customRGB);
		saved_game.read<int32_t>(customAlpha);
	}
} modelInfo_t;

typedef enum
{
	MODEL_LEGS = 0,
	MODEL_TORSO,
	MODEL_HEAD,
	MODEL_WEAPON1,
	MODEL_WEAPON2,
	MODEL_WEAPON3,
	MODEL_EXTRA1,
	MODEL_EXTRA2,
	NUM_TARGET_MODELS
} targetModel_t;

//renderFlags
#define	RF_LOCKEDANGLE	1

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class renderInfo_t
{
public:
	// Legs model, or full model on one piece entities
	union
	{
		modelInfo_t	legsModel;
		modelInfo_t	model;
	};

	union
	{
		char	legsModelName[32];		// -slc[]
		char	modelName[32];			// -slc[]
	};
	// Models for the other pieces (not used by one piece models)
	modelInfo_t	torsoModel;
	modelInfo_t	headModel;

	char	torsoModelName[32];			// -slc[]
	char	headModelName[32];			// -slc[]

	//In whole degrees, How far to let the different model parts yaw and pitch
	int		headYawRangeLeft;
	int		headYawRangeRight;
	int		headPitchRangeUp;
	int		headPitchRangeDown;

	int		torsoYawRangeLeft;
	int		torsoYawRangeRight;
	int		torsoPitchRangeUp;
	int		torsoPitchRangeDown;

	int		legsFrame;
	int		torsoFrame;

	float	legsFpsMod;
	float	torsoFpsMod;

	//Fields to apply to entire model set, individual model's equivalents will modify this value
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?

	//RF?
	int			renderFlags;

	//
	vec3_t		muzzlePoint;
	vec3_t		muzzleDir;
	vec3_t		muzzlePointOld;
	vec3_t		muzzleDirOld;
	//vec3_t		muzzlePointNext;	// Muzzle point one server frame in the future!
	//vec3_t		muzzleDirNext;
	int			mPCalcTime;//Last time muzzle point was calced

	//
	float		lockYaw;//

	//
	vec3_t		headPoint;//Where your tag_head is
	vec3_t		headAngles;//where the tag_head in the torso is pointing
	vec3_t		handRPoint;//where your right hand is
	vec3_t		handLPoint;//where your left hand is
	vec3_t		crotchPoint;//Where your crotch is
	vec3_t		footRPoint;//where your right hand is
	vec3_t		footLPoint;//where your left hand is
	vec3_t		torsoPoint;//Where your chest is
	vec3_t		torsoAngles;//Where the chest is pointing
	vec3_t		eyePoint;//Where your eyes are
	vec3_t		eyeAngles;//Where your eyes face
	int			lookTarget;//Which ent to look at with lookAngles
	lookMode_t	lookMode;
	int			lookTargetClearTime;//Time to clear the lookTarget
	int			lastVoiceVolume;//Last frame's voice volume
	vec3_t		lastHeadAngles;//Last headAngles, NOT actual facing of head model
	vec3_t		headBobAngles;//headAngle offsets
	vec3_t		targetHeadBobAngles;//head bob angles will try to get to targetHeadBobAngles
	int			lookingDebounceTime;//When we can stop using head looking angle behavior
	float		legsYaw;//yaw angle your legs are actually rendering at


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<>(legsModel);
		saved_game.write<int8_t>(legsModelName);
		saved_game.write<>(torsoModel);
		saved_game.write<>(headModel);
		saved_game.write<int8_t>(torsoModelName);
		saved_game.write<int8_t>(headModelName);
		saved_game.write<int32_t>(headYawRangeLeft);
		saved_game.write<int32_t>(headYawRangeRight);
		saved_game.write<int32_t>(headPitchRangeUp);
		saved_game.write<int32_t>(headPitchRangeDown);
		saved_game.write<int32_t>(torsoYawRangeLeft);
		saved_game.write<int32_t>(torsoYawRangeRight);
		saved_game.write<int32_t>(torsoPitchRangeUp);
		saved_game.write<int32_t>(torsoPitchRangeDown);
		saved_game.write<int32_t>(legsFrame);
		saved_game.write<int32_t>(torsoFrame);
		saved_game.write<float>(legsFpsMod);
		saved_game.write<float>(torsoFpsMod);
		saved_game.write<float>(customRGB);
		saved_game.write<int32_t>(customAlpha);
		saved_game.write<int32_t>(renderFlags);
		saved_game.write<float>(muzzlePoint);
		saved_game.write<float>(muzzleDir);
		saved_game.write<float>(muzzlePointOld);
		saved_game.write<float>(muzzleDirOld);
		saved_game.write<int32_t>(mPCalcTime);
		saved_game.write<float>(lockYaw);
		saved_game.write<float>(headPoint);
		saved_game.write<float>(headAngles);
		saved_game.write<float>(handRPoint);
		saved_game.write<float>(handLPoint);
		saved_game.write<float>(crotchPoint);
		saved_game.write<float>(footRPoint);
		saved_game.write<float>(footLPoint);
		saved_game.write<float>(torsoPoint);
		saved_game.write<float>(torsoAngles);
		saved_game.write<float>(eyePoint);
		saved_game.write<float>(eyeAngles);
		saved_game.write<int32_t>(lookTarget);
		saved_game.write<int32_t>(lookMode);
		saved_game.write<int32_t>(lookTargetClearTime);
		saved_game.write<int32_t>(lastVoiceVolume);
		saved_game.write<float>(lastHeadAngles);
		saved_game.write<float>(headBobAngles);
		saved_game.write<float>(targetHeadBobAngles);
		saved_game.write<int32_t>(lookingDebounceTime);
		saved_game.write<float>(legsYaw);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<>(legsModel);
		saved_game.read<int8_t>(legsModelName);
		saved_game.read<>(torsoModel);
		saved_game.read<>(headModel);
		saved_game.read<int8_t>(torsoModelName);
		saved_game.read<int8_t>(headModelName);
		saved_game.read<int32_t>(headYawRangeLeft);
		saved_game.read<int32_t>(headYawRangeRight);
		saved_game.read<int32_t>(headPitchRangeUp);
		saved_game.read<int32_t>(headPitchRangeDown);
		saved_game.read<int32_t>(torsoYawRangeLeft);
		saved_game.read<int32_t>(torsoYawRangeRight);
		saved_game.read<int32_t>(torsoPitchRangeUp);
		saved_game.read<int32_t>(torsoPitchRangeDown);
		saved_game.read<int32_t>(legsFrame);
		saved_game.read<int32_t>(torsoFrame);
		saved_game.read<float>(legsFpsMod);
		saved_game.read<float>(torsoFpsMod);
		saved_game.read<float>(customRGB);
		saved_game.read<int32_t>(customAlpha);
		saved_game.read<int32_t>(renderFlags);
		saved_game.read<float>(muzzlePoint);
		saved_game.read<float>(muzzleDir);
		saved_game.read<float>(muzzlePointOld);
		saved_game.read<float>(muzzleDirOld);
		saved_game.read<int32_t>(mPCalcTime);
		saved_game.read<float>(lockYaw);
		saved_game.read<float>(headPoint);
		saved_game.read<float>(headAngles);
		saved_game.read<float>(handRPoint);
		saved_game.read<float>(handLPoint);
		saved_game.read<float>(crotchPoint);
		saved_game.read<float>(footRPoint);
		saved_game.read<float>(footLPoint);
		saved_game.read<float>(torsoPoint);
		saved_game.read<float>(torsoAngles);
		saved_game.read<float>(eyePoint);
		saved_game.read<float>(eyeAngles);
		saved_game.read<int32_t>(lookTarget);
		saved_game.read<int32_t>(lookMode);
		saved_game.read<int32_t>(lookTargetClearTime);
		saved_game.read<int32_t>(lastVoiceVolume);
		saved_game.read<float>(lastHeadAngles);
		saved_game.read<float>(headBobAngles);
		saved_game.read<float>(targetHeadBobAngles);
		saved_game.read<int32_t>(lookingDebounceTime);
		saved_game.read<float>(legsYaw);
	}
}; // renderInfo_t

// Movement information structure

/*
typedef struct moveInfo_s	// !!!!!!!!!! LOADSAVE-affecting struct !!!!!!!!
{
	vec3_t	desiredAngles;	// Desired facing angles
	float	speed;	// Speed of movement
	float	aspeed;	// Speed of angular movement
	vec3_t	moveDir;	// Direction of movement
	vec3_t	velocity;	// movement velocity
	int		flags;			// Special state flags
} moveInfo_t;
*/

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class playerTeamState_t
{
public:
	playerTeamStateState_t	state;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(state);
		saved_game.write<int32_t>(captures);
		saved_game.write<int32_t>(basedefense);
		saved_game.write<int32_t>(carrierdefense);
		saved_game.write<int32_t>(flagrecovery);
		saved_game.write<int32_t>(fragcarrier);
		saved_game.write<int32_t>(assists);
		saved_game.write<float>(lasthurtcarrier);
		saved_game.write<float>(lastreturnedflag);
		saved_game.write<float>(flagsince);
		saved_game.write<float>(lastfraggedcarrier);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(state);
		saved_game.read<int32_t>(captures);
		saved_game.read<int32_t>(basedefense);
		saved_game.read<int32_t>(carrierdefense);
		saved_game.read<int32_t>(flagrecovery);
		saved_game.read<int32_t>(fragcarrier);
		saved_game.read<int32_t>(assists);
		saved_game.read<float>(lasthurtcarrier);
		saved_game.read<float>(lastreturnedflag);
		saved_game.read<float>(flagsince);
		saved_game.read<float>(lastfraggedcarrier);
	}
}; // playerTeamState_t

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class objectives_t
{
public:
	int			display;	// A displayable objective?
	int			status;	// Succeed or fail or pending


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(display);
		saved_game.write<int32_t>(status);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(display);
		saved_game.read<int32_t>(status);
	}
}; // objectives_t
#define MAX_MISSION_OBJ 80

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class missionStats_t
{
public:
	int				secretsFound;					// # of secret areas found
	int				totalSecrets;					// # of secret areas that could have been found
	int				shotsFired;						// total number of shots fired
	int				hits;							// Shots that did damage
	int				enemiesSpawned;					// # of enemies spawned
	int				enemiesKilled;					// # of enemies killed
	int				saberThrownCnt;					// # of times saber was thrown
	int				saberBlocksCnt;					// # of times saber was used to block
	int				legAttacksCnt;					// # of times legs were hit with saber
	int				armAttacksCnt;					// # of times arm were hit with saber
	int				torsoAttacksCnt;				// # of times torso was hit with saber
	int				otherAttacksCnt;				// # of times anything else on a monster was hit with saber
	int				forceUsed[NUM_FORCE_POWERS];	// # of times each force power was used
	int				weaponUsed[WP_NUM_WEAPONS];		// # of times each weapon was used


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(secretsFound);
		saved_game.write<int32_t>(totalSecrets);
		saved_game.write<int32_t>(shotsFired);
		saved_game.write<int32_t>(hits);
		saved_game.write<int32_t>(enemiesSpawned);
		saved_game.write<int32_t>(enemiesKilled);
		saved_game.write<int32_t>(saberThrownCnt);
		saved_game.write<int32_t>(saberBlocksCnt);
		saved_game.write<int32_t>(legAttacksCnt);
		saved_game.write<int32_t>(armAttacksCnt);
		saved_game.write<int32_t>(torsoAttacksCnt);
		saved_game.write<int32_t>(otherAttacksCnt);
		saved_game.write<int32_t>(forceUsed);
		saved_game.write<int32_t>(weaponUsed);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(secretsFound);
		saved_game.read<int32_t>(totalSecrets);
		saved_game.read<int32_t>(shotsFired);
		saved_game.read<int32_t>(hits);
		saved_game.read<int32_t>(enemiesSpawned);
		saved_game.read<int32_t>(enemiesKilled);
		saved_game.read<int32_t>(saberThrownCnt);
		saved_game.read<int32_t>(saberBlocksCnt);
		saved_game.read<int32_t>(legAttacksCnt);
		saved_game.read<int32_t>(armAttacksCnt);
		saved_game.read<int32_t>(torsoAttacksCnt);
		saved_game.read<int32_t>(otherAttacksCnt);
		saved_game.read<int32_t>(forceUsed);
		saved_game.read<int32_t>(weaponUsed);
	}
}; // missionStats_t

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
//
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class clientSession_t
{
public:
	int				missionObjectivesShown;	// Number of times mission objectives have been updated
	team_t			sessionTeam;
	objectives_t	mission_objectives[MAX_MISSION_OBJ];
	missionStats_t	missionStats;			// Various totals while on a mission


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(missionObjectivesShown);
		saved_game.write<int32_t>(sessionTeam);
		saved_game.write<>(mission_objectives);
		saved_game.write<>(missionStats);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(missionObjectivesShown);
		saved_game.read<int32_t>(sessionTeam);
		saved_game.read<>(mission_objectives);
		saved_game.read<>(missionStats);
	}
}; // clientSession_t

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
class clientPersistant_t
{
public:
	clientConnected_t	connected;
	usercmd_t	lastCommand;
	qboolean	localClient;		// true if "ip" info key is "localhost"
	char		netname[34];
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	short		cmd_angles[3];		// angles sent over in the last command

	playerTeamState_t teamState;	// status in teamplay games


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int32_t>(connected);
		saved_game.write<>(lastCommand);
		saved_game.write<int32_t>(localClient);
		saved_game.write<int8_t>(netname);
		saved_game.skip(2);
		saved_game.write<int32_t>(maxHealth);
		saved_game.write<int32_t>(enterTime);
		saved_game.write<int16_t>(cmd_angles);
		saved_game.skip(2);
		saved_game.write<>(teamState);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int32_t>(connected);
		saved_game.read<>(lastCommand);
		saved_game.read<int32_t>(localClient);
		saved_game.read<int8_t>(netname);
		saved_game.skip(2);
		saved_game.read<int32_t>(maxHealth);
		saved_game.read<int32_t>(enterTime);
		saved_game.read<int16_t>(cmd_angles);
		saved_game.skip(2);
		saved_game.read<>(teamState);
	}
}; // clientPersistant_t

typedef enum {
	BLK_NO,
	BLK_TIGHT,		// Block only attacks and shots around the saber itself, a bbox of around 12x12x12
	BLK_WIDE		// Block all attacks in an area around the player in a rough arc of 180 degrees
} saberBlockType_t;

typedef enum {
	BLOCKED_NONE,
	BLOCKED_PARRY_BROKEN,
	BLOCKED_ATK_BOUNCE,
	BLOCKED_UPPER_RIGHT,
	BLOCKED_UPPER_LEFT,
	BLOCKED_LOWER_RIGHT,
	BLOCKED_LOWER_LEFT,
	BLOCKED_TOP,
	BLOCKED_UPPER_RIGHT_PROJ,
	BLOCKED_UPPER_LEFT_PROJ,
	BLOCKED_LOWER_RIGHT_PROJ,
	BLOCKED_LOWER_LEFT_PROJ,
	BLOCKED_TOP_PROJ
} saberBlockedType_t;

// !!!!!!!!!! LOADSAVE-affecting structure !!!!!!!!!!
// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
class gclient_t
{
public:
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION

	usercmd_t	usercmd;			// most recent usercmd

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			accurateCount;		// for "impressive" reward sound

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			idleTime;			// for playing idleAnims

	int			airOutTime;

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	// Facial Expression Timers

	float		facial_blink;		// time before next blink. If a minus value, we are in blink mode
	float		facial_frown;		// time before next frown. If a minus value, we are in frown mode
	float		facial_aux;			// time before next aux. If a minus value, we are in aux mode

	//Client info - updated when ClientInfoChanged is called, instead of using configstrings
	clientInfo_t	clientInfo;
	signed char		forced_forwardmove;
	signed char		forced_rightmove;
	int				fireDelay;		//msec to delay calling G_FireWeapon after EV_FIREWEAPON event is called

	//Used to be in gentity_t, now here.. mostly formation stuff
	team_t		playerTeam;
	team_t		enemyTeam;
	char		*squadname;
	gentity_t	*team_leader;
	gentity_t	*leader;
	gentity_t	*follower;
	int			numFollowers;
	gentity_t	*formationGoal;
	int			nextFormGoal;
	class_t		NPC_class;

	//FIXME: could combine these
	float		hiddenDist;//How close ents have to be to pick you up as an enemy
	vec3_t		hiddenDir;//Normalized direction in which NPCs can't see you (you are hidden)

	renderInfo_t	renderInfo;
	saberTrail_t	saberTrail;

	//dismember tracker
	qboolean	dismembered;
	char		dismemberProbLegs;	// probability of the legs being dismembered (located in NPC.cfg, 0 = never, 100 = always)
	char		dismemberProbHead;  // probability of the head being dismembered (located in NPC.cfg, 0 = never, 100 = always)
	char		dismemberProbArms;  // probability of the arms being dismembered (located in NPC.cfg, 0 = never, 100 = always)
	char		dismemberProbHands; // probability of the hands being dismembered (located in NPC.cfg, 0 = never, 100 = always)
	char		dismemberProbWaist; // probability of the waist being dismembered (located in NPC.cfg, 0 = never, 100 = always)

	int			standheight;
	int			crouchheight;
	int			poisonDamage;				// Amount of poison damage to be given
	int			poisonTime;					// When to apply poison damage
	int			slopeRecalcTime;			// debouncer for slope-foot-height-diff calcing

	vec3_t		pushVec;
	int			pushVecTime;


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<>(ps);
		saved_game.write<>(pers);
		saved_game.write<>(sess);
		saved_game.write<int32_t>(noclip);
		saved_game.write<int32_t>(lastCmdTime);
		saved_game.write<>(usercmd);
		saved_game.write<int32_t>(buttons);
		saved_game.write<int32_t>(oldbuttons);
		saved_game.write<int32_t>(latched_buttons);
		saved_game.write<int32_t>(damage_armor);
		saved_game.write<int32_t>(damage_blood);
		saved_game.write<int32_t>(damage_knockback);
		saved_game.write<float>(damage_from);
		saved_game.write<int32_t>(damage_fromWorld);
		saved_game.write<int32_t>(accurateCount);
		saved_game.write<int32_t>(respawnTime);
		saved_game.write<int32_t>(inactivityTime);
		saved_game.write<int32_t>(inactivityWarning);
		saved_game.write<int32_t>(idleTime);
		saved_game.write<int32_t>(airOutTime);
		saved_game.write<int32_t>(timeResidual);
		saved_game.write<float>(facial_blink);
		saved_game.write<float>(facial_frown);
		saved_game.write<float>(facial_aux);
		saved_game.write<>(clientInfo);
		saved_game.write<int8_t>(forced_forwardmove);
		saved_game.write<int8_t>(forced_rightmove);
		saved_game.skip(2);
		saved_game.write<int32_t>(fireDelay);
		saved_game.write<int32_t>(playerTeam);
		saved_game.write<int32_t>(enemyTeam);
		saved_game.write<int32_t>(squadname);
		saved_game.write<int32_t>(team_leader);
		saved_game.write<int32_t>(leader);
		saved_game.write<int32_t>(follower);
		saved_game.write<int32_t>(numFollowers);
		saved_game.write<int32_t>(formationGoal);
		saved_game.write<int32_t>(nextFormGoal);
		saved_game.write<int32_t>(NPC_class);
		saved_game.write<float>(hiddenDist);
		saved_game.write<float>(hiddenDir);
		saved_game.write<>(renderInfo);
		saved_game.write<>(saberTrail);
		saved_game.write<int32_t>(dismembered);
		saved_game.write<int8_t>(dismemberProbLegs);
		saved_game.write<int8_t>(dismemberProbHead);
		saved_game.write<int8_t>(dismemberProbArms);
		saved_game.write<int8_t>(dismemberProbHands);
		saved_game.write<int8_t>(dismemberProbWaist);
		saved_game.skip(3);
		saved_game.write<int32_t>(standheight);
		saved_game.write<int32_t>(crouchheight);
		saved_game.write<int32_t>(poisonDamage);
		saved_game.write<int32_t>(poisonTime);
		saved_game.write<int32_t>(slopeRecalcTime);
		saved_game.write<float>(pushVec);
		saved_game.write<int32_t>(pushVecTime);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<>(ps);
		saved_game.read<>(pers);
		saved_game.read<>(sess);
		saved_game.read<int32_t>(noclip);
		saved_game.read<int32_t>(lastCmdTime);
		saved_game.read<>(usercmd);
		saved_game.read<int32_t>(buttons);
		saved_game.read<int32_t>(oldbuttons);
		saved_game.read<int32_t>(latched_buttons);
		saved_game.read<int32_t>(damage_armor);
		saved_game.read<int32_t>(damage_blood);
		saved_game.read<int32_t>(damage_knockback);
		saved_game.read<float>(damage_from);
		saved_game.read<int32_t>(damage_fromWorld);
		saved_game.read<int32_t>(accurateCount);
		saved_game.read<int32_t>(respawnTime);
		saved_game.read<int32_t>(inactivityTime);
		saved_game.read<int32_t>(inactivityWarning);
		saved_game.read<int32_t>(idleTime);
		saved_game.read<int32_t>(airOutTime);
		saved_game.read<int32_t>(timeResidual);
		saved_game.read<float>(facial_blink);
		saved_game.read<float>(facial_frown);
		saved_game.read<float>(facial_aux);
		saved_game.read<>(clientInfo);
		saved_game.read<int8_t>(forced_forwardmove);
		saved_game.read<int8_t>(forced_rightmove);
		saved_game.skip(2);
		saved_game.read<int32_t>(fireDelay);
		saved_game.read<int32_t>(playerTeam);
		saved_game.read<int32_t>(enemyTeam);
		saved_game.read<int32_t>(squadname);
		saved_game.read<int32_t>(team_leader);
		saved_game.read<int32_t>(leader);
		saved_game.read<int32_t>(follower);
		saved_game.read<int32_t>(numFollowers);
		saved_game.read<int32_t>(formationGoal);
		saved_game.read<int32_t>(nextFormGoal);
		saved_game.read<int32_t>(NPC_class);
		saved_game.read<float>(hiddenDist);
		saved_game.read<float>(hiddenDir);
		saved_game.read<>(renderInfo);
		saved_game.read<>(saberTrail);
		saved_game.read<int32_t>(dismembered);
		saved_game.read<int8_t>(dismemberProbLegs);
		saved_game.read<int8_t>(dismemberProbHead);
		saved_game.read<int8_t>(dismemberProbArms);
		saved_game.read<int8_t>(dismemberProbHands);
		saved_game.read<int8_t>(dismemberProbWaist);
		saved_game.skip(3);
		saved_game.read<int32_t>(standheight);
		saved_game.read<int32_t>(crouchheight);
		saved_game.read<int32_t>(poisonDamage);
		saved_game.read<int32_t>(poisonTime);
		saved_game.read<int32_t>(slopeRecalcTime);
		saved_game.read<float>(pushVec);
		saved_game.read<int32_t>(pushVecTime);
	}
}; // gclient_t

#define	MAX_PARMS	16
#define	MAX_PARM_STRING_LENGTH	MAX_QPATH//was 16, had to lengthen it so they could take a valid file path

class parms_t
{
public:
	char	parm[MAX_PARMS][MAX_PARM_STRING_LENGTH];


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<int8_t>(parm);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<int8_t>(parm);
	}
}; // parms_t

#define GAME_INCLUDE
#ifdef GAME_INCLUDE
//these hold the place for the enums in functions.h so i don't have to recompile everytime it changes
#define thinkFunc_t int
#define clThinkFunc_t int
#define reachedFunc_t int
#define blockedFunc_t int
#define touchFunc_t int
#define useFunc_t int
#define painFunc_t int
#define dieFunc_t int

#define MAX_FAILED_NODES 8

typedef struct centity_s centity_t;

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	gclient_t	*client;	// NULL if not a player (unless it's NPC ( if (this->NPC != NULL)  )  <sigh>... -slc)
	qboolean	inuse;
	qboolean	linked;				// qfalse if not in any good cluster

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by gi.SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;

	gentity_t	*owner;				// objects never interact with their owners, to
									// prevent player missiles from immediately
									// colliding with their owner
/*
Ghoul2 Insert Start
*/
	// this marker thing of Jake's is used for memcpy() length calcs, so don't put any ordinary fields (like above)
	//	below this point or they won't work, and will mess up all sorts of stuff.
	//
	CGhoul2Info_v	ghoul2;
/*
Ghoul2 Insert End
*/

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

//==========================================================================================

//Essential entity fields
	// note: all the char* fields from here on should be left as ptrs, not declared, because of the way that ent-parsing
	//	works by forcing field offset ptrs as char* and using G_NewString()!! (see G_ParseField() in gmae/g_spawn.cpp -slc
	//
	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	int			flags;				// FL_* variables

	char		*model;				// Normal model, or legs model on tri-models
	char		*model2;			// Torso model

	int			freetime;			// sv.time when the object was freed

	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

//Physics and movement fields
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance
//	moveInfo_t	moveInfo;		//FIXME: use this more?
	float		speed;
	vec3_t		movedir;
	vec3_t		lastOrigin;			//Where you were last frame
	vec3_t		lastAngles;			//Where you were looking last frame
	float		mass;				//How heavy you are
	int			lastImpact;			//Last time you impacted something

//Variables reflecting environment
	int			watertype;
	int			waterlevel;

//Targeting/linking fields
	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*target2;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target3;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target4;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*targetname;
	char		*team;

	union
	{
		char	*roff;			// the roff file to use, if there is one
		char	*fxFile;		// name of the external effect file
	};

	int		roff_ctr;		// current roff frame we are playing

	int			next_roff_time;
	int			fx_time;		// timer for beam in/out effects.

//Think Functions
	int			nextthink;//Used to determine if it's time to call e_ThinkFunc again
	thinkFunc_t		e_ThinkFunc;//Called once every game frame for every ent
	clThinkFunc_t	e_clThinkFunc;//Think func for equivalent centity
	reachedFunc_t	e_ReachedFunc;// movers call this when hitting endpoint
	blockedFunc_t	e_BlockedFunc;
	touchFunc_t		e_TouchFunc;
	useFunc_t		e_UseFunc;	//Called by G_UseTargets
	painFunc_t		e_PainFunc;	//Called by G_Damage when damage is taken
	dieFunc_t		e_DieFunc;	//Called by G_Damage when health reaches <= 0

//Health and damage fields
	int			health;
	int			max_health;
	qboolean	takedamage;
	material_t	material;
	int			damage;
	int			dflags;
	//explosives, breakable brushes
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;
	//int			hitLoc;//where you were last hit
	int			locationDamage[HL_MAX];		// Damage accumulated on different body locations

//Entity pointers
	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team
	gentity_t	*lastEnemy;

//Timing variables, counters and debounce times
	float		wait;
	float		random;
	int			delay;
	qboolean	alt_fire;
	int			count;
	int			bounceCount;
	int			fly_sound_debounce_time;	// wind tunnel
	int			painDebounceTime;
	int			disconnectDebounceTime;
	int			attackDebounceTime;
	int			pushDebounceTime;
	int			aimDebounceTime;
	int			useDebounceTime;

//Unions for miscellaneous fields used under very specific circumstances
	union
	{
		qboolean	trigger_formation;
		qboolean	misc_dlight_active;
		qboolean	has_bounced;	// for thermal Det.  we force at least one bounce to happen before it can do proximity checks
	};

//Navigation
	int			waypoint;			//Set once per frame, if you've moved, and if someone asks
	int			lastWaypoint;		//To make sure you don't double-back
	int			lastValidWaypoint;	//ALWAYS valid -used for tracking someone you lost
	int			noWaypointTime;		//Debouncer - so don't keep checking every waypoint in existance every frame that you can't find one
	int			combatPoint;
	int			failedWaypoints[MAX_FAILED_NODES];
	int			failedWaypointCheckTime;

//Animation
	qboolean	loopAnim;
	int		startFrame;
	int		endFrame;

//Script/ICARUS-related fields
	CSequencer		*sequencer;
	CTaskManager	*taskManager;
	int				taskID[NUM_TIDS];
	parms_t			*parms;
	char		*behaviorSet[NUM_BSETS];
	char		*script_targetname;
	int			delayScriptTime;
	char		*fullName;

// Ambient sound info
	char			*soundSet;	//Only used for local sets
	int				setTime;

//Used by cameras to locate subjects
	char			*cameraGroup;

//For damage
	team_t		noDamageTeam;

// Ghoul2 Animation info
	int			playerModel;
	int			weaponModel;
	int			handRBolt;
	int			handLBolt;
	int			headBolt;
	int			cervicalBolt;
	int			chestBolt;
	int			gutBolt;
	int			torsoBolt;
	int			crotchBolt;
	int			motionBolt;
	int			kneeLBolt;
	int			kneeRBolt;
	int			elbowLBolt;
	int			elbowRBolt;
	int			footLBolt;
	int			footRBolt;
	int			faceBone;
	int			craniumBone;
	int			cervicalBone;
	int			thoracicBone;
	int			upperLumbarBone;
	int			lowerLumbarBone;
	int			hipsBone;
	int			motionBone;
	int			rootBone;
	int			footLBone;
	int			footRBone;

	int			genericBone1;		// For bones special to an entity
	int			genericBone2;
	int			genericBone3;

	int			genericBolt1;		// For bolts special to an entity
	int			genericBolt2;
	int			genericBolt3;
	int			genericBolt4;
	int			genericBolt5;

	qhandle_t		cinematicModel;

//==========================================================================================

//FIELDS USED EXCLUSIVELY BY SPECIFIC CLASSES OF ENTITIES
	//NPC/Player entity fields
	//FIXME: Make these client only?
	gNPC_t		*NPC;//Only allocated if the entity becomes an NPC

	//Other NPC/Player-related entity fields
	char		*ownername;//Used by squadpaths to locate owning NPC

//FIXME: Only used by NPCs, move it to gNPC_t
	int			cantHitEnemyCounter;//HACK - Makes them look for another enemy on the same team if the one they're after can't be hit

//Only used by NPC_spawners
	char		*NPC_type;
	char		*NPC_targetname;
	char		*NPC_target;

//Variables used by movers (most likely exclusively by them)
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;
	vec3_t		pos3;
	int			sounds;
	char		*closetarget;
	char		*opentarget;
	char		*paintarget;
	int			lockCount;	//for maglocks- actually get put on the trigger for the door

//Variables used only by waypoints (for the most part)
	float		radius;

	union
	{
		int		wpIndex;
		int		fxID;			// id of the external effect file
	};

	int			noise_index;

	vec4_t		startRGBA;

	union
	{
		vec4_t		finalRGBA;
		vec3_t		pos4;
	};

//FIXME: Are these being used anymore?
	gitem_t		*item;			// for bonus items -
	char		*message;		//Used by triggers to print a message when activated

	float		lightLevel;

	//FIXME: can these be removed/condensed/absorbed?
	//Rendering info
	//int			color;

	//Force effects
	int			forcePushTime;
	int			forcePuller;	//who force-pulled me (so we don't damage them if we hit them)


	void sg_export(
		ojk::SavedGameHelper& saved_game) const
	{
		saved_game.write<>(s);
		saved_game.write<int32_t>(client);
		saved_game.write<int32_t>(inuse);
		saved_game.write<int32_t>(linked);
		saved_game.write<int32_t>(svFlags);
		saved_game.write<int32_t>(bmodel);
		saved_game.write<float>(mins);
		saved_game.write<float>(maxs);
		saved_game.write<int32_t>(contents);
		saved_game.write<float>(absmin);
		saved_game.write<float>(absmax);
		saved_game.write<float>(currentOrigin);
		saved_game.write<float>(currentAngles);
		saved_game.write<int32_t>(owner);
		saved_game.write<>(ghoul2);
		saved_game.write<int32_t>(classname);
		saved_game.write<int32_t>(spawnflags);
		saved_game.write<int32_t>(flags);
		saved_game.write<int32_t>(model);
		saved_game.write<int32_t>(model2);
		saved_game.write<int32_t>(freetime);
		saved_game.write<int32_t>(eventTime);
		saved_game.write<int32_t>(freeAfterEvent);
		saved_game.write<int32_t>(unlinkAfterEvent);
		saved_game.write<float>(physicsBounce);
		saved_game.write<int32_t>(clipmask);
		saved_game.write<float>(speed);
		saved_game.write<float>(movedir);
		saved_game.write<float>(lastOrigin);
		saved_game.write<float>(lastAngles);
		saved_game.write<float>(mass);
		saved_game.write<int32_t>(lastImpact);
		saved_game.write<int32_t>(watertype);
		saved_game.write<int32_t>(waterlevel);
		saved_game.write<float>(angle);
		saved_game.write<int32_t>(target);
		saved_game.write<int32_t>(target2);
		saved_game.write<int32_t>(target3);
		saved_game.write<int32_t>(target4);
		saved_game.write<int32_t>(targetname);
		saved_game.write<int32_t>(team);
		saved_game.write<int32_t>(roff);
		saved_game.write<int32_t>(roff_ctr);
		saved_game.write<int32_t>(next_roff_time);
		saved_game.write<int32_t>(fx_time);
		saved_game.write<int32_t>(nextthink);
		saved_game.write<int32_t>(e_ThinkFunc);
		saved_game.write<int32_t>(e_clThinkFunc);
		saved_game.write<int32_t>(e_ReachedFunc);
		saved_game.write<int32_t>(e_BlockedFunc);
		saved_game.write<int32_t>(e_TouchFunc);
		saved_game.write<int32_t>(e_UseFunc);
		saved_game.write<int32_t>(e_PainFunc);
		saved_game.write<int32_t>(e_DieFunc);
		saved_game.write<int32_t>(health);
		saved_game.write<int32_t>(max_health);
		saved_game.write<int32_t>(takedamage);
		saved_game.write<int32_t>(material);
		saved_game.write<int32_t>(damage);
		saved_game.write<int32_t>(dflags);
		saved_game.write<int32_t>(splashDamage);
		saved_game.write<int32_t>(splashRadius);
		saved_game.write<int32_t>(methodOfDeath);
		saved_game.write<int32_t>(splashMethodOfDeath);
		saved_game.write<int32_t>(locationDamage);
		saved_game.write<int32_t>(chain);
		saved_game.write<int32_t>(enemy);
		saved_game.write<int32_t>(activator);
		saved_game.write<int32_t>(teamchain);
		saved_game.write<int32_t>(teammaster);
		saved_game.write<int32_t>(lastEnemy);
		saved_game.write<float>(wait);
		saved_game.write<float>(random);
		saved_game.write<int32_t>(delay);
		saved_game.write<int32_t>(alt_fire);
		saved_game.write<int32_t>(count);
		saved_game.write<int32_t>(bounceCount);
		saved_game.write<int32_t>(fly_sound_debounce_time);
		saved_game.write<int32_t>(painDebounceTime);
		saved_game.write<int32_t>(disconnectDebounceTime);
		saved_game.write<int32_t>(attackDebounceTime);
		saved_game.write<int32_t>(pushDebounceTime);
		saved_game.write<int32_t>(aimDebounceTime);
		saved_game.write<int32_t>(useDebounceTime);
		saved_game.write<int32_t>(trigger_formation);
		saved_game.write<int32_t>(waypoint);
		saved_game.write<int32_t>(lastWaypoint);
		saved_game.write<int32_t>(lastValidWaypoint);
		saved_game.write<int32_t>(noWaypointTime);
		saved_game.write<int32_t>(combatPoint);
		saved_game.write<int32_t>(failedWaypoints);
		saved_game.write<int32_t>(failedWaypointCheckTime);
		saved_game.write<int32_t>(loopAnim);
		saved_game.write<int32_t>(startFrame);
		saved_game.write<int32_t>(endFrame);
		saved_game.write<int32_t>(sequencer);
		saved_game.write<int32_t>(taskManager);
		saved_game.write<int32_t>(taskID);
		saved_game.write<int32_t>(parms);
		saved_game.write<int32_t>(behaviorSet);
		saved_game.write<int32_t>(script_targetname);
		saved_game.write<int32_t>(delayScriptTime);
		saved_game.write<int32_t>(fullName);
		saved_game.write<int32_t>(soundSet);
		saved_game.write<int32_t>(setTime);
		saved_game.write<int32_t>(cameraGroup);
		saved_game.write<int32_t>(noDamageTeam);
		saved_game.write<int32_t>(playerModel);
		saved_game.write<int32_t>(weaponModel);
		saved_game.write<int32_t>(handRBolt);
		saved_game.write<int32_t>(handLBolt);
		saved_game.write<int32_t>(headBolt);
		saved_game.write<int32_t>(cervicalBolt);
		saved_game.write<int32_t>(chestBolt);
		saved_game.write<int32_t>(gutBolt);
		saved_game.write<int32_t>(torsoBolt);
		saved_game.write<int32_t>(crotchBolt);
		saved_game.write<int32_t>(motionBolt);
		saved_game.write<int32_t>(kneeLBolt);
		saved_game.write<int32_t>(kneeRBolt);
		saved_game.write<int32_t>(elbowLBolt);
		saved_game.write<int32_t>(elbowRBolt);
		saved_game.write<int32_t>(footLBolt);
		saved_game.write<int32_t>(footRBolt);
		saved_game.write<int32_t>(faceBone);
		saved_game.write<int32_t>(craniumBone);
		saved_game.write<int32_t>(cervicalBone);
		saved_game.write<int32_t>(thoracicBone);
		saved_game.write<int32_t>(upperLumbarBone);
		saved_game.write<int32_t>(lowerLumbarBone);
		saved_game.write<int32_t>(hipsBone);
		saved_game.write<int32_t>(motionBone);
		saved_game.write<int32_t>(rootBone);
		saved_game.write<int32_t>(footLBone);
		saved_game.write<int32_t>(footRBone);
		saved_game.write<int32_t>(genericBone1);
		saved_game.write<int32_t>(genericBone2);
		saved_game.write<int32_t>(genericBone3);
		saved_game.write<int32_t>(genericBolt1);
		saved_game.write<int32_t>(genericBolt2);
		saved_game.write<int32_t>(genericBolt3);
		saved_game.write<int32_t>(genericBolt4);
		saved_game.write<int32_t>(genericBolt5);
		saved_game.write<int32_t>(cinematicModel);
		saved_game.write<int32_t>(NPC);
		saved_game.write<int32_t>(ownername);
		saved_game.write<int32_t>(cantHitEnemyCounter);
		saved_game.write<int32_t>(NPC_type);
		saved_game.write<int32_t>(NPC_targetname);
		saved_game.write<int32_t>(NPC_target);
		saved_game.write<int32_t>(moverState);
		saved_game.write<int32_t>(soundPos1);
		saved_game.write<int32_t>(sound1to2);
		saved_game.write<int32_t>(sound2to1);
		saved_game.write<int32_t>(soundPos2);
		saved_game.write<int32_t>(soundLoop);
		saved_game.write<int32_t>(nextTrain);
		saved_game.write<int32_t>(prevTrain);
		saved_game.write<float>(pos1);
		saved_game.write<float>(pos2);
		saved_game.write<float>(pos3);
		saved_game.write<int32_t>(sounds);
		saved_game.write<int32_t>(closetarget);
		saved_game.write<int32_t>(opentarget);
		saved_game.write<int32_t>(paintarget);
		saved_game.write<int32_t>(lockCount);
		saved_game.write<float>(radius);
		saved_game.write<int32_t>(wpIndex);
		saved_game.write<int32_t>(noise_index);
		saved_game.write<float>(startRGBA);
		saved_game.write<float>(finalRGBA);
		saved_game.write<int32_t>(item);
		saved_game.write<int32_t>(message);
		saved_game.write<float>(lightLevel);
		saved_game.write<int32_t>(forcePushTime);
		saved_game.write<int32_t>(forcePuller);
	}

	void sg_import(
		ojk::SavedGameHelper& saved_game)
	{
		saved_game.read<>(s);
		saved_game.read<int32_t>(client);
		saved_game.read<int32_t>(inuse);
		saved_game.read<int32_t>(linked);
		saved_game.read<int32_t>(svFlags);
		saved_game.read<int32_t>(bmodel);
		saved_game.read<float>(mins);
		saved_game.read<float>(maxs);
		saved_game.read<int32_t>(contents);
		saved_game.read<float>(absmin);
		saved_game.read<float>(absmax);
		saved_game.read<float>(currentOrigin);
		saved_game.read<float>(currentAngles);
		saved_game.read<int32_t>(owner);
		saved_game.read<>(ghoul2);
		saved_game.read<int32_t>(classname);
		saved_game.read<int32_t>(spawnflags);
		saved_game.read<int32_t>(flags);
		saved_game.read<int32_t>(model);
		saved_game.read<int32_t>(model2);
		saved_game.read<int32_t>(freetime);
		saved_game.read<int32_t>(eventTime);
		saved_game.read<int32_t>(freeAfterEvent);
		saved_game.read<int32_t>(unlinkAfterEvent);
		saved_game.read<float>(physicsBounce);
		saved_game.read<int32_t>(clipmask);
		saved_game.read<float>(speed);
		saved_game.read<float>(movedir);
		saved_game.read<float>(lastOrigin);
		saved_game.read<float>(lastAngles);
		saved_game.read<float>(mass);
		saved_game.read<int32_t>(lastImpact);
		saved_game.read<int32_t>(watertype);
		saved_game.read<int32_t>(waterlevel);
		saved_game.read<float>(angle);
		saved_game.read<int32_t>(target);
		saved_game.read<int32_t>(target2);
		saved_game.read<int32_t>(target3);
		saved_game.read<int32_t>(target4);
		saved_game.read<int32_t>(targetname);
		saved_game.read<int32_t>(team);
		saved_game.read<int32_t>(roff);
		saved_game.read<int32_t>(roff_ctr);
		saved_game.read<int32_t>(next_roff_time);
		saved_game.read<int32_t>(fx_time);
		saved_game.read<int32_t>(nextthink);
		saved_game.read<int32_t>(e_ThinkFunc);
		saved_game.read<int32_t>(e_clThinkFunc);
		saved_game.read<int32_t>(e_ReachedFunc);
		saved_game.read<int32_t>(e_BlockedFunc);
		saved_game.read<int32_t>(e_TouchFunc);
		saved_game.read<int32_t>(e_UseFunc);
		saved_game.read<int32_t>(e_PainFunc);
		saved_game.read<int32_t>(e_DieFunc);
		saved_game.read<int32_t>(health);
		saved_game.read<int32_t>(max_health);
		saved_game.read<int32_t>(takedamage);
		saved_game.read<int32_t>(material);
		saved_game.read<int32_t>(damage);
		saved_game.read<int32_t>(dflags);
		saved_game.read<int32_t>(splashDamage);
		saved_game.read<int32_t>(splashRadius);
		saved_game.read<int32_t>(methodOfDeath);
		saved_game.read<int32_t>(splashMethodOfDeath);
		saved_game.read<int32_t>(locationDamage);
		saved_game.read<int32_t>(chain);
		saved_game.read<int32_t>(enemy);
		saved_game.read<int32_t>(activator);
		saved_game.read<int32_t>(teamchain);
		saved_game.read<int32_t>(teammaster);
		saved_game.read<int32_t>(lastEnemy);
		saved_game.read<float>(wait);
		saved_game.read<float>(random);
		saved_game.read<int32_t>(delay);
		saved_game.read<int32_t>(alt_fire);
		saved_game.read<int32_t>(count);
		saved_game.read<int32_t>(bounceCount);
		saved_game.read<int32_t>(fly_sound_debounce_time);
		saved_game.read<int32_t>(painDebounceTime);
		saved_game.read<int32_t>(disconnectDebounceTime);
		saved_game.read<int32_t>(attackDebounceTime);
		saved_game.read<int32_t>(pushDebounceTime);
		saved_game.read<int32_t>(aimDebounceTime);
		saved_game.read<int32_t>(useDebounceTime);
		saved_game.read<int32_t>(trigger_formation);
		saved_game.read<int32_t>(waypoint);
		saved_game.read<int32_t>(lastWaypoint);
		saved_game.read<int32_t>(lastValidWaypoint);
		saved_game.read<int32_t>(noWaypointTime);
		saved_game.read<int32_t>(combatPoint);
		saved_game.read<int32_t>(failedWaypoints);
		saved_game.read<int32_t>(failedWaypointCheckTime);
		saved_game.read<int32_t>(loopAnim);
		saved_game.read<int32_t>(startFrame);
		saved_game.read<int32_t>(endFrame);
		saved_game.read<int32_t>(sequencer);
		saved_game.read<int32_t>(taskManager);
		saved_game.read<int32_t>(taskID);
		saved_game.read<int32_t>(parms);
		saved_game.read<int32_t>(behaviorSet);
		saved_game.read<int32_t>(script_targetname);
		saved_game.read<int32_t>(delayScriptTime);
		saved_game.read<int32_t>(fullName);
		saved_game.read<int32_t>(soundSet);
		saved_game.read<int32_t>(setTime);
		saved_game.read<int32_t>(cameraGroup);
		saved_game.read<int32_t>(noDamageTeam);
		saved_game.read<int32_t>(playerModel);
		saved_game.read<int32_t>(weaponModel);
		saved_game.read<int32_t>(handRBolt);
		saved_game.read<int32_t>(handLBolt);
		saved_game.read<int32_t>(headBolt);
		saved_game.read<int32_t>(cervicalBolt);
		saved_game.read<int32_t>(chestBolt);
		saved_game.read<int32_t>(gutBolt);
		saved_game.read<int32_t>(torsoBolt);
		saved_game.read<int32_t>(crotchBolt);
		saved_game.read<int32_t>(motionBolt);
		saved_game.read<int32_t>(kneeLBolt);
		saved_game.read<int32_t>(kneeRBolt);
		saved_game.read<int32_t>(elbowLBolt);
		saved_game.read<int32_t>(elbowRBolt);
		saved_game.read<int32_t>(footLBolt);
		saved_game.read<int32_t>(footRBolt);
		saved_game.read<int32_t>(faceBone);
		saved_game.read<int32_t>(craniumBone);
		saved_game.read<int32_t>(cervicalBone);
		saved_game.read<int32_t>(thoracicBone);
		saved_game.read<int32_t>(upperLumbarBone);
		saved_game.read<int32_t>(lowerLumbarBone);
		saved_game.read<int32_t>(hipsBone);
		saved_game.read<int32_t>(motionBone);
		saved_game.read<int32_t>(rootBone);
		saved_game.read<int32_t>(footLBone);
		saved_game.read<int32_t>(footRBone);
		saved_game.read<int32_t>(genericBone1);
		saved_game.read<int32_t>(genericBone2);
		saved_game.read<int32_t>(genericBone3);
		saved_game.read<int32_t>(genericBolt1);
		saved_game.read<int32_t>(genericBolt2);
		saved_game.read<int32_t>(genericBolt3);
		saved_game.read<int32_t>(genericBolt4);
		saved_game.read<int32_t>(genericBolt5);
		saved_game.read<int32_t>(cinematicModel);
		saved_game.read<int32_t>(NPC);
		saved_game.read<int32_t>(ownername);
		saved_game.read<int32_t>(cantHitEnemyCounter);
		saved_game.read<int32_t>(NPC_type);
		saved_game.read<int32_t>(NPC_targetname);
		saved_game.read<int32_t>(NPC_target);
		saved_game.read<int32_t>(moverState);
		saved_game.read<int32_t>(soundPos1);
		saved_game.read<int32_t>(sound1to2);
		saved_game.read<int32_t>(sound2to1);
		saved_game.read<int32_t>(soundPos2);
		saved_game.read<int32_t>(soundLoop);
		saved_game.read<int32_t>(nextTrain);
		saved_game.read<int32_t>(prevTrain);
		saved_game.read<float>(pos1);
		saved_game.read<float>(pos2);
		saved_game.read<float>(pos3);
		saved_game.read<int32_t>(sounds);
		saved_game.read<int32_t>(closetarget);
		saved_game.read<int32_t>(opentarget);
		saved_game.read<int32_t>(paintarget);
		saved_game.read<int32_t>(lockCount);
		saved_game.read<float>(radius);
		saved_game.read<int32_t>(wpIndex);
		saved_game.read<int32_t>(noise_index);
		saved_game.read<float>(startRGBA);
		saved_game.read<float>(finalRGBA);
		saved_game.read<int32_t>(item);
		saved_game.read<int32_t>(message);
		saved_game.read<float>(lightLevel);
		saved_game.read<int32_t>(forcePushTime);
		saved_game.read<int32_t>(forcePuller);
	}
};
#endif //#ifdef GAME_INCLUDE

extern	gentity_t		g_entities[MAX_GENTITIES];
extern	game_import_t	gi;

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;		//for in view
	qhandle_t		weaponWorldModel;	//for in their hands
	qhandle_t		barrelModel[4];

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	qhandle_t		weaponIcon;			// The version of the icon with a glowy background
	qhandle_t		weaponIconNoAmmo;	// The version of the icon with no ammo warning
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );

	qhandle_t		alt_missileModel;
	sfxHandle_t		alt_missileSound;
	void			(*alt_missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );

//	sfxHandle_t		flashSound;
//	sfxHandle_t		altFlashSound;

	sfxHandle_t		firingSound;
	sfxHandle_t		altFiringSound;

	sfxHandle_t		stopSound;

	sfxHandle_t		missileHitSound;
	sfxHandle_t		altmissileHitSound;

	sfxHandle_t		chargeSound;
	sfxHandle_t		altChargeSound;

	sfxHandle_t		selectSound;	// sound played when weapon is selected
} weaponInfo_t;

extern sfxHandle_t CAS_GetBModelSound( const char *name, int stage );

enum
{
	EDGE_NORMAL,
	EDGE_PATH,
	EDGE_BLOCKED,
	EDGE_FAILED,
	EDGE_MOVEDIR
};

enum
{
	NODE_NORMAL,
	NODE_START,
	NODE_GOAL,
	NODE_NAVGOAL,
};

#endif // #ifndef __G_SHARED_H__

/*
structures heirarchy

centity_t	cg_entities[MAX_GENTITIES]
{
	entityState_t	currentState, nextState
	{
		trajectory_t	pos, apos
	}
	playerEntity_t	pe
	{
		lerpFrame_t	legs, torso
		{
			animation_t	animation
		}
	}
	gentity_t		gent, g_entities[MAX_GENTITIES]
	{
		gentity_t	owner, nextTrain, prevTrain, chain, enemy, activator, teamchain, teammaster, team_leader, leader, follower,	formationGoal, lastEnemy
		entityState_t	s
		{
			trajectory_t	pos, apos
		}
		gclient_t		client
		{
			playerState_t	ps
			clientPersistant_t	pers;
			{
				playerTeamState_t teamState;	// status in teamplay games
			}
			clientSession_t		sess
			usercmd_t	usercmd
			clientInfo_t	clientInfo
			{
				animation_t		animation[MAX_ANIMATIONS]
				animsounds_t	torsoAnimSnds[MAX_ANIM_SOUNDS], legsAnimSnds[MAX_ANIM_SOUNDS]
			}
		}
		gitem_t			item
		gNPC_t			NPC
		{
			gNPCstats_t	stats;
			usercmd_t	last_ucmd;
		}
		moveInfo_t		moveInfo
		renderInfo_t	renderInfo
		{
			modelInfo_t	torsoModel, headModel
			boltOn_t	boltOns[MAX_BOLT_ONS]
		}
	}
}

level_locals_t
{
	gclient_t		clients
	gentity_t		locationHead
	interestPoint_t	interestPoints[MAX_INTEREST_POINTS]
	combatPoint_t	combatPoints[MAX_COMBAT_POINTS]
}
*/

