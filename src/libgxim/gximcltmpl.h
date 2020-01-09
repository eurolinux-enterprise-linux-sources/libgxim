/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximcltmpl.h
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
#ifndef __G_XIM_CL_TMPL_H__
#define __G_XIM_CL_TMPL_H__

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>
#include <libgxim/gximcore.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_CL_TMPL		(g_xim_cl_tmpl_get_type())
#define G_XIM_CL_TMPL(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_CL_TMPL, GXimClientTemplate))
#define G_XIM_CL_TMPL_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_CL_TMPL, GXimClientTemplateClass))
#define G_IS_XIM_CL_TMPL(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_CL_TMPL))
#define G_IS_XIM_CL_TMPL_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_CL_TMPL))
#define G_XIM_CL_TMPL_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_CL_TMPL, GXimClientTemplateClass))

/**
 * G_XIM_CL_TMPL_ERROR:
 *
 * Error domain for #GXimClientTemplate. Errors in this domain will be from
 * the #GXimClientTemplateError or #GXimStandardError enumeration.
 * See #GError for more information on error domains.
 */
#define G_XIM_CL_TMPL_ERROR		(g_xim_cl_tmpl_get_error_quark())

/**
 * GXC_NONE:
 *
 * a state not yet starting to negotiate the connection.
 */
#define GXC_NONE	0
/**
 * GXC_NEGOTIATING:
 *
 * a statte that is negotiating the connection to XIM server.
 */
#define GXC_NEGOTIATING	1
/**
 * GXC_ESTABLISHED:
 *
 * the connection has been established and ready to do something with
 * XIM protocol.
 */
#define GXC_ESTABLISHED	2


/**
 * GXimClientTemplateError:
 * @G_XIM_CL_TMPL_ERROR_BEGIN: Unused.
 * @G_XIM_CL_TMPL_ERROR_INVALID_CONNECTION_TYPE: The connection type isn't inherited from #GXimConnection.
 *
 * Error codes returned by #GXimClientTemplate functions.
 */
typedef enum {
	G_XIM_CL_TMPL_ERROR_BEGIN = 128,
	G_XIM_CL_TMPL_ERROR_INVALID_CONNECTION_TYPE,
} GXimClientTemplateError;

/**
 * GXimClientTemplate:
 * @parent: a #GXimCore.
 *
 * An implementation of XIM client class
 **/
typedef struct _GXimClientTemplateClass		GXimClientTemplateClass;

struct _GXimClientTemplate {
	GXimCore        parent_instance;
	GdkAtom         atom_xim_servers;
	GdkAtom         atom_server;
	GXimConnection *connection;

	/*< private >*/
	GdkWindow *server_window;
	guint      is_locale_initialized;
	guint      is_transport_initialized;
	guint      is_connection_initialized;

	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
};

struct _GXimClientTemplateClass {
	GXimCoreClass  parent_class;

	/*< private >*/
	void     (* notify_locales)   (GXimClientTemplate  *cltmpl,
				       gchar              **locales);
	void     (* notify_transport) (GXimClientTemplate  *cltmpl,
				       gchar              **transport);
	gboolean (* xconnect)         (GXimClientTemplate  *cltmpl,
				       GdkEventClient      *event);

	/* Padding for future expansion */
	void (* _g_xim_reserved1) (void);
	void (* _g_xim_reserved2) (void);
	void (* _g_xim_reserved3) (void);
	void (* _g_xim_reserved4) (void);
	void (* _g_xim_reserved5) (void);
	void (* _g_xim_reserved6) (void);
	void (* _g_xim_reserved7) (void);
	void (* _g_xim_reserved8) (void);
	void (* reserved9) (void);
};


GQuark   g_xim_cl_tmpl_get_error_quark       (void);
GType    g_xim_cl_tmpl_get_type              (void) G_GNUC_CONST;
gboolean g_xim_cl_tmpl_is_initialized        (GXimClientTemplate  *cltmpl);
gboolean g_xim_cl_tmpl_is_pending_negotiation(GXimClientTemplate  *cltmpl);
gboolean g_xim_cl_tmpl_start_negotiation     (GXimClientTemplate  *cltmpl,
					      GError             **error);
gboolean g_xim_cl_tmpl_send_selection_request(GXimClientTemplate  *cltmpl,
					      GdkAtom              atom,
					      GError             **error);
gboolean g_xim_cl_tmpl_connect_to_server     (GXimClientTemplate  *cltmpl,
					      GError             **error);


G_END_DECLS

#endif /* __G_XIM_CL_TMPL_H__ */
