//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#if HAVE_OGGVORBIS

#include "audio/sfx_openal.hpp"

#include <assert.h>
#include <stdio.h>
#include <string>

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif

#include "config/user_config.hpp"
#include "io/file_manager.hpp"

SFXOpenAL::SFXOpenAL(ALuint buffer, bool positional, float rolloff, float gain) : SFXBase()
{
    m_soundBuffer = buffer;
    m_soundSource = 0;
    m_ok          = false;
    m_positional  = false;
    m_defaultGain = gain;

    // Don't initialise anything else if the sfx manager was not correctly
    // initialised. First of all the initialisation will not work, and it
    // will not be used anyway.
    if(sfx_manager->sfxAllowed())
    {
        alGenSources(1, &m_soundSource );
        if (!SFXManager::checkError("generating a source")) return;

        assert( alIsBuffer(m_soundBuffer) );
        assert( alIsSource(m_soundSource) );

        //std::cout << "Setting a source with buffer " << m_soundBuffer << ", rolloff " << rolloff
        //          << ", gain=" << m_defaultGain << ", positional=" << (positional ? "true" : "false") << std::endl;

        alSourcei (m_soundSource, AL_BUFFER,          m_soundBuffer);

        if (!SFXManager::checkError("attaching the buffer to the source")) return;
    
        alSource3f(m_soundSource, AL_POSITION,        0.0, 0.0, 0.0);
        alSource3f(m_soundSource, AL_VELOCITY,        0.0, 0.0, 0.0);
        alSource3f(m_soundSource, AL_DIRECTION,       0.0, 0.0, 0.0);
        alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  rolloff      );
        alSourcef (m_soundSource, AL_GAIN,            m_defaultGain);
    

        if (positional) alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_FALSE);
        else            alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE);

        m_positional = positional;
        m_ok = SFXManager::checkError("setting up the source");
    }
}   // SFXOpenAL

//-----------------------------------------------------------------------------
SFXOpenAL::~SFXOpenAL()
{
    alDeleteSources(1, &m_soundSource);
}   // ~SFXOpenAL

//-----------------------------------------------------------------------------
/** Changes the pitch of a sound effect.
 *  \param factor Speedup/slowdown between 0.5 and 2.0
 */
void SFXOpenAL::speed(float factor)
{
    if(!m_ok) return;

    //OpenAL only accepts pitches in the range of 0.5 to 2.0
    if(factor > 2.0f)
    {
        factor = 2.0f;
    }
    if(factor < 0.5f)
    {
        factor = 0.5f;
    }
    alSourcef(m_soundSource,AL_PITCH,factor);
    SFXManager::checkError("changing the speed");
}   // speed

//-----------------------------------------------------------------------------
/** Changes the volume of a sound effect.
 *  \param gain Volume adjustment between 0.0 (mute) and 1.0 (full volume).
 */
void SFXOpenAL::volume(float gain)
{
    if(!m_ok) return;

    alSourcef(m_soundSource, AL_GAIN, m_defaultGain * gain);
    SFXManager::checkError("setting volume");
}   // volume

//-----------------------------------------------------------------------------
/** Loops this sound effect.
 */
void SFXOpenAL::loop()
{
    if(!m_ok) return;

    alSourcei(m_soundSource, AL_LOOPING, AL_TRUE);
    SFXManager::checkError("looping");
}   // loop

//-----------------------------------------------------------------------------
/** Stops playing this sound effect.
 */
void SFXOpenAL::stop()
{
    if(!m_ok) return;

    alSourcei(m_soundSource, AL_LOOPING, AL_FALSE);
    alSourceStop(m_soundSource);
    SFXManager::checkError("stoping");
}   // stop

//-----------------------------------------------------------------------------
/** Pauses a SFX that's currently played. Nothing happens it the effect is
 *  currently not being played.
 */
void SFXOpenAL::pause()
{
    if(!m_ok) return;
    alSourcePause(m_soundSource);
    SFXManager::checkError("pausing");
}   // pause

//-----------------------------------------------------------------------------
/** Resumes a sound effect.
 */
void SFXOpenAL::resume()
{
    if(!m_ok) return;

    alSourcePlay(m_soundSource);
    SFXManager::checkError("resuming");
}   // resume

//-----------------------------------------------------------------------------
/** Plays this sound effect.
 */
void SFXOpenAL::play()
{
    if(!m_ok) return;

    alSourcePlay(m_soundSource);
    SFXManager::checkError("playing");
}   // play

//-----------------------------------------------------------------------------
/** Sets the position where this sound effects is played.
 *  \param position Position of the sound effect.
 */
void SFXOpenAL::position(const Vec3 &position)
{
    if(!m_ok||!m_positional) return;

    alSource3f(m_soundSource, AL_POSITION,
               (float)position.getX(), (float)position.getY(), (float)position.getZ());
    SFXManager::checkError("positioning");
}   // position

//-----------------------------------------------------------------------------
/** Returns the status of this sound effect.
 */
SFXManager::SFXStatus SFXOpenAL::getStatus()
{
    if(!m_ok) return SFXManager::SFX_UNKNOWN;

    int state = 0;
    alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
    switch(state)
    {
    case AL_STOPPED: return SFXManager::SFX_STOPED;
    case AL_PLAYING: return SFXManager::SFX_PLAYING;
    case AL_PAUSED:  return SFXManager::SFX_PAUSED;
    case AL_INITIAL: return SFXManager::SFX_INITIAL;
    default:         return SFXManager::SFX_UNKNOWN;
    }
}   // getStatus

#endif //if HAVE_OGGVORBIS
