/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximsrvtmpl.h
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
#ifndef __G_XIM_SRV_TMPL_H__
#define __G_XIM_SRV_TMPL_H__

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>
#include <libgxim/gximcore.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_SRV_TMPL		(g_xim_srv_tmpl_get_type())
#define G_XIM_SRV_TMPL(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_SRV_TMPL, GXimServerTemplate))
#define G_XIM_SRV_TMPL_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_SRV_TMPL, GXimServerTemplateClass))
#define G_IS_XIM_SRV_TMPL(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_SRV_TMPL))
#define G_IS_XIM_SRV_TMPL_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_SRV_TMPL))
#define G_XIM_SRV_TMPL_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_SRV_TMPL, GXimServerTemplateClass))

/**
 * G_XIM_SRV_TMPL_ERROR:
 *
 * Error domain for #GXimServerTemplate. Errors in this domain will be from
 * the #GXimServerTemplateError or #GXimStandardError enumeration.
 * See #GError for more information on error domains.
 */
#define G_XIM_SRV_TMPL_ERROR		(g_xim_srv_tmpl_get_error_quark())

/**
 * GXimServerTemplateError:
 * @G_XIM_SRV_TMPL_ERROR_BEGIN: Unused.
 * @G_XIM_SRV_TMPL_ERROR_SAME_SERVER_IS_RUNNING: Server is already running in the instance.
 * @G_XIM_SRV_TMPL_ERROR_ANOTHER_SERVER_IS_RUNNING: Server in another process is already running with the same name.
 * @G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ACQUIRE_SERVER_OWNER: Unable to acquire the owner of the name.
 * @G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ADD_SERVER: Unable to add the server to %XIM_SERVERS.
 * @G_XIM_SRV_TMPL_ERROR_UNABLE_TO_SEND_PROPERTY_NOTIFY: Failed to send %PropertyNotify.
 *
 * Error codes returned by #GXimServerTemplate functions.
 */
typedef enum {
	G_XIM_SRV_TMPL_ERROR_BEGIN = 128,
	G_XIM_SRV_TMPL_ERROR_SAME_SERVER_IS_RUNNING,
	G_XIM_SRV_TMPL_ERROR_ANOTHER_SERVER_IS_RUNNING,
	G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ACQUIRE_SERVER_OWNER,
	G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ADD_SERVER,
	G_XIM_SRV_TMPL_ERROR_UNABLE_TO_SEND_PROPERTY_NOTIFY,
} GXimServerTemplateError;

/**
 * GXimServerTemplate:
 * @selection_window:
 * @atom_server:
 *
 * An abstract implementation of XIM server class
 **/
typedef struct _GXimServerTemplateClass		GXimServerTemplateClass;

struct _GXimServerTemplate {
	GXimCore   parent_instance;

	GdkWindow *selection_window;
	GdkAtom    atom_server;
#ifndef G_XIM_DISABLE_DEPRECATED
	/* Duplicate entries to GXimCore. and these isn't used anymore. */
	GdkAtom    atom_xim_servers;
	GdkAtom    atom_locales;
	GdkAtom    atom_transport;
#endif /* G_XIM_DISABLE_DEPRECATED */

	/*< private >*/
	gchar      *server_name;
	GHashTable *conn_table;

	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
};

struct _GXimServerTemplateClass {
	GXimCoreClass parent_class;

	/*< private >*/
	void             (* destroy)                 (GXimServerTemplate *server);
	gchar          * (* get_supported_locales)   (GXimServerTemplate *server);
	gchar          * (* get_supported_transport) (GXimServerTemplate *server);
	GXimConnection * (* xconnect)                (GXimServerTemplate *server,
						      GdkEventClient     *event);
	/* Padding for future expansion */
	void (* _g_xim_reserved1) (void);
	void (* _g_xim_reserved2) (void);
	void (* _g_xim_reserved3) (void);
	void (* _g_xim_reserved4) (void);
	void (* _g_xim_reserved5) (void);
	void (* _g_xim_reserved6) (void);
	void (* _g_xim_reserved7) (void);
	void (* _g_xim_reserved8) (void);
};


GType           g_xim_srv_tmpl_get_type                            (void) G_GNUC_CONST;
GQuark          g_xim_srv_tmpl_get_error_quark                     (void);
gboolean        g_xim_srv_tmpl_is_running                          (GXimServerTemplate  *srvtmpl,
                                                                    GError             **error);
gboolean        g_xim_srv_tmpl_take_ownership                      (GXimServerTemplate  *srvtmpl,
                                                                    gboolean             force,
                                                                    GError             **error);
gboolean        g_xim_srv_tmpl_send_selection_notify               (GXimServerTemplate  *srvtmpl,
                                                                    GdkEventSelection   *event,
                                                                    const gchar         *data,
                                                                    gsize                length,
                                                                    GError             **error);
void            g_xim_srv_tmpl_add_connection                      (GXimServerTemplate  *srvtmpl,
                                                                    GXimConnection      *conn,
                                                                    GdkNativeWindow      window);
void            g_xim_srv_tmpl_remove_connection                   (GXimServerTemplate  *srvtmpl,
                                                                    GdkNativeWindow      window);
GXimConnection *g_xim_srv_tmpl_lookup_connection                   (GXimServerTemplate  *srvtmpl,
                                                                    GdkWindow           *window);
GXimConnection *g_xim_srv_tmpl_lookup_connection_with_native_window(GXimServerTemplate  *srvtmpl,
                                                                    GdkNativeWindow      window);


G_END_DECLS

#endif /* __G_XIM_SERVER_H__ */
