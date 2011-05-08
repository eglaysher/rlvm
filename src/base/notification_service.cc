// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/notification_service.h"

#include <cstdio>

#include "base/notification_observer.h"

// static
NotificationService* NotificationService::current() {
  static NotificationService service;
  return &service;
}

// static
bool NotificationService::HasKey(const NotificationSourceMap& map,
                                 const NotificationSource& source) {
  return map.find(source.map_key()) != map.end();
}

NotificationService::NotificationService() {
}

void NotificationService::AddObserver(NotificationObserver* observer,
                                      NotificationType type,
                                      const NotificationSource& source) {
  NotificationObserverList* observer_list;
  if (HasKey(observers_[type.value], source)) {
    observer_list = observers_[type.value][source.map_key()];
  } else {
    observer_list = new NotificationObserverList;
    observers_[type.value][source.map_key()] = observer_list;
  }

  observer_list->AddObserver(observer);
}

void NotificationService::RemoveObserver(NotificationObserver* observer,
                                         NotificationType type,
                                         const NotificationSource& source) {
  NotificationObserverList* observer_list =
      observers_[type.value][source.map_key()];
  if (observer_list) {
    observer_list->RemoveObserver(observer);
  }

  // TODO(jhughes): Remove observer list from map if empty?
}

void NotificationService::Notify(NotificationType type,
                                 const NotificationSource& source,
                                 const NotificationDetails& details) {
  // There's no particular reason for the order in which the different
  // classes of observers get notified here.

  // Notify observers of all types and all sources
  if (HasKey(observers_[NotificationType::ALL], AllSources()) &&
      source != AllSources()) {
    FOR_EACH_OBSERVER(NotificationObserver,
       *observers_[NotificationType::ALL][AllSources().map_key()],
       Observe(type, source, details));
  }

  // Notify observers of all types and the given source
  if (HasKey(observers_[NotificationType::ALL], source)) {
    FOR_EACH_OBSERVER(NotificationObserver,
        *observers_[NotificationType::ALL][source.map_key()],
        Observe(type, source, details));
  }

  // Notify observers of the given type and all sources
  if (HasKey(observers_[type.value], AllSources()) &&
      source != AllSources()) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[type.value][AllSources().map_key()],
                      Observe(type, source, details));
  }

  // Notify observers of the given type and the given source
  if (HasKey(observers_[type.value], source)) {
    FOR_EACH_OBSERVER(NotificationObserver,
                      *observers_[type.value][source.map_key()],
                      Observe(type, source, details));
  }
}


NotificationService::~NotificationService() {
  for (int i = 0; i < NotificationType::NOTIFICATION_TYPE_COUNT; i++) {
    NotificationSourceMap omap = observers_[i];
    for (NotificationSourceMap::iterator it = omap.begin();
         it != omap.end(); ++it)
      delete it->second;
  }
}

NotificationObserver::NotificationObserver() {}

NotificationObserver::~NotificationObserver() {}
