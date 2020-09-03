// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_accessible.h"

struct _FlAccessible {
  AtkObject parent_instance;

  AtkObject* parent;
  int32_t id;
  gchar* name;
  gint x, y, width, height;
  GPtrArray* children;
};

static void fl_accessible_component_interface_init(AtkComponentIface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlAccessible,
    fl_accessible,
    ATK_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(ATK_TYPE_COMPONENT,
                          fl_accessible_component_interface_init))

static void fl_accessible_dispose(GObject* object) {
  FlAccessible* self = FL_ACCESSIBLE(object);

  g_clear_pointer(&self->name, g_free);
  g_clear_pointer(&self->children, g_ptr_array_unref);

  G_OBJECT_CLASS(fl_accessible_parent_class)->dispose(object);
}

// Implements AtkObject::get_name.
static const gchar* fl_accessible_get_name(AtkObject* accessible) {
  FlAccessible* self = FL_ACCESSIBLE(accessible);
  return self->name;
}

// Implements AtkObject::get_parent.
static AtkObject* fl_accessible_get_parent(AtkObject* accessible) {
  FlAccessible* self = FL_ACCESSIBLE(accessible);
  return self->parent;
}

// Implements AtkObject::get_n_children.
static gint fl_accessible_get_n_children(AtkObject* accessible) {
  FlAccessible* self = FL_ACCESSIBLE(accessible);
  return self->children->len;
}

// Implements AtkObject::ref_child.
static AtkObject* fl_accessible_ref_child(AtkObject* accessible, gint i) {
  FlAccessible* self = FL_ACCESSIBLE(accessible);

  if (i < 0 || static_cast<guint>(i) >= self->children->len) {
    return nullptr;
  }

  return reinterpret_cast<AtkObject*>(
      g_object_ref(g_ptr_array_index(self->children, i)));
}

// Implements AtkObject::get_role.
static AtkRole fl_accessible_get_role(AtkObject* accessible) {
  return ATK_ROLE_FRAME;
}

// Implements AtkComponent::get_extents.
static void fl_accessible_get_extents(AtkComponent* component,
                                      gint* x,
                                      gint* y,
                                      gint* width,
                                      gint* height,
                                      AtkCoordType coord_type) {
  FlAccessible* self = FL_ACCESSIBLE(component);

  *x = 0;
  *y = 0;
  if (self->parent != nullptr) {
    atk_component_get_extents(ATK_COMPONENT(self->parent), x, y, NULL, NULL,
                              coord_type);
  }

  *x += self->x;
  *y += self->y;
  *width = self->width;
  *height = self->height;
}

// Implements AtkComponent::get_layer.
static AtkLayer fl_accessible_get_layer(AtkComponent* component) {
  return ATK_LAYER_WIDGET;
}

static void fl_accessible_class_init(FlAccessibleClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_accessible_dispose;
  ATK_OBJECT_CLASS(klass)->get_name = fl_accessible_get_name;
  ATK_OBJECT_CLASS(klass)->get_parent = fl_accessible_get_parent;
  ATK_OBJECT_CLASS(klass)->get_n_children = fl_accessible_get_n_children;
  ATK_OBJECT_CLASS(klass)->ref_child = fl_accessible_ref_child;
  ATK_OBJECT_CLASS(klass)->get_role = fl_accessible_get_role;
}

static void fl_accessible_component_interface_init(AtkComponentIface* iface) {
  iface->get_extents = fl_accessible_get_extents;
  iface->get_layer = fl_accessible_get_layer;
}

static void fl_accessible_init(FlAccessible* self) {
  self->children = g_ptr_array_new_with_free_func(g_object_unref);
}

FlAccessible* fl_accessible_new(int32_t id) {
  FlAccessible* self = static_cast<FlAccessible*>(
      g_object_new(fl_accessible_get_type(), nullptr));
  self->id = id;
  return self;
}

void fl_accessible_set_parent(FlAccessible* self, AtkObject* parent) {
  g_return_if_fail(FL_IS_ATK_OBJECT(self));
  self->parent = parent;  // FIXME: Weak ref?
}

void fl_accessible_set_name(FlAccessible* self, const gchar* name) {
  g_return_if_fail(FL_IS_ATK_OBJECT(self));
  g_free(self->name);
  self->name = g_strdup(name);
}

void fl_accessible_set_extents(FlAccessible* self,
                               gint x,
                               gint y,
                               gint width,
                               gint height) {
  g_return_if_fail(FL_IS_ATK_OBJECT(self));
  self->x = x;
  self->y = y;
  self->width = width;
  self->height = height;
}

void fl_accessible_set_children(FlAccessible* self, GPtrArray* children) {
  g_return_if_fail(FL_IS_ATK_OBJECT(self));

  // FIXME: Do the delta
  g_ptr_array_remove_range(self->children, 0, self->children->len);
  for (guint i = 0; i < children->len; i++) {
    AtkObject* object =
        reinterpret_cast<AtkObject*>(g_ptr_array_index(children, i));
    g_ptr_array_add(self->children, g_object_ref(object));
    g_signal_emit_by_name(self, "children-changed::add", i, object, nullptr);
  }
}
