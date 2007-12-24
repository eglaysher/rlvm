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

struct MusicTrack
{
  
};

// -----------------------------------------------------------------------

/**
 *
 */
class SoundSystem
{
private:
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
  typedef std::map<int, std::pair<std::string, int> > SeTable;
  SeTable m_seTable;

  /// @}

protected:
  SeTable& seTable() { return m_seTable; }

public: 
  SoundSystem(Gameexe& gexe);
  virtual ~SoundSystem();

  // ---------------------------------------------------------------------

  /**
   * @name Sound Effect functions
   * 
   * @{
   */
  /**
   * Sets whether we should have interface sound effects
   */
  void setSeEnabled(const int in);

  /**
   * Returns whether (interface) sound effects are enabled
   */
  int seEnabled() const;
  
  /** 
   * Plays an interface sound effect.
   * 
   * @param seNum Index into the \#SE table 
   */
  virtual void playSe(RLMachine& machine, const int seNum) = 0;
  /// @}
};	// end of class SoundSystem

#endif
