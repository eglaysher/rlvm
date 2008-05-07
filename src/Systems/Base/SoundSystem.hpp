// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#ifndef __SoundSystem_hpp__
#define __SoundSystem_hpp__

// -----------------------------------------------------------------------

#include <map>
#include <string>

// -----------------------------------------------------------------------

class Gameexe;
class RLMachine;

// -----------------------------------------------------------------------

const int NUM_BASE_CHANNELS = 16;
const int NUM_EXTRA_WAVPLAY_CHANNELS = 8;

// -----------------------------------------------------------------------

/**
 *
 */
class SoundSystem
{
public:
  /**
   * Defines a piece of background music who's backed by a file,
   * usually VisualArt's nwa format.
   */
  struct DSTrack {
    DSTrack();
    DSTrack(const std::string name, const std::string file,
            int from, int to, int loop);

    std::string name;
    std::string file;
    int from;
    int to;
    int loop;
  };

  /**
   * Defines a piece of background music who's backed by a cd audio
   * track.
   */
  struct CDTrack {
    CDTrack();
    CDTrack(const std::string name, int from, int to, int loop);

    std::string name;
    int from;
    int to;
    int loop;
  };

protected:
  /// Type for a parsed \#SE table.
  typedef std::map<int, std::pair<std::string, int> > SeTable;

  /// Type for parsed \#DSTRACK entries.
  typedef std::map<std::string, DSTrack> DSTable;

  /// Type for parsed \#CDTRACK entries.
  typedef std::map<std::string, CDTrack> CDTable;

  /** 
   * Stores data about an ongoing volume adjustment (such as those
   * started by fun wavSetVolume(int, int, int).)
   */
  struct VolumeAdjustTask {
    VolumeAdjustTask(unsigned int currentTime, int inStartVolume, 
                     int inFinalVolume, int fadeTimeInMs);

    unsigned int startTime;
    unsigned int endTime;

    int startVolume;
    int finalVolume;

    /// Calculate the volume for inTime
    int calculateVolumeFor(unsigned int inTime);
  };

  typedef std::map<int, VolumeAdjustTask> ChannelAdjustmentMap;

private:

  /**
   * Number passed in from RealLive that represents the 
   *
   * 0 	          	11 kHz 	          	8 bit
   * 1 	          	11 kHz 	          	16 bit
   * 2 	          	22 kHz 	          	8 bit
   * 3 	          	22 kHz 	          	16 bit
   * 4 	          	44 kHz 	          	8 bit
   * 5 	          	44 kHz 	          	16 bit
   * 6 	          	48 kHz 	          	8 bit
   * 7 	          	48 hKz 	          	16 bit
   * 
   */
  int m_soundQuality;

  /**
   * @name Background Music data
   * 
   * @{
   */
  
  /// Whether music playback is enabled
  bool m_bgmEnabled;

  /// Volume for the music
  unsigned char m_bgmVolume;

  /**
   * Status of the music subsystem
   *
   * - 0 Idle
   * - 1 Playing music
   * - 2 Fading out music
   */
  int m_bgmStatus;

  /// Defined music tracks (files)
  DSTable m_dsTracks;

  /// Defined music tracks (cd tracks)
  CDTable m_cdTracks;

  /// @}

  // ---------------------------------------------------------------------

  /**
   * @name PCM/Wave sound effect data
   * 
   * @{
   */
  /// Whether the Wav functions are enabled
  bool m_pcmEnabled;

  /**
   * Volume of wave files relative to other sound playback.
   */
  unsigned char m_pcmVolume;

  /// Per channel volume
  unsigned char m_channelVolume[NUM_BASE_CHANNELS];

  /**
   * Open tasks that adjust the volume of a wave channel. We do this
   * because SDL_mixer doesn't provide this functionality and I'm
   * guessing other mixers don't either.
   *
   * @note Depending on features in other systems, I may push this
   *       down to SDLSoundSystem later.
   */
  ChannelAdjustmentMap m_pcmAdjustmentTasks;

  /// @}

  // ---------------------------------------------------------------------

  /**
   * @name Interface sound effect data
   * 
   * @{
   */

  /// Whether the Se functions are enabled
  bool m_seEnabled;

  /** Volume of interface sound effects relative to other sound
   * playback.
   */
  unsigned char m_seVolume;

  /**
   * Parsed \#SE.index entries. Maps a sound effect number to the
   * filename to play and the channel to play it on.
   */
  SeTable m_seTable;

  /// @}

protected:
  SeTable& seTable() { return m_seTable; }
  const DSTable& getDSTable() { return m_dsTracks; }
  const CDTable& getCDTable() { return m_cdTracks; }

  /** 
   * Computes the actual volume for a channel based on the per channel
   * and the per system volume.
   */
  int computeChannelVolume(const int channelVolume, const int systemVolume) {
    return (channelVolume * systemVolume) / 255;
  }

  static void checkChannel(int channel, const char* functionName);
  static void checkVolume(int level, const char* functionName);

public: 
  SoundSystem(Gameexe& gexe);
  virtual ~SoundSystem();

  /**
   * Gives the sound system a chance to run; done once per game loop.
   *
   * @note Overriders MUST call SoundSystem::executeSoundSystem
   *       because we rely on it to handle volume adjustment tasks.
   */
  virtual void executeSoundSystem(RLMachine& machine);

  // ---------------------------------------------------------------------
  
  /**
   * Sets how much sound hertz.
   */
  virtual void setSoundQuality(const int quality) 
  { m_soundQuality = quality; }

  int soundQuality() const { return m_soundQuality; }

  // ---------------------------------------------------------------------
  /**
   * @name BGM functions
   * 
   * @{
   */
  virtual void setBgmEnabled(const int in);
  
  int bgmEnabled() const;
  virtual void setBgmVolume(const int in);
  int bgmVolume() const;

  int bgmStatus() const;

  /// @}

  // ---------------------------------------------------------------------

  /**
   * @name PCM/Wave functions
   * 
   * @{
   */
  /// Sets whether the wav* functions play
  virtual void setPcmEnabled(const int in);

  /// Whether the wav* functions play
  int pcmEnabled() const;

  virtual void setPcmVolume(const int in);
  int pcmVolume() const;

  /// Sets an individual channel volume
  virtual void setChannelVolume(const int channel, const int level);

  /// Change the volume smoothly; the change from the current volume
  /// to level will take fadeTimeInMs
  void setChannelVolume(RLMachine& machine, const int channel, 
                        const int level, const int fadeTimeInMs);

  /// Fetches an individual channel volume
  int channelVolume(const int channel);
  
  virtual void wavPlay(RLMachine& machine, const std::string& wavFile) = 0;
  virtual void wavPlay(RLMachine& machine, const std::string& wavFile,
                       const int channel) = 0;
  virtual void wavPlay(RLMachine& machine, const std::string& wavFile,
                       const int channel, const int fadeinMs) = 0;

  /// @}

  // ---------------------------------------------------------------------

  /**
   * @name Sound Effect functions
   * 
   * @{
   */
  /**
   * Sets whether we should have interface sound effects
   */
  virtual void setSeEnabled(const int in);

  /**
   * Returns whether (interface) sound effects are enabled
   */
  int seEnabled() const;

  /** 
   * Sets the volume of interface sound effects relative to other
   * sound playback.
   * 
   * @param in Sound Effect volume (0-255)
   */
  virtual void setSeVolume(const int in);

  /** 
   * Gets the current sound effect volume.
   */
  int seVolume() const;
  
  /** 
   * Plays an interface sound effect.
   * 
   * @param seNum Index into the \#SE table 
   */
  virtual void playSe(RLMachine& machine, const int seNum) = 0;
  /// @}
};	// end of class SoundSystem

#endif
