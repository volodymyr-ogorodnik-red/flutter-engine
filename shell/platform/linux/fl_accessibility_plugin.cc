// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_accessibility_plugin.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_basic_message_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_message_codec.h"

struct _FlAccessibilityPlugin {
  GObject parent_instance;

  FlBasicMessageChannel* channel;

  // Semantics nodes keyed by ID
  GHashTable* semantics_nodes_by_id;
};

G_DEFINE_TYPE(FlAccessibilityPlugin, fl_accessibility_plugin, G_TYPE_OBJECT)

// Handles announce acessibility events from Flutter.
static FlValue* handle_announce(FlAccessibilityPlugin* self, FlValue* data) {
  FlValue* message_value = fl_value_lookup_string(data, "message");
  if (message_value == nullptr ||
      fl_value_get_type(message_value) != FL_VALUE_TYPE_STRING) {
    g_warning("Expected message string");
    return nullptr;
  }
  const gchar* message = fl_value_get_string(message_value);

  g_printerr("ANNOUNCE '%s'\n", message);

  return nullptr;
}

// Handles tap acessibility events from Flutter.
static FlValue* handle_tap(FlAccessibilityPlugin* self, int64_t node_id) {
  g_printerr("TAP '%" G_GINT64_FORMAT "'\n", node_id);

  return nullptr;
}

// Handles long press acessibility events from Flutter.
static FlValue* handle_long_press(FlAccessibilityPlugin* self,
                                  int64_t node_id) {
  g_printerr("LONG-PRESS '%" G_GINT64_FORMAT "'\n", node_id);

  return nullptr;
}

// Handles tooltip acessibility events from Flutter.
static FlValue* handle_tooltip(FlAccessibilityPlugin* self, FlValue* data) {
  FlValue* message_value = fl_value_lookup_string(data, "message");
  if (message_value == nullptr ||
      fl_value_get_type(message_value) != FL_VALUE_TYPE_STRING) {
    g_warning("Expected message string");
    return nullptr;
  }
  const gchar* message = fl_value_get_string(message_value);

  g_printerr("TOOLTIP '%s'\n", message);

  return nullptr;
}

// Handles acessibility events from Flutter.
static FlValue* handle_message(FlAccessibilityPlugin* self, FlValue* message) {
  if (fl_value_get_type(message) != FL_VALUE_TYPE_MAP) {
    g_warning("Ignoring unknown flutter/accessibility message type");
    return nullptr;
  }

  FlValue* type_value = fl_value_lookup_string(message, "type");
  if (type_value == nullptr ||
      fl_value_get_type(type_value) != FL_VALUE_TYPE_STRING) {
    g_warning(
        "Ignoring unknown flutter/accessibility message with unknown type");
    return nullptr;
  }
  const gchar* type = fl_value_get_string(type_value);
  FlValue* data = fl_value_lookup_string(message, "data");
  FlValue* node_id_value = fl_value_lookup_string(message, "nodeId");
  int64_t node_id = -1;
  if (node_id_value != nullptr &&
      fl_value_get_type(node_id_value) == FL_VALUE_TYPE_INT)
    node_id = fl_value_get_int(node_id_value);

  if (strcmp(type, "announce") == 0) {
    return handle_announce(self, data);
  } else if (strcmp(type, "tap") == 0) {
    return handle_tap(self, node_id);
  } else if (strcmp(type, "longPress") == 0) {
    return handle_long_press(self, node_id);
  } else if (strcmp(type, "tooltip") == 0) {
    return handle_tooltip(self, data);
  } else {
    g_debug("Got unknown accessibility message: %s", type);
    return nullptr;
  }
}

// Called when a message is received on this channel
static void message_cb(FlBasicMessageChannel* channel,
                       FlValue* message,
                       FlBasicMessageChannelResponseHandle* response_handle,
                       gpointer user_data) {
  FlAccessibilityPlugin* self = FL_ACCESSIBILITY_PLUGIN(user_data);

  g_autoptr(FlValue) response = handle_message(self, message);

  g_autoptr(GError) error = nullptr;
  if (!fl_basic_message_channel_respond(channel, response_handle, response,
                                        &error))
    g_warning("Failed to send message response: %s", error->message);
}

static void fl_accessibility_plugin_dispose(GObject* object) {
  FlAccessibilityPlugin* self = FL_ACCESSIBILITY_PLUGIN(object);

  g_clear_object(&self->channel);

  G_OBJECT_CLASS(fl_accessibility_plugin_parent_class)->dispose(object);
}

static void fl_accessibility_plugin_class_init(
    FlAccessibilityPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_accessibility_plugin_dispose;
}

static void fl_accessibility_plugin_init(FlAccessibilityPlugin* self) {
  self->semantics_nodes_by_id =
      g_hash_table_new_full(g_int_hash, g_int_equal, nullptr, nullptr);
}

FlAccessibilityPlugin* fl_accessibility_plugin_new(
    FlBinaryMessenger* messenger) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlAccessibilityPlugin* self = FL_ACCESSIBILITY_PLUGIN(
      g_object_new(fl_accessibility_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMessageCodec) codec = fl_standard_message_codec_new();
  self->channel = fl_basic_message_channel_new(
      messenger, "flutter/accessibility", FL_MESSAGE_CODEC(codec));
  fl_basic_message_channel_set_message_handler(self->channel, message_cb, self,
                                               nullptr);

  return self;
}

void fl_accessibility_plugin_handle_update_semantics_node(
    FlAccessibilityPlugin* plugin,
    const FlutterSemanticsNode* node) {
  if (node->id == kFlutterSemanticsCustomActionIdBatchEnd) {
    g_printerr("Semantics Nodes End\n");
    return;
  }

  g_printerr("Semantics Node\n");
  g_printerr("  id: %d\n", node->id);
  if (node->label[0] != '\0')
    g_printerr("  label: %s\n", node->label);
  if (node->hint[0] != '\0')
    g_printerr("  hint: %s\n", node->hint);
  if (node->value[0] != '\0')
    g_printerr("  value: %s\n", node->value);
  if (node->increased_value[0] != '\0')
    g_printerr("  increased_value: %s\n", node->increased_value);
  if (node->decreased_value[0] != '\0')
    g_printerr("  decreased_value: %s\n", node->decreased_value);
  g_printerr("  rect: %f %f %f %f (lrtb)\n", node->rect.left, node->rect.right,
             node->rect.top, node->rect.bottom);
  g_printerr("  transform:");
  if (node->transform.transX != 0 || node->transform.transY != 0)
    g_printerr(" translate(%f %f)", node->transform.transX,
               node->transform.transY);
  if (node->transform.scaleX != 1 || node->transform.scaleY != 1)
    g_printerr(" scale(%f %f)", node->transform.scaleX, node->transform.scaleY);
  if (node->transform.skewX != 0 || node->transform.skewY != 0)
    g_printerr(" skew(%f %f)", node->transform.skewX, node->transform.skewY);
  if (node->transform.pers0 != 0 || node->transform.pers1 != 0 ||
      node->transform.pers2 != 1)
    g_printerr(" perspective(%f %f %f)", node->transform.pers0,
               node->transform.pers1, node->transform.pers2);
  g_printerr("\n");
  if (node->child_count > 0) {
    g_printerr("  children_in_traversal_order:");
    for (size_t i = 0; i < node->child_count; i++)
      g_printerr(" %d", node->children_in_traversal_order[i]);
    g_printerr("\n");
    g_printerr("  children_in_hit_test_order:");
    for (size_t i = 0; i < node->child_count; i++)
      g_printerr(" %d", node->children_in_hit_test_order[i]);
    g_printerr("\n");
  }
  if (node->custom_accessibility_actions_count > 0) {
    g_printerr("  custom_accessibility_actions:");
    for (size_t i = 0; i < node->custom_accessibility_actions_count; i++)
      g_printerr(" %d", node->custom_accessibility_actions[i]);
    g_printerr("\n");
  }
}
