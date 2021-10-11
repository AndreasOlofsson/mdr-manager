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

#include "app.h"
#include "window.h"
#include "devices_view.h"
#include "no_devices_view.h"
#include "device.h"

struct _MDRWindow
{
    GtkApplicationWindow parent;
};

typedef struct _MDRWindowPrivate MDRWindowPrivate;

struct _MDRWindowPrivate
{
    GtkWidget* stack;
    MDRDevicesView* devices_view;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRWindow,
                           mdr_window,
                           GTK_TYPE_APPLICATION_WINDOW)

static void mdr_window_init(MDRWindow* win)
{
    gtk_widget_init_template(GTK_WIDGET(win));

}

static void device_manager_connected(MDRApp* app,
                                     MDRDeviceManager* device_manager,
                                     gpointer user_data);

MDRWindow* mdr_window_new(MDRApp* app)
{
    MDRWindow* win = g_object_new(MDR_WINDOW_TYPE, "application", app, NULL);

    g_signal_connect(app,
                     "connected",
                     G_CALLBACK(device_manager_connected),
                     win);

    return win;
}

static void device_connected(MDRDeviceManager* device_manager,
                             MDRDevice* device,
                             gpointer user_data);

static void device_disconnected(MDRDeviceManager* device_manager,
                                MDRDevice* device,
                                gpointer user_data);

static void device_manager_connected(MDRApp* app,
                                     MDRDeviceManager* device_manager,
                                     gpointer user_data)
{
    MDRWindow* win = user_data;
    MDRWindowPrivate* private = mdr_window_get_instance_private(win);

    mdr_devices_view_set_manager(private->devices_view,
                                    device_manager);

    g_signal_connect(device_manager,
                     "device-connected",
                     G_CALLBACK(device_connected),
                     win);

    g_signal_connect(device_manager,
                     "device-disconnected",
                     G_CALLBACK(device_disconnected),
                     win);

    if (mdr_device_manager_get_device_count(device_manager) == 0)
    {
        gtk_stack_set_visible_child_name(GTK_STACK(private->stack),
                                         "no_devices");
    }
    else
    {
        gtk_stack_set_visible_child_name(GTK_STACK(private->stack),
                                         "devices");
    }
}


static void device_connected(MDRDeviceManager* device_manager,
                             MDRDevice* device,
                             gpointer user_data)
{
    MDRWindow* win = user_data;
    MDRWindowPrivate* private = mdr_window_get_instance_private(win);

    gtk_stack_set_visible_child_name(GTK_STACK(private->stack),
                                     "devices");
}

static void device_disconnected(MDRDeviceManager* device_manager,
                                MDRDevice* device,
                                gpointer user_data)
{
    MDRWindow* win = user_data;
    MDRWindowPrivate* private = mdr_window_get_instance_private(win);

    if (mdr_device_manager_get_device_count(device_manager) == 0)
    {
        gtk_stack_set_visible_child_name(GTK_STACK(private->stack),
                                         "no_devices");
    }
}

static void mdr_window_class_init(MDRWindowClass* class)
{
    g_type_ensure(MDR_DEVICES_VIEW_TYPE);
    g_type_ensure(MDR_NO_DEVICES_VIEW_TYPE);
    gtk_widget_class_set_template_from_resource(
            GTK_WIDGET_CLASS(class),
            "/org/mdr/manager/window.ui");
    gtk_widget_class_bind_template_child_private(
            GTK_WIDGET_CLASS(class),
            MDRWindow,
            stack);
    gtk_widget_class_bind_template_child_private(
            GTK_WIDGET_CLASS(class),
            MDRWindow,
            devices_view);
}

