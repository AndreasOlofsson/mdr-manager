/*
 * MDR Manager
 * 
 *  Copyright (C) 2021 Andreas Olofsson
 * 
 * 
 * This file is part of MDR Manager.
 * 
 * MDR Manager is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * MDR MAnager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with mdrd. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "device_view.h"
#include "heading.h"

#include "mdr_device.h"

struct _MDRDeviceView
{
    GtkScrolledWindow parent;
};

typedef struct _MDRDeviceViewPrivate MDRDeviceViewPrivate;

struct _MDRDeviceViewPrivate
{
    GtkButton*       power_off_button;

    GtkBox*          battery_view;
    GtkLevelBar*     battery_bar;
    GtkLabel*        battery_label;

    GtkBox*          lr_battery_view;
    GtkLevelBar*     left_battery_bar;
    GtkLabel*        left_battery_label;
    GtkLevelBar*     right_battery_bar;
    GtkLabel*        right_battery_label;

    GtkBox*          cradle_battery_view;
    GtkLevelBar*     cradle_battery_bar;
    GtkLabel*        cradle_battery_label;

    GtkBox*          nc_view;
    GtkSwitch*       nc_switch;

    GtkBox*          asm_view;
    GtkScale*        asm_amount;

    GtkBox*          eq_view;
    GtkComboBoxText* eq_preset;
    GtkBox*          eq_container;

    GtkBox*          auto_power_off_view;
    GtkComboBoxText* auto_power_off_timeout;

    GtkBox*          key_functions_view;
    GtkBox*          key_functions;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRDeviceView,
                           mdr_device_view,
                           GTK_TYPE_SCROLLED_WINDOW)

static void mdr_device_view_init(MDRDeviceView* view)
{
    gtk_widget_init_template(GTK_WIDGET(view));
}

#define begin_iface_init(name, type) \
    static void init_##name(MDRDeviceView* view, \
                               type* name); \
    \
    static void iface_##name##_changed(MDRDevice* device, \
                                       GParamSpec* param_spec, \
                                       MDRDeviceView* view) \
    { \
        GValue value = { 0 }; \
        g_object_get_property(G_OBJECT(device), #name, &value); \
        type* name = g_value_get_object(&value); \
        \
        if (name != NULL) \
        { \
            init_##name(view, name); \
        } \
    } \
    \
    static void init_##name(MDRDeviceView* view, \
                               type* name) \
    { \
        MDRDeviceViewPrivate* private \
                = mdr_device_view_get_instance_private(view); \

#define end_iface_init }

static void power_off_clicked(GtkWidget* button, OrgMdrPowerOff* power_off);

begin_iface_init(power_off, OrgMdrPowerOff)
    gtk_widget_set_visible(GTK_WIDGET(private->power_off_button), TRUE);

    g_signal_connect(private->power_off_button,
                     "clicked",
                     G_CALLBACK(power_off_clicked),
                     power_off);
end_iface_init

static void power_off_clicked(GtkWidget* button, OrgMdrPowerOff* power_off)
{
    org_mdr_power_off_call_power_off(
            power_off,
            NULL,
            NULL,
            NULL);
}

begin_iface_init(battery, OrgMdrBattery)
    g_message("init battery");
    gtk_widget_set_visible(GTK_WIDGET(private->battery_view), TRUE);
    g_object_bind_property(
            battery,
            "level",
            private->battery_bar,
            "value",
            G_BINDING_SYNC_CREATE);
    g_object_bind_property(
            battery,
            "level",
            private->battery_label,
            "label",
            G_BINDING_SYNC_CREATE);
end_iface_init

begin_iface_init(left_right_battery, OrgMdrLeftRightBattery)
    gtk_widget_set_visible(GTK_WIDGET(private->lr_battery_view), TRUE);
    g_object_bind_property(
            left_right_battery,
            "left_level",
            private->left_battery_bar,
            "value",
            G_BINDING_SYNC_CREATE);
    g_object_bind_property(
            left_right_battery,
            "left_level",
            private->left_battery_label,
            "label",
            G_BINDING_SYNC_CREATE);
    g_object_bind_property(
            left_right_battery,
            "right_level",
            private->right_battery_bar,
            "value",
            G_BINDING_SYNC_CREATE);
    g_object_bind_property(
            left_right_battery,
            "right_level",
            private->right_battery_label,
            "label",
            G_BINDING_SYNC_CREATE);
end_iface_init

begin_iface_init(cradle_battery, OrgMdrCradleBattery)
    gtk_widget_set_visible(GTK_WIDGET(private->cradle_battery_view), TRUE);
    g_object_bind_property(
            cradle_battery,
            "level",
            private->cradle_battery_bar,
            "value",
            G_BINDING_SYNC_CREATE);
    g_object_bind_property(
            cradle_battery,
            "level",
            private->cradle_battery_label,
            "label",
            G_BINDING_SYNC_CREATE);
end_iface_init

static gboolean nc_toggled(GtkSwitch* nc_switch,
                           gboolean state,
                           OrgMdrNoiseCancelling* nc_iface);

begin_iface_init(noise_cancelling, OrgMdrNoiseCancelling)
    gtk_widget_set_visible(GTK_WIDGET(private->nc_view), TRUE);
    g_object_bind_property(
            noise_cancelling,
            "enabled",
            private->nc_switch,
            "active",
            G_BINDING_SYNC_CREATE);

    g_signal_connect_object(
            private->nc_switch,
            "state-set",
            G_CALLBACK(nc_toggled),
            noise_cancelling,
            0);
end_iface_init

static void nc_toggle_on_finish(GObject* source_object,
                                GAsyncResult* res,
                                gpointer user_data)
{
    GtkSwitch* nc_switch = user_data;

    if (org_mdr_noise_cancelling_call_enable_finish(
            ORG_MDR_NOISE_CANCELLING(source_object),
            res,
            NULL))
        gtk_switch_set_state(nc_switch, TRUE);

    g_object_unref(nc_switch);
}

static void nc_toggle_off_finish(GObject* source_object,
                                 GAsyncResult* res,
                                 gpointer user_data)
{
    GtkSwitch* nc_switch = user_data;

    if (org_mdr_noise_cancelling_call_disable_finish(
            ORG_MDR_NOISE_CANCELLING(source_object),
            res,
            NULL))
        gtk_switch_set_state(nc_switch, FALSE);

    g_object_unref(nc_switch);
}

static gboolean nc_toggled(GtkSwitch* nc_switch,
                           gboolean state,
                           OrgMdrNoiseCancelling* nc_iface)
{
    if (state == org_mdr_noise_cancelling_get_enabled(nc_iface))
        return FALSE;

    g_object_ref(nc_switch);

    if (state)
        org_mdr_noise_cancelling_call_enable(nc_iface,
                                             NULL,
                                             nc_toggle_on_finish,
                                             nc_switch);
    else
        org_mdr_noise_cancelling_call_disable(nc_iface,
                                              NULL,
                                              nc_toggle_off_finish,
                                              nc_switch);

    return TRUE;
}

static void asm_set_amount_finish(GObject* source_object,
                                  GAsyncResult* res,
                                  gpointer user_data);

static gboolean asm_set_amount(GtkRange* range,
                               GtkScrollType* scroll,
                               gdouble value,
                               gpointer user_data)
{
    OrgMdrAmbientSoundMode* asm_iface = user_data;

    if (value < 0) value = 0;
    if (value > G_MAXUINT32) value = G_MAXUINT32 - 1;

    guint target_amount = (guint) (value + 0.5);

    if (target_amount > 20) target_amount = 20;

    if (target_amount != org_mdr_ambient_sound_mode_get_amount(asm_iface))
    {
        org_mdr_ambient_sound_mode_call_set_amount(asm_iface,
                                                   target_amount,
                                                   NULL,
                                                   asm_set_amount_finish,
                                                   range);
    }

    return FALSE;
}

static void asm_set_amount_finish(GObject* source_object,
                                  GAsyncResult* res,
                                  gpointer user_data)
{
    OrgMdrAmbientSoundMode* asm_iface
            = ORG_MDR_AMBIENT_SOUND_MODE(source_object);
    GtkRange* range = user_data;

    GError* error = NULL;
    if (!org_mdr_ambient_sound_mode_call_set_amount_finish(asm_iface,
                                                           res,
                                                           &error))
    {
        g_error("Failed to set ASM amount: %s", error->message);

        gtk_range_set_value(range,
                            org_mdr_ambient_sound_mode_get_amount(asm_iface));
    }
}

begin_iface_init(ambient_sound_mode, OrgMdrAmbientSoundMode)
    gtk_widget_set_visible(GTK_WIDGET(private->asm_view), TRUE);

    g_object_bind_property(
            ambient_sound_mode,
            "amount",
            gtk_range_get_adjustment(GTK_RANGE(private->asm_amount)),
            "value",
            G_BINDING_SYNC_CREATE);

    g_signal_connect_object(
            private->asm_amount,
            "change-value",
            G_CALLBACK(asm_set_amount),
            ambient_sound_mode,
            0);
end_iface_init

static void eq_preset_changed_finish(GObject* source_object,
                                     GAsyncResult* res,
                                     gpointer user_data)
{
    OrgMdrEq* eq = ORG_MDR_EQ(source_object);
    GtkComboBox* preset_view = GTK_COMBO_BOX(user_data);

    GError* error = NULL;
    if (!org_mdr_eq_call_set_levels_finish(eq, res, &error))
    {
        g_error("Failed to set Eq preset: %s", error->message);
        gtk_combo_box_set_active_id(preset_view, org_mdr_eq_get_preset(eq));
    }
}

static void eq_preset_changed(GtkComboBox* preset_view,
                              gpointer user_data)
{
    OrgMdrEq* eq = user_data;

    const gchar* target_preset = gtk_combo_box_get_active_id(preset_view);

    if (target_preset == NULL)
    {
        return;
    }

    org_mdr_eq_call_set_preset(eq,
                               target_preset,
                               NULL,
                               eq_preset_changed_finish,
                               preset_view);
}

static gboolean eq_level_transform(GBinding* binding,
                                   const GValue* from_value,
                                   GValue* to_value,
                                   gpointer user_data)
{
    GVariant* values = g_value_get_variant(from_value);
    if (values == NULL) return FALSE;

    guint32 level = 0;
    g_variant_get_child(values, (gsize) user_data, "u", &level);

    g_value_set_double(to_value, level);

    return TRUE;
}

typedef struct
{
    int refs;
    GWeakRef eq_iface;
    GWeakRef range;
    int index;
}
eq_callback_data_t;

static void free_eq_callback_data(eq_callback_data_t* data)
{
    data->refs--;
    if (data->refs == 0)
    {
        g_free(data);
    }
}

static void eq_set_level_finish(GObject* source_object,
                                GAsyncResult* res,
                                gpointer user_data);

static gboolean eq_set_level(GtkRange* range,
                             GtkScrollType* scroll,
                             gdouble value,
                             gpointer user_data)
{
    eq_callback_data_t* data = user_data;

    OrgMdrEq* eq = ORG_MDR_EQ(g_weak_ref_get(&data->eq_iface));

    if (eq == NULL) return TRUE;

    guint max_level = org_mdr_eq_get_level_steps(eq) - 1;

    if (value < 0) value = 0;
    if (value > G_MAXUINT32) value = G_MAXUINT32 - 1;

    guint target_level = (guint) (value + 0.5f);

    if (target_level > max_level) value = target_level;

    GVariant* levels = org_mdr_eq_get_levels(eq);

    guint32 level = 0;
    g_variant_get_child(levels, data->index, "u", &level);

    if (level != target_level)
    {
        GVariantBuilder target_levels_builder;
        g_variant_builder_init(&target_levels_builder, G_VARIANT_TYPE("au"));

        GVariantIter* iter = g_variant_iter_new(levels);

        for (int i = 0; g_variant_iter_next(iter, "u", &level); i++)
        {
            if (i == data->index) level = target_level;

            g_variant_builder_add(&target_levels_builder, "u", level);
        }

        g_variant_iter_free(iter);

        data->refs++;
        org_mdr_eq_call_set_levels(
                eq,
                g_variant_builder_end(&target_levels_builder),
                NULL,
                eq_set_level_finish,
                data);

        g_variant_builder_clear(&target_levels_builder);
    }

    g_object_unref(eq);

    return FALSE;
}

static void eq_set_level_finish(GObject* source_object,
                                GAsyncResult* res,
                                gpointer user_data)
{
    eq_callback_data_t* data = user_data;

    OrgMdrEq* eq = ORG_MDR_EQ(g_weak_ref_get(&data->eq_iface));

    if (eq == NULL) return;

    GError* error = NULL;
    if (!org_mdr_eq_call_set_levels_finish(eq, res, &error))
    {
        g_message("Failed to set levels: %s", error->message);

        GtkRange* range = GTK_RANGE(g_weak_ref_get(&data->range));

        if (range != NULL)
        {
            GVariant* levels = org_mdr_eq_get_levels(eq);

            guint level;
            g_variant_get_child(levels, data->index, "u", &level);

            gtk_range_set_value(range, level);

            g_object_unref(range);
        }
    }

    g_object_unref(eq);

    free_eq_callback_data(data);
}

begin_iface_init(eq, OrgMdrEq)
    gtk_widget_set_visible(GTK_WIDGET(private->eq_view), TRUE);

    for (const gchar* const* preset = org_mdr_eq_get_available_presets(eq);
         *preset != NULL;
         preset++)
    {
        gtk_combo_box_text_append(private->eq_preset,
                                  *preset,
                                  *preset);
    }

    g_object_bind_property(
            eq,
            "preset",
            private->eq_preset,
            "active-id",
            G_BINDING_SYNC_CREATE);

    g_signal_connect_object(
            private->eq_preset,
            "changed",
            G_CALLBACK(eq_preset_changed),
            eq,
            G_CONNECT_AFTER);

    guint band_count = org_mdr_eq_get_band_count(eq);
    guint level_steps = org_mdr_eq_get_level_steps(eq);

    for (int i = 0; i < band_count; i++)
    {
        GtkScale* scale
                = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL,
                                                     0.0,
                                                     level_steps - 1,
                                                     1.0));

        GValue height_request = { 0 };
        g_value_init(&height_request, G_TYPE_INT);
        g_value_set_int(&height_request, 128);
        g_object_set_property(G_OBJECT(scale),
                              "height-request",
                              &height_request);

        gtk_scale_set_value_pos(scale, GTK_POS_BOTTOM);
        gtk_scale_set_digits(scale, 0);
        gtk_range_set_inverted(GTK_RANGE(scale), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(scale), TRUE);

        g_object_bind_property_full(
                eq,
                "levels",
                gtk_range_get_adjustment(GTK_RANGE(scale)),
                "value",
                G_BINDING_SYNC_CREATE,
                eq_level_transform,
                NULL,
                (gpointer) (gsize) i,
                NULL);

        eq_callback_data_t* callback_data = g_new(eq_callback_data_t, 1);
        g_object_ref(eq);
        callback_data->refs = 1;
        g_weak_ref_init(&callback_data->eq_iface, eq);
        g_weak_ref_init(&callback_data->range, scale);
        callback_data->index = i;

        g_signal_connect_data(
                scale,
                "change-value",
                G_CALLBACK(eq_set_level),
                callback_data,
                (GClosureNotify) free_eq_callback_data,
                0);

        gtk_container_add(GTK_CONTAINER(private->eq_container),
                          GTK_WIDGET(scale));
    }
end_iface_init

static void auto_power_off_timeout_changed_finish(GObject* source_object,
                                                  GAsyncResult* res,
                                                  gpointer user_data)
{
    OrgMdrAutoPowerOff* auto_power_off = ORG_MDR_AUTO_POWER_OFF(source_object);
    GtkComboBox* timeout_view = GTK_COMBO_BOX(user_data);

    GError* error = NULL;
    if (!org_mdr_auto_power_off_call_set_timeout_finish(auto_power_off,
                                                        res,
                                                        &error))
    {
        g_error("Failed to set auto power-off timeout: %s", error->message);
        gtk_combo_box_set_active_id(
                timeout_view,
                org_mdr_auto_power_off_get_timeout(auto_power_off));
    }
}

static void auto_power_off_timeout_changed(GtkComboBox* timeout_view,
                                           gpointer user_data)
{
    OrgMdrAutoPowerOff* auto_power_off = user_data;

    const gchar* target_timeout = gtk_combo_box_get_active_id(timeout_view);

    if (target_timeout == NULL)
    {
        return;
    }

    org_mdr_auto_power_off_call_set_timeout(
            auto_power_off,
            target_timeout,
            NULL,
            auto_power_off_timeout_changed_finish,
            timeout_view);
}

begin_iface_init(auto_power_off, OrgMdrAutoPowerOff)
    gtk_widget_set_visible(GTK_WIDGET(private->auto_power_off_view), TRUE);

    for (const gchar* const* timeout
                = org_mdr_auto_power_off_get_available_timeouts(auto_power_off);
         *timeout != NULL;
         timeout++)
    {
        gtk_combo_box_text_append(private->auto_power_off_timeout,
                                  *timeout,
                                  *timeout);
    }

    g_object_bind_property(
            auto_power_off,
            "timeout",
            private->auto_power_off_timeout,
            "active-id",
            G_BINDING_SYNC_CREATE);

    g_signal_connect_object(
            private->auto_power_off_timeout,
            "changed",
            G_CALLBACK(auto_power_off_timeout_changed),
            auto_power_off,
            G_CONNECT_AFTER);
end_iface_init

begin_iface_init(key_functions, OrgMdrKeyFunctions)
    gtk_widget_set_visible(GTK_WIDGET(private->key_functions_view), TRUE);

    GValue interface = {0};
    g_value_init(&interface, TYPE_ORG_MDR_KEY_FUNCTIONS_PROXY);
    g_value_set_object(&interface, key_functions);
    g_object_set_property(G_OBJECT(private->key_functions), "interface", &interface);
    // GVariant* a = org_mdr_key_functions_get_
end_iface_init

MDRDeviceView* mdr_device_view_new(MDRDevice* device)
{
    MDRDeviceView* view
            = g_object_new(MDR_DEVICE_VIEW_TYPE, NULL);

#define init_iface(name, type, signal_name) \
    { \
        GValue value = { 0 }; \
        g_object_get_property(G_OBJECT(device), #name, &value); \
        type* name = g_value_get_object(&value); \
        \
        if (name != NULL) \
        { \
            init_##name(view, name); \
        } \
        else \
        { \
            g_signal_connect_object( \
                    device, \
                    "notify::" signal_name, \
                    G_CALLBACK(iface_##name##_changed), \
                    view, \
                    G_CONNECT_AFTER); \
        } \
    }

    init_iface(power_off, OrgMdrPowerOff, "power-off");
    init_iface(battery, OrgMdrBattery, "battery");
    init_iface(left_right_battery, OrgMdrLeftRightBattery, "left-right-battery");
    init_iface(cradle_battery, OrgMdrCradleBattery, "cradle-battery");
    init_iface(noise_cancelling, OrgMdrNoiseCancelling, "noise-cancelling");
    init_iface(ambient_sound_mode, OrgMdrAmbientSoundMode, "ambient-sound-mode");
    init_iface(eq, OrgMdrEq, "eq");
    init_iface(auto_power_off, OrgMdrAutoPowerOff, "auto-power-off");
    init_iface(key_functions, OrgMdrKeyFunctions, "key-functions");

    return view;
}

static void mdr_device_view_class_init(MDRDeviceViewClass* class)
{
    g_type_ensure(MDR_HEADING_TYPE);

    gtk_widget_class_set_template_from_resource(
            GTK_WIDGET_CLASS(class),
            "/org/mdr/manager/device.ui");

#define bind_template_child(name) \
    gtk_widget_class_bind_template_child_private( \
            GTK_WIDGET_CLASS(class), \
            MDRDeviceView, \
            name);

    bind_template_child(power_off_button);

    bind_template_child(battery_view);
    bind_template_child(battery_bar);
    bind_template_child(battery_label);

    bind_template_child(lr_battery_view);
    bind_template_child(left_battery_bar);
    bind_template_child(left_battery_label);
    bind_template_child(right_battery_bar);
    bind_template_child(right_battery_label);

    bind_template_child(cradle_battery_view);
    bind_template_child(cradle_battery_bar);
    bind_template_child(cradle_battery_label);

    bind_template_child(nc_view);
    bind_template_child(nc_switch);

    bind_template_child(asm_view);
    bind_template_child(asm_amount);

    bind_template_child(eq_view);
    bind_template_child(eq_preset);
    bind_template_child(eq_container);

    bind_template_child(auto_power_off_view);
    bind_template_child(auto_power_off_timeout);

    bind_template_child(key_functions_view);
    bind_template_child(key_functions);
}

