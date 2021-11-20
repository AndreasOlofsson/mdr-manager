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

#include "device.h"

#include "mdr_device.h"

#include "key_functions_view.h"

struct _MDRDevice
{
    GObject parent;

    gchar* object_path;
};

typedef struct _MDRDevicePrivate MDRDevicePrivate;

struct _MDRDevicePrivate
{
    GDBusConnection* connection;

    OrgMdrDevice* device_iface;

    OrgMdrPowerOff* power_off_iface;

    OrgMdrBattery* battery_iface;
    OrgMdrLeftRightBattery* left_right_battery_iface;
    OrgMdrCradleBattery* cradle_battery_iface;

    OrgMdrNoiseCancelling* noise_cancelling_iface;
    OrgMdrAmbientSoundMode* ambient_sound_mode_iface;

    OrgMdrEq* eq_iface;

    OrgMdrAutoPowerOff* auto_power_off_iface;

    OrgMdrKeyFunctions* key_functions_iface;
};

G_DEFINE_TYPE_WITH_PRIVATE(MDRDevice,
                           mdr_device,
                           G_TYPE_OBJECT)

enum
{
    PROPERTY_NAME = 1,
    PROPERTY_POWER_OFF,
    PROPERTY_BATTERY,
    PROPERTY_LEFT_RIGHT_BATTERY,
    PROPERTY_CRADLE_BATTERY,
    PROPERTY_NOISE_CANCELLING,
    PROPERTY_AMBIENT_SOUND_MODE,
    PROPERTY_EQ,
    PROPERTY_AUTO_POWER_OFF,
    PROPERTY_KEY_FUNCTIONS,
    PROPERTY_COUNT,
};

enum
{
    SIGNAL_DEVICE_DISCONNECTED,
    SIGNAL_COUNT,
};

static guint signals[SIGNAL_COUNT] = { 0 };

static void mdr_device_init(MDRDevice* device)
{
    device->object_path = NULL;

    MDRDevicePrivate* private
            = mdr_device_get_instance_private(device);

    private->connection = NULL;
    private->device_iface = NULL;
    private->power_off_iface = NULL;
    private->battery_iface = NULL;
    private->left_right_battery_iface = NULL;
    private->cradle_battery_iface = NULL;
    private->noise_cancelling_iface = NULL;
    private->ambient_sound_mode_iface = NULL;
    private->eq_iface = NULL;
    private->auto_power_off_iface = NULL;
    private->key_functions_iface = NULL;
}

static void device_introspect_finish(GObject* source_object,
                                     GAsyncResult* res,
                                     gpointer user_data);

MDRDevice* mdr_device_new(GDBusConnection* connection,
                                const gchar* object_path)
{
    MDRDevice* device
            = g_object_new(MDR_DEVICE_TYPE, NULL);

    device->object_path = g_strdup(object_path);

    MDRDevicePrivate* private
            = mdr_device_get_instance_private(device);

    private->connection = connection;

    g_object_ref(G_OBJECT(device));
    g_dbus_connection_call(connection,
                           "org.mdr",
                           device->object_path,
                           "org.freedesktop.DBus.Introspectable",
                           "Introspect",
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL, // cancellable
                           device_introspect_finish,
                           device);

    return device;
}

#define device_interface_handler(iface_name) \
    static void device_##iface_name##_finish(GObject* source_object, \
                                             GAsyncResult* res, \
                                             gpointer user_data) \
    { \
        MDRDevice* device = user_data; \
        \
        MDRDevicePrivate* private \
                = mdr_device_get_instance_private(device); \
        \
        GError* error = NULL; \
        private->iface_name##_iface \
                = org_mdr_##iface_name##_proxy_new_finish(res, &error); \
        \
        if (private->iface_name##_iface == NULL) \
        { \
            g_error("Failed to connect to " #iface_name " interface: %s", \
                    error->message); \
        } \
        else \
        { \
            g_object_notify(G_OBJECT(device), #iface_name); \
        } \
        \
        g_object_unref(G_OBJECT(device)); \
    }

device_interface_handler(power_off)
device_interface_handler(battery)
device_interface_handler(left_right_battery)
device_interface_handler(cradle_battery)
device_interface_handler(noise_cancelling)
device_interface_handler(ambient_sound_mode)
device_interface_handler(eq)
device_interface_handler(auto_power_off)
device_interface_handler(key_functions)

static void device_disconnected(OrgMdrDevice* device_iface,
                                MDRDevice* device)
{
    g_signal_emit(device, signals[SIGNAL_DEVICE_DISCONNECTED], 0);
}

static void device_device_iface_finish(GObject* source_object,
                                 GAsyncResult* res,
                                 gpointer user_data)
{
    MDRDevice* device = user_data;

    MDRDevicePrivate* private \
            = mdr_device_get_instance_private(device); \

    GError* error = NULL;
    private->device_iface
            = org_mdr_device_proxy_new_finish(res, &error);

    if (private->device_iface == NULL)
    {
        g_error("Failed to connect to device interface: %s",
                error->message);
        g_signal_emit(device, signals[SIGNAL_DEVICE_DISCONNECTED], 0);
    }
    else
    {
        g_object_notify(G_OBJECT(device), "name");

        g_signal_connect_object(
                private->device_iface,
                "disconnected",
                G_CALLBACK(device_disconnected),
                device,
                G_CONNECT_AFTER);
    }

    g_object_unref(G_OBJECT(device));
}

#define init_interface_if_present(iface_path, iface_name) \
    if (g_str_equal(interface_name, iface_path)) \
    { \
        g_object_ref(G_OBJECT(device)); \
        org_mdr_##iface_name##_proxy_new(private->connection, \
                                         G_DBUS_PROXY_FLAGS_NONE, \
                                         "org.mdr", \
                                         device->object_path, \
                                         NULL, \
                                         device_##iface_name##_finish, \
                                         device); \
        continue; \
    }

static void device_introspect_finish(GObject* source_object,
                                     GAsyncResult* res,
                                     gpointer user_data)
{
    MDRDevice* device = user_data;
    MDRDevicePrivate* private = mdr_device_get_instance_private(device);

    GError* error = NULL;

    GVariant* result
            = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object),
                                            res,
                                            &error);

    if (result == NULL)
    {
        g_error("Failed to get device interfaces: %s", error->message);
        g_object_unref(device);
        g_signal_emit(device, signals[SIGNAL_DEVICE_DISCONNECTED], 0);
        return;
    }

    gchar* xml_data = NULL;
    g_variant_get(result, "(&s)", &xml_data);

    GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml(xml_data, NULL);

    g_variant_unref(result);

    gboolean device_iface_found = FALSE;

    for (GDBusInterfaceInfo** interface = node_info->interfaces;
         *interface != NULL;
         interface++)
    {
        gchar* interface_name = (*interface)->name;

        if (g_str_equal(interface_name, "org.mdr.Device"))
        {
            g_object_ref(G_OBJECT(device));
            org_mdr_device_proxy_new(private->connection,
                                     G_DBUS_PROXY_FLAGS_NONE,
                                     "org.mdr",
                                     device->object_path,
                                     NULL,
                                     device_device_iface_finish,
                                     device);

            device_iface_found = TRUE;

            continue;
        }

        init_interface_if_present("org.mdr.PowerOff", power_off);
        init_interface_if_present("org.mdr.Battery", battery);
        init_interface_if_present("org.mdr.LeftRightBattery", left_right_battery);
        init_interface_if_present("org.mdr.CradleBattery", cradle_battery);
        init_interface_if_present("org.mdr.NoiseCancelling", noise_cancelling);
        init_interface_if_present("org.mdr.AmbientSoundMode", ambient_sound_mode);
        init_interface_if_present("org.mdr.Eq", eq);
        init_interface_if_present("org.mdr.AutoPowerOff", auto_power_off);
        init_interface_if_present("org.mdr.KeyFunctions", key_functions);
    }

    if (!device_iface_found)
    {
        g_signal_emit(device, signals[SIGNAL_DEVICE_DISCONNECTED], 0);
    }

    g_dbus_node_info_unref(node_info);
}

const gchar* mdr_device_get_object_path(MDRDevice* device)
{
    return device->object_path;
}

static void mdr_device_get_property(GObject* object,
                                       guint prop_id,
                                       GValue* value,
                                       GParamSpec* pspec)
{
    MDRDevice* device = MDR_DEVICE(object);
    MDRDevicePrivate* private = mdr_device_get_instance_private(device);

    switch (prop_id)
    {
        case PROPERTY_NAME:
        {
            if (private->device_iface != NULL)
            {
                const gchar* name = org_mdr_device_get_name(private->device_iface);
                if (name != NULL)
                {
                    g_value_set_string(value, name);
                }
            }
        }
        break;

        case PROPERTY_POWER_OFF:
            g_value_set_object(value, private->power_off_iface);
            break;

        case PROPERTY_BATTERY:
            g_value_set_object(value, private->battery_iface);
            break;

        case PROPERTY_LEFT_RIGHT_BATTERY:
            g_value_set_object(value, private->left_right_battery_iface);
            break;

        case PROPERTY_CRADLE_BATTERY:
            g_value_set_object(value, private->cradle_battery_iface);
            break;

        case PROPERTY_NOISE_CANCELLING:
            g_value_set_object(value, private->noise_cancelling_iface);
            break;

        case PROPERTY_AMBIENT_SOUND_MODE:
            g_value_set_object(value, private->ambient_sound_mode_iface);
            break;

        case PROPERTY_EQ:
            g_value_set_object(value, private->eq_iface);
            break;

        case PROPERTY_AUTO_POWER_OFF:
            g_value_set_object(value, private->auto_power_off_iface);
            break;

        case PROPERTY_KEY_FUNCTIONS:
            g_value_set_object(value, private->key_functions_iface);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void mdr_device_set_property(GObject* object,
                                       guint prop_id,
                                       const GValue* value,
                                       GParamSpec* pspec)
{
    switch (prop_id)
    {
        case PROPERTY_NAME:
        case PROPERTY_POWER_OFF:
        case PROPERTY_BATTERY:
        case PROPERTY_LEFT_RIGHT_BATTERY:
        case PROPERTY_CRADLE_BATTERY:
        case PROPERTY_NOISE_CANCELLING:
        case PROPERTY_AMBIENT_SOUND_MODE:
        case PROPERTY_EQ:
        case PROPERTY_AUTO_POWER_OFF:
        case PROPERTY_KEY_FUNCTIONS:
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

// TODO add finalizer

static void mdr_device_class_init(MDRDeviceClass* class)
{
    g_type_ensure(MDR_KEY_FUNCTIONS_VIEW_TYPE);

    GObjectClass* object_class = G_OBJECT_CLASS(class);

    object_class->get_property = mdr_device_get_property;
    object_class->set_property = mdr_device_set_property;

    g_object_class_install_property(
            object_class,
            PROPERTY_NAME,
            g_param_spec_string("name",
                                "Name",
                                "The name of the device. ",
                                NULL,
                                G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_POWER_OFF,
            g_param_spec_object("power_off",
                               "Power Off interface",
                               "",
                               TYPE_ORG_MDR_POWER_OFF,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_BATTERY,
            g_param_spec_object("battery",
                               "Battery interface",
                               "",
                               TYPE_ORG_MDR_BATTERY,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_LEFT_RIGHT_BATTERY,
            g_param_spec_object("left_right_battery",
                               "Left-Right battery interface",
                               "",
                               TYPE_ORG_MDR_LEFT_RIGHT_BATTERY,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_CRADLE_BATTERY,
            g_param_spec_object("cradle_battery",
                               "Cradle battery interface",
                               "",
                               TYPE_ORG_MDR_CRADLE_BATTERY,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_NOISE_CANCELLING,
            g_param_spec_object("noise_cancelling",
                               "Noise cancelling interface",
                               "",
                               TYPE_ORG_MDR_NOISE_CANCELLING,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_AMBIENT_SOUND_MODE,
            g_param_spec_object("ambient_sound_mode",
                               "Ambient sound mode interface",
                               "",
                               TYPE_ORG_MDR_AMBIENT_SOUND_MODE,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_EQ,
            g_param_spec_object("eq",
                               "Equaliser interface",
                               "",
                               TYPE_ORG_MDR_EQ,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_AUTO_POWER_OFF,
            g_param_spec_object("auto_power_off",
                               "Auto Power-Off interface",
                               "",
                               TYPE_ORG_MDR_AUTO_POWER_OFF,
                               G_PARAM_READABLE));

    g_object_class_install_property(
            object_class,
            PROPERTY_KEY_FUNCTIONS,
            g_param_spec_object("key_functions",
                               "Key Functions interface",
                               "",
                               TYPE_ORG_MDR_KEY_FUNCTIONS,
                               G_PARAM_READABLE));

    signals[SIGNAL_DEVICE_DISCONNECTED] =
            g_signal_new("disconnected",
                          MDR_DEVICE_TYPE,
                          G_SIGNAL_RUN_LAST
                              | G_SIGNAL_NO_RECURSE
                              | G_SIGNAL_NO_HOOKS,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          0);
}

