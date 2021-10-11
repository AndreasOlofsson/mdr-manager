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

struct _MDRApp
{
    GtkApplication parent;

    MDRWindow* win;
};

G_DEFINE_TYPE(MDRApp, mdr_app, GTK_TYPE_APPLICATION)

enum
{
    SIGNAL_CONNECTED,
    SIGNAL_COUNT,
};

static guint signals[SIGNAL_COUNT] = { 0 };

static void mdr_app_init(MDRApp* app) {}

static void reconnect(GSimpleAction* action,
                      GVariant* parameter,
                      gpointer app)
{}

static void quit_activated(GSimpleAction* action,
                           GVariant* parameter,
                           gpointer app)
{
    g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] =
{
    { "reconnect", reconnect, NULL, NULL, NULL },
    { "quit", quit_activated, NULL, NULL, NULL }
};

static void bus_connect_finish(GObject* source_object,
                               GAsyncResult* res,
                               gpointer user_data);

static void mdr_app_startup(GApplication* app)
{
    G_APPLICATION_CLASS(mdr_app_parent_class)->startup(app);

    const gchar *reconnect_accels[2] = { "<Ctrl>R", NULL };
    const gchar *quit_accels[2] = { "<Ctrl>Q", NULL };

    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    app_entries, G_N_ELEMENTS(app_entries),
                                    app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
                                          "app.reconnect",
                                          reconnect_accels);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
                                          "app.quit",
                                          quit_accels);

    GtkBuilder* builder
        = gtk_builder_new_from_resource("/org/mdr/manager/app-menu.ui");
    GMenuModel* app_menu
        = G_MENU_MODEL(gtk_builder_get_object(builder, "appmenu"));
    gtk_application_set_app_menu(GTK_APPLICATION(app), app_menu);
    g_object_unref(builder);

    g_bus_get(G_BUS_TYPE_SYSTEM, NULL, bus_connect_finish, app);
}

static void bus_connect_finish(GObject* source_object,
                               GAsyncResult* res,
                               gpointer user_data)
{
    GApplication* app = user_data;

    MDRDeviceManager* device_manager
        = mdr_device_manager_new(g_bus_get_finish(res, NULL));

    g_signal_emit(app, signals[SIGNAL_CONNECTED], 0, device_manager);
}

static void mdr_app_activate(GApplication* app)
{
    MDRWindow* win = mdr_window_new(MDR_APP(app));

    MDR_APP(app)->win = win;

    gtk_window_present(GTK_WINDOW(win));
}

static void mdr_app_class_init(MDRAppClass* class)
{
    G_APPLICATION_CLASS(class)->startup = mdr_app_startup;
    G_APPLICATION_CLASS(class)->activate = mdr_app_activate;

    signals[SIGNAL_CONNECTED] =
            g_signal_new("connected",
                          MDR_APP_TYPE,
                          G_SIGNAL_RUN_LAST
                              | G_SIGNAL_NO_RECURSE
                              | G_SIGNAL_NO_HOOKS,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          1,
                          MDR_DEVICE_MANAGER_TYPE);
}

MDRApp* mdr_app_new(void)
{
    return g_object_new(MDR_APP_TYPE,
                        "application-id", "org.mdr.manager",
                        "flags", G_APPLICATION_FLAGS_NONE,
                        NULL);
}
