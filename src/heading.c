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

#include "heading.h"

struct _MDRHeading
{
    GtkBox parent;

    GtkSeparator* before_sep;
    GtkLabel* label;
    GtkSeparator* after_sep;
};

G_DEFINE_TYPE(MDRHeading,
              mdr_heading,
              GTK_TYPE_BOX)

enum
{
    PROPERTY_NAME = 1,
    PROPERTY_COUNT,
};

static void mdr_heading_init(MDRHeading* heading)
{
    gtk_widget_init_template(GTK_WIDGET(heading));

    gtk_style_context_add_class(
            gtk_widget_get_style_context(GTK_WIDGET(heading->before_sep)),
            "heading-sep");
    gtk_style_context_add_class(
            gtk_widget_get_style_context(GTK_WIDGET(heading->after_sep)),
            "heading-sep");

    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider,
                                        "/org/mdr/manager/style.css");

    gtk_style_context_add_provider(
            gtk_widget_get_style_context(GTK_WIDGET(heading->before_sep)),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_provider(
            gtk_widget_get_style_context(GTK_WIDGET(heading->after_sep)),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static void heading_get_property(GObject* object,
                                 guint prop_id,
                                 GValue* value,
                                 GParamSpec* pspec)
{
    MDRHeading* heading = MDR_HEADING(object);

    switch (prop_id)
    {
        case PROPERTY_NAME:
            g_object_get_property(G_OBJECT(heading->label), "label", value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void heading_set_property(GObject* object,
                                 guint prop_id,
                                 const GValue* value,
                                 GParamSpec* pspec)
{
    MDRHeading* heading = MDR_HEADING(object);

    switch (prop_id)
    {
        case PROPERTY_NAME:
        {
            g_object_set_property(G_OBJECT(heading->label), "label", value);
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void mdr_heading_class_init(MDRHeadingClass* class)
{
    gtk_widget_class_set_template_from_resource(
            GTK_WIDGET_CLASS(class),
            "/org/mdr/manager/heading.ui");

    gtk_widget_class_bind_template_child(
            GTK_WIDGET_CLASS(class),
            MDRHeading,
            before_sep);
    gtk_widget_class_bind_template_child(
            GTK_WIDGET_CLASS(class),
            MDRHeading,
            label);
    gtk_widget_class_bind_template_child(
            GTK_WIDGET_CLASS(class),
            MDRHeading,
            after_sep);

    GObjectClass* object_class = G_OBJECT_CLASS(class);

    object_class->get_property = heading_get_property;
    object_class->set_property = heading_set_property;

    g_object_class_install_property(
            object_class,
            PROPERTY_NAME,
            g_param_spec_string("label",
                                "Label",
                                "The contents of the heading. ",
                                "",
                                G_PARAM_READWRITE));
}

