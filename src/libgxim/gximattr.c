/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximattr.c
 * Copyright (C) 2008 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <glib/gi18n-lib.h>
#include "gximmessage.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximattr.h"


#define G_XIM_ATTR_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_ATTR, GXimAttrPrivate))
#define G_XIM_IM_ATTR_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_IM_ATTR, GXimIMAttrPrivate))
#define G_XIM_IC_ATTR_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_IC_ATTR, GXimICAttrPrivate))


enum {
	PROP_0,
	PROP_ENABLE_ATTRS,
	LAST_PROP
};
enum {
	PROP_IM_0,
	PROP_IM_QUERY_INPUT_STYLE,
	LAST_IM_PROP
};
enum {
	PROP_IC_0,
	PROP_IC_INPUT_STYLE,
	PROP_IC_CLIENT_WINDOW,
	PROP_IC_FOCUS_WINDOW,
	PROP_IC_FILTER_EVENTS,
	PROP_IC_PREEDIT_ATTRIBUTES,
	PROP_IC_STATUS_ATTRIBUTES,
	PROP_IC_FONT_SET,
	PROP_IC_AREA,
	PROP_IC_AREA_NEEDED,
	PROP_IC_COLORMAP,
	PROP_IC_STD_COLORMAP,
	PROP_IC_FOREGROUND,
	PROP_IC_BACKGROUND,
	PROP_IC_BACKGROUND_PIXMAP,
	PROP_IC_SPOT_LOCATION,
	PROP_IC_LINE_SPACE,
	PROP_IC_RESET_STATE,
	PROP_IC_PREEDIT_STATE,
	PROP_IC_CURSOR,
	PROP_IC_SEPARATOR_OF_NESTED_LIST,
	LAST_IC_PROP
};

struct _GXimAttrPrivate {
	GHashTable  *attrs_enabled__named_index;
	GHashTable  *attrs_enabled__id_index;
};
struct _GXimIMAttrPrivate {
	GXimStyles *input_styles;
};
struct _GXimICAttrPrivate {
	gulong             input_styles;
	GdkWindow         *client_window;
	GdkWindow         *focus_window;
	gulong             filter_events;
	GXimNestedList    *preedit_attributes;
	GXimNestedList    *status_attributes;
	GXimFontSet       *fontset;
	GXimRectangle     *area;
	GXimRectangle     *area_needed;
	gulong             colormap;
	gulong             std_colormap;
	gulong             foreground;
	gulong             background;
	gulong             background_pixmap;
	GXimPoint         *spot_location;
	gulong             line_space;
	gulong             reset_state;
	gulong             preedit_state;
	gulong             cursor;
	GXimSepNestedList *separator;
};

G_DEFINE_ABSTRACT_TYPE (GXimAttr, g_xim_attr, G_TYPE_OBJECT);
G_DEFINE_TYPE (GXimIMAttr, g_xim_im_attr, G_TYPE_XIM_ATTR);
G_DEFINE_TYPE (GXimICAttr, g_xim_ic_attr, G_TYPE_XIM_ATTR);

/*
 * Private functions
 */
/* GXimAttr */
static void
g_xim_attr_real_set_property(GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	GXimAttrPrivate *priv = G_XIM_ATTR_GET_PRIVATE (object);
	gchar **strv;
	gint i;

	switch (prop_id) {
	    case PROP_ENABLE_ATTRS:
		    strv = g_value_get_boxed(value);
		    for (i = 0; strv[i] != NULL; i++) {
			    g_hash_table_insert(priv->attrs_enabled__named_index,
						g_strdup(strv[i]),
						GINT_TO_POINTER (i+1));
			    g_hash_table_insert(priv->attrs_enabled__id_index,
						GINT_TO_POINTER (i),
						g_strdup(strv[i]));
		    }
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_attr_real_get_property(GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	GXimAttrPrivate *priv = G_XIM_ATTR_GET_PRIVATE (object);
	GHashTableIter iter;
	gchar **strv;
	gpointer k, v;
	guint i;

	switch (prop_id) {
	    case PROP_ENABLE_ATTRS:
		    strv = g_new0(gchar *, g_hash_table_size(priv->attrs_enabled__named_index));
		    G_XIM_CHECK_ALLOC_WITH_NO_RET (strv);

		    g_hash_table_iter_init(&iter, priv->attrs_enabled__named_index);
		    i = 0;
		    while (g_hash_table_iter_next(&iter, &k, &v)) {
			    strv[i++] = g_strdup(k);
		    }
		    strv[i] = NULL;
		    g_value_set_boxed(value, strv);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_attr_real_finalize(GObject *object)
{
	GXimAttrPrivate *priv = G_XIM_ATTR_GET_PRIVATE (object);

	g_hash_table_destroy(priv->attrs_enabled__named_index);
	g_hash_table_destroy(priv->attrs_enabled__id_index);

	if (G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize) (object);
}

static GSList *
g_xim_attr_real_get_supported_attributes(GXimAttr *attr)
{
	GParamSpec **pspecs;
	guint i, n;
	GSList *retval = NULL;

	pspecs = g_object_class_list_properties(G_OBJECT_GET_CLASS (attr),
						&n);
	for (i = 0; i < n; i++) {
		const gchar *name = g_param_spec_get_name(pspecs[i]);

		retval = g_slist_append(retval,
					g_xim_attr_attribute_is_enabled(attr, name) ?
					g_strdup(name) : NULL);
	}

	return retval;
}

static void
g_xim_attr_class_init(GXimAttrClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (GXimAttrPrivate));

	object_class->set_property = g_xim_attr_real_set_property;
	object_class->get_property = g_xim_attr_real_get_property;
	object_class->finalize     = g_xim_attr_real_finalize;

	klass->get_supported_attributes = g_xim_attr_real_get_supported_attributes;

	/* properties */
	g_object_class_install_property(object_class, PROP_ENABLE_ATTRS,
					g_param_spec_boxed("attrs_enabled",
							   _("Attributes enabled"),
							   _("XIM Attributes allowed to use"),
							   G_TYPE_STRV,
							   G_PARAM_READWRITE));
}

static void
g_xim_attr_init(GXimAttr *attr)
{
	GXimAttrPrivate *priv = G_XIM_ATTR_GET_PRIVATE (attr);

	priv->attrs_enabled__named_index = g_hash_table_new_full(g_str_hash,
								 g_str_equal,
								 g_free,
								 NULL);
	priv->attrs_enabled__id_index = g_hash_table_new_full(g_direct_hash,
							      g_direct_equal,
							      NULL,
							      g_free);
	attr->message = g_xim_message_new();
}

/* GXimIMAttr */
static void
g_xim_im_attr_real_set_property(GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GXimIMAttrPrivate *priv = G_XIM_IM_ATTR_GET_PRIVATE (object);
	const gchar *name = g_param_spec_get_name(pspec);

	if (!g_xim_attr_attribute_is_enabled(G_XIM_ATTR (object),
					     name)) {
		g_xim_message_warning(G_XIM_ATTR (object)->message,
				      "Attribute `%s' isn't supported in this instance.",
				      name);
		return;
	}

	switch (prop_id) {
	    case PROP_IM_QUERY_INPUT_STYLE:
		    g_xim_styles_free(priv->input_styles);
		    priv->input_styles = g_xim_styles_copy(g_value_get_boxed(value));
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_im_attr_real_get_property(GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	GXimIMAttrPrivate *priv = G_XIM_IM_ATTR_GET_PRIVATE (object);
	const gchar *name = g_param_spec_get_name(pspec);

	if (!g_xim_attr_attribute_is_enabled(G_XIM_ATTR (object),
					     name)) {
		g_xim_message_warning(G_XIM_ATTR (object)->message,
				      "Attribute `%s' isn't supported in this instance.",
				      name);
		return;
	}

	switch (prop_id) {
	    case PROP_IM_QUERY_INPUT_STYLE:
		    g_value_set_boxed(value, priv->input_styles);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_im_attr_real_finalize(GObject *object)
{
	GXimIMAttrPrivate *priv = G_XIM_IM_ATTR_GET_PRIVATE (object);

	g_xim_styles_free(priv->input_styles);

	if (G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize) (object);
}

static void
g_xim_im_attr_class_init(GXimIMAttrClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (GXimIMAttrPrivate));

	object_class->set_property = g_xim_im_attr_real_set_property;
	object_class->get_property = g_xim_im_attr_real_get_property;
	object_class->finalize     = g_xim_im_attr_real_finalize;

	/* properties */
	g_object_class_install_property(object_class, PROP_IM_QUERY_INPUT_STYLE,
					g_param_spec_boxed(XNQueryInputStyle,
							   XNQueryInputStyle,
							   XNQueryInputStyle,
							   G_TYPE_XIM_STYLES,
							   G_PARAM_READWRITE));

	/* signals */
}

static void
g_xim_im_attr_init(GXimIMAttr *attr)
{
}

/* GXimICAttr */
static gboolean
g_xim_ic_attr_set_attr(GXimNestedListNode *node,
		       gpointer            data)
{
	GXimICAttr *attr = G_XIM_IC_ATTR (data);

	g_object_set(attr, node->name, node->value, NULL);

	return FALSE;
}

static void
g_xim_ic_attr_real_set_property(GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GXimICAttrPrivate *priv = G_XIM_IC_ATTR_GET_PRIVATE (object);
	const gchar *name = g_param_spec_get_name(pspec);
	GXimNestedList *nested;

	if (!g_xim_attr_attribute_is_enabled(G_XIM_ATTR (object),
					     name)) {
		g_xim_message_warning(G_XIM_ATTR (object)->message,
				      "Attribute `%s' isn't supported in this instance.",
				      name);
		return;
	}

	switch (prop_id) {
	    case PROP_IC_INPUT_STYLE:
		    priv->input_styles = g_value_get_ulong(value);
		    break;
	    case PROP_IC_CLIENT_WINDOW:
		    if (priv->client_window)
			    gdk_drawable_unref(priv->client_window);
		    priv->client_window = g_value_dup_object(value);
		    break;
	    case PROP_IC_FOCUS_WINDOW:
		    if (priv->focus_window)
			    gdk_drawable_unref(priv->focus_window);
		    priv->focus_window = g_value_dup_object(value);
		    break;
	    case PROP_IC_FILTER_EVENTS:
		    priv->filter_events = g_value_get_ulong(value);
		    break;
	    case PROP_IC_PREEDIT_ATTRIBUTES:
		    nested = g_value_get_boxed(value);
		    g_xim_nested_list_foreach(nested, g_xim_ic_attr_set_attr, object);
		    break;
	    case PROP_IC_STATUS_ATTRIBUTES:
		    nested = g_value_get_boxed(value);
		    g_xim_nested_list_foreach(nested, g_xim_ic_attr_set_attr, object);
		    break;
	    case PROP_IC_FONT_SET:
		    g_xim_fontset_free(priv->fontset);
		    priv->fontset = g_xim_fontset_copy(g_value_get_boxed(value));
		    break;
	    case PROP_IC_AREA:
		    g_xim_rectangle_free(priv->area);
		    priv->area = g_xim_rectangle_copy(g_value_get_boxed(value));
		    break;
	    case PROP_IC_AREA_NEEDED:
		    g_xim_rectangle_free(priv->area_needed);
		    priv->area_needed = g_xim_rectangle_copy(g_value_get_boxed(value));
		    break;
	    case PROP_IC_COLORMAP:
		    priv->colormap = g_value_get_ulong(value);
		    break;
	    case PROP_IC_STD_COLORMAP:
		    priv->std_colormap = g_value_get_ulong(value);
		    break;
	    case PROP_IC_FOREGROUND:
		    priv->foreground = g_value_get_ulong(value);
		    break;
	    case PROP_IC_BACKGROUND:
		    priv->background = g_value_get_ulong(value);
		    break;
	    case PROP_IC_BACKGROUND_PIXMAP:
		    priv->background_pixmap = g_value_get_ulong(value);
		    break;
	    case PROP_IC_SPOT_LOCATION:
		    g_xim_point_free(priv->spot_location);
		    priv->spot_location = g_xim_point_copy(g_value_get_boxed(value));
		    break;
	    case PROP_IC_LINE_SPACE:
		    priv->line_space = g_value_get_ulong(value);
		    break;
	    case PROP_IC_RESET_STATE:
		    priv->reset_state = g_value_get_ulong(value);
		    break;
	    case PROP_IC_PREEDIT_STATE:
		    priv->preedit_state = g_value_get_ulong(value);
		    break;
	    case PROP_IC_CURSOR:
		    priv->cursor = g_value_get_ulong(value);
		    break;
	    case PROP_IC_SEPARATOR_OF_NESTED_LIST:
		    g_xim_sep_nested_list_free(priv->separator);
		    priv->separator = g_xim_sep_nested_list_copy(g_value_get_boxed(value));
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_ic_attr_real_get_property(GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	GXimICAttrPrivate *priv = G_XIM_IC_ATTR_GET_PRIVATE (object);
	const gchar *name = g_param_spec_get_name(pspec);

	if (!g_xim_attr_attribute_is_enabled(G_XIM_ATTR (object),
					     name)) {
		g_xim_message_warning(G_XIM_ATTR (object)->message,
				      "Attribute `%s' isn't supported in this instance.",
				      name);
		return;
	}

	switch (prop_id) {
	    case PROP_IC_INPUT_STYLE:
		    g_value_set_ulong(value, priv->input_styles);
		    break;
	    case PROP_IC_CLIENT_WINDOW:
		    g_value_set_object(value, priv->client_window);
		    break;
	    case PROP_IC_FOCUS_WINDOW:
		    g_value_set_object(value, priv->focus_window);
		    break;
	    case PROP_IC_FILTER_EVENTS:
		    g_value_set_ulong(value, priv->filter_events);
		    break;
	    case PROP_IC_FONT_SET:
		    g_value_set_boxed(value, priv->fontset);
		    break;
	    case PROP_IC_AREA:
		    g_value_set_boxed(value, priv->area);
		    break;
	    case PROP_IC_AREA_NEEDED:
		    g_value_set_boxed(value, priv->area_needed);
		    break;
	    case PROP_IC_COLORMAP:
		    g_value_set_ulong(value, priv->colormap);
		    break;
	    case PROP_IC_STD_COLORMAP:
		    g_value_set_ulong(value, priv->std_colormap);
		    break;
	    case PROP_IC_FOREGROUND:
		    g_value_set_ulong(value, priv->foreground);
		    break;
	    case PROP_IC_BACKGROUND:
		    g_value_set_ulong(value, priv->background);
		    break;
	    case PROP_IC_BACKGROUND_PIXMAP:
		    g_value_set_ulong(value, priv->background_pixmap);
		    break;
	    case PROP_IC_SPOT_LOCATION:
		    g_value_set_boxed(value, priv->spot_location);
		    break;
	    case PROP_IC_LINE_SPACE:
		    g_value_set_ulong(value, priv->line_space);
		    break;
	    case PROP_IC_RESET_STATE:
		    g_value_set_ulong(value, priv->reset_state);
		    break;
	    case PROP_IC_PREEDIT_STATE:
		    g_value_set_ulong(value, priv->preedit_state);
		    break;
	    case PROP_IC_CURSOR:
		    g_value_set_ulong(value, priv->cursor);
		    break;
	    case PROP_IC_SEPARATOR_OF_NESTED_LIST:
		    g_value_set_boxed(value, priv->separator);
		    break;
	    case PROP_IC_PREEDIT_ATTRIBUTES:
	    case PROP_IC_STATUS_ATTRIBUTES:
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_ic_attr_real_finalize(GObject *object)
{
	GXimICAttrPrivate *priv = G_XIM_IC_ATTR_GET_PRIVATE (object);

	if (priv->client_window)
		gdk_drawable_unref(priv->client_window);
	if (priv->focus_window)
		gdk_drawable_unref(priv->focus_window);
	g_xim_nested_list_free(priv->preedit_attributes);
	g_xim_nested_list_free(priv->status_attributes);
	g_xim_fontset_free(priv->fontset);
	g_xim_rectangle_free(priv->area);
	g_xim_rectangle_free(priv->area_needed);
	g_xim_point_free(priv->spot_location);
	g_xim_sep_nested_list_free(priv->separator);

	if (G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_attr_parent_class)->finalize) (object);
}

static void
g_xim_ic_attr_class_init(GXimICAttrClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (GXimICAttrPrivate));

	object_class->set_property = g_xim_ic_attr_real_set_property;
	object_class->get_property = g_xim_ic_attr_real_get_property;
	object_class->finalize     = g_xim_ic_attr_real_finalize;

	/* properties */
	g_object_class_install_property(object_class, PROP_IC_INPUT_STYLE,
					g_param_spec_ulong(XNInputStyle,
							   XNInputStyle,
							   XNInputStyle,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_CLIENT_WINDOW,
					g_param_spec_object(XNClientWindow,
							    XNClientWindow,
							    XNClientWindow,
							    GDK_TYPE_WINDOW,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_FOCUS_WINDOW,
					g_param_spec_object(XNFocusWindow,
							    XNFocusWindow,
							    XNFocusWindow,
							    GDK_TYPE_WINDOW,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_FILTER_EVENTS,
					g_param_spec_ulong(XNFilterEvents,
							   XNFilterEvents,
							   XNFilterEvents,
							   0,
							   G_MAXULONG,
							   (1L<<0) | (1L<<1),
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_PREEDIT_ATTRIBUTES,
					g_param_spec_boxed(XNPreeditAttributes,
							   XNPreeditAttributes,
							   XNPreeditAttributes,
							   G_TYPE_XIM_NESTEDLIST,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_STATUS_ATTRIBUTES,
					g_param_spec_boxed(XNStatusAttributes,
							   XNStatusAttributes,
							   XNStatusAttributes,
							   G_TYPE_XIM_NESTEDLIST,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_FONT_SET,
					g_param_spec_boxed(XNFontSet,
							   XNFontSet,
							   XNFontSet,
							   G_TYPE_XIM_FONTSET,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_AREA,
					g_param_spec_boxed(XNArea,
							   XNArea,
							   XNArea,
							   G_TYPE_XIM_RECTANGLE,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_AREA_NEEDED,
					g_param_spec_boxed(XNAreaNeeded,
							   XNAreaNeeded,
							   XNAreaNeeded,
							   G_TYPE_XIM_RECTANGLE,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_COLORMAP,
					g_param_spec_ulong(XNColormap,
							   XNColormap,
							   XNColormap,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_STD_COLORMAP,
					g_param_spec_ulong(XNStdColormap,
							   XNStdColormap,
							   XNStdColormap,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_FOREGROUND,
					g_param_spec_ulong(XNForeground,
							   XNForeground,
							   XNForeground,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_BACKGROUND,
					g_param_spec_ulong(XNBackground,
							   XNBackground,
							   XNBackground,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_BACKGROUND_PIXMAP,
					g_param_spec_ulong(XNBackgroundPixmap,
							   XNBackgroundPixmap,
							   XNBackgroundPixmap,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_SPOT_LOCATION,
					g_param_spec_boxed(XNSpotLocation,
							   XNSpotLocation,
							   XNSpotLocation,
							   G_TYPE_XIM_POINT,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_LINE_SPACE,
					g_param_spec_ulong(XNLineSpace,
							   XNLineSpace,
							   XNLineSpace,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_RESET_STATE,
					g_param_spec_ulong(XNResetState,
							   XNResetState,
							   XNResetState,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_PREEDIT_STATE,
					g_param_spec_ulong(XNPreeditState,
							   XNPreeditState,
							   XNPreeditState,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_CURSOR,
					g_param_spec_ulong(XNCursor,
							   XNCursor,
							   XNCursor,
							   0,
							   G_MAXULONG,
							   0,
							   G_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_IC_SEPARATOR_OF_NESTED_LIST,
					g_param_spec_boxed(XNSeparatorofNestedList,
							   XNSeparatorofNestedList,
							   XNSeparatorofNestedList,
							   G_TYPE_XIM_SEP_NESTEDLIST,
							   G_PARAM_READWRITE));

	/* signals */
}

static void
g_xim_ic_attr_init(GXimICAttr *attr)
{
}

/*
 * Public functions
 */
GQuark
g_xim_attr_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-attr-error");

	return quark;
}

GSList *
g_xim_attr_get_supported_attributes(GXimAttr *attr)
{
	g_return_val_if_fail (G_IS_XIM_ATTR (attr), NULL);
	g_return_val_if_fail (G_XIM_ATTR_GET_CLASS (attr)->get_supported_attributes, NULL);

	return G_XIM_ATTR_GET_CLASS (attr)->get_supported_attributes(attr);
}

guint
g_xim_attr_get_n_supported_attributes(GXimAttr *attr)
{
	GSList *result = g_xim_attr_get_supported_attributes(attr);
	guint retval = 0;

	if (result) {
		retval = g_slist_length(result);
		g_slist_foreach(result, (GFunc)g_free, NULL);
		g_slist_free(result);
	}

	return retval;
}

gboolean
g_xim_attr_attribute_is_enabled(GXimAttr    *attr,
				const gchar *attribute_name)
{
	GXimAttrPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), FALSE);
	g_return_val_if_fail (attribute_name != NULL, FALSE);

	priv = G_XIM_ATTR_GET_PRIVATE (attr);

	return g_hash_table_lookup(priv->attrs_enabled__named_index, attribute_name) != NULL;
}

gint
g_xim_attr_get_attribute_id(GXimAttr    *attr,
			    const gchar *attribute_name)
{
	GXimAttrPrivate *priv;
	guint id;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), -1);
	g_return_val_if_fail (attribute_name != NULL, -1);

	priv = G_XIM_ATTR_GET_PRIVATE (attr);
	id = GPOINTER_TO_INT (g_hash_table_lookup(priv->attrs_enabled__named_index, attribute_name));
	if (id == 0)
		return -1;

	return id - 1;
}

gchar *
g_xim_attr_get_attribute_name(GXimAttr *attr,
			      gint      attribute_id)
{
	GXimAttrPrivate *priv;
	gchar *retval;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), NULL);
	g_return_val_if_fail (attribute_id >= 0, NULL);

	priv = G_XIM_ATTR_GET_PRIVATE (attr);
	retval = g_hash_table_lookup(priv->attrs_enabled__id_index, GINT_TO_POINTER (attribute_id));

	return g_strdup(retval);
}

gpointer
g_xim_attr_get_value_by_id(GXimAttr *attr,
			   gint      attribute_id)
{
	gpointer retval = NULL;
	gchar *name;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), NULL);
	g_return_val_if_fail (attribute_id >= 0, NULL);

	name = g_xim_attr_get_attribute_name(attr, attribute_id);
	retval = g_xim_attr_get_value_by_name(attr, name);
	g_free(name);

	return retval;
}

gpointer
g_xim_attr_get_value_by_name(GXimAttr    *attr,
			     const gchar *attribute_name)
{
	gpointer retval = NULL;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), NULL);
	g_return_val_if_fail (attribute_name != NULL, NULL);

	g_object_get(attr, attribute_name, &retval, NULL);

	return retval;
}

GType
g_xim_attr_get_gtype_by_id(GXimAttr *attr,
			   gint      attribute_id)
{
	gchar *name;
	GType retval;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), G_TYPE_INVALID);
	g_return_val_if_fail (attribute_id >= 0, G_TYPE_INVALID);

	name = g_xim_attr_get_attribute_name(attr, attribute_id);
	retval = g_xim_attr_get_gtype_by_name(attr, name);
	g_free(name);

	return retval;
}

GType
g_xim_attr_get_gtype_by_name(GXimAttr    *attr,
			     const gchar *attribute_name)
{
	GType retval = G_TYPE_INVALID;
	GParamSpec *pspec;

	g_return_val_if_fail (G_IS_XIM_ATTR (attr), G_TYPE_INVALID);
	g_return_val_if_fail (attribute_name != NULL, G_TYPE_INVALID);

	pspec = g_object_class_find_property(G_OBJECT_GET_CLASS (attr),
					     attribute_name);
	if (pspec) {
		retval = G_PARAM_SPEC_VALUE_TYPE (pspec);
	}

	return retval;
}

void
g_xim_attr_set_raw_attr(GXimAttr    *attr,
			GXimRawAttr *raw)
{
	GType gtype;
	GXimValueType vtype;
	GXimAttrPrivate *priv;

	g_return_if_fail (G_IS_XIM_ATTR (attr));
	g_return_if_fail (raw != NULL);
	g_return_if_fail (raw->base.vtype != G_XIM_TYPE_INVALID);

	priv = G_XIM_ATTR_GET_PRIVATE (attr);
	gtype = g_xim_attr_get_gtype_by_name(attr, raw->attribute_name->str);
	vtype = g_xim_gtype_to_value_type(gtype);
	if (vtype != raw->base.vtype) {
		g_xim_message_bug(attr->message,
				  "Attribute %s is defined as different type: expected type: %s, actual type: %s",
				  raw->attribute_name->str,
				  g_xim_value_type_name(vtype),
				  g_xim_value_type_name(raw->base.vtype));
	}

	g_hash_table_insert(priv->attrs_enabled__named_index,
			    g_strdup(raw->attribute_name->str),
			    GUINT_TO_POINTER (raw->base.id + 1));
	g_hash_table_insert(priv->attrs_enabled__id_index,
			    GUINT_TO_POINTER ((guint)raw->base.id),
			    g_strdup(raw->attribute_name->str));
}

gsize
g_xim_attr_put_attribute_to_stream(GXimProtocol       *proto,
				   GXimAttr           *attr,
				   guint               attribute_id,
				   GCancellable       *cancellable,
				   GError            **error)
{
	GType gtype;
	GXimValueType vtype;
	gpointer value;
	gchar *name;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (G_IS_XIM_ATTR (attr), 0);
	g_return_val_if_fail (error != NULL, 0);

	gtype = g_xim_attr_get_gtype_by_id(attr, attribute_id);
	vtype = g_xim_gtype_to_value_type(gtype);
	if (vtype == G_XIM_TYPE_INVALID) {
		g_set_error(error, G_XIM_ATTR_ERROR,
			    G_XIM_ATTR_ERROR_UNABLE_TO_CONVERT_TYPE | G_XIM_NOTICE_BUG,
			    "Unable to convert GType %s to any XIM value type. this may be likely a libgxim bug. please report a bug.",
			    g_type_name(gtype));
		return 0;
	}
	value = g_xim_attr_get_value_by_id(attr, attribute_id);
	name = g_xim_attr_get_attribute_name(attr, attribute_id);
	g_xim_message_debug(attr->message, "proto/attr",
			    "%d: Attribute: %s [%s]",
			    attribute_id, name, g_xim_value_type_name(vtype));
	g_free(name);

	return g_xim_protocol_send_format(proto, cancellable, error, 3,
					  G_XIM_TYPE_WORD, attribute_id,
					  G_XIM_TYPE_MARKER_N_BYTES_2, vtype,
					  vtype, value);
}

gpointer
g_xim_attr_get_attribute_from_stream(GXimProtocol      *proto,
				     GXimAttr          *attr,
				     GDataInputStream  *stream,
				     GCancellable      *cancellable,
				     GError           **error)
{
	guint16 attr_id;
	GType gtype;
	GXimValueType vtype;
	gpointer value = NULL;
	gchar *name;
	GXimAttribute *new_attr;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_XIM_ATTR (attr), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					1,
					G_XIM_TYPE_WORD, &attr_id))
		return NULL;

	gtype = g_xim_attr_get_gtype_by_id(attr, attr_id);
	if (gtype == G_TYPE_INVALID) {
		g_set_error(error, G_XIM_ATTR_ERROR,
			    G_XIM_ATTR_ERROR_NO_SUCH_PROPERTY | G_XIM_NOTICE_WARNING,
			    "Attribute ID %d isn't available in %s",
			    attr_id, g_type_name(G_TYPE_FROM_INSTANCE (attr)));
		return NULL;
	}
	vtype = g_xim_gtype_to_value_type(gtype);
	if (vtype == G_XIM_TYPE_INVALID) {
		g_set_error(error, G_XIM_ATTR_ERROR,
			    G_XIM_ATTR_ERROR_UNABLE_TO_CONVERT_TYPE | G_XIM_NOTICE_BUG,
			    "Unable to convert GType %s to any XIM value type. this may be likely a libgxim bug. please report a bug.",
			    g_type_name(gtype));
		return NULL;
	}
	name = g_xim_attr_get_attribute_name(attr, attr_id);
	g_xim_message_debug(attr->message, "proto/attr",
			    "%d: Attribute: %s [%s]",
			    attr_id, name, g_xim_value_type_name(vtype));
	g_free(name);
	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					2,
					G_XIM_TYPE_MARKER_N_BYTES_2, vtype,
					vtype, &value))
		return NULL;

	new_attr = g_xim_attribute_new_with_value(attr_id, vtype, value);
	G_XIM_CHECK_ALLOC (new_attr, NULL);
	g_xim_free_by_gtype(gtype, value);

	return new_attr;
}

/* GXimIMAttr */
GXimIMAttr *
g_xim_im_attr_new(const gchar *attrs)
{
	gchar **strv;

	g_return_val_if_fail (attrs != NULL, NULL);

	strv = g_strsplit(attrs, ",", -1);

	return G_XIM_IM_ATTR (g_object_new(G_TYPE_XIM_IM_ATTR,
					   "attrs_enabled", strv,
					   NULL));
}

void
g_xim_im_attr_set_input_styles(GXimIMAttr *attr,
			       GXimStyles *style)
{
	g_return_if_fail (G_IS_XIM_IM_ATTR (attr));
	g_return_if_fail (style != NULL);

	g_object_set(attr,
		     XNQueryInputStyle, style,
		     NULL);
}

const GXimStyles *
g_xim_im_attr_get_input_styles(GXimIMAttr *attr)
{
	g_return_val_if_fail (G_IS_XIM_IM_ATTR (attr), NULL);

	return (GXimStyles *)g_xim_attr_get_value_by_name(G_XIM_ATTR (attr), XNQueryInputStyle);
}

/* GXimICAttr */
GXimICAttr *
g_xim_ic_attr_new(const gchar *attrs)
{
	gchar **strv;

	g_return_val_if_fail (attrs != NULL, NULL);

	strv = g_strsplit(attrs, ",", -1);

	return G_XIM_IC_ATTR (g_object_new(G_TYPE_XIM_IC_ATTR,
					   "attrs_enabled", strv,
					   NULL));
}
