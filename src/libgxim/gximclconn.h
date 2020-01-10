/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximclconn.h
 * Copyright (C) 2008-2011 Akira TAGOH
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
#ifndef __G_XIM_CL_CONN_H__
#define __G_XIM_CL_CONN_H__

#include <glib.h>
#include <glib-object.h>
#include <libgxim/gximtypes.h>
#include <libgxim/gximconnection.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_CLIENT_CONNECTION		(g_xim_client_connection_get_type())
#define G_XIM_CLIENT_CONNECTION(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_CLIENT_CONNECTION, GXimClientConnection))
#define G_XIM_CLIENT_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_CLIENT_CONNECTION, GXimClientConnectionClass))
#define G_IS_XIM_CLIENT_CONNECTION(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_CLIENT_CONNECTION))
#define G_IS_XIM_CLIENT_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_CLIENT_CONNECTION))
#define G_XIM_CLIENT_CONNECTION_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_CLIENT_CONNECTION, GXimClientConnectionClass))

/**
 * GXimClientConnection:
 * @parent: a #GXimConnection.
 *
 * An implementation of client connection class
 **/
typedef struct _GXimClientConnectionClass	GXimClientConnectionClass;

struct _GXimClientConnection {
	GXimConnection  parent_instance;

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

struct _GXimClientConnectionClass {
	GXimConnectionClass  parent_class;

	/*< private >*/
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
	void (* reserved11) (void);
	void (* reserved12) (void);
};


GType    g_xim_client_connection_get_type                (void) G_GNUC_CONST;
gboolean g_xim_client_connection_cmd_connect             (GXimClientConnection *conn,
							  guint16               protocol_major_version,
							  guint16               protocol_minor_version,
							  GSList               *auth_list,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_disconnect          (GXimClientConnection *conn,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_open_im             (GXimClientConnection *conn,
							  const GXimStr        *locale,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_close_im            (GXimClientConnection *conn,
							  guint16               imid,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_trigger_notify      (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  guint32               flag,
							  guint32               index_,
							  guint32               event_mask,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_encoding_negotiation(GXimClientConnection *conn,
							  guint16               imid,
							  GSList               *encodings,
							  GSList               *details,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_query_extension     (GXimClientConnection *conn,
							  guint16               imid,
							  const GSList         *extensions,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_set_im_values       (GXimClientConnection *conn,
							  guint16               imid,
							  const GSList         *attributes,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_get_im_values       (GXimClientConnection *conn,
							  guint16               imid,
							  const GSList         *attr_id,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_create_ic           (GXimClientConnection *conn,
							  guint16               imid,
							  const GSList         *attributes,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_destroy_ic          (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_set_ic_values       (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  const GSList         *attributes,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_get_ic_values       (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  const GSList         *attr_id,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_set_ic_focus        (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid);
gboolean g_xim_client_connection_cmd_unset_ic_focus      (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid);
gboolean g_xim_client_connection_cmd_sync                (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_reset_ic            (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  gboolean              is_async);
gboolean g_xim_client_connection_cmd_preedit_start_reply (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  gint32                return_value);
gboolean g_xim_client_connection_cmd_preedit_caret_reply (GXimClientConnection *conn,
							  guint16               imid,
							  guint16               icid,
							  guint32               position);


G_END_DECLS

#endif /* __G_XIM_CL_CONN_H__ */
