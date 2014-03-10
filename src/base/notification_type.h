// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_BASE_NOTIFICATION_TYPE_H_
#define SRC_BASE_NOTIFICATION_TYPE_H_

// A list of all possible notifications.
//
// This is the only file in the notification system to significantly change for
// rlvm, due to a disjoint set of notifications.
class NotificationType {
 public:
  enum Type {
    // General -----------------------------------------------------------------

    // Special signal value to represent an interest in all notifications.
    // Not valid when posting a notification.
    ALL = 0,

    // Copied notification types from chromium. Used only in the Notification*
    // unit tests.
    IDLE,
    BUSY,

    // Signals that the screen mode changed (between fullscreen/windowed
    // mode). The source is a GraphicsSystem and the details is a boolean of
    // the new value.
    FULLSCREEN_STATE_CHANGED,

    // Sent whenever we change whether we're skipping text. The TextSystem is
    // the source, the details are an int of the new state.
    SKIP_MODE_STATE_CHANGED,

    // Whether the user can turn on skip mode. The TextSystem is the source,
    // the details are an int of the new state.
    SKIP_MODE_ENABLED_CHANGED,

    // Sent whenever automode is turned on/off. The TextSystem is the source,
    // the details are an int of the new state.
    AUTO_MODE_STATE_CHANGED,

    // Count (must be last) ----------------------------------------------------
    // Used to determine the number of notification types.  Not valid as
    // a type parameter when registering for or posting notifications.
    NOTIFICATION_TYPE_COUNT
  };

  // TODO(erg): Our notification system relies on implicit conversion.
  NotificationType(Type v) : value(v) {}  // NOLINT

  bool operator==(NotificationType t) const { return value == t.value; }
  bool operator!=(NotificationType t) const { return value != t.value; }

  // Comparison to explicit enum values.
  bool operator==(Type v) const { return value == v; }
  bool operator!=(Type v) const { return value != v; }

  Type value;
};

inline bool operator==(NotificationType::Type a, NotificationType b) {
  return a == b.value;
}
inline bool operator!=(NotificationType::Type a, NotificationType b) {
  return a != b.value;
}

#endif  // SRC_BASE_NOTIFICATION_TYPE_H_
