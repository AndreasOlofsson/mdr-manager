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

#include "no_devices_view.h"

struct _MDRNoDevicesView
{
    GtkBox parent;
};

typedef struct _MDRNoDevicesViewPrivate MDRNoDevicesViewPrivate;

struct _MDRNoDevicesViewPrivate
{
    int _no_use;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRNoDevicesView,
                           mdr_no_devices_view,
                           GTK_TYPE_BOX)

static void mdr_no_devices_view_init(MDRNoDevicesView* view)
{
    gtk_widget_init_template(GTK_WIDGET(view));
}

static void mdr_no_devices_view_class_init(MDRNoDevicesViewClass* class)
{
    gtk_widget_class_set_template_from_resource(
            GTK_WIDGET_CLASS(class),
            "/org/mdr/manager/no-devices.ui");
}

