/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximconnection.h
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA  02110-1301  USA
 */
#ifndef __G_XIM_CONNECTION_H__
#define __G_XIM_CONNECTION_H__

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_CONNECTION		(g_xim_connection_get_type())
#define G_XIM_CONNECTION(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_CONNECTION, GXimConnection))
#define G_XIM_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_CONNECTION, GXimConnectionClass))
#define G_IS_XIM_CONNECTION(_o_)	(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_CONNECTION))
#define G_IS_XIM_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_CONNECTION))
#define G_XIM_CONNECTION_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_CONNECTION, GXimConnectionClass))

/**
 * GXimConnection:
 * @parent: a #GObject.
 *
 * An abstract implementation of connection class
 **/
typedef struct _GXimConnectionClass	GXimConnectionClass;

struct _GXimConnection {
	GObject      parent_instance;

	/*< private >*/
	GSList     *proto_signals;
	guint16     imid;
	GXimIMAttr *imattr;
	GXimICAttr *default_icattr;
	GArray     *encodings;
	GArray     *encoding_details;
	guint16     encoding_category;
	gint16      encoding_index;

	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
};

struct _GXimConnectionClass {
	GObjectClass  parent_class;

	/*< private >*/
	void (* protocol_init)  (GXimProtocol  *proto);
	void (* transport_init) (GXimTransport *trans);

	/* Padding for future expansion */
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
	void (* reserved6) (void);
	void (* reserved7) (void);
	void (* reserved8) (void);
	void (* reserved9) (void);
	void (* reserved10) (void);
};


GType    g_xim_connection_get_type         (void) G_GNUC_CONST;
void     g_xim_connection_setup            (GXimConnection *conn);
gboolean g_xim_connection_cmd_error        (GXimConnection *conn,
					    guint16         imid,
					    guint16         icid,
					    GXimErrorMask   flag,
					    GXimErrorCode   error_code,
					    guint16         detail,
					    const gchar    *error_message);
gboolean g_xim_connection_cmd_auth_ng      (GXimConnection *conn);
gboolean g_xim_connection_cmd_forward_event(GXimConnection *conn,
					    guint16         imid,
					    guint16         icid,
					    guint16         flag,
					    GdkEvent       *event);
gboolean g_xim_connection_cmd_sync_reply   (GXimConnection *conn,
					    guint16         imid,
					    guint16         icid);


G_END_DECLS

#endif /* __G_XIM_CONNECTION_H__ */
