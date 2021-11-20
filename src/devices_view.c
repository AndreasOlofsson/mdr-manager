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

#include "device_manager.h"
#include "device.h"

#include "devices_view.h"
#include "device_view.h"

struct _MDRDevicesView
{
    GtkBox parent;
};

typedef struct _MDRDevicesViewPrivate MDRDevicesViewPrivate;

struct _MDRDevicesViewPrivate
{
    GtkStack* device_stack;
    MDRDeviceManager* device_manager;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRDevicesView,
                           mdr_devices_view,
                           GTK_TYPE_BOX)

static void mdr_devices_view_init(MDRDevicesView* view)
{
    gtk_widget_init_template(GTK_WIDGET(view));

    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(view);

    private->device_manager = NULL;
}

static void device_connected(MDRDeviceManager* device_manager,
                             MDRDevice* device,
                             gpointer user_data);

void mdr_devices_view_set_manager(MDRDevicesView* view,
                                     MDRDeviceManager* device_manager)
{
    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(view);

    if (private->device_manager != NULL)
    {
        g_signal_handlers_disconnect_by_data(
                private->device_manager,
                view);

        g_object_unref(G_OBJECT(private->device_manager));
    }

    private->device_manager = device_manager;

    g_signal_connect(device_manager,
                     "device-connected",
                     G_CALLBACK(device_connected),
                     view);

    g_object_ref(G_OBJECT(private->device_manager));
}

static void device_name_changed(MDRDevice*,
                                GParamSpec* param_spec,
                                MDRDevicesView*);

typedef struct
{
    MDRDevicesView* view;
    MDRDeviceView* device_view;
}
device_disconnect_data;

static void device_disconnected(MDRDevice*,
                                device_disconnect_data* data);

static void device_connected(MDRDeviceManager* device_manager,
                             MDRDevice* device,
                             gpointer user_data)
{
    MDRDevicesView* view = user_data;
    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(view);

    MDRDeviceView* device_view = mdr_device_view_new(device);
    gtk_widget_show(GTK_WIDGET(device_view));

    gtk_stack_add_titled(private->device_stack,
                         GTK_WIDGET(device_view),
                         mdr_device_get_object_path(device),
                         "Device");

    g_signal_connect_object(
            device,
            "notify::name",
            G_CALLBACK(device_name_changed),
            view,
            G_CONNECT_AFTER);

    device_disconnect_data* disconnect_data = g_new(device_disconnect_data, 1);
    disconnect_data->view = view;
    disconnect_data->device_view = device_view;

    g_signal_connect_data(
            device,
            "disconnected",
            G_CALLBACK(device_disconnected),
            disconnect_data,
            (GClosureNotify) g_free,
            G_CONNECT_AFTER);
}


static void device_name_changed(MDRDevice* device,
                                GParamSpec* param_spec,
                                MDRDevicesView* view)
{
    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(view);

    const gchar* object_path = mdr_device_get_object_path(device);

    GtkWidget* device_view
            = gtk_stack_get_child_by_name(private->device_stack,
                                          object_path);

    GValue device_name = { 0 };
    g_object_get_property(G_OBJECT(device), "name", &device_name);

    gtk_container_child_set_property(GTK_CONTAINER(private->device_stack),
                                     device_view,
                                     "title",
                                     &device_name);
}

static void device_disconnected(MDRDevice* device,
                                device_disconnect_data* data)
{
    MDRDevicesView* view = data->view;
    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(view);

    MDRDeviceView* device_view = data->device_view;

    gtk_container_remove(
            GTK_CONTAINER(private->device_stack),
            GTK_WIDGET(device_view));
}

static void mdr_devices_view_finalize(GObject* object)
{
    MDRDevicesViewPrivate* private
            = mdr_devices_view_get_instance_private(
                MDR_DEVICES_VIEW(object));

    if (private->device_manager != NULL)
    {
        g_signal_handlers_disconnect_by_data(
                private->device_manager,
                MDR_DEVICES_VIEW(object));

        g_object_unref(G_OBJECT(private->device_manager));
    }

    G_OBJECT_CLASS(mdr_devices_view_parent_class)->finalize(object);
}

static void mdr_devices_view_class_init(MDRDevicesViewClass* class)
{
    g_type_ensure(MDR_DEVICE_VIEW_TYPE);

    G_OBJECT_CLASS(class)->finalize = mdr_devices_view_finalize;

    gtk_widget_class_set_template_from_resource(
            GTK_WIDGET_CLASS(class),
            "/org/mdr/manager/devices.ui");
    gtk_widget_class_bind_template_child_private(
            GTK_WIDGET_CLASS(class),
            MDRDevicesView,
            device_stack);
}

