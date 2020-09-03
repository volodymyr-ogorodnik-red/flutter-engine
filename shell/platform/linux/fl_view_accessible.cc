// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_view_accessible.h"
#include "flutter/shell/platform/linux/fl_accessible.h"

struct _FlViewAccessible {
  GtkWidgetAccessible parent_instance;

  // Semantics nodes keyed by ID
  GHashTable* semantics_nodes_by_id;
};

G_DEFINE_TYPE(FlViewAccessible, fl_view_accessible, GTK_TYPE_WIDGET_ACCESSIBLE)

static FlAccessible* get_node(FlViewAccessible* self, int32_t id) {
  FlAccessible* accessible = reinterpret_cast<FlAccessible*>(
      g_hash_table_lookup(self->semantics_nodes_by_id, GINT_TO_POINTER(id)));
  if (accessible == nullptr) {
    accessible = fl_accessible_new(id);
    g_hash_table_insert(self->semantics_nodes_by_id, GINT_TO_POINTER(id),
                        reinterpret_cast<gpointer>(accessible));
  }
  return accessible;
}

static gboolean has_flag(FlutterSemanticsFlag* flags,
                         FlutterSemanticsFlag flag) {
  if ((*flags & flag) == 0)
    return FALSE;

  *flags = static_cast<FlutterSemanticsFlag>(*flags ^ flag);
  return TRUE;
}

static gboolean has_action(FlutterSemanticsAction* actions,
                           FlutterSemanticsAction action) {
  if ((*actions & action) == 0)
    return FALSE;

  *actions = static_cast<FlutterSemanticsAction>(*actions ^ action);
  return TRUE;
}

// Implements AtkObject::get_n_children
static gint fl_view_accessible_get_n_children(AtkObject* accessible) {
  return 1;
}

// Implements AtkObject::ref_child
static AtkObject* fl_view_accessible_ref_child(AtkObject* accessible, gint i) {
  FlViewAccessible* self = FL_VIEW_ACCESSIBLE(accessible);

  if (i != 0) {
    return nullptr;
  }

  FlAccessible* node = get_node(self, 0);
  return reinterpret_cast<AtkObject*>(g_object_ref(node));
}

// Implements AtkObject::get_role
static AtkRole fl_view_accessible_get_role(AtkObject* accessible) {
  return ATK_ROLE_FRAME;
}

static void fl_view_accessible_class_init(FlViewAccessibleClass* klass) {
  ATK_OBJECT_CLASS(klass)->get_n_children = fl_view_accessible_get_n_children;
  ATK_OBJECT_CLASS(klass)->ref_child = fl_view_accessible_ref_child;
  ATK_OBJECT_CLASS(klass)->get_role = fl_view_accessible_get_role;
}

static void fl_view_accessible_init(FlViewAccessible* self) {
  self->semantics_nodes_by_id = g_hash_table_new_full(
      g_direct_hash, g_direct_equal, nullptr, g_object_unref);
}

void fl_view_accessible_handle_update_semantics_node(
    FlViewAccessible* self,
    const FlutterSemanticsNode* node) {
  if (node->id == kFlutterSemanticsCustomActionIdBatchEnd) {
    return;
  }

  FlAccessible* accessible = get_node(self, node->id);

  fl_accessible_set_name(accessible, node->label);
  fl_accessible_set_extents(
      accessible, node->rect.left + node->transform.transX,
      node->rect.top + node->transform.transY,
      node->rect.right - node->rect.left, node->rect.bottom - node->rect.top);

  g_autoptr(GPtrArray) children = g_ptr_array_new();
  for (size_t i = 0; i < node->child_count; i++) {
    g_ptr_array_add(children,
                    get_node(self, node->children_in_traversal_order[i]));
  }
  fl_accessible_set_children(accessible, children);

  return;
  g_printerr("Semantics Node\n");
  g_printerr("  id: %d\n", node->id);
  if (node->flags != 0) {
    g_printerr("  flags:");
    FlutterSemanticsFlag flags = node->flags;
    if (has_flag(&flags, kFlutterSemanticsFlagHasCheckedState))
      g_printerr(" HasCheckedState");
    if (has_flag(&flags, kFlutterSemanticsFlagIsChecked))
      g_printerr(" IsChecked");
    if (has_flag(&flags, kFlutterSemanticsFlagIsSelected))
      g_printerr(" IsSelected");
    if (has_flag(&flags, kFlutterSemanticsFlagIsButton))
      g_printerr(" IsButton");
    if (has_flag(&flags, kFlutterSemanticsFlagIsTextField))
      g_printerr(" IsTextField");
    if (has_flag(&flags, kFlutterSemanticsFlagIsFocused))
      g_printerr(" IsFocused");
    if (has_flag(&flags, kFlutterSemanticsFlagHasEnabledState))
      g_printerr(" HasEnabledState");
    if (has_flag(&flags, kFlutterSemanticsFlagIsEnabled))
      g_printerr(" IsEnabled");
    if (has_flag(&flags, kFlutterSemanticsFlagIsInMutuallyExclusiveGroup))
      g_printerr(" IsInMutuallyExclusiveGroup");
    if (has_flag(&flags, kFlutterSemanticsFlagIsHeader))
      g_printerr(" IsHeader");
    if (has_flag(&flags, kFlutterSemanticsFlagIsObscured))
      g_printerr(" IsObscured");
    if (has_flag(&flags, kFlutterSemanticsFlagScopesRoute))
      g_printerr(" ScopesRoute");
    if (has_flag(&flags, kFlutterSemanticsFlagNamesRoute))
      g_printerr(" NamesRoute");
    if (has_flag(&flags, kFlutterSemanticsFlagIsHidden))
      g_printerr(" IsHidden");
    if (has_flag(&flags, kFlutterSemanticsFlagIsImage))
      g_printerr(" IsImage");
    if (has_flag(&flags, kFlutterSemanticsFlagIsLiveRegion))
      g_printerr(" IsLiveRegion");
    if (has_flag(&flags, kFlutterSemanticsFlagHasToggledState))
      g_printerr(" HasToggledState");
    if (has_flag(&flags, kFlutterSemanticsFlagIsToggled))
      g_printerr(" IsToggled");
    if (has_flag(&flags, kFlutterSemanticsFlagHasImplicitScrolling))
      g_printerr(" HasImplicitScrolling");
    if (has_flag(&flags, kFlutterSemanticsFlagIsReadOnly))
      g_printerr(" IsReadOnly");
    if (has_flag(&flags, kFlutterSemanticsFlagIsFocusable))
      g_printerr(" IsFocusable");
    if (has_flag(&flags, kFlutterSemanticsFlagIsLink))
      g_printerr(" IsLink");
    if (flags != 0)
      g_printerr(" 0x%x", flags);
    g_printerr("\n");
  }
  if (node->actions != 0) {
    g_printerr("  actions:");
    FlutterSemanticsAction actions = node->actions;
    if (has_action(&actions, kFlutterSemanticsActionTap))
      g_printerr(" Tap");
    if (has_action(&actions, kFlutterSemanticsActionLongPress))
      g_printerr(" LongPress");
    if (has_action(&actions, kFlutterSemanticsActionScrollLeft))
      g_printerr(" ScrollLeft");
    if (has_action(&actions, kFlutterSemanticsActionScrollRight))
      g_printerr(" ScrollRight");
    if (has_action(&actions, kFlutterSemanticsActionScrollUp))
      g_printerr(" ScrollUp");
    if (has_action(&actions, kFlutterSemanticsActionScrollDown))
      g_printerr(" ScrollDown");
    if (has_action(&actions, kFlutterSemanticsActionIncrease))
      g_printerr(" Increase");
    if (has_action(&actions, kFlutterSemanticsActionDecrease))
      g_printerr(" Decrease");
    if (has_action(&actions, kFlutterSemanticsActionShowOnScreen))
      g_printerr(" ShowOnScreen");
    if (has_action(&actions,
                   kFlutterSemanticsActionMoveCursorForwardByCharacter))
      g_printerr(" MoveCursorForwardByCharacter");
    if (has_action(&actions,
                   kFlutterSemanticsActionMoveCursorBackwardByCharacter))
      g_printerr(" MoveCursorBackwardByCharacter");
    if (has_action(&actions, kFlutterSemanticsActionSetSelection))
      g_printerr(" SetSelection");
    if (has_action(&actions, kFlutterSemanticsActionCopy))
      g_printerr(" Copy");
    if (has_action(&actions, kFlutterSemanticsActionCut))
      g_printerr(" Cut");
    if (has_action(&actions, kFlutterSemanticsActionPaste))
      g_printerr(" Paste");
    if (has_action(&actions, kFlutterSemanticsActionDidGainAccessibilityFocus))
      g_printerr("DidGainAccessibilityFocus");
    if (has_action(&actions, kFlutterSemanticsActionDidLoseAccessibilityFocus))
      g_printerr("DidLoseAccessibilityFocus");
    if (has_action(&actions, kFlutterSemanticsActionCustomAction))
      g_printerr(" CustomAction");
    if (has_action(&actions, kFlutterSemanticsActionDismiss))
      g_printerr("Dismiss");
    if (has_action(&actions, kFlutterSemanticsActionMoveCursorForwardByWord))
      g_printerr("MoveCursorForwardByWord");
    if (has_action(&actions, kFlutterSemanticsActionMoveCursorBackwardByWord))
      g_printerr("MoveCursorBackwardByWord");
    if (actions != 0)
      g_printerr(" 0x%x", actions);
    g_printerr("\n");
  }
  if (node->text_selection_base != -1)
    g_printerr("  text_selection_base: %d\n", node->text_selection_base);
  if (node->text_selection_extent != -1)
    g_printerr("  text_selection_extent: %d\n", node->text_selection_extent);
  if (node->scroll_child_count != 0)
    g_printerr("  scroll_child_count: %d\n", node->scroll_child_count);
  if (node->scroll_index != 0)
    g_printerr("  scroll_index: %d\n", node->scroll_index);
  if (!isnan(node->scroll_position))
    g_printerr("  scroll_position: %g\n", node->scroll_position);
  if (!isnan(node->scroll_extent_max))
    g_printerr("  scroll_extent_max: %g\n", node->scroll_extent_max);
  if (!isnan(node->scroll_extent_min))
    g_printerr("  scroll_extent_min: %g\n", node->scroll_extent_min);
  if (node->elevation != 0)
    g_printerr("  elevation: %g\n", node->elevation);
  if (node->thickness != 0)
    g_printerr("  thickness: %g\n", node->thickness);
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
  if (node->text_direction != kFlutterTextDirectionUnknown) {
    g_printerr("  text_direction: ");
    switch (node->text_direction) {
      case kFlutterTextDirectionRTL:
        g_printerr("RTL");
        break;
      case kFlutterTextDirectionLTR:
        g_printerr("LTR");
        break;
      default:
        g_printerr("%d\n", node->text_direction);
        break;
    }
    g_printerr("\n");
  }
  g_printerr("  rect: %g %g %g %g (lrtb)\n", node->rect.left, node->rect.right,
             node->rect.top, node->rect.bottom);
  if (node->transform.transX != 0 || node->transform.transY != 0 ||
      node->transform.scaleX != 1 || node->transform.scaleY != 1 ||
      node->transform.skewX != 0 || node->transform.skewY != 0 ||
      node->transform.pers0 != 0 || node->transform.pers1 != 0 ||
      node->transform.pers2 != 1) {
    g_printerr("  transform:");
    if (node->transform.transX != 0 || node->transform.transY != 0)
      g_printerr(" translate(%g, %g)", node->transform.transX,
                 node->transform.transY);
    if (node->transform.scaleX != 1 || node->transform.scaleY != 1)
      g_printerr(" scale(%g, %g)", node->transform.scaleX,
                 node->transform.scaleY);
    if (node->transform.skewX != 0 || node->transform.skewY != 0)
      g_printerr(" skew(%g, %g)", node->transform.skewX, node->transform.skewY);
    if (node->transform.pers0 != 0 || node->transform.pers1 != 0 ||
        node->transform.pers2 != 1)
      g_printerr("  perspective(%g, %g, %g) ", node->transform.pers0,
                 node->transform.pers1, node->transform.pers2);
    g_printerr("\n");
  }
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
  if (node->platform_view_id != -1)
    g_printerr("  platform_view_id: %" G_GINT64_FORMAT "\n",
               node->platform_view_id);
}
