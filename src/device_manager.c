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

#include "device_manager.h"

#include "device.h"
#include "mdr_device.h"

struct _MDRDeviceManager
{
    GObject parent;
};

typedef struct _MDRDeviceManagerPrivate MDRDeviceManagerPrivate;

struct _MDRDeviceManagerPrivate
{
    GDBusConnection* connection;

    GHashTable* devices;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRDeviceManager,
                           mdr_device_manager,
                           G_TYPE_OBJECT)

enum
{
    SIGNAL_DEVICE_CONNECTED,
    SIGNAL_DEVICE_DISCONNECTED,
    SIGNAL_COUNT,
};

static guint signals[SIGNAL_COUNT] = { 0 };

static void device_connected(GDBusConnection* connection,
                             const gchar* sender_name,
                             const gchar* object_path,
                             const gchar* interface_name,
                             const gchar* signal_name,
                             GVariant* parameters,
                             gpointer user_data);

static void device_disconnected(MDRDevice* device,
                                MDRDeviceManager* device_manager);

static void get_connected_devices(MDRDeviceManager* device_manager);

static void mdr_device_manager_init(MDRDeviceManager* device_manager)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    private->connection = NULL;

    private->devices = g_hash_table_new(g_str_hash, g_str_equal);
}

MDRDeviceManager* mdr_device_manager_new(GDBusConnection* connection)
{
    MDRDeviceManager* device_manager
            = g_object_new(MDR_DEVICE_MANAGER_TYPE, NULL);

    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    private->connection = connection;

    g_dbus_connection_signal_subscribe(connection,
                                       "org.mdr",
                                       "org.mdr.Device",
                                       "connected",
                                       NULL, // object_path
                                       NULL, // arg0
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       device_connected,
                                       device_manager,
                                       NULL);

     get_connected_devices(device_manager);

    return device_manager;
}

GDBusConnection* mdr_device_manager_get_connection(
        MDRDeviceManager* device_manager)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    return private->connection;
}

guint mdr_device_manager_get_device_count(
        MDRDeviceManager* device_manager)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    return g_hash_table_size(private->devices);
}

static void device_connected(GDBusConnection* connection,
                             const gchar* sender_name,
                             const gchar* object_path,
                             const gchar* interface_name,
                             const gchar* signal_name,
                             GVariant* parameters,
                             gpointer user_data)
{
    MDRDeviceManager* device_manager = user_data;

    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    if (!g_hash_table_contains(private->devices, object_path))
    {
        MDRDevice* device = mdr_device_new(connection, object_path);

        g_hash_table_insert(private->devices,
                            g_strdup(object_path),
                            device);

        g_signal_emit(device_manager,
                      signals[SIGNAL_DEVICE_CONNECTED],
                      0,
                      device);

        g_signal_connect(device,
                         "disconnected",
                         G_CALLBACK(device_disconnected),
                         device_manager);
    }
}

static void get_connected_devices_interfaces_result(GObject* source_object,
                                                    GAsyncResult* res,
                                                    gpointer user_data);

static void get_connected_devices(MDRDeviceManager* device_manager)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    GDBusConnection* connection = private->connection;

    g_dbus_connection_call(connection,
                           "org.mdr",
                           "/org/bluez",
                           "org.freedesktop.DBus.Introspectable",
                           "Introspect",
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL, // cancellable
                           get_connected_devices_interfaces_result,
                           device_manager);
}

static void get_connected_devices_device_result(GObject* source_object,
                                                GAsyncResult* res,
                                                gpointer user_data);

typedef struct
{
    MDRDeviceManager* device_manager;
    const gchar* interface_name;
}
get_connected_devices_interface_data;

static void get_connected_devices_interfaces_result(GObject* source_object,
                                                    GAsyncResult* res,
                                                    gpointer user_data)
{
    MDRDeviceManager* device_manager = user_data;

    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    GDBusConnection* connection = private->connection;

    GError* error = NULL;

    GVariant* result
            = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object),
                                            res,
                                            &error);

    if (result == NULL)
    {
        g_error("Failed to get bluez interfaces: %s", error->message);
        return;
    }

    gchar* xml_data = NULL;
    g_variant_get(result, "(&s)", &xml_data);

    GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml(xml_data, NULL);

    g_variant_unref(result);

    for (GDBusNodeInfo** node = node_info->nodes;
         *node != NULL;
         node++)
    {
        const gchar* interface_name = (*node)->path;

        gchar* interface_path = g_strconcat("/org/bluez/",
                                            interface_name,
                                            NULL);

        get_connected_devices_interface_data* data
                = g_new(get_connected_devices_interface_data, 1);
        data->device_manager = device_manager;
        data->interface_name = g_strdup(interface_name);

        g_dbus_connection_call(connection,
                               "org.mdr",
                               interface_path,
                               "org.freedesktop.DBus.Introspectable",
                               "Introspect",
                               NULL,
                               NULL,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL, // cancellable
                               get_connected_devices_device_result,
                               data);

        g_free(interface_path);
    }

    g_dbus_node_info_unref(node_info);
}

static void get_connected_devices_device_result(GObject* source_object,
                                                GAsyncResult* res,
                                                gpointer user_data)
{
    get_connected_devices_interface_data* data = user_data;

    MDRDeviceManager* device_manager = data->device_manager;

    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    GDBusConnection* connection = private->connection;

    GError* error = NULL;

    GVariant* result
            = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object),
                                            res,
                                            &error);

    if (result == NULL)
    {
        g_error("Failed to get bluez devices: %s", error->message);
        g_free(data);
        return;
    }

    gchar* xml_data = NULL;
    g_variant_get(result, "(&s)", &xml_data);

    GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml(xml_data, NULL);

    g_variant_unref(result);

    for (GDBusNodeInfo** node = node_info->nodes;
         *node != NULL;
         node++)
    {
        const gchar* device_name = (*node)->path;

        gchar* device_path = g_strconcat("/org/bluez/",
                                         data->interface_name,
                                         "/",
                                         device_name,
                                         NULL);

        if (!g_hash_table_contains(private->devices, device_path))
        {
            MDRDevice* device = mdr_device_new(connection, device_path);

            g_hash_table_insert(private->devices,
                                device_path,
                                device);

            g_signal_emit(device_manager,
                          signals[SIGNAL_DEVICE_CONNECTED],
                          0,
                          device);

            g_signal_connect(device,
                             "disconnected",
                             G_CALLBACK(device_disconnected),
                             device_manager);
        }
        else
        {
            g_free(device_path);
        }
    }

    g_dbus_node_info_unref(node_info);

    g_free(data);
}

static void device_disconnected(MDRDevice* device,
                                MDRDeviceManager* device_manager)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(device_manager);

    g_hash_table_remove(
            private->devices,
            mdr_device_get_object_path(device));

    g_signal_emit(
            device_manager,
            signals[SIGNAL_DEVICE_DISCONNECTED],
            0,
            device);
}

static void mdr_device_manager_finalize(GObject* object)
{
    MDRDeviceManagerPrivate* private
            = mdr_device_manager_get_instance_private(
                MDR_DEVICE_MANAGER(object));

    g_object_unref(private->connection);

    g_hash_table_destroy(private->devices);

    G_OBJECT_CLASS(mdr_device_manager_parent_class)->finalize(object);
}

static void mdr_device_manager_class_init(MDRDeviceManagerClass* class)
{
    G_OBJECT_CLASS(class)->finalize = mdr_device_manager_finalize;

    signals[SIGNAL_DEVICE_CONNECTED] =
            g_signal_new("device-connected",
                          MDR_DEVICE_MANAGER_TYPE,
                          G_SIGNAL_RUN_LAST
                              | G_SIGNAL_NO_RECURSE
                              | G_SIGNAL_NO_HOOKS,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          1,
                          MDR_DEVICE_TYPE);

    signals[SIGNAL_DEVICE_DISCONNECTED] =
            g_signal_new("device-disconnected",
                          MDR_DEVICE_MANAGER_TYPE,
                          G_SIGNAL_RUN_LAST
                              | G_SIGNAL_NO_RECURSE
                              | G_SIGNAL_NO_HOOKS,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          1,
                          MDR_DEVICE_TYPE);
}

