/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximtransport.h
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
#ifndef __G_XIM_TRANSPORT_H__
#define __G_XIM_TRANSPORT_H__

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_XIM_TRANSPORT_SIZE		20
#define G_XIM_TRANSPORT_MAX		20

#define G_TYPE_XIM_TRANSPORT		(g_xim_transport_get_type())
#define G_XIM_TRANSPORT(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_TRANSPORT, GXimTransport))
#define G_IS_XIM_TRANSPORT(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_TRANSPORT))
#define G_XIM_TRANSPORT_GET_IFACE(_o_)	(G_TYPE_INSTANCE_GET_INTERFACE ((_o_), G_TYPE_XIM_TRANSPORT, GXimTransportIface))

typedef struct _GXimTransportIface	GXimTransportIface;
typedef struct _GXimTransportPrivate	GXimTransportPrivate;

typedef enum {
	G_XIM_DIR_RIGHT = 0,
	G_XIM_DIR_LEFT
} GXimDirection;

struct _GXimTransportIface {
	GTypeInterface  parent_iface;

	GXimMessage    *message;
	GdkAtom         atom_xim_protocol;
	GdkAtom         atom_xim_moredata;

	/*< private >*/
	/* interfaces */
	GdkWindow       * (* do_create_channel)           (GXimTransport  *trans,
							   GdkWindow      *parent_window);
	GdkNativeWindow   (* do_get_native_channel)       (GXimTransport  *trans,
							   gpointer        drawable);
	void              (* do_destroy_channel)          (GXimTransport  *trans);
	gboolean          (* do_send_via_property)        (GXimTransport  *trans,
							   const gchar    *data,
							   gsize           length);
	gboolean          (* do_send_via_cm)              (GXimTransport  *trans,
							   const gchar    *data,
							   gsize           length,
							   gsize           threshold);
	gboolean          (* do_send_via_property_notify) (GXimTransport  *trans,
							   const gchar    *data,
							   gsize           length);
	gboolean          (* do_get_property)             (GXimTransport  *trans,
							   GdkWindow      *window,
							   GdkAtom         property,
							   GdkAtom         type,
							   gulong          length,
							   GdkAtom        *actual_property_type,
							   gint           *actual_format,
							   gint           *actual_length,
							   guchar        **data);
};

struct _GXimTransportPrivate {
	GdkDisplay      *display;
	GdkWindow       *comm_window;
	GHashTable      *prop_offset;
	GdkAtom          atom_comm;
	GdkNativeWindow  client_window;
	gsize            transport_size;
	gsize            transport_max;
	GXimDirection    direction;
	guint8           major_version;
	guint8           minor_version;
};


GType                 g_xim_transport_get_type                (void) G_GNUC_CONST;
void                  g_xim_transport_init                    (GXimTransport    *trans);
void                  g_xim_transport_finalize                (GXimTransport    *trans);
GXimTransportPrivate *g_xim_transport_get_private             (GXimTransport    *trans);
void                  g_xim_transport_destroy                 (GXimTransport    *trans);
void                  g_xim_transport_set_version             (GXimTransport    *trans,
                                                               guint8            major_version,
                                                               guint8            minor_version);
gboolean              g_xim_transport_get_version             (GXimTransport    *trans,
                                                               guint8           *major_version,
                                                               guint8           *minor_version);
void                  g_xim_transport_set_transport_size      (GXimTransport    *trans,
                                                               gsize             size);
gsize                 g_xim_transport_get_transport_size      (GXimTransport    *trans);
void                  g_xim_transport_set_transport_max       (GXimTransport    *trans,
                                                               gsize             size);
gsize                 g_xim_transport_get_transport_max       (GXimTransport    *trans);
void                  g_xim_transport_set_display             (GXimTransport    *trans,
                                                               GdkDisplay       *dpy);
GdkDisplay           *g_xim_transport_get_display             (GXimTransport    *trans);
GdkAtom               g_xim_transport_get_atom                (GXimTransport    *trans);
void                  g_xim_transport_set_client_window       (GXimTransport    *trans,
                                                               GdkNativeWindow   client_window);
GdkNativeWindow       g_xim_transport_get_client_window       (GXimTransport    *trans);
GdkNativeWindow       g_xim_transport_get_native_channel      (GXimTransport    *trans);
GdkNativeWindow       g_xim_transport_get_native_channel_from (GXimTransport    *trans,
                                                               gpointer          drawable);
GdkWindow            *g_xim_transport_get_channel             (GXimTransport    *trans,
                                                               GdkWindow        *parent_window);
gboolean              g_xim_transport_send_via_property       (GXimTransport    *trans,
                                                               const gchar      *data,
                                                               gsize             length);
gboolean              g_xim_transport_send_via_cm             (GXimTransport    *trans,
                                                               const gchar      *data,
                                                               gsize             length,
                                                               gsize             threshold);
gboolean              g_xim_transport_send_via_property_notify(GXimTransport    *trans,
                                                               const gchar      *data,
                                                               gsize             length);
void                  g_xim_transport_set_direction           (GXimTransport    *trans,
                                                               GXimDirection     direction);
GXimDirection         g_xim_transport_get_direction           (GXimTransport    *trans);
gboolean              g_xim_transport_get_property            (GXimTransport    *trans,
                                                               GdkWindow        *window,
                                                               GdkAtom           property,
                                                               GdkAtom           type,
                                                               gulong            length,
                                                               GdkAtom          *actual_property_type,
                                                               gint             *actual_format,
                                                               gint             *actual_length,
                                                               guchar          **data);
void                  g_xim_transport_dump                    (GXimTransport    *trans,
                                                               const gchar      *data,
                                                               gsize             length,
                                                               gboolean          is_sent);


G_END_DECLS

#endif /* __G_XIM_TRANSPORT_H__ */
