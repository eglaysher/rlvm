// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------
//
// Copyright (c) 2006 Peter Jolly.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// -----------------------------------------------------------------------

#include "systems/base/rlbabel_dll.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "encodings/codepage.h"
#include "encodings/western.h"
#include "libreallive/gameexe.h"
#include "libreallive/intmemref.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"
#include "utilities/string_utilities.h"

using std::cerr;
using std::endl;

namespace {

const int kCodeMapSize = 6;
const getcReturn codemap[] = {getcEndOfString, getcEndOfString,
                              getcEndOfString, getcNewLine,
                              getcSetIndent,   getcClearIndent};

inline bool token_delimiter(char val) {
  return val < 6 || (val > 7 && val <= 32) || val == '-';
}

}  // namespace

// -----------------------------------------------------------------------
// Gloss
// -----------------------------------------------------------------------
Gloss::Gloss(const std::shared_ptr<TextWindow>& window,
             const std::string& cp932_src,
             int x1,
             int y1,
             int x2,
             int y2)
    : text_(cp932_src) {
  int line_height = window->line_height();
  while (y1 < y2) {
    // Special case for multi-line links.  Hopefully these will be rare...
    link_areas_.push_back(
        Rect::GRP(x1, y1, window->GetTextWindowSize().width(), y1 + line_height));

    y1 += line_height;
    x1 = window->current_indentation();
  }

  link_areas_.push_back(Rect::GRP(x1, y1, x2, y2 + line_height));
}

Gloss::~Gloss() {}

bool Gloss::Contains(const Point& point) {
  return std::find_if(link_areas_.begin(), link_areas_.end(), [&](Rect& r) {
           return r.Contains(point);
         }) != link_areas_.end();
}

// -----------------------------------------------------------------------
// RlBabelDLL
// -----------------------------------------------------------------------
RlBabelDLL::RlBabelDLL(RLMachine& machine)
    : add_is_italic(false),
      gloss_start_x_(0),
      gloss_start_y_(0),
      machine_(machine) {}

RlBabelDLL::~RlBabelDLL() {}

int RlBabelDLL::CallDLL(RLMachine& machine,
                        int func,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4) {
  switch (func) {
    case dllInitialise:
      return Initialize(arg1, arg2);
    case dllTextoutStart:
      if (arg1 == -1) {
        TextoutClear();
        return 1;
      } else {
        TextoutClear();
        return TextoutAdd(*GetSvar(arg1));
      }
    case dllTextoutAppend:
      return TextoutAdd(*GetSvar(arg1));
    case dllTextoutGetChar:
      return TextoutGetChar(GetSvar(arg1), GetIvar(arg2));
    case dllTextoutNewScreen:
      return StartNewScreen(*GetSvar(arg1));
    case dllSetNameMod: {
      std::shared_ptr<TextWindow> textWindow = GetWindow(arg1);
      int original_mod = textWindow->name_mod();
      textWindow->set_name_mod(arg2);
      return original_mod;
    }
    case dllGetNameMod:
      return GetWindow(arg1)->name_mod();
    case dllGetTextWindow:
      return GetWindow(-1)->window_number();
    case dllSetWindowName:
      return SetCurrentWindowName(GetSvar(arg1));
    case endSetWindowName:
    case endGetCharWinNam: {
      // Should never happen since we don't do the insane trick of overwriting
      // instruction memory to set the name of set the window name.
      return getcError;
    }
    case dllClearGlosses:
      return ClearGlosses();
    case dllNewGloss:
      return NewGloss();
    case dllAddGloss:
      return AddGloss(*GetSvar(arg1));
    case dllTestGlosses:
      return TestGlosses(arg1, arg2, GetSvar(arg3), arg4);
    case dllGetRCommandMod: {
      int window = GetWindow(arg1)->window_number();
      return machine.system().gameexe()("WINDOW")(window)("R_COMMAND_MOD");
    }
    case dllMessageBox:
    //      return rlMsgBox(arg1, arg2);
    default:
      return -1;
  }
}

const std::string& RlBabelDLL::GetDLLName() const {
  static std::string n("rlBabel");
  return n;
}

int RlBabelDLL::Initialize(int dllno, int windname) {
  // rlBabel hangs onto the DLL index and uses it for something in his
  // SetCurrentWindowName implementation.
  return 1;
}

int RlBabelDLL::TextoutAdd(const std::string& str) {
  const char* string = str.c_str();

  while (*string) {
    if (string[0] == 0x81 && (string[1] == 0x93 || string[1] == 0x96) &&
        string[2] == 0x82 && (string[3] >= 0x60 && string[3] <= 0x79)) {
      // Name reference: expand it.
      bool global = string[1] == 0x96;
      // Get index.
      int idx = string[3] - 0x60;
      string += 4;
      if (string[0] == 0x82 && (string[1] >= 0x60 && string[1] <= 0x79)) {
        idx = (idx + 1) * 26 + string[1] - 0x60;
        string += 2;
      }
      Memory& memory = machine_.memory();
      const char* namestr = global ? memory.GetName(idx).c_str()
                                   : memory.GetLocalName(idx).c_str();

      // Copy to string.
      if (string[0] == 0x82 && (string[1] >= 0x4f && string[1] <= 0x58)) {
        // Just one character
        int offset = string[1] - 0x4f;
        string += 2;
        // Find character
        while (offset-- && *namestr) {
          if (shiftjis_lead_byte(*namestr++))
            ++namestr;
        }
        // Copy character
        if (*namestr) {
          AppendChar(namestr);
        }
      } else {
        // The whole string
        while (*namestr) {
          AppendChar(namestr);
        }
      }
    } else if (string[0] == 0x08) {
      // Quotation mark
      //      string[0] = '"';
      const char* quote_mark = "\"";
      ++string;
      AppendChar(quote_mark);
    } else if (string[0] == 0x09) {
      add_is_italic = true;
      ++string;
    } else if (string[0] == 0x0a) {
      add_is_italic = false;
      ++string;
    } else {
      // Normal char: copy 1 or 2 bytes as appropriate.
      AppendChar(string);
    }
  }

  return !*string;  // 0 if the full string wasn't copied
}

void RlBabelDLL::AppendChar(const char*& ch) {
  if (add_is_italic) {
    uint16_t uc = *ch++;
    if (shiftjis_lead_byte(uc))
      uc = (uc << 8) | *ch++;
    uc = Italicise(uc);
    if (uc > 0xff)
      cp932_text_buffer += uc >> 8;
    cp932_text_buffer += uc & 0xff;
  } else {
    CopyOneShiftJisCharacter(ch, cp932_text_buffer);
  }
}

void RlBabelDLL::TextoutClear() {
  cp932_text_buffer.clear();
  text_index = 0;
  end_token_index = 0;
  add_is_italic = false;
}

int RlBabelDLL::TextoutLineBreak(StringReferenceIterator buf) {
  std::shared_ptr<TextWindow> window = GetWindow(-1);

  // If there's room on this page, break the line, otherwise break
  // the page.
  if (window->line_height() <
      window->GetTextWindowSize().height() - window->insertion_point_y()) {
    return getcNewLine;
  } else {
    // TODO(erg): This will be harder then it looks.
    // - Need to redo SetName so that conversion from cp932 is done at the
    //   absolute last minute, after (inside?) TextWindow::SetName().
    // - Need to record the cp932 name.
    // - Need accessors (none written)
    // - Pipe to here.
    //     const char* const cnam = (const char*) interpreter->getCurrentName();
    //     if (cnam)
    //       *buf =
    //     else
    *buf = "";
    return getcNewScreen;
  }
}

int RlBabelDLL::TextoutGetChar(StringReferenceIterator buffer,
                               IntReferenceIterator xmod) {
  int rv = getcPrintChar;

  // No data? Error!
  if (cp932_text_buffer.empty())
    return getcEndOfString;

  // If a new token is needed, find it and determine whether to add
  // a line break.  If a line break is needed, bail at some point
  // and return getcNewLine.
  if (text_index == end_token_index) {
    if (!end_token())
      return getcEndOfString;

    switch (end_token()) {
      case 1: {  // \{...} -> \x01...\x02
        // If this is a name setter, we read the name.
        //
        // Processing of names is complicated.  There are three
        // modes, each with their own considerations:
        //
        // 0 - the name is displayed in the window.  In this case,
        // we set it with SetCurrentWindowName, then disable name
        // display ourselves and render it manually.
        //
        // 1 - the name is displayed in a separate window.  We
        // have no direct control over how the name is laid out,
        // so we need to render it with tile bitmaps (as for
        // select() calls).
        //
        // 2 - name display is disabled.  In this case, we set it
        // with SetCurrentWindowName, and do nothing further.
        //
        // So, if #WINDOW.NAME_MOD is 0, we fill the current token
        // with it; either way, we pass it to SetCurrentWindowName
        // to process.  This function is then called again
        // automatically, and will either read the current token
        // containing the name, or start reading a new token.
        text_index = ++end_token_index;
        while (end_token() && end_token() != 2)
          ++end_token_index;
        if (!end_token()) {
          //           MessageBox(NULL, "Error: mismatched name delimiters",
          //                      "RLdev.Textout", MB_OK | MB_ICONERROR);
          // For now, just bail out on this.
          return getcEndOfString;
        }

        // Haeleth's rlBabel for RealLive does a big ugly hack right about here
        // because the name window code has something preventing him from
        // hooking the text out function so he made a whole bunch of name moji
        // tiles and rendered into those. This is silly and unnecessary since
        // I control the rlvm code...
        *buffer =
            cp932_text_buffer.substr(text_index, end_token_index - text_index);

        // If name display is not inline, skip the token to avoid
        // rendering it inline.
        if (GetWindow(-1)->name_mod() >= 1)
          text_index = end_token_index;

        // Set the window name.
        rv = SetCurrentWindowName(buffer);

        // By default, Haeleth's rlBabel will rewrite a big chunk of
        // instruction memory to writeout the equivalent of '\{name}' and a
        // CallDLL() call to signal that it's done and that rlBabel should put
        // the instruction memory back to how it was found, and then print out
        // the buffer. This is, of course, completely insane! But it was
        // probably the only we could get around RealLives lousy binary
        // interface. We can maintain compatibility by just printing the
        // current buffer out right here.
        return TextoutGetChar(buffer, xmod);
      }
      case 2: {
        Codepage& cp = Cp::instance(machine_.GetTextEncoding());
        // End of previously-handled name setter.
        // If this is the end of the string, do nothing.
        if (!end_token(1))
          return getcEndOfString;
        // Add indentation as appropriate.
        text_index = --end_token_index;
        if (end_token(2) == '"' || end_token(2) == '(' || end_token(2) == '\'') {
          end_token(0) = ' ';
          end_token(1) = end_token(2);
          end_token(2) = 4;
        } else if (end_token(2) == ':' || end_token(2) == '.' ||
                   end_token(2) == ',') {
          if (end_token(3) == ' ')
            text_index = ++end_token_index;
          end_token(0) = end_token(1);
          end_token(1) = ' ';
          end_token(2) = 4;
        } else if (cp.DbcsDelim(&(cp932_text_buffer[end_token_index + 2]))) {
          end_token(0) = ' ';
          end_token(1) = end_token(2);
          end_token(2) = end_token(3);
          end_token(3) = 4;
        } else {
          end_token(0) = ' ';
          end_token(1) = 4;
        }
        return TextoutGetChar(buffer, xmod);
      }
      case 3: {
        // Line break code (\n)
        text_index = ++end_token_index;
        return TextoutLineBreak(buffer);
      }
      case 4: {
        // Set indent code
        if (GetWindow(-1)->use_indentation()) {
          unsigned char index = end_token();
          if (index < kCodeMapSize)
            rv = codemap[index];
          else
            rv = getcError;
          text_index = ++end_token_index;
          return rv;
        }
        // If #WINDOW.INDENT_USE is 0, we ignore this code.
        text_index = ++end_token_index;
        return TextoutGetChar(buffer, xmod);
      }
      case 5: {
        unsigned char index = end_token();
        if (index < kCodeMapSize)
          rv = codemap[index];
        else
          rv = getcError;
        text_index = ++end_token_index;
        return rv;
      }
      // Tags 6 and 7 are exfont moji, which are handled as part
      // of regular tokens.
      case 8:
      case 9:
      case 10: {
        // Shouldn't happen (should be expanded away in TextoutAdd).
        // For now, ignore them.
        text_index = ++end_token_index;
        return TextoutGetChar(buffer, xmod);
      }
      case 0x1f:  // Begin gloss marker.  Treat as text.
        ++end_token_index;
      // Fall through!
      default: {
        // We tokenize text and break lines appropriately.
        // TODO(erg): Haeleth appears to be caching horizontal width. Should do
        // this once I have something working.
        //
        // ZeroMemory(widthcache, 256 * sizeof(LONG));

        // Scan ahead for next delimiter, including spaces before
        // and the first hyphen after the token.
        while (end_token() == ' ')
          ++end_token_index;
        while (!token_delimiter(end_token()))
          end_token_index += 1 + shiftjis_lead_byte(end_token()) +
                             (end_token() == 6 || end_token() == 7 ? 2 : 0);
        if (end_token() == '-')
          ++end_token_index;
        // If the token will not fit on the current line, insert a
        // line break and strip leading spaces.
        if (LineBreakRequired()) {
          while (cur_pos() == ' ')
            ++text_index;
          return TextoutLineBreak(buffer);
        }
        // Otherwise the token will begin displaying.
      }
    }
  }

  // If we got here, we have a token and no line break is needed.

  // Special case for begin-gloss:
  if (cur_pos() == 0x1f) {
    ++text_index;
    rv = getcBeginGloss;
  }

  if (cur_pos() == 6 || cur_pos() == 7) {
    // TODO(erg): Missing special case for emoji...
    cerr << "Implement special case for emoji in textoutGetChar!" << endl;
    return getcError;
  }

  // Fill buf with the next character, and xmod with how far to
  // adjust its width.
  uint16_t full_char = 0;
  char first_byte = cp932_text_buffer[text_index++];
  std::string cp932_char_out;
  cp932_char_out += first_byte;
  if (shiftjis_lead_byte(first_byte)) {
    cp932_char_out += cp932_text_buffer[text_index++];
    full_char = (cp932_char_out[0] << 8) | cp932_char_out[1];
  } else {
    full_char = cp932_char_out[0];
  }

  // FIXME: what's wrong with nbsps?
  if (full_char == 0x89a0) {
    cp932_char_out.clear();
    cp932_char_out += 0x20;
    full_char = 0x20;
  }

  *buffer = cp932_char_out;
  *xmod = GetCharWidth(full_char, true);
  return rv;
}

int RlBabelDLL::StartNewScreen(const std::string& cnam) {
  if (cnam.empty())
    return getcEndOfString;

  string buf = cnam;
  buf += "\x02\"";
  buf += cp932_text_buffer.substr(text_index);
  TextoutClear();
  return TextoutAdd(buf);
}

int RlBabelDLL::SetCurrentWindowName(StringReferenceIterator buffer) {
  // Haeleth's implementation of SetCurrentWindowName in rlBabel goes through
  // some monstrous hacks, including temporarily rewriting the bytecode at the
  // instruction pointer. I *think* I can get away with a simple:
  GetWindow(-1)->SetNameWithoutDisplay(*buffer);
  return 1;
}

int RlBabelDLL::ClearGlosses() {
  glosses_.clear();
  return 1;
}

int RlBabelDLL::NewGloss() {
  std::shared_ptr<TextWindow> window = GetWindow(-1);
  gloss_start_x_ = window->insertion_point_x();
  gloss_start_y_ = window->insertion_point_y();
  return 1;
}

int RlBabelDLL::AddGloss(const std::string& cp932_gloss_text) {
  std::shared_ptr<TextWindow> window = GetWindow(-1);
  glosses_.emplace_back(window,
                        cp932_gloss_text,
                        gloss_start_x_,
                        gloss_start_y_,
                        window->insertion_point_x(),
                        window->insertion_point_y());
  return 1;
}

int RlBabelDLL::TestGlosses(int x,
                            int y,
                            StringReferenceIterator text,
                            int globalwaku) {
  // Does this handle all cases?
  std::shared_ptr<TextWindow> window = GetWindow(-1);
  Point textOrigin = window->GetTextSurfaceRect().origin();
  x -= textOrigin.x();
  y -= textOrigin.y();

  std::vector<Gloss>::const_iterator it =
      std::find_if(glosses_.begin(), glosses_.end(), [&](Gloss& gloss) {
        return gloss.Contains(Point(x, y));
      });
  if (it == glosses_.end())
    return 0;

  *text = it->text();
  return 1;
}

int RlBabelDLL::GetCharWidth(uint16_t cp932_char, bool as_xmod) {
  Codepage& cp = Cp::instance(machine_.GetTextEncoding());
  uint16_t native_char = cp.JisDecode(cp932_char);
  uint16_t unicode_codepoint = cp.Convert(native_char);
  std::shared_ptr<TextWindow> window = GetWindow(-1);
  int font_size = window->font_size_in_pixels();
  // TODO(erg): Can I somehow modify this to try to do proper kerning?
  int width = machine_.system().text().GetCharWidth(font_size, unicode_codepoint);
  return as_xmod ? window->insertion_point_x() + width : width;
}

bool RlBabelDLL::LineBreakRequired() {
  std::shared_ptr<TextWindow> window = GetWindow(-1);

  int width = 0;
  std::string::size_type ptr = text_index;
  while (ptr < end_token_index) {
    uint16_t cp932_char = ConsumeNextCharacter(ptr);
    if (text_index < end_token_index) {
      width += GetCharWidth(cp932_char, false);
    } else {
      width += window->font_size_in_pixels();
    }
  }

  int max_space = window->GetTextWindowSize().width();
  int remaining_space = max_space - window->insertion_point_x();

  // If the token will fit on the current line, no line break is required.
  if (width < remaining_space) {
    return false;
  }

  // If the token will not fit on the next line either, truncate it.
  max_space -= window->current_indentation();
  if (width >= max_space) {
    ptr = text_index;
    uint16_t cp932_char = ConsumeNextCharacter(ptr);
    width = GetCharWidth(cp932_char, false);

    // If the first character will fit on the current line, a line break is not
    // required.
    if (width < remaining_space) {
      while (ptr < end_token_index) {
        cp932_char = ConsumeNextCharacter(ptr);
        int cw = GetCharWidth(cp932_char, false);
        if (width + cw >= remaining_space)
          break;
        ptr += 1 + (cp932_char > 0xff);
        width += cw;
      }

      end_token_index = ptr;
      return false;
    }

    // If nothing CAN fit, EVER, something is seriously wrong; we
    // bail out to avoid disaster.
    if (width >= max_space)
      return false;

    // If this is not the case, however, we truncate to fit on the
    // next line, and a break is required.
    while (ptr < end_token_index) {
      cp932_char = ConsumeNextCharacter(ptr);
      int cw = GetCharWidth(cp932_char, false);
      if (width + cw >= remaining_space)
        break;
      ptr += 1 + (cp932_char > 0xff);
      width += cw;
    }

    end_token_index = ptr;
    // Don't return, but move on to the regular line-break-required handling
    // below.
  }

  // The token will fit on the next line, but not the current line.
  return true;
}

uint16_t RlBabelDLL::ConsumeNextCharacter(std::string::size_type& index) {
  char cp932sb = cp932_text_buffer[index++];
  uint16_t cp932 = cp932sb;
  if (shiftjis_lead_byte(cp932sb))
    cp932 = (cp932 << 8) | cp932_text_buffer[index++];

  return cp932;
}

IntReferenceIterator RlBabelDLL::GetIvar(int addr) {
  int bank = (addr >> 16) % 26;
  int location = addr & 0xffff;
  return IntReferenceIterator(&(machine_.memory()), bank, location);
}

StringReferenceIterator RlBabelDLL::GetSvar(int addr) {
  Memory* m = &(machine_.memory());
  int bank = addr >> 16;

  switch (bank) {
    case libreallive::STRS_LOCATION:
    case libreallive::STRM_LOCATION:
      return StringReferenceIterator(m, bank, addr & 0xfff);
    case libreallive::STRK_LOCATION:
      // To be bug for bug compatible with the real rlBabel so people don't
      // start targetting rlvm.
      cerr << "strK[] support is not implemented: falling back on strS[0]"
           << endl;
      break;
    default: {
      cerr << "Invalid string address; falling back on strS[0]" << endl;
      break;
    }
  }

  // Error.
  return StringReferenceIterator(m, libreallive::STRS_LOCATION, 0);
}

std::shared_ptr<TextWindow> RlBabelDLL::GetWindow(int id) {
  TextSystem& text_system = machine_.system().text();
  if (id >= 0) {
    return text_system.GetTextWindow(id);
  } else {
    return text_system.GetCurrentWindow();
  }
}
