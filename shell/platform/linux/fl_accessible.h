// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_H_

#include <gtk/gtk.h>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

G_BEGIN_DECLS

#define FL_ACCESSIBLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), fl_accessible_get_type(), FlAccessible))
#define FL_IS_ATK_OBJECT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), fl_accessible_get_type()))

typedef struct _FlAccessible FlAccessible;
typedef struct _FlAccessibleClass FlAccessibleClass;

struct _FlAccessibleClass {
  AtkObjectClass parent_instance;
};

/**
 * FlAccessible:
 *
 * #FlAccessible is an object that exposes Flutter accessibility information to
 * ATK.
 */

/**
 * fl_accessible_new:
 * @id: FIXME
 *
 * Creates a new accessibility object that exposes Flutter accessibility
 * information to ATK.
 *
 * Returns: a new #FlAccessible.
 */
FlAccessible* fl_accessible_new(int32_t id);

void fl_accessible_set_parent(FlAccessible* accessible, AtkObject* parent);

void fl_accessible_set_name(FlAccessible* accessible, const gchar* name);

void fl_accessible_set_extents(FlAccessible* self,
                               gint x,
                               gint y,
                               gint width,
                               gint height);

void fl_accessible_set_children(FlAccessible* accessible, GPtrArray* children);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBLE_H_
