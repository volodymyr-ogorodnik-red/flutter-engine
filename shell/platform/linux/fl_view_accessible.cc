// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_view_accessible.h"

struct _FlViewAccessible {
  GtkWidgetAccessible parent_instance;
};

G_DEFINE_TYPE(FlViewAccessible, fl_view_accessible, GTK_TYPE_WIDGET_ACCESSIBLE)

static void fl_view_accessible_class_init(FlViewAccessibleClass* klass) {}

static void fl_view_accessible_init(FlViewAccessible* self) {}
