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

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <gio/gio.h>

#define MDR_DEVICE_TYPE (mdr_device_get_type())
G_DECLARE_FINAL_TYPE(MDRDevice,
                     mdr_device,
                     MDR,
                     DEVICE,
                     GObject)

MDRDevice* mdr_device_new(GDBusConnection*, const gchar* object_path);

const gchar* mdr_device_get_object_path(MDRDevice*);

#endif /* __DEVICE_H__ */

