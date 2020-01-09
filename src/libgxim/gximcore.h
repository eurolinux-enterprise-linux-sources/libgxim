/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximcore.h
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
#ifndef __G_XIM_CORE_H__
#define __G_XIM_CORE_H__

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_CORE			(g_xim_core_get_type())
#define G_XIM_CORE(_o_)			(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_CORE, GXimCore))
#define G_XIM_CORE_CLASS(_c_)		(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_CORE, GXimCoreClass))
#define G_IS_XIM_CORE(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_CORE))
#define G_IS_XIM_CORE_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_CORE))
#define G_XIM_CORE_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_CORE, GXimCoreClass))

#ifndef G_XIM_DISABLE_DEPRECATED
#define G_XIM_CORE_ERROR		g_xim_core_get_error_quark()
#endif

/**
 * GXimCore:
 * @message: a #GXimMessage.
 *
 * An abstract implementation of XIM protocol event handling class.
 **/
typedef struct _GXimCoreClass		GXimCoreClass;

struct _GXimCore {
	GObject      parent_instance;
	GXimMessage *message;
	GdkAtom      atom_xim_xconnect;
	GdkAtom      atom_xim_servers;
	GdkAtom      atom_locales;
	GdkAtom      atom_transport;

	/*< private >*/
	void (* reserved1) (void);
	void (* reserved2) (void);
	void (* reserved3) (void);
	void (* reserved4) (void);
	void (* reserved5) (void);
};

struct _GXimCoreClass {
	GObjectClass parent_class;

	/*< private >*/
	GdkFilterReturn (* translate_events) (GXimCore       *core,
					      GdkXEvent      *gdk_xevent,
					      GdkEvent       *event);
	void            (* setup_connection) (GXimCore       *core,
					      GXimConnection *conn);

	/* XIM events */
	gboolean (* selection_request_event) (GXimCore          *core,
					      GdkEventSelection *event);
	gboolean (* selection_clear_event)   (GXimCore          *core,
					      GdkEventSelection *event);
	gboolean (* selection_notify_event)  (GXimCore          *core,
					      GdkEventSelection *event);
	gboolean (* expose_event)            (GXimCore          *core,
					      GdkEventExpose    *event);
	gboolean (* destroy_event)           (GXimCore          *core,
					      GdkEventAny       *event);
	gboolean (* client_event)            (GXimCore          *core,
					      GdkEventClient    *event);
	void     (* keys_changed)            (GXimCore          *core);
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
	void (* reserved10) (void);
	void (* reserved11) (void);
};


GType       g_xim_core_get_type                     (void) G_GNUC_CONST;
void        g_xim_core_watch_event                  (GXimCore       *core,
                                                     GdkWindow      *window);
void        g_xim_core_unwatch_event                (GXimCore       *core,
                                                     GdkWindow      *window);
GdkDisplay *g_xim_core_get_display                  (GXimCore       *core);
GdkWindow  *g_xim_core_get_selection_window         (GXimCore       *core);
GType       g_xim_core_get_connection_gtype         (GXimCore       *core);
GSList     *g_xim_core_get_protocol_signal_connector(GXimCore       *core) G_GNUC_CONST;
void        g_xim_core_setup_connection             (GXimCore       *core,
						     GXimConnection *conn);
void        g_xim_core_add_client_message_filter    (GXimCore       *core,
						     GdkAtom         atom);
void        g_xim_core_remove_client_message_filter (GXimCore       *core,
						     GdkAtom         atom);
gboolean    g_xim_core_lookup_client_message_filter (GXimCore       *core,
						     GdkAtom         atom);
#ifndef G_XIM_DISABLE_DEPRECATED
GQuark      g_xim_core_get_error_quark              (void);
#endif

G_END_DECLS

#endif /* __G_XIM_CORE_H__ */
