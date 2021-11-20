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

#include "key_functions_view.h"

#include "mdr_device.h"

struct _MDRKeyFunctionsView
{
    GtkBox parent;
};

typedef struct _MDRKeyFunctionsViewPrivate MDRKeyFunctionsViewPrivate;

struct _MDRKeyFunctionsViewPrivate
{
    OrgMdrKeyFunctions* interface;
    guint interface_signal;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRKeyFunctionsView,
                           mdr_key_functions_view,
                           GTK_TYPE_BOX)

enum
{
    PROPERTY_INTERFACE = 1,
    PROPERTY_COUNT,
};

static void mdr_key_functions_view_init(MDRKeyFunctionsView* view)
{
    MDRKeyFunctionsViewPrivate* private
            = mdr_key_functions_view_get_instance_private(view);

    private->interface = NULL;
}

static void get_property(GObject* object,
                         guint prop_id,
                         GValue* value,
                         GParamSpec* pspec)
{
    MDRKeyFunctionsView* view = MDR_KEY_FUNCTIONS_VIEW(object);

    MDRKeyFunctionsViewPrivate* private
            = mdr_key_functions_view_get_instance_private(view);

    switch (prop_id)
    {
        case PROPERTY_INTERFACE:
            g_value_set_object(value, private->interface);

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static GtkWidget* get_child_by_index(GtkContainer* container, guint n)
{
    GList* children = gtk_container_get_children(container);
    GtkWidget* child = GTK_WIDGET(g_list_nth_data(children, n));
    g_list_free(children);
    return child;
}

static GtkWidget* get_child_by_name(GtkContainer* container, const gchar* name)
{
    GList* children = gtk_container_get_children(container);
    
    for (GList* iter = children; iter != NULL; iter = iter->next)
    {
        GtkWidget* child = GTK_WIDGET(iter->data);

        if (g_str_equal(gtk_widget_get_name(child), name))
        {
            g_list_free(children);

            return child;
        }
    }

    g_list_free(children);

    return NULL;
}

static void update_active_presets(MDRKeyFunctionsView* view)
{
    MDRKeyFunctionsViewPrivate* private
            = mdr_key_functions_view_get_instance_private(view);

    GVariant* key_functions
        = org_mdr_key_functions_get_available_presets(
                private->interface);

    if (key_functions == NULL)
    {
        return;
    }

    GVariant* active_presets
        = org_mdr_key_functions_get_current_presets(private->interface);

    if (active_presets == NULL)
    {
        return;
    }

    GVariantIter* keys_iter = g_variant_iter_new(key_functions);
    const gchar* key_name;
    GVariant* key_presets;

    guint i = 0;
    while (g_variant_iter_loop(keys_iter,
                               "{s(@s@s@a{sa{ss}})}",
                               &key_name,
                               NULL, NULL,
                               &key_presets))
    {
        GtkBox* key_view = GTK_BOX(get_child_by_index(GTK_CONTAINER(view), i));
        i++;

        GtkComboBox* preset_view
            = GTK_COMBO_BOX(get_child_by_name(
                        GTK_CONTAINER(get_child_by_name(GTK_CONTAINER(key_view),
                                                        "main")),
                        "presets"));

        const gchar* active_preset = NULL;
        g_variant_lookup(active_presets, key_name, "s", &active_preset);
        gtk_combo_box_set_active_id(preset_view, active_preset);

        GtkBox* function_list
            = GTK_BOX(get_child_by_name(GTK_CONTAINER(key_view),
                                        "function_list"));

        GList* children = gtk_container_get_children(
                GTK_CONTAINER(function_list));
        for (GList* iter = children; iter != NULL; iter = iter->next)
        {
            gtk_container_remove(GTK_CONTAINER(function_list), iter->data);
        }
        g_list_free(children);


        GVariantIter* functions_iter;
        if (g_variant_lookup(key_presets,
                             active_preset,
                             "a{ss}",
                             &functions_iter))
        {
            const gchar* function, *action;
            while (g_variant_iter_loop(functions_iter,
                                       "{ss}",
                                       &function,
                                       &action))
            {
                GtkBuilder* function_builder = gtk_builder_new_from_resource(
                        "/org/mdr/manager/key-functions-function.ui");

                GtkImage* icon = GTK_IMAGE(gtk_builder_get_object(
                            function_builder, "icon"));

                if (g_str_equal(function, "Single Tap"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "single-press-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Double Tap"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "double-press-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Triple Tap"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "triple-press-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Single Tap and Hold"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "single-hold-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Double Tap and Hold"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "double-hold-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Long Press and Activate"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "long-hold-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }
                else if (g_str_equal(function, "Long Press during Activation"))
                {
                    gtk_image_set_from_icon_name(icon,
                                                 "long-hold-symbolic",
                                                 GTK_ICON_SIZE_INVALID);
                }

                gtk_image_set_pixel_size(icon, 32);

                GtkLabel* label = GTK_LABEL(gtk_builder_get_object(
                            function_builder, "label"));
                gtk_label_set_label(label, action);

                gtk_container_add(
                        GTK_CONTAINER(function_list),
                        GTK_WIDGET(gtk_builder_get_object(function_builder,
                                                          "function")));

                g_object_unref(function_builder);
            }
        }
    }
}

static void active_presets_changed(OrgMdrKeyFunctions* iface,
                                   GParamSpec* param_spec,
                                   MDRKeyFunctionsView* view)
{
    update_active_presets(view);
}

typedef struct
{
    MDRKeyFunctionsView* view;
    gchar* key_name;
}
PresetChangeData;

static PresetChangeData* preset_change_data_new(MDRKeyFunctionsView* view,
                                                const gchar* key_name)
{
    PresetChangeData* data = g_malloc(sizeof(PresetChangeData));

    data->view = view;
    data->key_name = g_strdup(key_name);

    g_object_ref(view);

    return data;
}

static void preset_change_data_free(PresetChangeData* data)
{
    g_object_unref(data->view);
    g_free(data->key_name);
    g_free(data);
}

static void change_presets_finish(GObject* source_object,
                                  GAsyncResult* res,
                                  gpointer user_data)
{
    org_mdr_key_functions_call_set_presets_finish(
            ORG_MDR_KEY_FUNCTIONS(source_object),
            res,
            NULL);
}

static void change_presets(GtkComboBox* preset_view, PresetChangeData* data)
{
    MDRKeyFunctionsView* view = data->view;

    MDRKeyFunctionsViewPrivate* private
            = mdr_key_functions_view_get_instance_private(view);

    const gchar* key_name = data->key_name;

    GVariant* active_presets
        = org_mdr_key_functions_get_current_presets(private->interface);

    const gchar* current_preset;
    if (!g_variant_lookup(active_presets, key_name, "s", &current_preset))
    {
        return;
    }

    const gchar* new_preset = gtk_combo_box_get_active_id(preset_view);

    if (g_str_equal(current_preset, new_preset))
    {
        return;
    }

    gtk_combo_box_set_active_id(preset_view, current_preset);

    GVariantIter* iter = g_variant_iter_new(active_presets);

    GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));

    const gchar* iter_key_name, *iter_preset;
    while (g_variant_iter_loop(iter, "{ss}", &iter_key_name, &iter_preset))
    {
        if (g_str_equal(iter_key_name, key_name))
        {
            g_variant_builder_add(builder, "{ss}", iter_key_name, new_preset);
        }
        else
        {
            g_variant_builder_add(builder, "{ss}", iter_key_name, iter_preset);
        }
    }

    org_mdr_key_functions_call_set_presets(
            private->interface,
            g_variant_builder_end(builder),
            NULL,
            change_presets_finish,
            NULL);
}

static void set_property(GObject* object,
                         guint prop_id,
                         const GValue* value,
                         GParamSpec* pspec)
{
    MDRKeyFunctionsView* view = MDR_KEY_FUNCTIONS_VIEW(object);

    MDRKeyFunctionsViewPrivate* private
            = mdr_key_functions_view_get_instance_private(view);

    switch (prop_id)
    {
        case PROPERTY_INTERFACE:
        {
            if (private->interface != NULL)
            {
                g_signal_handler_disconnect(private->interface,
                                            private->interface_signal);

                GList* children
                    = gtk_container_get_children(GTK_CONTAINER(view));
                for (GList* iter = children; iter != NULL; iter = iter->next)
                {
                    gtk_container_remove(GTK_CONTAINER(view), iter->data);
                }
                g_list_free(children);
            }

            private->interface = g_value_get_object(value);

            if (private->interface == NULL)
            {
                break;
            }

            GVariant* key_functions
                = org_mdr_key_functions_get_available_presets(
                        private->interface);

            GVariant* active_presets
                = org_mdr_key_functions_get_current_presets(private->interface);

            GVariantIter* keys_iter = g_variant_iter_new(key_functions);
            const gchar* key_name;
            GVariantIter* key_presets;

            while (g_variant_iter_loop(keys_iter,
                                       "{s(@s@sa{sa{ss}})}",
                                       &key_name,
                                       NULL, NULL,
                                       &key_presets))
            {
                const gchar* active_preset = NULL;
                g_variant_lookup(active_presets, key_name, "s", &active_preset);

                GtkBuilder* builder = gtk_builder_new_from_resource(
                        "/org/mdr/manager/key-functions.ui");

                GtkContainer* container = GTK_CONTAINER(
                        gtk_builder_get_object(builder, "key"));

                gtk_label_set_label(
                        GTK_LABEL(gtk_builder_get_object(builder, "label")),
                        key_name);

                GtkComboBox* presets_view
                    = GTK_COMBO_BOX(gtk_builder_get_object(builder, "presets"));

                g_signal_connect_data(
                        presets_view,
                        "changed",
                        G_CALLBACK(change_presets),
                        preset_change_data_new(view, key_name),
                        (GClosureNotify) preset_change_data_free,
                        G_CONNECT_AFTER);

                const gchar* preset_name;
                GVariantIter* functions;
                while (g_variant_iter_loop(key_presets,
                                           "{sa{ss}}",
                                           &preset_name,
                                           &functions))
                {
                    gtk_combo_box_text_append(
                            GTK_COMBO_BOX_TEXT(presets_view),
                            preset_name,
                            preset_name);
                }

                gtk_container_add(GTK_CONTAINER(view),
                                  GTK_WIDGET(container));

                g_object_unref(builder);

            }

            update_active_presets(view);

            private->interface_signal = g_signal_connect_object(
                    private->interface,
                    "notify::current-presets",
                    G_CALLBACK(active_presets_changed),
                    view,
                    G_CONNECT_AFTER);
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void mdr_key_functions_view_class_init(MDRKeyFunctionsViewClass* class)
{
    gtk_icon_theme_add_resource_path(
            gtk_icon_theme_get_default(),
            "/org/mdr/manager/icons/scalable/actions");

    GObjectClass* object_class = G_OBJECT_CLASS(class);

    object_class->get_property = get_property;
    object_class->set_property = set_property;

    g_object_class_install_property(
            object_class,
            PROPERTY_INTERFACE,
            g_param_spec_object(
                "interface",
                "Interface",
                "iface",
                TYPE_ORG_MDR_KEY_FUNCTIONS_PROXY,
                G_PARAM_READWRITE));
}

