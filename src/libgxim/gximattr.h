/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximattr.h
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
#ifndef __G_XIM_ATTR_H__
#define __G_XIM_ATTR_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_ATTR			(g_xim_attr_get_type())
#define G_XIM_ATTR(_o_)			(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_ATTR, GXimAttr))
#define G_XIM_ATTR_CLASS(_c_)		(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_ATTR, GXimAttrClass))
#define G_IS_XIM_ATTR(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_ATTR))
#define G_IS_XIM_ATTR_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_ATTR))
#define G_XIM_ATTR_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_ATTR, GXimAttrClass))
#define G_TYPE_XIM_IM_ATTR		(g_xim_im_attr_get_type())
#define G_XIM_IM_ATTR(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_IM_ATTR, GXimIMAttr))
#define G_XIM_IM_ATTR_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_IM_ATTR, GXimIMAttrClass))
#define G_IS_XIM_IM_ATTR(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_IM_ATTR))
#define G_IS_XIM_IM_ATTR_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_IM_ATTR))
#define G_XIM_IM_ATTR_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_IM_ATTR, GXimIMAttrClass))
#define G_TYPE_XIM_IC_ATTR		(g_xim_ic_attr_get_type())
#define G_XIM_IC_ATTR(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_IC_ATTR, GXimICAttr))
#define G_XIM_IC_ATTR_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_IC_ATTR, GXimICAttrClass))
#define G_IS_XIM_IC_ATTR(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_IC_ATTR))
#define G_IS_XIM_IC_ATTR_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_IC_ATTR))
#define G_XIM_IC_ATTR_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_IC_ATTR, GXimICAttrClass))

#define G_XIM_ATTR_ERROR		(g_xim_attr_get_error_quark())

/* im-attributes */
#define XNQueryInputStyle		"queryInputStyle"

/* ic-attributes */
#define XNClientWindow			"clientWindow"
#define XNInputStyle			"inputStyle"
#define XNFocusWindow			"focusWindow"
#define XNFilterEvents			"filterEvents"
#define XNPreeditAttributes		"preeditAttributes"
#define XNStatusAttributes		"statusAttributes"
#define XNArea				"area"
#define XNAreaNeeded			"areaNeeded"
#define XNSpotLocation			"spotLocation"
#define XNColormap			"colorMap"
#define XNStdColormap			"stdColorMap"
#define XNForeground			"foreground"
#define XNBackground			"background"
#define XNBackgroundPixmap		"backgroundPixmap"
#define XNFontSet			"fontSet"
#define XNLineSpace			"lineSpace"
#define XNCursor			"cursor"
#define XNResetState			"resetState"
#define XNPreeditState			"preeditState"
#define XNSeparatorofNestedList		"separatorofNestedList"

/**
 * GXimAttr:
 * @parent: a #GObject.
 *
 * An abstract implementation of XIM Attributes class
 **/
typedef struct _GXimAttrClass		GXimAttrClass;
typedef struct _GXimAttrPrivate		GXimAttrPrivate;
typedef enum _GXimAttrError		GXimAttrError;

struct _GXimAttr {
	GObject      parent_instance;
	GXimMessage *message;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
};
struct _GXimAttrClass {
	GObjectClass  parent_class;

	GSList   * (* get_supported_attributes) (GXimAttr           *attr);

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
};
enum _GXimAttrError {
	G_XIM_ATTR_ERROR_BEGIN = 128,
	G_XIM_ATTR_ERROR_NO_SUCH_PROPERTY,
	G_XIM_ATTR_ERROR_UNABLE_TO_CONVERT_TYPE,
};

GType     g_xim_attr_get_type                  (void) G_GNUC_CONST;
GQuark    g_xim_attr_get_error_quark           (void);
GSList   *g_xim_attr_get_supported_attributes  (GXimAttr    *attr);
guint     g_xim_attr_get_n_supported_attributes(GXimAttr    *attr);
gboolean  g_xim_attr_attribute_is_enabled      (GXimAttr    *attr,
                                                const gchar *attribute_name);
gint      g_xim_attr_get_attribute_id          (GXimAttr    *attr,
                                                const gchar *attribute_name);
gchar    *g_xim_attr_get_attribute_name        (GXimAttr    *attr,
                                                gint         attribute_id) G_GNUC_MALLOC;
gpointer  g_xim_attr_get_value_by_id           (GXimAttr    *attr,
                                                gint         attribute_id);
gpointer  g_xim_attr_get_value_by_name         (GXimAttr    *attr,
                                                const gchar *attribute_name);
GType     g_xim_attr_get_gtype_by_id           (GXimAttr    *attr,
                                                gint         attribute_id);
GType     g_xim_attr_get_gtype_by_name         (GXimAttr    *attr,
                                                const gchar *attribute_name);
void      g_xim_attr_set_raw_attr              (GXimAttr    *attr,
                                                GXimRawAttr *raw);

/**
 * GXimIMAttr:
 * @parent: a #GXimAttr.
 *
 * An implementation of XIM IM Attributes class
 **/
typedef struct _GXimIMAttrClass		GXimIMAttrClass;
typedef struct _GXimIMAttrPrivate	GXimIMAttrPrivate;

struct _GXimIMAttr {
	GXimAttr  parent_instance;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
	void (* reserved8) (void);
};

struct _GXimIMAttrClass {
	GXimAttrClass  parent_class;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
	void (* reserved8) (void);
};


GType             g_xim_im_attr_get_type                (void) G_GNUC_CONST;
GXimIMAttr       *g_xim_im_attr_new                     (const gchar       *attrs);
void              g_xim_im_attr_set_input_styles        (GXimIMAttr        *attr,
                                                         GXimStyles        *style);
const GXimStyles *g_xim_im_attr_get_input_styles        (GXimIMAttr        *attr);

/**
 * GXimICAttr:
 * @parent: a #GXimAttr.
 *
 * An implementation of XIM IC Attributes class
 **/
typedef struct _GXimICAttrClass		GXimICAttrClass;
typedef struct _GXimICAttrPrivate	GXimICAttrPrivate;

struct _GXimICAttr {
	GXimAttr  parent_instance;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
	void (* reserved8) (void);
};

struct _GXimICAttrClass {
	GXimAttrClass  parent_class;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
	void (* reserved8) (void);
};


GType       g_xim_ic_attr_get_type                (void) G_GNUC_CONST;
GXimICAttr *g_xim_ic_attr_new                     (const gchar       *attrs);


G_END_DECLS

#endif /* __G_XIM_ATTR_H__ */
