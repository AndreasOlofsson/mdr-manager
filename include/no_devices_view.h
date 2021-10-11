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

#ifndef __NO_DEVICES_VIEW_H__
#define __NO_DEVICES_VIEW_H__

#include <gtk/gtk.h>

#define MDR_NO_DEVICES_VIEW_TYPE (mdr_no_devices_view_get_type())
G_DECLARE_FINAL_TYPE(MDRNoDevicesView,
                     mdr_no_devices_view,
                     MDR, NO_DEVICES_VIEW,
                     GtkBox)

#endif /* __NO_DEVICES_VIEW_H__ */
