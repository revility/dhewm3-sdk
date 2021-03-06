/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "renderer/RenderWorld.h"

#include "gamesys/SysCvar.h"
#include "gamesys/SaveGame.h"
#include "GameBase.h"
#include "Player.h"

#include "PlayerView.h"

static int MakePowerOfTwo( int num ) { //un noted code change from original sdk
	int		pot;
	for (pot = 1 ; pot < num ; pot<<=1) {
	}
	return pot;
}

const int IMPULSE_DELAY = 150;

const char *blurxMaterial[5] = { "textures/fsfx/blurx64", "textures/fsfx/blurx128", "textures/fsfx/blurx256", "textures/fsfx/blurx512", "textures/fsfx/blurx1024" }; //un noted code change from original sdk
const char *bluryMaterial[5] = { "textures/fsfx/blury32", "textures/fsfx/blury64", "textures/fsfx/blury128", "textures/fsfx/blury256", "textures/fsfx/blury512" };
	
/*
==============
idPlayerView::idPlayerView
==============
*/
idPlayerView::idPlayerView() {
	memset( screenBlobs, 0, sizeof( screenBlobs ) );
	memset( &view, 0, sizeof( view ) );
	player = NULL;
	dvMaterial = declManager->FindMaterial( "_scratch" );
	tunnelMaterial = declManager->FindMaterial( "textures/decals/tunnel" );
	armorMaterial = declManager->FindMaterial( "armorViewEffect" );
	berserkMaterial = declManager->FindMaterial( "textures/decals/berserk" );
	irGogglesMaterial = declManager->FindMaterial( "textures/decals/irblend" );
	bloodSprayMaterial = declManager->FindMaterial( "textures/decals/bloodspray" );
	bfgMaterial = declManager->FindMaterial( "textures/decals/bfgvision" );
	lagoMaterial = declManager->FindMaterial( LAGO_MATERIAL, false );
	bfgVision = false;
#ifdef _DENTONMOD
	//initWeights = false;
	shiftSensitivityDelay = 0;
	screenHeight = screenWidth = 0;
#endif
	dvFinishTime = 0;
	kickFinishTime = 0;
	kickAngles.Zero();
	lastDamageTime = 0.0f;
	fadeTime = 0;
	fadeRate = 0.0;
	fadeFromColor.Zero();
	fadeToColor.Zero();
	fadeColor.Zero();
	shakeAng.Zero();

	ClearEffects();
}

/*
==============
idPlayerView::Save
==============
*/
void idPlayerView::Save( idSaveGame *savefile ) const {
	int i;
	const screenBlob_t *blob;

	blob = &screenBlobs[ 0 ];
	for( i = 0; i < MAX_SCREEN_BLOBS; i++, blob++ ) {
		savefile->WriteMaterial( blob->material );
		savefile->WriteFloat( blob->x );
		savefile->WriteFloat( blob->y );
		savefile->WriteFloat( blob->w );
		savefile->WriteFloat( blob->h );
		savefile->WriteFloat( blob->s1 );
		savefile->WriteFloat( blob->t1 );
		savefile->WriteFloat( blob->s2 );
		savefile->WriteFloat( blob->t2 );
		savefile->WriteInt( blob->finishTime );
		savefile->WriteInt( blob->startFadeTime );
		savefile->WriteFloat( blob->driftAmount );
	}

	savefile->WriteInt( dvFinishTime );
	savefile->WriteMaterial( dvMaterial );
	savefile->WriteInt( kickFinishTime );
	savefile->WriteAngles( kickAngles );
	savefile->WriteBool( bfgVision );

	savefile->WriteMaterial( tunnelMaterial );
	savefile->WriteMaterial( armorMaterial );
	savefile->WriteMaterial( berserkMaterial );
	savefile->WriteMaterial( irGogglesMaterial );
	savefile->WriteMaterial( bloodSprayMaterial );
	savefile->WriteMaterial( bfgMaterial );
	savefile->WriteFloat( lastDamageTime );

	savefile->WriteVec4( fadeColor );
	savefile->WriteVec4( fadeToColor );
	savefile->WriteVec4( fadeFromColor );
	savefile->WriteFloat( fadeRate );
	savefile->WriteInt( fadeTime );

	savefile->WriteAngles( shakeAng );

	savefile->WriteObject( player );
	savefile->WriteRenderView( view );
}

/*
==============
idPlayerView::Restore
==============
*/
void idPlayerView::Restore( idRestoreGame *savefile ) {
	int i;
	screenBlob_t *blob;

	blob = &screenBlobs[ 0 ];
	for( i = 0; i < MAX_SCREEN_BLOBS; i++, blob++ ) {
		savefile->ReadMaterial( blob->material );
		savefile->ReadFloat( blob->x );
		savefile->ReadFloat( blob->y );
		savefile->ReadFloat( blob->w );
		savefile->ReadFloat( blob->h );
		savefile->ReadFloat( blob->s1 );
		savefile->ReadFloat( blob->t1 );
		savefile->ReadFloat( blob->s2 );
		savefile->ReadFloat( blob->t2 );
		savefile->ReadInt( blob->finishTime );
		savefile->ReadInt( blob->startFadeTime );
		savefile->ReadFloat( blob->driftAmount );
	}

	savefile->ReadInt( dvFinishTime );
	savefile->ReadMaterial( dvMaterial );
	savefile->ReadInt( kickFinishTime );
	savefile->ReadAngles( kickAngles );
	savefile->ReadBool( bfgVision );

	savefile->ReadMaterial( tunnelMaterial );
	savefile->ReadMaterial( armorMaterial );
	savefile->ReadMaterial( berserkMaterial );
	savefile->ReadMaterial( irGogglesMaterial );
	savefile->ReadMaterial( bloodSprayMaterial );
	savefile->ReadMaterial( bfgMaterial );
	savefile->ReadFloat( lastDamageTime );

	savefile->ReadVec4( fadeColor );
	savefile->ReadVec4( fadeToColor );
	savefile->ReadVec4( fadeFromColor );
	savefile->ReadFloat( fadeRate );
	savefile->ReadInt( fadeTime );

	savefile->ReadAngles( shakeAng );

	savefile->ReadObject( reinterpret_cast<idClass *&>( player ) );
	savefile->ReadRenderView( view );
}

/*
==============
idPlayerView::SetPlayerEntity
==============
*/
void idPlayerView::SetPlayerEntity( idPlayer *playerEnt ) {
	player = playerEnt;
}

/*
==============
idPlayerView::ClearEffects
==============
*/
void idPlayerView::ClearEffects() {
	lastDamageTime = MS2SEC( gameLocal.time - 99999 );

	dvFinishTime = ( gameLocal.time - 99999 );
	kickFinishTime = ( gameLocal.time - 99999 );

	for ( int i = 0 ; i < MAX_SCREEN_BLOBS ; i++ ) {
		screenBlobs[i].finishTime = gameLocal.time;
	}

	fadeTime = 0;
	bfgVision = false;
}

/*
==============
idPlayerView::GetScreenBlob
==============
*/
screenBlob_t *idPlayerView::GetScreenBlob() {
	screenBlob_t	*oldest = &screenBlobs[0];

	for ( int i = 1 ; i < MAX_SCREEN_BLOBS ; i++ ) {
		if ( screenBlobs[i].finishTime < oldest->finishTime ) {
			oldest = &screenBlobs[i];
		}
	}
	return oldest;
}

/*
==============
idPlayerView::DamageImpulse

LocalKickDir is the direction of force in the player's coordinate system,
which will determine the head kick direction
==============
*/
void idPlayerView::DamageImpulse( idVec3 localKickDir, const idDict *damageDef ) {
	//
	// double vision effect
	//
	if ( lastDamageTime > 0.0f && SEC2MS( lastDamageTime ) + IMPULSE_DELAY > gameLocal.time ) {
		// keep shotgun from obliterating the view
		return;
	}

	float	dvTime = damageDef->GetFloat( "dv_time" );
	if ( dvTime ) {
		if ( dvFinishTime < gameLocal.time ) {
			dvFinishTime = gameLocal.time;
		}
		dvFinishTime += g_dvTime.GetFloat() * dvTime;
		// don't let it add up too much in god mode
		if ( dvFinishTime > gameLocal.time + 5000 ) {
			dvFinishTime = gameLocal.time + 5000;
		}
	}

	//
	// head angle kick
	//
	float	kickTime = damageDef->GetFloat( "kick_time" );
	if ( kickTime ) {
		kickFinishTime = gameLocal.time + g_kickTime.GetFloat() * kickTime;

		// forward / back kick will pitch view
		kickAngles[0] = localKickDir[0];

		// side kick will yaw view
		kickAngles[1] = localKickDir[1]*0.5f;

		// up / down kick will pitch view
		kickAngles[0] += localKickDir[2];

		// roll will come from  side
		kickAngles[2] = localKickDir[1];

		float kickAmplitude = damageDef->GetFloat( "kick_amplitude" );
		if ( kickAmplitude ) {
			kickAngles *= kickAmplitude;
		}
	}

	//
	// screen blob
	//
	float	blobTime = damageDef->GetFloat( "blob_time" );
	if ( blobTime ) {
		screenBlob_t	*blob = GetScreenBlob();
		blob->startFadeTime = gameLocal.time;
		blob->finishTime = gameLocal.time + blobTime * g_blobTime.GetFloat();

		const char *materialName = damageDef->GetString( "mtr_blob" );
		blob->material = declManager->FindMaterial( materialName );
		blob->x = damageDef->GetFloat( "blob_x" );
		blob->x += ( gameLocal.random.RandomInt()&63 ) - 32;
		blob->y = damageDef->GetFloat( "blob_y" );
		blob->y += ( gameLocal.random.RandomInt()&63 ) - 32;

		float scale = ( 256 + ( ( gameLocal.random.RandomInt()&63 ) - 32 ) ) / 256.0f;
		blob->w = damageDef->GetFloat( "blob_width" ) * g_blobSize.GetFloat() * scale;
		blob->h = damageDef->GetFloat( "blob_height" ) * g_blobSize.GetFloat() * scale;
		blob->s1 = 0;
		blob->t1 = 0;
		blob->s2 = 1;
		blob->t2 = 1;
	}

	//
	// save lastDamageTime for tunnel vision accentuation
	//
	lastDamageTime = MS2SEC( gameLocal.time );

}

/*
==================
idPlayerView::AddBloodSpray

If we need a more generic way to add blobs then we can do that
but having it localized here lets the material be pre-looked up etc.
==================
*/
void idPlayerView::AddBloodSpray( float duration ) {
	/* //un noted code change from original sdk
	if ( duration <= 0 || bloodSprayMaterial == NULL || g_skipViewEffects.GetBool() ) { //un noted code change from original sdk
	return; //un noted code change from original sdk
	}
	// visit this for chainsaw
	screenBlob_t *blob = GetScreenBlob();
	blob->startFadeTime = gameLocal.time;
	blob->finishTime = gameLocal.time + ( duration * 1000 );
	blob->material = bloodSprayMaterial;
	blob->x = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->y = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->driftAmount = 0.5f + gameLocal.random.CRandomFloat() * 0.5;
	float scale = ( 256 + ( ( gameLocal.random.RandomInt()&63 ) - 32 ) ) / 256.0f;
	blob->w = 600 * g_blobSize.GetFloat() * scale;
	blob->h = 480 * g_blobSize.GetFloat() * scale;
	float s1 = 0.0f;
	float t1 = 0.0f;
	float s2 = 1.0f;
	float t2 = 1.0f;
	if ( blob->driftAmount < 0.6 ) {
	s1 = 1.0f; //un noted code change from original sdk
	s2 = 0.0f;
	} else if ( blob->driftAmount < 0.75 ) { //un noted code change from original sdk
	t1 = 1.0f;
	t2 = 0.0f;
	} else if ( blob->driftAmount < 0.85 ) { //un noted code change from original sdk
	s1 = 1.0f;
	s2 = 0.0f;
	t1 = 1.0f;
	t2 = 0.0f;
	}
	blob->s1 = s1;
	blob->t1 = t1;
	blob->s2 = s2;
	blob->t2 = t2;
	*/ //un noted code change from original sdk
}

/*
==================
idPlayerView::WeaponFireFeedback

Called when a weapon fires, generates head twitches, etc
==================
*/
void idPlayerView::WeaponFireFeedback( const idDict *weaponDef ) {
	int		recoilTime;

	recoilTime = weaponDef->GetInt( "recoilTime" );
	// don't shorten a damage kick in progress
	if ( recoilTime && kickFinishTime < gameLocal.time ) {
		idAngles angles;
		weaponDef->GetAngles( "recoilAngles", "5 0 0", angles );
		kickAngles = angles;
		int	finish = gameLocal.time + g_kickTime.GetFloat() * recoilTime;
		kickFinishTime = finish;
	}

}

/*
===================
idPlayerView::CalculateShake
===================
*/
void idPlayerView::CalculateShake() {
	idVec3	origin, matrix;

	float shakeVolume = gameSoundWorld->CurrentShakeAmplitudeForPosition( gameLocal.time, player->firstPersonViewOrigin );
	//
	// shakeVolume should somehow be molded into an angle here
	// it should be thought of as being in the range 0.0 -> 1.0, although
	// since CurrentShakeAmplitudeForPosition() returns all the shake sounds
	// the player can hear, it can go over 1.0 too.
	//
	shakeAng[0] = gameLocal.random.CRandomFloat() * shakeVolume;
	shakeAng[1] = gameLocal.random.CRandomFloat() * shakeVolume;
	shakeAng[2] = gameLocal.random.CRandomFloat() * shakeVolume;
}

/*
===================
idPlayerView::ShakeAxis
===================
*/
idMat3 idPlayerView::ShakeAxis() const {
	return shakeAng.ToMat3();
}

/*
===================
idPlayerView::AngleOffset

kickVector, a world space direction that the attack should //un noted code change from original sdk
===================
*/
idAngles idPlayerView::AngleOffset() const {
	idAngles	ang;

	ang.Zero();

	if ( gameLocal.time < kickFinishTime ) {
		float offset = kickFinishTime - gameLocal.time;

		ang = kickAngles * offset * offset * g_kickAmplitude.GetFloat();

		for ( int i = 0 ; i < 3 ; i++ ) {
			if ( ang[i] > 70.0f ) {
				ang[i] = 70.0f;
			} else if ( ang[i] < -70.0f ) {
				ang[i] = -70.0f;
			}
		}
	}
	return ang;
}

/*
==================
idPlayerView::SingleView
==================
*/
void idPlayerView::SingleView( idUserInterface *hud, const renderView_t *view ) {

	// normal rendering
	if ( !view ) {
		return;
	}

	// place the sound origin for the player
	gameSoundWorld->PlaceListener( view->vieworg, view->viewaxis, player->entityNumber + 1, gameLocal.time, hud ? hud->State().GetString( "location" ) : "Undefined" );

	// if the objective system is up, don't do normal drawing
	if ( player->objectiveSystemOpen ) {
		player->objectiveSystem->Redraw( gameLocal.time );
		return;
	}

	// hack the shake in at the very last moment, so it can't cause any consistency problems
	renderView_t	hackedView = *view;
	hackedView.viewaxis = hackedView.viewaxis * ShakeAxis();

#ifdef _PORTALSKY //un noted code change from original sdk
	if ( gameLocal.portalSkyEnt.GetEntity() && gameLocal.IsPortalSkyAcive() && g_enablePortalSky.GetBool() ) {

		renderView_t	portalView = hackedView;
		portalView.vieworg = gameLocal.portalSkyEnt.GetEntity()->GetPhysics()->GetOrigin();

		// setup global fixup projection vars
		if ( 1 ) {
			int vidWidth, vidHeight;
			idVec2 shiftScale;

			renderSystem->GetGLSettings( vidWidth, vidHeight );

			float pot;
			int	 w = vidWidth;
			pot = MakePowerOfTwo( w );
			shiftScale.x = (float)w / pot;

			int	 h = vidHeight;
			pot = MakePowerOfTwo( h );
			shiftScale.y = (float)h / pot;

			hackedView.shaderParms[4] = shiftScale.x;
			hackedView.shaderParms[5] = shiftScale.y;
		}

		gameRenderWorld->RenderScene( &portalView );
		renderSystem->CaptureRenderToImage( "_currentRender" );

		hackedView.forceUpdate = true;				// FIX: for smoke particles not drawing when portalSky present
	}
#endif // _PORTALSKY

	gameRenderWorld->RenderScene( &hackedView );

	if ( player->spectating ) {
		return;
	}

	/*
////Enable thirdperson hud code start REVILITY	- commented out by ivan
		player->DrawHUD( hud );
////Enable thirdperson hud code END REVILITY	
*/
	// draw screen blobs

#ifdef _WATER_PHYSICS //un noted code change from original sdk
	//the following code is now always executed
#else
	if ( !pm_thirdPerson.GetBool() && !g_skipViewEffects.GetBool() ) {
#endif //un noted code change from original sdk
		for ( int i = 0 ; i < MAX_SCREEN_BLOBS ; i++ ) {
			screenBlob_t	*blob = &screenBlobs[i];
			if ( blob->finishTime <= gameLocal.time ) {
				continue;
			}

			blob->y += blob->driftAmount;

			float	fade = (float)( blob->finishTime - gameLocal.time ) / ( blob->finishTime - blob->startFadeTime );
			if ( fade > 1.0f ) {
				fade = 1.0f;
			}
			if ( fade ) {
				renderSystem->SetColor4( 1,1,1,fade );
				renderSystem->DrawStretchPic( blob->x, blob->y, blob->w, blob->h,blob->s1, blob->t1, blob->s2, blob->t2, blob->material );
			}
		}

		/*		player->DrawHUD( hud );*/ //un noted code change from original sdk

		// armor impulse feedback
		float	armorPulse = ( gameLocal.time - player->lastArmorPulse ) / 250.0f;

		if ( armorPulse > 0.0f && armorPulse < 1.0f ) {
			renderSystem->SetColor4( 1, 1, 1, 1.0 - armorPulse );
			renderSystem->DrawStretchPic( 0, 0, 640, 480, 0, 0, 1, 1, armorMaterial );
		}


		// tunnel vision
		float	health = 0.0f;
		if ( g_testHealthVision.GetFloat() != 0.0f ) {
			health = g_testHealthVision.GetFloat();
		} else {
			health = player->health;
		}
		float alpha = health / 100.0f;
		if ( alpha < 0.0f ) {
			alpha = 0.0f;
		}
		if ( alpha > 1.0f ) {
			alpha = 1.0f;
		}

		if ( alpha < 1.0f  ) {
			renderSystem->SetColor4( ( player->health <= 0.0f ) ? MS2SEC( gameLocal.time ) : lastDamageTime, 1.0f, 1.0f, ( player->health <= 0.0f ) ? 0.0f : alpha );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, 1.0f, tunnelMaterial );
		}

		if ( player->PowerUpActive(BERSERK) ) {
			int berserkTime = player->inventory.powerupEndTime[ BERSERK ] - gameLocal.time;
			if ( berserkTime > 0 ) {
				// start fading if within 10 seconds of going away
				alpha = (berserkTime < 10000) ? (float)berserkTime / 10000 : 1.0f;
				renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, alpha );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, 1.0f, berserkMaterial );
			}
		}

		if ( bfgVision ) {
			renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, 1.0f, bfgMaterial );
		}

#ifndef _WATER_PHYSICS
	}
#endif //un noted code change from original sdk

	// test a single material drawn over everything
	if ( g_testPostProcess.GetString()[0] ) {
		const idMaterial *mtr = declManager->FindMaterial( g_testPostProcess.GetString(), false );
		if ( !mtr ) {
			common->Printf( "Material not found.\n" );
			g_testPostProcess.SetString( "" );
		} else {

#ifdef _WATER_PHYSICS //un noted code change from original sdk
	//insert this line of code to enable shading of GUI features 6th venom
            renderSystem->CaptureRenderToImage( "_currentRender" );
	//insert end
#endif
			renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, 1.0f, mtr );
		}
	}
}

/*
===================
idPlayerView::DoubleVision
===================
*/
void idPlayerView::DoubleVision( idUserInterface *hud, const renderView_t *view, int offset ) {

	if ( !g_doubleVision.GetBool() ) {
		SingleView( hud, view );
		return;
	}

	float	scale = offset * g_dvAmplitude.GetFloat();
	if ( scale > 0.5f ) {
		scale = 0.5f;
	}
	float shift = scale * sin( sqrtf( offset ) * g_dvFrequency.GetFloat() );
	shift = fabs( shift );

	// if double vision, render to a texture
	renderSystem->CropRenderSize( 512, 256, true );
	SingleView( hud, view );
	renderSystem->CaptureRenderToImage( "_scratch" );
	renderSystem->UnCrop();

	// carry red tint if in berserk mode
	idVec4 color(1, 1, 1, 1);
	if ( gameLocal.time < player->inventory.powerupEndTime[ BERSERK ] ) {
		color.y = 0;
		color.z = 0;
	}

	renderSystem->SetColor4( color.x, color.y, color.z, 1.0f );
	renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, shift, 1, 1, 0, dvMaterial );
	renderSystem->SetColor4( color.x, color.y, color.z, 0.5f );
	renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1-shift, 0, dvMaterial );
}

/*
===================
idPlayerView::BerserkVision
===================
*/
void idPlayerView::BerserkVision( idUserInterface *hud, const renderView_t *view ) {
	renderSystem->CropRenderSize( 512, 256, true );
	SingleView( hud, view );
	renderSystem->CaptureRenderToImage( "_scratch" );
	renderSystem->UnCrop();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, dvMaterial );
}


/*
=================
idPlayerView::Flash

flashes the player view with the given color
=================
*/
void idPlayerView::Flash(idVec4 color, int time ) {
	Fade(idVec4(0, 0, 0, 0), time);
	fadeFromColor = colorWhite;
}

/*
=================
idPlayerView::Fade

used for level transition fades
assumes: color.w is 0 or 1
=================
*/
void idPlayerView::Fade( idVec4 color, int time ) {

	if ( !fadeTime ) {
		fadeFromColor.Set( 0.0f, 0.0f, 0.0f, 1.0f - color[ 3 ] );
	} else {
		fadeFromColor = fadeColor;
	}
	fadeToColor = color;

	if ( time <= 0 ) {
		fadeRate = 0;
		time = 0;
		fadeColor = fadeToColor;
	} else {
		fadeRate = 1.0f / ( float )time;
	}

	if ( gameLocal.realClientTime == 0 && time == 0 ) {
		fadeTime = 1;
	} else {
		fadeTime = gameLocal.realClientTime + time;
	}
}

/*
=================
idPlayerView::ScreenFade
=================
*/
void idPlayerView::ScreenFade() {
	int		msec;
	float	t;

	if ( !fadeTime ) {
		return;
	}

	msec = fadeTime - gameLocal.realClientTime;

	if ( msec <= 0 ) {
		fadeColor = fadeToColor;
		if ( fadeColor[ 3 ] == 0.0f ) {
			fadeTime = 0;
		}
	} else {
		t = ( float )msec * fadeRate;
		fadeColor = fadeFromColor * t + fadeToColor * ( 1.0f - t );
	}

	if ( fadeColor[ 3 ] != 0.0f ) {
		renderSystem->SetColor4( fadeColor[ 0 ], fadeColor[ 1 ], fadeColor[ 2 ], fadeColor[ 3 ] );
		renderSystem->DrawStretchPic( 0, 0, 640, 480, 0, 0, 1, 1, declManager->FindMaterial( "_white" ) );
	}
}

/*
===================
idPlayerView::InfluenceVision
===================
*/
void idPlayerView::InfluenceVision( idUserInterface *hud, const renderView_t *view ) {

	float distance = 0.0f;
	float pct = 1.0f;
	if ( player->GetInfluenceEntity() ) {
		distance = ( player->GetInfluenceEntity()->GetPhysics()->GetOrigin() - player->GetPhysics()->GetOrigin() ).Length();
		if ( player->GetInfluenceRadius() != 0.0f && distance < player->GetInfluenceRadius() ) {
			pct = distance / player->GetInfluenceRadius();
			pct = 1.0f - idMath::ClampFloat( 0.0f, 1.0f, pct );
		}
	}
	if ( player->GetInfluenceMaterial() ) {
		SingleView( hud, view );
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, pct );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f, 1.0f, player->GetInfluenceMaterial() );
	} else if ( player->GetInfluenceEntity() == NULL ) {
		SingleView( hud, view );
		return;
	} else {
		int offset =  25 + sinf( gameLocal.time );
		DoubleVision( hud, view, pct * offset );
	}
}

/*
===================
idPlayerView::RenderPlayerView
===================
*/
void idPlayerView::RenderPlayerView( idUserInterface *hud ) {
	const renderView_t *view = player->GetRenderView();

	if ( g_skipViewEffects.GetBool() ) {
		SingleView( hud, view );
	} else {

#ifdef _DENTONMOD //un noted code change from original sdk
		// This condition makes sure that, the 2 loops inside run once only when resolution changes or map starts.
		if( screenHeight != renderSystem->GetScreenHeight() || screenWidth !=renderSystem->GetScreenWidth() ) {
			int width = 1, height = 1;	

			screenHeight	= renderSystem->GetScreenHeight();
			screenWidth		= renderSystem->GetScreenWidth();

			while( width < screenWidth ) {
				width <<= 1;
			}
			while( height < screenHeight ) {
				height <<= 1;
			}
			shiftScale_x = screenWidth  / (float)width;
			shiftScale_y = screenHeight / (float)height;
		}
#endif

		/* Render the standard view */
		if ( player->GetInfluenceMaterial() || player->GetInfluenceEntity() ) {
			InfluenceVision( hud, view );
		} else if ( gameLocal.time < dvFinishTime ) {
			DoubleVision( hud, view, dvFinishTime - gameLocal.time );
		} else if ( player->PowerUpActive( BERSERK ) ) {
			BerserkVision( hud, view );
		} else {
			SingleView( hud, view );
		}

#ifdef _DENTONMOD //un noted code change from original sdk
		/* Render bloom */
		int bloomType = r_bloom.GetInteger();
		if (bloomType != 0 && !player->objectiveSystemOpen) {

			float blur_str = 0.0f, blur_cut = 0.0f, src_str = 0.0f, src_cut = 0.0f;

			renderSystem->CaptureRenderToImage( "_currentRender" );
			//------------------------------------------------
			// Maha_x's Shift Sensitivity - Modified for compatibilty by Clone JCDenton
			//------------------------------------------------

			bool disableShiftSensitivity = ( r_bloom_shiftSensitivity_delay.GetInteger() == -1 );
			if( !disableShiftSensitivity && (bloomType == 1 || bloomType == 2) && gameLocal.time > shiftSensitivityDelay ) { 
				renderSystem->CropRenderSize(2, 2, true, true);
				shiftSensitivityDelay = gameLocal.time + r_bloom_shiftSensitivity_delay.GetInteger(); 
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/AFX/AFXweight" ) );
				renderSystem->CaptureRenderToImage( "_afxweight" );
				renderSystem->UnCrop();
			}
			//------------------------------------------------

			//-------------------------------------------------
			// User configurable buffer size - By Clone JC Denton
			//-------------------------------------------------

			int bufferMult = 5 - r_bloom_buffer.GetInteger();
			int blurMtrIndex;
			if( bufferMult < 5 ) {
				if( bufferMult <= 0 ) {
					bufferMult = 1;
					blurMtrIndex = 4;
				}
				else {
					blurMtrIndex = 4 - bufferMult;
					bufferMult = 1<<bufferMult;
				}
				renderSystem->CropRenderSize(1024/bufferMult, 512/bufferMult, true, true);
			}
			else {
				renderSystem->CropRenderSize(512, 256, true, true);
				blurMtrIndex = 3;
			}
			//-------------------------------------------------
			//renderSystem->CropRenderSize(256, 128, true, true);

			renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, shiftScale_y, shiftScale_x, 0, declManager->FindMaterial( "_currentRender" ) );

			int bloomContrast = r_bloom_contrast.GetInteger();

			if( bloomType == 2 && bloomContrast > 0 ) {
				renderSystem->CaptureRenderToImage( "_fsfx_input" );

				if( !disableShiftSensitivity ) {
					renderSystem->SetColor4( r_bloom_contrast_mult.GetFloat(), r_bloom_contrast_min.GetFloat(), 1.0f, 1.0f );
				} else {
					renderSystem->SetColor4( r_bloom_contrast_mult.GetFloat(), r_bloom_contrast_mult.GetFloat(), 1.0f, 1.0f );
				}
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/AFX/AFXmodulate" ) );
			} 
			else if( bloomType > 2 ) {
				blur_str = r_bloom_blur_mult.GetFloat();
				blur_cut = blur_str - (int)blur_str;
				src_str = r_bloom_src_mult.GetFloat();
				src_cut = src_str - (int)src_str;

				if( bloomContrast == 1) {
					renderSystem->CaptureRenderToImage( "_fsfx_input" );
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/blend_modulate_2x" ) );
				} else if( bloomContrast == 2 ) {
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/invert_dst" ) );

					renderSystem->SetColor4( 0.1f, 0.1f, 0.1f, 1.0f);
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/add_color" ) );

					renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f);
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/invert_dst" ) );

					renderSystem->CaptureRenderToImage( "_fsfx_input" );
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/blend_add" ) );
				}
			}

			if( bloomType == 1 && bloomContrast > 0 ) {
				//Unlike other bloomType, we are using the blurred image for obtaining bloom contrast.
				renderSystem->CaptureRenderToImage( "_fsfx_input" );
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( blurxMaterial[blurMtrIndex] ) );
				renderSystem->CaptureRenderToImage( "_fsfx_input" );
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( bluryMaterial[blurMtrIndex] ) );
				renderSystem->CaptureRenderToImage( "_fsfx_input" );	

				if( !disableShiftSensitivity ) {
					renderSystem->SetColor4(r_bloom_contrast_mult.GetFloat(), r_bloom_contrast_min.GetFloat(), 1, 1);
				} else {
					renderSystem->SetColor4(r_bloom_contrast_mult.GetFloat(), r_bloom_contrast_mult.GetFloat(), 1, 1);
				}
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/AFX/AFXmodulate" ) );
			}

			for( int i=0; i < r_bloom_blurIterations.GetInteger(); i++ ) {
				// Two pass gaussian filter					
				renderSystem->CaptureRenderToImage( "_fsfx_input" );
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( blurxMaterial[blurMtrIndex] ) );
				renderSystem->CaptureRenderToImage( "_fsfx_input" );
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( bluryMaterial[blurMtrIndex] ) );
			}


			if( bloomType > 2 ) {
				if ((blur_str == 0.0f) || (blur_cut > 0.0f)) {
					renderSystem->SetColor4( 0, 0, 0, 1.0f - blur_cut);
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, declManager->FindMaterial( "_white" ) );
					blur_str -= blur_cut;
				} else {
					blur_str -= 1.0f;
				} 

				renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

				if( blur_str >= 1.0f ) {									// two new lines, minor fix by Clone JCD	
					renderSystem->CaptureRenderToImage( "_fsfx_input" );	// Need to capture the current image before drawing blend_add
					while (blur_str >= 1.0f) {
						renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/blend_add" ) );
						blur_str -= 1.0f;
					}
				}
			}
			renderSystem->CaptureRenderToImage( "_fsfx_input" );

			renderSystem->UnCrop();

			if( bloomType <= 2) {
				renderSystem->SetColor4( r_bloom_blur_mult.GetFloat(), r_bloom_src_mult.GetFloat(), 1.0f, 1.0f );
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/AFX/AFXadd" ) );
			}
			else {
				if ((src_str == 0.0f) || (src_cut > 0.0f)) {
					renderSystem->SetColor4( src_cut, src_cut, src_cut, 1.0f);
					src_str -= src_cut;
				} else {
					src_str -= 1.0f;
				}

				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, shiftScale_y, shiftScale_x, 0, declManager->FindMaterial( "_currentRender" ) );

				renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
				while (src_str >= 1.0f) {
					renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, shiftScale_y, shiftScale_x, 0, declManager->FindMaterial( "textures/fsfx/current_blend_add" ) );
					src_str -= 1.0f;
				}
				renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, 1, 0, declManager->FindMaterial( "textures/fsfx/blend_add" ) );
			}

		}	
#endif
			ScreenFade();
	}

	/* Render the hud on top of everything */ //un noted code change from original sdk
	if ( !g_skipViewEffects.GetBool() && !player->objectiveSystemOpen && !player->spectating ) { //ivan - "!pm_thirdPerson.GetBool() &&" removed
		player->DrawHUD( hud );
	}

	if ( net_clientLagOMeter.GetBool() && lagoMaterial && gameLocal.isClient ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 10.0f, 380.0f, 64.0f, 64.0f, 0.0f, 0.0f, 1.0f, 1.0f, lagoMaterial );
	}
}
