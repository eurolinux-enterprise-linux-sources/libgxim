/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximsrvconn.h
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
#ifndef __G_XIM_SRV_CONN_H__
#define __G_XIM_SRV_CONN_H__

#include <glib.h>
#include <glib-object.h>
#include <libgxim/gximtypes.h>
#include <libgxim/gximconnection.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_SERVER_CONNECTION		(g_xim_server_connection_get_type())
#define G_XIM_SERVER_CONNECTION(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_SERVER_CONNECTION, GXimServerConnection))
#define G_XIM_SERVER_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_SERVER_CONNECTION, GXimServerConnectionClass))
#define G_IS_XIM_SERVER_CONNECTION(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_SERVER_CONNECTION))
#define G_IS_XIM_SERVER_CONNECTION_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_SERVER_CONNECTION))
#define G_XIM_SERVER_CONNECTION_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_SERVER_CONNECTION, GXimServerConnectionClass))

/**
 * GXimServerConnection:
 * @parent: a #GXimConnection.
 *
 * An implementation of server connection class
 **/
typedef struct _GXimServerConnectionClass	GXimServerConnectionClass;

struct _GXimServerConnection {
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

struct _GXimServerConnectionClass {
	GXimConnectionClass  parent_class;

	/*< private >*/
	gboolean (* is_auth_required) (GXimServerConnection *conn);

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
};


GType    g_xim_server_connection_get_type                      (void) G_GNUC_CONST;
gboolean g_xim_server_connection_cmd_connect_reply             (GXimServerConnection *conn,
								guint16               major_version,
								guint16               minor_version);
gboolean g_xim_server_connection_cmd_disconnect_reply          (GXimServerConnection *conn);
gboolean g_xim_server_connection_cmd_open_reply                (GXimServerConnection *conn,
								guint16               imid,
								const GSList         *imattr_list,
								const GSList         *icattr_list);
gboolean g_xim_server_connection_cmd_close_reply               (GXimServerConnection *conn,
								guint16               imid);
gboolean g_xim_server_connection_cmd_register_triggerkeys      (GXimServerConnection *conn,
								guint16               imid,
								const GSList         *onkeys,
								const GSList         *offkeys);
gboolean g_xim_server_connection_cmd_trigger_notify_reply      (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_set_event_mask            (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								guint32               forward_event,
								guint32               sync_event);
gboolean g_xim_server_connection_cmd_encoding_negotiation_reply(GXimServerConnection *conn,
								guint16               imid,
								guint16               category,
								gint16                index_);
gboolean g_xim_server_connection_cmd_query_extension_reply     (GXimServerConnection *conn,
								guint16               imid,
								const GSList         *extensions);
gboolean g_xim_server_connection_cmd_set_im_values_reply       (GXimServerConnection *conn,
								guint16               imid);
gboolean g_xim_server_connection_cmd_get_im_values_reply       (GXimServerConnection *conn,
								guint16               imid,
								const GSList         *attributes);
gboolean g_xim_server_connection_cmd_create_ic_reply           (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_destroy_ic_reply          (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_set_ic_values_reply       (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_get_ic_values_reply       (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								const GSList         *attributes);
gboolean g_xim_server_connection_cmd_sync                      (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_commit                    (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								guint16               flag,
								guint32               keysym,
								GString              *string);
gboolean g_xim_server_connection_cmd_reset_ic_reply            (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								const GString        *preedit_string);
gboolean g_xim_server_connection_cmd_preedit_start             (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_preedit_draw              (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								GXimPreeditDraw      *draw);
gboolean g_xim_server_connection_cmd_preedit_caret             (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								GXimPreeditCaret     *caret);
gboolean g_xim_server_connection_cmd_preedit_done              (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_status_start              (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);
gboolean g_xim_server_connection_cmd_status_draw               (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid,
								GXimStatusDraw       *draw);
gboolean g_xim_server_connection_cmd_status_done               (GXimServerConnection *conn,
								guint16               imid,
								guint16               icid);


G_END_DECLS

#endif /* __G_XIM_SRV_CONN_H__ */
