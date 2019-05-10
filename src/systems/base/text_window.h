// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_TEXT_WINDOW_H_
#define SRC_SYSTEMS_BASE_TEXT_WINDOW_H_

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <utility>

#include "systems/base/rect.h"
#include "systems/base/colour.h"

class Gameexe;
class GameexeInterpretObject;
class GraphicsSystem;
class Point;
class RLMachine;
class SelectionElement;
class Surface;
class System;
class TextSystem;
class TextWaku;
class TextWindowButton;

const int kNumFaceSlots = 8;

// Abstract representation of a TextWindow. Aggregated by TextSystem, and
// rendered in conjunction with GraphicsSystem.
//
// Sets of TextWindows should be reconstructable by the state in TextPage,
// though there are some notable exceptions, specifically Select_LongOperation.
//
// This class has all sorts of complex, rarely used text rendering options,
// including multiple co-ordinate systems, which I'm sure was done to give
// reverse engineers a headache.
class TextWindow {
 public:
  TextWindow(System& system, int window_num);
  virtual ~TextWindow();

  void Execute();

  int window_number() const { return window_num_; }

  // TODO(erg): This is nowhere near good enough and handling waku better needs
  // to be put on the long term goal list. Should waku_set() live on the
  // TextWindow? O RLY?
  int waku_set() const { return waku_set_; }

  // Sets the size of the text window in characters. Represented by
  // #WINDOW.xxx.MOJI_CNT.
  void SetWindowSizeInCharacters(const std::vector<int>& pos_data);

  // Sets the size of the spacing between characters. Represented by
  // #WINDOW.xxx.MOJI_REP.
  void SetSpacingBetweenCharacters(const std::vector<int>& pos_data);

  // Sets the size of the ruby (furigana; pronunciation guide) text in
  // pixels. If zero, ruby text is disabled in this window. Represented by
  // #WINDOW.xxx.LUBY_SIZE.
  void set_ruby_text_size(const int i) { ruby_size_ = i; }
  int ruby_text_size() const { return ruby_size_; }

  // Sets the size of the font. Represented by #WINDOW.xxx.MOJI.SIZE.
  void set_font_size_to_default() {
    font_size_in_pixels_ = default_font_size_in_pixels_;
  }
  void set_font_size_in_pixels(int i) { font_size_in_pixels_ = i; }
  int font_size_in_pixels() const { return font_size_in_pixels_; }

  void SetTextboxPadding(const std::vector<int>& pos_data);

  void set_use_indentation(const int i) { use_indentation_ = i; }
  int use_indentation() const { return use_indentation_; }

  void SetIndentation();
  int current_indentation() const { return current_indentation_in_pixels_; }

  void SetDefaultTextColor(const std::vector<int>& colour_data);
  virtual void SetFontColor(const std::vector<int>& colour_data);

  void SetWindowPosition(const std::vector<int>& pos_data);

  // We have to differentiate between the official size where we're supposed to
  // put text (GetTextWindowSize()) and the actual size of the surface
  // (GetTextSurfaceSize()) because GetTextSurfaceSize() contains extra space
  // for one squashed kinsoku character.
  Size GetTextWindowSize() const;
  Size GetTextSurfaceSize() const;

  Rect GetWindowRect() const;
  Rect GetTextSurfaceRect() const;

  // Locations of the namebox waku (will be a relative calculation to
  // GetWindowRect()) and the name text surface (will be a relative calculation
  // to the waku).
  Rect GetNameboxWakuRect() const;

  // The size of the writable text area.
  Size GetNameboxTextArea() const;

  // TODO(erg): What's SetMousePosition and how does it differ from mouse
  // listeners?
  void SetMousePosition(const Point& pos);
  bool HandleMouseClick(RLMachine& machine,
                        const Point& pos,
                        bool pressed);

  // Sets how the name is displayed
  void set_name_mod(const int in) { name_mod_ = in; }
  int name_mod() const { return name_mod_; }

  // Sets the size of the spacing between characters. Represented by
  // #WINDOW.xxx.NAME_MOJI_REP.
  void SetNameSpacingBetweenCharacters(const std::vector<int>& pos_data);

  void SetNameboxPadding(const std::vector<int>& pos_data);
  void SetNameboxPosition(const std::vector<int>& pos_data);

  // Set and access the position of the animated icon that appears when waiting
  // for user input. The TextWindow only owns the position which is queried by
  // the TextKeyCursor object which owns the actual image and other
  // properties. This is an artifact of how RealLive handles this.
  void SetKeycursorMod(const std::vector<int>& keycur);
  Point KeycursorPosition(const Size& cursor_size) const;

  // Accessors regarding the background colour of the window.
  //
  // Represents the data parsed from #WINDOW_ATTR,
  // #WINDOW.index.ATTR_MOD, and #WINDOW.index.ATTR
  int windowAttrMod() const { return window_attr_mod_; }

  void set_filter(int i) { filter_ = i; }
  void SetRGBAF(const std::vector<int>& rgba_values);

  int r() const { return colour_.r(); }
  int g() const { return colour_.g(); }
  int b() const { return colour_.b(); }
  int alpha() const { return colour_.a(); }
  const RGBAColour& colour() const { return colour_; }
  int filter() const { return filter_; }

  void set_is_visible(int in) { is_visible_ = in; }
  bool is_visible() const { return is_visible_; }

  void set_action_on_pause(const int i) { action_on_pause_ = i; }
  bool action_on_pause() const { return action_on_pause_; }

  int insertion_point_x() const { return text_insertion_point_x_; }
  int insertion_point_y() const { return text_insertion_point_y_; }
  void offset_insertion_point_x(int offset) { text_insertion_point_x_ += offset; }
  void offset_insertion_point_y(int offset) { text_insertion_point_y_ += offset; }
  void set_insertion_point_x(int x) { text_insertion_point_x_ = x; }
  void set_insertion_point_y(int y) { text_insertion_point_y_ = y; }

  int line_height() const {
    return font_size_in_pixels_ + y_spacing_ + ruby_size_;
  }

  // Loads |filename| into face slot |index|.
  void FaceOpen(const std::string& filename, int index);

  // Clears face slot |index|.
  void FaceClose(int index);

  // Marks that the next character rendered in the window should be italic.
  void NextCharIsItalic();

  // ------------------------------------------------ [ Abstract interface ]
  void Render(std::ostream* tree);

  // Returns a surface that is the text.
  virtual std::shared_ptr<Surface> GetTextSurface() = 0;
  virtual std::shared_ptr<Surface> GetNameSurface() = 0;

  // Clears the text window of all text and resets the insertion
  // point.
  virtual void ClearWin();

  // Displays one character, and performs line breaking logic based on the next
  // character. Returns true if the character fits on the screen. False if it
  // does not and was not displayed.
  virtual bool DisplayCharacter(const std::string& current,
                                const std::string& rest);

  // Checks to make sure that not only will |cur_codepoint| fit on the line,
  // but also that we'll perform kinsoku rules correctly.
  bool MustLineBreak(int cur_codepoint, const std::string& rest);

  // Returns whether another character can be placed on the screen.
  bool IsFull() const;

  // Sets (and displays, if appropriate) the name of the current speaker.
  virtual void SetName(const std::string& utf8name,
                       const std::string& next_char);
  void SetNameWithoutDisplay(const std::string& utf8name);

  virtual void RenderNameInBox(const std::string& utf8str) = 0;

  virtual void KoeMarker(int id);
  virtual void HardBrake();
  virtual void ResetIndentation();
  virtual void MarkRubyBegin();
  virtual void DisplayRubyText(const std::string& utf8str) = 0;

  // Text Windows are responsible for presenting the questions from
  // select() and select_s() calls. (select_w() is not done here.)
  virtual void StartSelectionMode();

  bool in_selection_mode() { return in_selection_mode_; }

  virtual void AddSelectionItem(const std::string& utf8str,
                                int selection_id) = 0;
  virtual void SetSelectionCallback(const std::function<void(int)>& func);

  void EndSelectionMode();

 protected:
  // Accessor for the |selection_callback_| for TextWindow subclasses
  const std::function<void(int)>& selectionCallback() {
    return selection_callback_;
  }

  // The actual selection items in this TextWindow.
  typedef std::vector<std::unique_ptr<SelectionElement>> Selections;
  Selections selections_;

  System& system() { return system_; }

  // Render faces implementation.
  void RenderFaces(std::ostream* tree, int behind);

  void RenderKoeReplayButtons(std::ostream* tree);

  int GetWrappingWidthFor(int cur_codepoint);

 protected:
  // We cache the size of the screen so we don't need the machine in
  // some accessors.
  int screen_width_, screen_height_;

  // Our numeric window identifier.
  int window_num_;

  // The waku set number.
  int waku_set_;

  // The window decorations for the text window
  std::unique_ptr<TextWaku> textbox_waku_;

  // The text insertion point. These two numbers are relative to the
  // text window location and represent the top left corner of where
  // the next piece of text should be inserted.
  int text_insertion_point_x_;
  int text_insertion_point_y_;

  // In Japanese games, this will be exactly the same as
  // |text_insertion_point_x_|. In official English edition games, this will be
  // an internal count as if all characters were monospaced.
  int text_wrapping_point_x_;

  // Current ruby insertion point (or -1 if MarkRubyBegin() hasn't
  // been called)
  int ruby_begin_point_;

  // The line number in this text window; used to detect whether we
  // have filled this text box
  int current_line_number_;

  // The initial value of text_insertion_point_y_ on new lines.
  int current_indentation_in_pixels_;

  // The initial value of |text_wrapping_point_x_| on new lines.
  int current_indentation_in_chars_;

  // Whether the last token was a SetName. This is used to control indentation
  // for quotes.
  bool last_token_was_name_;

  // The default font size.
  int default_font_size_in_pixels_;

  // The current size of the font
  int font_size_in_pixels_;

  // The current size of the ruby text in pixels
  int ruby_size_;

  // Size of the window in characters
  int x_window_size_in_chars_, y_window_size_in_chars_;

  // Spacing between characters
  int x_spacing_, y_spacing_;

  // Whether to indent (INDENT_USE)
  int use_indentation_;

  // The default colour. Initialized to #COLOR_TABLE.000, but can be
  // changed with the SetFontColour() command.
  RGBColour default_colour_;

  // The current colour. Initialized to the default colour on every
  // clearWin() call.
  RGBColour font_colour_;

  // Determines how the window will react to pause()
  // calls. Initialized to #WINDOW.x.R_COMMAND_MOD.
  int action_on_pause_;

  int origin_, x_distance_from_origin_, y_distance_from_origin_;

  int upper_box_padding_;
  int lower_box_padding_;
  int left_box_padding_;
  int right_box_padding_;

  // Whether r_, etc is a per-window colour.
  int window_attr_mod_;

  // The default window background colour.
  RGBAColour colour_;
  int filter_;

  int is_visible_;

  // Determines the position of the keycursor (the animated cursor
  // that appears when the game is waiting for a click to move to the
  // next page of text).
  //
  // If type is 0, the cursor appears at the bottom right corner of
  // the text area; if it is 1, it appears directly after the final
  // character printed; if it is 2, it appears at (x, y) relative to
  // the top left of the text area. (x and y are ignored when type is
  // 0 or 1.)
  int keycursor_type_;
  Point keycursor_pos_;

  // Describes how to render character names.
  // - 0: Display names inline (default)
  // - 1: Display names in a separate window
  // - 2: Do not display names
  int name_mod_;

  // Waku set to use for the text box in the case where name_mod_ == 1.
  int name_waku_set_;

  std::unique_ptr<TextWaku> namebox_waku_;

  int name_font_size_in_pixels_;

  int name_waku_dir_set_;

  // Spacing between characters
  int name_x_spacing_;

  int horizontal_namebox_padding_;
  int vertical_namebox_padding_;

  int namebox_x_offset_;
  int namebox_y_offset_;

  int namebox_centering_;

  int minimum_namebox_size_;

  int name_size_;

  // The number of characters in the window (or |minimum_namebox_size_|).
  int namebox_characters_;

  // Text boxes can be in selection mode, in which case a Select_LongOperation
  // is on the top of the RLMachine's call stack.
  bool in_selection_mode_;

  bool next_char_italic_;

  // Callback function for when item is selected; usually will call a
  // specific method on Select_LongOperation
  std::function<void(int)> selection_callback_;

  struct FaceSlot;
  std::unique_ptr<FaceSlot> face_slot_[kNumFaceSlots];

  // A list of visible koe replay buttons. The Point is the top left corner of
  // the button and int is the corresponding id number.
  std::vector<std::pair<Point, int>> koe_replay_button_;

  // We lazily parse and load data about displaying the koe icon on demand.
  struct KoeReplayInfo {
    std::shared_ptr<const Surface> icon;
    Size repos;
  };
  std::unique_ptr<KoeReplayInfo> koe_replay_info_;

  System& system_;
  TextSystem& text_system_;
};

#endif  // SRC_SYSTEMS_BASE_TEXT_WINDOW_H_
