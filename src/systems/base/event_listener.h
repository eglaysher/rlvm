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

#ifndef SRC_SYSTEMS_BASE_EVENT_LISTENER_H_
#define SRC_SYSTEMS_BASE_EVENT_LISTENER_H_

class RLMachine;
class Point;

enum MouseButton {
  MOUSE_NONE = 0,
  MOUSE_LEFT,
  MOUSE_RIGHT,
  MOUSE_MIDDLE,
  MOUSE_WHEELUP,
  MOUSE_WHEELDOWN
};

// Note that this looks suspiciously like the SDLKey definition with
// s/SDLK_/RLKEY/;. This was done because I'm lazy and SDL was the
// first backend I used, and they're right that lining up with ASCII
// as much as possible is a really good idea.
enum KeyCode {
  // The keyboard syms have been cleverly chosen to map to ASCII
  RLKEY_UNKNOWN = 0,
  RLKEY_FIRST = 0,
  RLKEY_BACKSPACE = 8,
  RLKEY_TAB = 9,
  RLKEY_CLEAR = 12,
  RLKEY_RETURN = 13,
  RLKEY_PAUSE = 19,
  RLKEY_ESCAPE = 27,
  RLKEY_SPACE = 32,
  RLKEY_EXCLAIM = 33,
  RLKEY_QUOTEDBL = 34,
  RLKEY_HASH = 35,
  RLKEY_DOLLAR = 36,
  RLKEY_AMPERSAND = 38,
  RLKEY_QUOTE = 39,
  RLKEY_LEFTPAREN = 40,
  RLKEY_RIGHTPAREN = 41,
  RLKEY_ASTERISK = 42,
  RLKEY_PLUS = 43,
  RLKEY_COMMA = 44,
  RLKEY_MINUS = 45,
  RLKEY_PERIOD = 46,
  RLKEY_SLASH = 47,
  RLKEY_0 = 48,
  RLKEY_1 = 49,
  RLKEY_2 = 50,
  RLKEY_3 = 51,
  RLKEY_4 = 52,
  RLKEY_5 = 53,
  RLKEY_6 = 54,
  RLKEY_7 = 55,
  RLKEY_8 = 56,
  RLKEY_9 = 57,
  RLKEY_COLON = 58,
  RLKEY_SEMICOLON = 59,
  RLKEY_LESS = 60,
  RLKEY_EQUALS = 61,
  RLKEY_GREATER = 62,
  RLKEY_QUESTION = 63,
  RLKEY_AT = 64,
  // Skip uppercase letters
  RLKEY_LEFTBRACKET = 91,
  RLKEY_BACKSLASH = 92,
  RLKEY_RIGHTBRACKET = 93,
  RLKEY_CARET = 94,
  RLKEY_UNDERSCORE = 95,
  RLKEY_BACKQUOTE = 96,
  RLKEY_a = 97,
  RLKEY_b = 98,
  RLKEY_c = 99,
  RLKEY_d = 100,
  RLKEY_e = 101,
  RLKEY_f = 102,
  RLKEY_g = 103,
  RLKEY_h = 104,
  RLKEY_i = 105,
  RLKEY_j = 106,
  RLKEY_k = 107,
  RLKEY_l = 108,
  RLKEY_m = 109,
  RLKEY_n = 110,
  RLKEY_o = 111,
  RLKEY_p = 112,
  RLKEY_q = 113,
  RLKEY_r = 114,
  RLKEY_s = 115,
  RLKEY_t = 116,
  RLKEY_u = 117,
  RLKEY_v = 118,
  RLKEY_w = 119,
  RLKEY_x = 120,
  RLKEY_y = 121,
  RLKEY_z = 122,
  RLKEY_DELETE = 127,
  // End of ASCII mapped keysyms

  // International keyboard syms
  RLKEY_WORLD_0 = 160,  // 0xA0
  RLKEY_WORLD_1 = 161,
  RLKEY_WORLD_2 = 162,
  RLKEY_WORLD_3 = 163,
  RLKEY_WORLD_4 = 164,
  RLKEY_WORLD_5 = 165,
  RLKEY_WORLD_6 = 166,
  RLKEY_WORLD_7 = 167,
  RLKEY_WORLD_8 = 168,
  RLKEY_WORLD_9 = 169,
  RLKEY_WORLD_10 = 170,
  RLKEY_WORLD_11 = 171,
  RLKEY_WORLD_12 = 172,
  RLKEY_WORLD_13 = 173,
  RLKEY_WORLD_14 = 174,
  RLKEY_WORLD_15 = 175,
  RLKEY_WORLD_16 = 176,
  RLKEY_WORLD_17 = 177,
  RLKEY_WORLD_18 = 178,
  RLKEY_WORLD_19 = 179,
  RLKEY_WORLD_20 = 180,
  RLKEY_WORLD_21 = 181,
  RLKEY_WORLD_22 = 182,
  RLKEY_WORLD_23 = 183,
  RLKEY_WORLD_24 = 184,
  RLKEY_WORLD_25 = 185,
  RLKEY_WORLD_26 = 186,
  RLKEY_WORLD_27 = 187,
  RLKEY_WORLD_28 = 188,
  RLKEY_WORLD_29 = 189,
  RLKEY_WORLD_30 = 190,
  RLKEY_WORLD_31 = 191,
  RLKEY_WORLD_32 = 192,
  RLKEY_WORLD_33 = 193,
  RLKEY_WORLD_34 = 194,
  RLKEY_WORLD_35 = 195,
  RLKEY_WORLD_36 = 196,
  RLKEY_WORLD_37 = 197,
  RLKEY_WORLD_38 = 198,
  RLKEY_WORLD_39 = 199,
  RLKEY_WORLD_40 = 200,
  RLKEY_WORLD_41 = 201,
  RLKEY_WORLD_42 = 202,
  RLKEY_WORLD_43 = 203,
  RLKEY_WORLD_44 = 204,
  RLKEY_WORLD_45 = 205,
  RLKEY_WORLD_46 = 206,
  RLKEY_WORLD_47 = 207,
  RLKEY_WORLD_48 = 208,
  RLKEY_WORLD_49 = 209,
  RLKEY_WORLD_50 = 210,
  RLKEY_WORLD_51 = 211,
  RLKEY_WORLD_52 = 212,
  RLKEY_WORLD_53 = 213,
  RLKEY_WORLD_54 = 214,
  RLKEY_WORLD_55 = 215,
  RLKEY_WORLD_56 = 216,
  RLKEY_WORLD_57 = 217,
  RLKEY_WORLD_58 = 218,
  RLKEY_WORLD_59 = 219,
  RLKEY_WORLD_60 = 220,
  RLKEY_WORLD_61 = 221,
  RLKEY_WORLD_62 = 222,
  RLKEY_WORLD_63 = 223,
  RLKEY_WORLD_64 = 224,
  RLKEY_WORLD_65 = 225,
  RLKEY_WORLD_66 = 226,
  RLKEY_WORLD_67 = 227,
  RLKEY_WORLD_68 = 228,
  RLKEY_WORLD_69 = 229,
  RLKEY_WORLD_70 = 230,
  RLKEY_WORLD_71 = 231,
  RLKEY_WORLD_72 = 232,
  RLKEY_WORLD_73 = 233,
  RLKEY_WORLD_74 = 234,
  RLKEY_WORLD_75 = 235,
  RLKEY_WORLD_76 = 236,
  RLKEY_WORLD_77 = 237,
  RLKEY_WORLD_78 = 238,
  RLKEY_WORLD_79 = 239,
  RLKEY_WORLD_80 = 240,
  RLKEY_WORLD_81 = 241,
  RLKEY_WORLD_82 = 242,
  RLKEY_WORLD_83 = 243,
  RLKEY_WORLD_84 = 244,
  RLKEY_WORLD_85 = 245,
  RLKEY_WORLD_86 = 246,
  RLKEY_WORLD_87 = 247,
  RLKEY_WORLD_88 = 248,
  RLKEY_WORLD_89 = 249,
  RLKEY_WORLD_90 = 250,
  RLKEY_WORLD_91 = 251,
  RLKEY_WORLD_92 = 252,
  RLKEY_WORLD_93 = 253,
  RLKEY_WORLD_94 = 254,
  RLKEY_WORLD_95 = 255,  // 0xFF

  // Numeric keypad
  RLKEY_KP0 = 256,
  RLKEY_KP1 = 257,
  RLKEY_KP2 = 258,
  RLKEY_KP3 = 259,
  RLKEY_KP4 = 260,
  RLKEY_KP5 = 261,
  RLKEY_KP6 = 262,
  RLKEY_KP7 = 263,
  RLKEY_KP8 = 264,
  RLKEY_KP9 = 265,
  RLKEY_KP_PERIOD = 266,
  RLKEY_KP_DIVIDE = 267,
  RLKEY_KP_MULTIPLY = 268,
  RLKEY_KP_MINUS = 269,
  RLKEY_KP_PLUS = 270,
  RLKEY_KP_ENTER = 271,
  RLKEY_KP_EQUALS = 272,

  // Arrows + Home/End pad
  RLKEY_UP = 273,
  RLKEY_DOWN = 274,
  RLKEY_RIGHT = 275,
  RLKEY_LEFT = 276,
  RLKEY_INSERT = 277,
  RLKEY_HOME = 278,
  RLKEY_END = 279,
  RLKEY_PAGEUP = 280,
  RLKEY_PAGEDOWN = 281,

  // Function keys
  RLKEY_F1 = 282,
  RLKEY_F2 = 283,
  RLKEY_F3 = 284,
  RLKEY_F4 = 285,
  RLKEY_F5 = 286,
  RLKEY_F6 = 287,
  RLKEY_F7 = 288,
  RLKEY_F8 = 289,
  RLKEY_F9 = 290,
  RLKEY_F10 = 291,
  RLKEY_F11 = 292,
  RLKEY_F12 = 293,
  RLKEY_F13 = 294,
  RLKEY_F14 = 295,
  RLKEY_F15 = 296,

  // Key state modifier keys
  RLKEY_NUMLOCK = 300,
  RLKEY_CAPSLOCK = 301,
  RLKEY_SCROLLOCK = 302,
  RLKEY_RSHIFT = 303,
  RLKEY_LSHIFT = 304,
  RLKEY_RCTRL = 305,
  RLKEY_LCTRL = 306,
  RLKEY_RALT = 307,
  RLKEY_LALT = 308,
  RLKEY_RMETA = 309,
  RLKEY_LMETA = 310,
  RLKEY_LSUPER = 311,   // Left "Windows" key
  RLKEY_RSUPER = 312,   // Right "Windows" key
  RLKEY_MODE = 313,     // "Alt Gr" key
  RLKEY_COMPOSE = 314,  // Multi-key compose key

  // Miscellaneous function keys
  RLKEY_HELP = 315,
  RLKEY_PRINT = 316,
  RLKEY_SYSREQ = 317,
  RLKEY_BREAK = 318,
  RLKEY_MENU = 319,
  RLKEY_POWER = 320,  // Power Macintosh power key
  RLKEY_EURO = 321,   // Some European keyboards
  RLKEY_UNDO = 322,   // Atari keyboard has Undo
};

// Interface to receive information from the Event system when the
// mouse is moved.
class EventListener {
 public:
  virtual ~EventListener();

  // Notifies of the new location of the mouse hotspot.
  virtual void MouseMotion(const Point& new_location);

  // A notification of a mouse or key press. Returns true if this EventListener
  // handled the message (and this message shouldn't be Dispatched to other
  // EventListeners).
  virtual bool MouseButtonStateChanged(MouseButton mouse_button, bool pressed);

  // A notification that a key was pressed or unpressed. Returns true if this
  // EventListener handled the message (and this message shouldn't be
  // Dispatched to other EventListeners).
  virtual bool KeyStateChanged(KeyCode key_code, bool pressed);
};

#endif  // SRC_SYSTEMS_BASE_EVENT_LISTENER_H_
