/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximprotocol.c
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

#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <glib/gi18n-lib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "gximacc.h"
#include "gximattr.h"
#include "gximconnection.h"
#include "gximmarshal.h"
#include "gximmessage.h"
#include "gximmisc.h"
#include "gximprotocol10.h"
#include "gximtransport.h"
#include "gximprotocol.h"

#define G_XIM_MARKER_IS_ITEM_BASED(_m_)	((_m_)->type == G_XIM_TYPE_MARKER_N_ITEMS_2)
#define G_XIM_GET_MARKER_DATA(_m_)	((_m_)->type == G_XIM_TYPE_BYTE ? (_m_)->value.b : ((_m_)->type == G_XIM_TYPE_WORD || (_m_)->type == G_XIM_TYPE_MARKER_N_ITEMS_2 ? (_m_)->value.w : (_m_)->value.l))


/**
 * SECTION:gximprotocol
 * @Title: GXimProtocol
 * @Short_Description: Base interface for XIM protocol
 *
 * GXimProtocol provides an interface to deal with XIM protocol events.
 */

typedef struct _GXimProtocolMarker {
	GXimValueType type;
	GXimValueType where;
	goffset       offset;
	union {
		guint8  b;
		guint16 w;
		guint32 l;
	} value;
} GXimProtocolMarker;
typedef struct _GXimProtocolSyncable {
	GXimOpcode  major_opcode;
	guint8      minor_opcode;
	gboolean    result;
	GError     *error;
} GXimProtocolSyncable;
typedef struct _GXimProtocolClosureNode {
	GCallback  func;
	gpointer   user_data;
} GXimProtocolClosureNode;

enum {
	SIGNAL_0,
	PARSER_ERROR,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_LOCK_DEFINE_STATIC (g_xim_protocol_marker);
G_LOCK_DEFINE_STATIC (g_xim_protocol_syncable);

/*
 * Private functions
 */
static void
g_xim_protocol_base_class_init(gpointer g_iface)
{
	GXimProtocolIface *iface = g_iface;
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;

		iface->atom_xim_protocol = gdk_atom_intern_static_string("_XIM_PROTOCOL");
		iface->atom_xim_moredata = gdk_atom_intern_static_string("_XIM_MOREDATA");
		gdk_flush();

		iface->message = g_xim_message_new();
		signals[PARSER_ERROR] = g_signal_new("parser_error",
						     G_TYPE_FROM_INTERFACE (iface),
						     G_SIGNAL_RUN_LAST,
						     G_STRUCT_OFFSET (GXimProtocolIface, parser_error),
						     _gxim_acc_signal_accumulator__BOOLEAN,
						     NULL,
						     gxim_marshal_BOOLEAN__UINT_UINT_UINT_UINT,
						     G_TYPE_BOOLEAN, 4,
						     G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
	} else {
		g_object_ref(iface->message);
	}
}

static void
g_xim_protocol_base_class_finalize(gpointer g_iface)
{
	GXimProtocolIface *iface = g_iface;

	g_object_unref(iface->message);
}

static gboolean
g_xim_protocol_send_header(GXimProtocol   *proto,
			   GXimOpcode      major_opcode,
			   guint8          minor_opcode,
			   GCancellable   *cancellable,
			   GError        **error)
{
	GXimProtocolPrivate *priv;

	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, FALSE);

	g_data_output_stream_set_byte_order(priv->send_ostream, priv->byte_order);
	g_data_output_stream_put_byte(priv->send_ostream, major_opcode, cancellable, error);
	g_data_output_stream_put_byte(priv->send_ostream, minor_opcode, cancellable, error);
	/* dummy for CARD16 */
	g_data_output_stream_put_byte(priv->send_ostream, 0, cancellable, error);
	g_data_output_stream_put_byte(priv->send_ostream, 0, cancellable, error);
	priv->n_sent += 4;

	return error ? *error != NULL : TRUE;
}

static gsize
g_xim_protocol_send_fixate_size(GXimProtocol   *proto,
				GCancellable   *cancellable,
				GError        **error)
{
	GXimProtocolIface *iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	GXimProtocolPrivate *priv;
	goffset pos;
	gsize osize;

	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), 0);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, 0);
	g_return_val_if_fail (priv->n_sent > 0, 0);
	g_return_val_if_fail (error != NULL, 0);

	/* Update the packet size */
	osize = priv->n_sent;
	if (osize % 4)
		g_xim_message_warning(iface->message,
				      "Bad padding: the number of packets: %" G_GSIZE_FORMAT,
				      osize);

	pos = g_seekable_tell(G_SEEKABLE (priv->base_send_ostream));
	g_seekable_seek(G_SEEKABLE (priv->base_send_ostream),
			2, G_SEEK_SET, cancellable, error);
	g_xim_protocol_send_format(proto, cancellable, error, 1,
				   G_XIM_TYPE_WORD, (osize - 4) / 4);
	g_seekable_seek(G_SEEKABLE (priv->base_send_ostream),
			pos, G_SEEK_SET, cancellable, error);

	if (error && *error != NULL)
		osize = 0;

	return osize;
}

static gboolean
g_xim_protocol_send_terminate(GXimProtocol   *proto,
			      GCancellable   *cancellable,
			      GError        **error)
{
	GXimProtocolIface *iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	GXimProtocolPrivate *priv;

	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, FALSE);

	/* XXX: workaround for failing to get the number of bytes written */
	g_seekable_seek(G_SEEKABLE (priv->base_send_ostream),
			0, G_SEEK_SET, cancellable, error);
	G_LOCK (g_xim_protocol_marker);
	if (!g_queue_is_empty(priv->markerq)) {
		g_xim_message_bug(iface->message,
				  "Unused marker(s) given: %u is still in the queue.",
				  g_queue_get_length(priv->markerq));
		g_queue_foreach(priv->markerq, (GFunc)g_free, NULL);
		g_queue_clear(priv->markerq);
	}
	G_UNLOCK (g_xim_protocol_marker);
	priv->n_sent = 0;

	return error ? *error != NULL : TRUE;
}

static void
g_xim_protocol_closure_marshal_BOOLEAN__OBJECT_OBJECT_POINTER(GClosure     *closure,
							      GValue       *return_value,
							      guint         n_param_values,
							      const GValue *param_values,
							      gpointer      invocation_hint,
							      gpointer      marshal_data)
{
	register GCClosure *cc = (GCClosure *)closure;
	register GXimProtocol *proto;
	register GDataInputStream *stream;
	register GError **error;
	register gpointer data;
	register GXimProtocolClosureFunc callback;
	gboolean retval;

	g_return_if_fail (n_param_values == 3);

	proto = g_value_get_object(&param_values[0]);
	g_return_if_fail (G_IS_XIM_PROTOCOL (proto));

	stream = g_value_get_object(&param_values[1]);
	g_return_if_fail (G_IS_DATA_INPUT_STREAM (stream));

	error = g_value_get_pointer(&param_values[2]);

	data = closure->data;
	callback = cc->callback;

	retval = callback((GXimProtocolClosure *)closure, proto, stream, error, data);

	g_value_set_boolean(return_value, retval);
}

static GXimProtocolMarker *
g_xim_protocol_find_marker(GQueue        *markerq,
			   GXimValueType  vtype)
{
	GXimProtocolMarker *marker_data = NULL;

	G_LOCK (g_xim_protocol_marker);
	if (g_queue_is_empty(markerq))
		goto end;

	marker_data = g_queue_peek_tail(markerq);
	if (marker_data->where == vtype)
		marker_data = g_queue_pop_tail(markerq);
	else
		marker_data = NULL;
  end:
	G_UNLOCK (g_xim_protocol_marker);

	return marker_data;
}

static void
_free_sendq(gpointer data,
	    gpointer user_data)
{
	GXimProtocolQueueNode *node = data;

	g_free(node->data);
	g_free(node);
}

/*
 * Public functions
 */
GType
g_xim_protocol_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;
		GTypeInfo info = {
			.class_size     = sizeof (GXimProtocolIface),
			.base_init      = (GBaseInitFunc)g_xim_protocol_base_class_init,
			.base_finalize  = (GBaseFinalizeFunc)g_xim_protocol_base_class_finalize,
			.class_init     = NULL,
			.class_finalize = NULL,
			.class_data     = NULL,
			.instance_size  = 0,
			.n_preallocs    = 0,
			.instance_init  = NULL,
			.value_table    = NULL
		};

		type_id = g_type_register_static(G_TYPE_INTERFACE, "GXimProtocol",
						 &info, 0);
		g_type_interface_add_prerequisite(type_id, G_TYPE_XIM_TRANSPORT);

		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

/**
 * g_xim_protocol_init:
 * @proto: a #GXimProtocol.
 *
 * Initializes @proto. this isn't being invoked from the class automatically
 * which has a #GXimProtocol interface. you need to call this function manually
 * before doing something with @proto.
 */
void
g_xim_protocol_init(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;

	priv = g_new0(GXimProtocolPrivate, 1);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (priv);

	priv->base_send_ostream = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
	priv->base_recv_ostream = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
	priv->send_ostream = g_data_output_stream_new(priv->base_send_ostream);
	priv->recv_ostream = g_data_output_stream_new(priv->base_recv_ostream);
	priv->proto_table__named_index = g_hash_table_new_full(g_str_hash, g_str_equal,
							       g_free, (GDestroyNotify)g_closure_unref);
	priv->proto_table__id_index = g_hash_table_new(g_direct_hash, g_direct_equal);
	priv->markerq = g_queue_new();
	priv->syncableq = g_queue_new();
	priv->sendq = g_queue_new();
	priv->byte_order = G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN;
	priv->is_disconnected = FALSE;
	/* XXX: workaround for failing to get the number of bytes written */
	priv->n_sent = 0;
	priv->n_received = 0;

	g_object_set_data(G_OBJECT (proto), "libgxim-protocol-private", priv);

	g_xim_protocol10_closure_init(proto, NULL);
}

/**
 * g_xim_protocol_dispose:
 * @object: a #GObject.
 *
 * Releases all references to other objects. This can be used to break
 * reference cycles.
 *
 * This function should only be called from #GObject::dispose which has
 * the #GXimProtocol interface.
 */
void
g_xim_protocol_dispose(GObject *object)
{
	GXimProtocol *proto = G_XIM_PROTOCOL (object);

	g_xim_protocol10_closure_finalize(proto);
}

/**
 * g_xim_protocol_finalize:
 * @object: a #GObject.
 *
 * Finalizes the instance.
 *
 * This function should only be called from #GObject::finalize which has
 * the #GXimProtocol interface.
 */
void
g_xim_protocol_finalize(GObject *object)
{
	GXimProtocol *proto = G_XIM_PROTOCOL (object);
	GXimProtocolPrivate *priv = g_xim_protocol_get_private(proto);

	if (priv) {
		G_LOCK (g_xim_protocol_marker);
		g_queue_foreach(priv->markerq, (GFunc)g_free, NULL);
		g_queue_free(priv->markerq);
		priv->markerq = NULL;
		G_UNLOCK (g_xim_protocol_marker);
		G_LOCK (g_xim_protocol_syncable);
		g_queue_foreach(priv->syncableq, (GFunc)g_free, NULL);
		g_queue_free(priv->syncableq);
		priv->syncableq = NULL;
		G_UNLOCK (g_xim_protocol_syncable);
		g_queue_foreach(priv->sendq, (GFunc)_free_sendq, NULL);
		g_queue_free(priv->sendq);
		priv->sendq = NULL;
		g_hash_table_destroy(priv->proto_table__id_index);
		g_hash_table_destroy(priv->proto_table__named_index);
		g_object_unref(priv->recv_ostream);
		g_object_unref(priv->send_ostream);
		g_object_unref(priv->base_recv_ostream);
		g_object_unref(priv->base_send_ostream);
		g_free(priv);
	}
}

/**
 * g_xim_protocol_get_private:
 * @proto: a #GXimProtocol.
 *
 * Obtains the #GXimProtocolPrivate which is referenced @proto.
 */
GXimProtocolPrivate *
g_xim_protocol_get_private(GXimProtocol *proto)
{
	return g_object_get_data(G_OBJECT (proto), "libgxim-protocol-private");
}

GQuark
g_xim_protocol_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-protocol-error");

	return quark;
}

/**
 * g_xim_protocol_process_event:
 * @proto: a #GXimProtocol.
 * @event: the #GdkEventClient which wants to process this time.
 * @error: a location to store error, or %NULL.
 *
 * Picks up XIM protocol packets from @event and deal with it as needed.
 *
 * Returns: %TRUE to be processed successfully.
 */
gboolean
g_xim_protocol_process_event(GXimProtocol    *proto,
			     GdkEventClient  *event,
			     GError         **error)
{
	GXimProtocolIface *iface;
	GXimProtocolPrivate *priv;
	GXimTransport *trans;
	gsize transport_size;
	gboolean retval = FALSE;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	g_return_val_if_fail (event->type == GDK_CLIENT_EVENT, FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	trans = G_XIM_TRANSPORT (proto);
	transport_size = g_xim_transport_get_transport_size(trans);

	if (event->message_type == iface->atom_xim_protocol) {
		GdkAtom atom_type;
		gint i, format, bytes;
		guchar *prop = NULL;
		GdkDisplay *dpy = g_xim_transport_get_display(trans);

		if (event->data_format == 32) {
			g_xim_transport_get_property(trans, event->window,
						     gdk_x11_xatom_to_atom_for_display(dpy, event->data.l[1]),
						     GDK_NONE, event->data.l[0],
						     &atom_type, &format, &bytes, &prop);
			if (prop) {
				for (i = 0; i < bytes; i++) {
					g_data_output_stream_put_byte(priv->recv_ostream,
								      prop[i],
								      NULL,
								      NULL);
					priv->n_received++;
				}
			}
		} else if (event->data_format == 8) {
			gsize i;

			for (i = 0; i < transport_size; i++) {
				g_data_output_stream_put_byte(priv->recv_ostream,
							      event->data.b[i],
							      NULL,
							      NULL);
				priv->n_received++;
			}
		} else {
			g_set_error(error, G_XIM_PROTOCOL_ERROR,
				    G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_WARNING,
				    _("Invalid packets on _XIM_PROTOCOL: format: %d."),
				    event->data_format);
		}
		g_free(prop);

		if (priv->n_received > 0) {
			gpointer p;
			gchar *data;
			gssize length = priv->n_received;

			p = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM (priv->base_recv_ostream));
			data = g_new0(gchar, sizeof (gchar) * length + 1);
			memcpy(data, p, sizeof (gchar) * length);

			/* XXX: workaround for failing to get the number of bytes written */
			g_seekable_seek(G_SEEKABLE (priv->base_recv_ostream),
					0, G_SEEK_SET, NULL, NULL);
			priv->n_received = 0;

			retval = g_xim_protocol_translate(proto, data, length, error);

			g_free(data);
		}
	} else if (event->message_type == iface->atom_xim_moredata) {
		gsize i;

		for (i = 0; i < transport_size; i++) {
			g_data_output_stream_put_byte(priv->recv_ostream,
						      event->data.b[i],
						      NULL,
						      NULL);
			priv->n_received++;
		}
		/* not yet process the packet.
		 * there are still more packets.
		 */
		retval = TRUE;
	}
	if (priv->is_disconnected) {
		/* unref here */
		g_object_unref(proto);
	}

	return retval;
}

/**
 * g_xim_protocol_translate:
 * @proto: a #GXimProtocol.
 * @data: a chunk of XIM protocol packets
 * @length: the number of @data.
 * @error: a location to store error, or %NULL.
 *
 * Translate the packets and deliver it to the appropriate places.
 *
 * Returns: %TRUE to be processed successfully.
 */
#pragma GCC diagnostic ignored "-Wformat-extra-args" /* to prevent an error of using G_GSIZE_FORMAT */
gboolean
g_xim_protocol_translate(GXimProtocol     *proto,
			 gpointer          data,
			 gssize            length,
			 GError          **error)
{
	GXimProtocolIface *iface;
	GXimProtocolPrivate *priv;
	GXimTransport *trans;
	GInputStream *base_istream = NULL;
	GDataInputStream *istream = NULL;
	guint8 major_opcode, minor_opcode;
	guint16 packlen = 0;
	gsize avail;
	gboolean ret = FALSE;
	GXimProtocolSyncable *on_sync = NULL;
	GClosure *closure;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	trans = G_XIM_TRANSPORT (proto);

	if (length == 0) {
		g_set_error(error, G_XIM_PROTOCOL_ERROR,
			    G_XIM_PROTOCOL_ERROR_NO_DATA | G_XIM_NOTICE_ERROR,
			    "No data to translate.");
		goto end;
	}
	g_xim_transport_dump(trans, data, length, FALSE);

	base_istream = g_memory_input_stream_new_from_data(data, length, NULL);
	istream = g_data_input_stream_new(base_istream);

	major_opcode = g_data_input_stream_read_byte(istream, NULL, NULL);
	minor_opcode = g_data_input_stream_read_byte(istream, NULL, error);
	if (*error)
		goto end;
	if (major_opcode == G_XIM_CONNECT) {
		/* decide the byte order */
		if (((gchar *)data)[4] == 0x42) {
			/* this is a big endian */
			priv->byte_order = G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN;
		} else if (((gchar *)data)[4] == 0x6c) {
			/* this is a little endian */
			priv->byte_order = G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN;
		} else {
			g_set_error(error, G_XIM_PROTOCOL_ERROR,
				    G_XIM_PROTOCOL_ERROR_UNKNOWN_ENDIAN | G_XIM_NOTICE_ERROR,
				    "Unknown endian `%02X` or invalid packet received.",
				    ((gchar *)data)[4]);
			goto end;
		}
		g_xim_message_debug(iface->message, "proto",
				    "Byte order is a %s",
				    priv->byte_order == G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN ?
				    "big endian" : "little endian");
	}
	g_data_input_stream_set_byte_order(istream, priv->byte_order);
	packlen = g_data_input_stream_read_uint16(istream, NULL, error);
	if (*error)
		goto end;
	avail = g_buffered_input_stream_get_buffer_size(G_BUFFERED_INPUT_STREAM (istream));
	if (avail < (packlen * 4)) {
		g_set_error(error, G_XIM_PROTOCOL_ERROR,
			    G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
			    "Received packets size `%" G_GUINT16_FORMAT "' is less than `%" G_GSIZE_FORMAT "'",
			    packlen * 4, avail);
		goto end;
	}
	/* After here, all the read should succeeds */

	G_LOCK (g_xim_protocol_syncable);
	if (!g_queue_is_empty(priv->syncableq)) {
		guint len, i;
		GXimProtocolSyncable *syncable;

		len = g_queue_get_length(priv->syncableq);
		for (i = 0; i < len; i++) {
			syncable = g_queue_peek_nth(priv->syncableq, i);
			if (syncable->major_opcode == major_opcode &&
			    syncable->minor_opcode == minor_opcode) {
				on_sync = syncable;
				syncable->result = TRUE;
				break;
			} else if (major_opcode == G_XIM_ERROR &&
				   minor_opcode == 0) {
				on_sync = syncable;
				syncable->result = FALSE;
				break;
			}
		}
	}
	G_UNLOCK (g_xim_protocol_syncable);

	closure = (GClosure *)g_xim_protocol_lookup_protocol_by_id(proto, major_opcode, minor_opcode);
	if (closure == NULL) {
		g_xim_message_bug(iface->message,
				  "Unknown opcode in the packets: major: %d, minor: %d",
				  major_opcode, minor_opcode);
	} else {
		GValue v = { 0, }, *pv;
		guint n = 3, i;

		g_value_init(&v, G_TYPE_BOOLEAN);
		pv = g_new0(GValue, n);
		G_XIM_GERROR_CHECK_ALLOC (pv, error,
					  G_XIM_PROTOCOL_ERROR, FALSE);

		g_value_init(&pv[0], G_TYPE_OBJECT);
		g_value_set_object(&pv[0], proto);
		g_value_init(&pv[1], G_TYPE_DATA_INPUT_STREAM);
		g_value_set_object(&pv[1], istream);
		g_value_init(&pv[2], G_TYPE_POINTER);
		g_value_set_pointer(&pv[2], error);
		g_closure_invoke(closure, &v, n, pv, NULL);

		for (i = 0; i < n; i++) {
			g_value_unset(&pv[i]);
		}
		ret = g_value_get_boolean(&v);
		if (!ret && *error == NULL) {
			g_set_error(error, G_XIM_PROTOCOL_ERROR,
				    G_XIM_PROTOCOL_ERROR_NO_PARSER | G_XIM_NOTICE_ERROR,
				    "No parser available for %s",
				    ((GXimProtocolClosure *)closure)->name);
		}
		g_free(pv);
	}

  end:
	if (istream)
		g_object_unref(istream);
	if (base_istream)
		g_object_unref(base_istream);
	if (*error) {
		if (on_sync)
			on_sync->error = g_error_copy(*error);
	}
	if (on_sync) {
		if (*error == NULL && !ret)
			g_set_error(&on_sync->error, G_XIM_PROTOCOL_ERROR,
				    G_XIM_PROTOCOL_ERROR_DELIVERY_FAILURE | G_XIM_NOTICE_ERROR,
				    "Unable to marshal a signal.");
		on_sync->result &= ret;
		G_LOCK (g_xim_protocol_syncable);
		g_queue_remove(priv->syncableq, on_sync);
		G_UNLOCK (g_xim_protocol_syncable);
	}
	length -= (packlen * 4 + 4);
	if (ret &&
	    length > 4 &&
	    ((gchar *)data)[(packlen * 4) + 4] != 0) {
		/* some IMs sends the multiple protocols in one request. */
		ret = g_xim_protocol_translate(proto, &((gchar *)data)[packlen * 4 + 4], length, error);
	}

	return ret; /* XXX */
}
#pragma GCC diagnostic error "-Wformat-extra-args" /* this might be wrong to get back the status */

/**
 * g_xim_protocol_send_packets:
 * @proto: a #GXimProtocol.
 * @data: a chunk of XIM protocol packets.
 * @length: the number of @data.
 *
 * Sends @data to the opposite direction on the connection.
 *
 * Returns: %TRUE to be sent successfully.
 */
gboolean
g_xim_protocol_send_packets(GXimProtocol  *proto,
			    const gchar   *data,
			    gsize          length)
{
	GXimProtocolIface *iface;
	GXimTransport *trans;
	gboolean retval = FALSE;
	guint8 major, minor;
	gsize transport_size, transport_max;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);

	trans = G_XIM_TRANSPORT (proto);
	g_return_val_if_fail (g_xim_transport_get_version(trans, &major, &minor), FALSE);

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	transport_size = g_xim_transport_get_transport_size(trans);
	transport_max = g_xim_transport_get_transport_max(trans);

	switch (major) {
	    case 0:
		    switch (minor) {
			case 0:
				if (length > transport_size) {
					/* Send data via Property */
					retval = g_xim_transport_send_via_property(trans,
										   data,
										   length);
				} else {
					/* Send data via ClientMessage */
					retval = g_xim_transport_send_via_cm(trans,
									     data,
									     length,
									     transport_size);
				}
				break;
			case 1:
				/* Send data via ClientMessage or multiple ClientMessages */
				retval = g_xim_transport_send_via_cm(trans,
								     data,
								     length,
								     transport_size);
				break;
			case 2:
				if (length > transport_max) {
					/* Send data via Property */
					retval = g_xim_transport_send_via_property(trans,
										   data,
										   length);
				} else {
					/* Send data via ClientMessage or multiple ClientMessages */
					retval = g_xim_transport_send_via_cm(trans,
									     data,
									     length,
									     transport_size);
				}
				break;
			default:
				g_xim_message_warning(iface->message,
						      "Unsupported protocol version: %d.%d",
						      major, minor);
				break;
		    }
		    break;
	    case 1:
		    switch (minor) {
			case 0:
				/* Send data via PropertyNotify */
				retval = g_xim_transport_send_via_property_notify(trans,
										  data,
										  length);
				break;
			default:
				g_xim_message_warning(iface->message,
						      "Unsupported protocol version: %d.%d",
						      major, minor);
				break;
		    }
		    break;
	    case 2:
		    switch (minor) {
			case 0:
				if (length > transport_size) {
					/* Send data via PropertyNotify */
					retval = g_xim_transport_send_via_property_notify(trans,
											  data,
											  length);
				} else {
					/* Send data via ClientMessage */
					retval = g_xim_transport_send_via_cm(trans,
									     data,
									     length,
									     transport_size);
				}
				break;
			case 1:
				if (length > transport_max) {
					/* Send data via Property */
					retval = g_xim_transport_send_via_property_notify(trans,
											  data,
											  length);
				} else {
					/* Send data via ClientMessage or multiple ClientMessages */
					retval = g_xim_transport_send_via_cm(trans,
									     data,
									     length,
									     transport_size);
				}
				break;
			default:
				g_xim_message_warning(iface->message,
						      "Unsupported protocol version: %d.%d",
						      major, minor);
				break;
		    }
		    break;
	    default:
		    g_xim_message_warning(iface->message,
					  "Unsupported protocol version: %d.%d",
					  major, minor);
		    break;
	}

	return retval;
}

/**
 * g_xim_protocol_send_vformat:
 * @proto: a #GXimProtocol.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a location to store error, or %NULL.
 * @n_params: the number of a set of parameters.
 * @args: a #va_list.
 *
 * Converts the objects to the XIM protocol packets according to format
 * and store it into the send buffer. you shouldn't use this function directly.
 * use g_xim_protocol_send() instead.
 *
 * Returns: the number of data stored.
 */
gsize
g_xim_protocol_send_vformat(GXimProtocol  *proto,
			    GCancellable  *cancellable,
			    GError       **error,
			    guint          n_params,
			    va_list        args)
{
	GXimProtocolIface *iface;
	GXimProtocolPrivate *priv;
	GXimTransport *trans;
	guint i, j;
	gsize n_sent, prev_size = 0;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), 0);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, 0);
	g_return_val_if_fail (error != NULL, 0);

	trans = G_XIM_TRANSPORT (proto);
	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	n_sent = priv->n_sent;
	for (i = 0; i < n_params; i++) {
		GXimValueType type, mtype;
		gchar *p;
		gsize sz, padding;
		GXimAttr *attr;
		goffset cur_pos = priv->n_sent;
		GXimProtocolMarker *marker_data;
		GSList *list, *l;
		gpointer value;

		type = va_arg(args, GXimValueType);
		switch (type) {
		    case G_XIM_TYPE_PADDING:
			    padding = va_arg(args, unsigned int);
		    padding:
			    for (sz = 0; sz < padding; sz++) {
				    g_data_output_stream_put_byte(priv->send_ostream, 0,
								  cancellable, error);
				    if (*error)
					    goto end;
				    priv->n_sent++;
			    }
			    break;
		    case G_XIM_TYPE_AUTO_PADDING:
			    sz = va_arg(args, unsigned int);
			    padding = g_xim_protocol_n_pad4(prev_size + sz);
			    goto padding;
		    case G_XIM_TYPE_MARKER_N_BYTES_1:
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, 0);

			    marker_data->type = G_XIM_TYPE_BYTE;
			    marker_data->where = mtype;
			    marker_data->offset = g_seekable_tell(G_SEEKABLE (priv->base_send_ostream));
			    g_queue_push_tail(priv->markerq, marker_data);
			    g_data_output_stream_put_byte(priv->send_ostream, 0, NULL, NULL);
			    priv->n_sent++;
			    break;
		    case G_XIM_TYPE_MARKER_N_BYTES_2:
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, 0);

			    marker_data->type = G_XIM_TYPE_WORD;
			    marker_data->where = mtype;
			    marker_data->offset = g_seekable_tell(G_SEEKABLE (priv->base_send_ostream));
			    g_queue_push_tail(priv->markerq, marker_data);
			    g_data_output_stream_put_uint16(priv->send_ostream, 0,
							    cancellable, error);
			    priv->n_sent += 2;
			    break;
		    case G_XIM_TYPE_MARKER_N_BYTES_4:
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, 0);

			    marker_data->type = G_XIM_TYPE_LONG;
			    marker_data->where = mtype;
			    marker_data->offset = g_seekable_tell(G_SEEKABLE (priv->base_send_ostream));
			    g_queue_push_tail(priv->markerq, marker_data);
			    g_data_output_stream_put_uint32(priv->send_ostream, 0,
							    cancellable, error);
			    priv->n_sent += 4;
			    break;
		    case G_XIM_TYPE_STR:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_str_put_to_stream((GXimStr *)value,
								    priv->send_ostream,
								    cancellable, error);
			    break;
		    case G_XIM_TYPE_GSTRING:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_gstring_put_to_stream((GString *)value,
									priv->send_ostream,
									cancellable, error);
			    /* We don't want to include the length of characters for markers */
			    cur_pos += 2;
			    break;
		    case G_XIM_TYPE_PREEDIT_CARET:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_preedit_caret_put_to_stream((GXimPreeditCaret *)value,
									      proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_PREEDIT_DRAW:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_preedit_draw_put_to_stream((GXimPreeditDraw *)value,
									     proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_GDKEVENT:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_gdkevent_put_to_stream((GdkEvent *)value,
									 proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_XIMTEXT:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_text_put_to_stream((GXimText *)value,
								     proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_HOTKEY_TRIGGER:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_hotkey_trigger_put_to_stream((GXimHotkeyTrigger *)value,
									       proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_PIXMAP:
			    value = va_arg(args, gpointer);
			    g_data_output_stream_put_uint32(priv->send_ostream,
							    g_xim_transport_get_native_channel_from(trans, value),
							    cancellable,
							    error);
			    priv->n_sent += 4;
			    break;
		    case G_XIM_TYPE_STATUS_DRAW:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_status_draw_put_to_stream((GXimStatusDraw *)value,
									    proto, cancellable, error);
			    break;
		    case G_XIM_TYPE_LIST_OF_IMATTR:
		    case G_XIM_TYPE_LIST_OF_ICATTR:
			    list = va_arg(args, gpointer);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_raw_attr_put_to_stream(l->data, proto,
										 cancellable, error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_IMATTRIBUTE:
			    if (!G_IS_XIM_IM_ATTR (G_XIM_CONNECTION (proto)->imattr)) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
						"Protocol order might be invalid. XIM_OPEN must be brought up first.");
				    goto end;
			    } else {
				    attr = G_XIM_ATTR (G_XIM_CONNECTION (proto)->imattr);
			    }
			    goto process_attribute;
		    case G_XIM_TYPE_LIST_OF_ICATTRIBUTE:
			    if (!G_IS_XIM_IC_ATTR (G_XIM_CONNECTION (proto)->default_icattr)) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
						"Protocol order might be invalid. XIM_OPEN must be brought up first.");
				    goto end;
			    } else {
				    attr = G_XIM_ATTR (G_XIM_CONNECTION (proto)->default_icattr);
			    }
		    process_attribute:
			    list = va_arg(args, gpointer);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_attribute_put_to_stream(l->data, proto,
										  cancellable, error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_EXT:
			    list = va_arg(args, gpointer);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_ext_put_to_stream(l->data,
									    proto,
									    cancellable,
									    error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_STRING:
			    list = va_arg(args, GSList *);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_string_put_to_stream(l->data,
									       priv->send_ostream,
									       cancellable,
									       error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_STR:
			    list = va_arg(args, GSList *);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_str_put_to_stream(l->data,
									    priv->send_ostream,
									    cancellable,
									    error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_ENCODINGINFO:
			    list = va_arg(args, GSList *);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_encodinginfo_put_to_stream(l->data,
										     priv->send_ostream,
										     cancellable,
										     error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_BYTE:
			    value = va_arg(args, gpointer);
			    for (j = 0; j < ((GString *)value)->len; j++) {
				    g_data_output_stream_put_byte(priv->send_ostream,
								  ((GString *)value)->str[j],
								  cancellable, error);
				    priv->n_sent++;
				    if (*error)
					    goto end;
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_CARD16:
			    list = va_arg(args, GSList *);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    g_data_output_stream_put_uint16(priv->send_ostream,
								    GPOINTER_TO_UINT (l->data),
								    cancellable,
								    error);
				    priv->n_sent += 2;
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER:
			    list = va_arg(args, GSList *);
			    for (l = list; l != NULL; l = g_slist_next(l)) {
				    priv->n_sent += g_xim_hotkey_trigger_put_to_stream(l->data,
										       proto,
										       cancellable,
										       error);
				    if (*error) {
					    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
					    } else {
						    goto end;
					    }
				    }
			    }
			    break;
		    case G_XIM_TYPE_SEPARATOR:
			    break;
		    case G_XIM_TYPE_BYTE:
			    g_data_output_stream_put_byte(priv->send_ostream,
							  (guint8)va_arg(args, int),
							  cancellable, error);
			    priv->n_sent++;
			    break;
		    case G_XIM_TYPE_WORD:
			    g_data_output_stream_put_uint16(priv->send_ostream,
							    (guint16)va_arg(args, unsigned int),
							    cancellable, error);
			    priv->n_sent += 2;
			    break;
		    case G_XIM_TYPE_LONG:
			    g_data_output_stream_put_uint32(priv->send_ostream,
							    (guint32)va_arg(args, unsigned long),
							    cancellable, error);
			    priv->n_sent += 4;
			    break;
		    case G_XIM_TYPE_CHAR:
			    p = va_arg(args, gchar *);
			    for (; p != NULL && *p != 0; p++) {
				    g_data_output_stream_put_byte(priv->send_ostream, *p,
								  cancellable, error);
				    priv->n_sent++;
				    if (*error)
					    goto end;
			    }
			    break;
		    case G_XIM_TYPE_WINDOW:
			    value = va_arg(args, gpointer);
			    g_data_output_stream_put_uint32(priv->send_ostream,
							    g_xim_transport_get_native_channel_from(trans, value),
							    cancellable,
							    error);
			    priv->n_sent += 4;
			    break;
		    case G_XIM_TYPE_XIMSTYLES:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_styles_put_to_stream((GXimStyles *)value,
								       proto,
								       cancellable,
								       error);
			    break;
		    case G_XIM_TYPE_XRECTANGLE:
			    priv->n_sent += g_xim_rectangle_put_to_stream(va_arg(args, GXimRectangle *),
									  proto,
									  cancellable,
									  error);
			    break;
		    case G_XIM_TYPE_XPOINT:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_point_put_to_stream((GXimPoint *)value,
								      proto,
								      cancellable,
								      error);
			    break;
		    case G_XIM_TYPE_XFONTSET:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_fontset_put_to_stream((GXimFontSet *)value,
									proto,
									cancellable,
									error);
			    break;
		    case G_XIM_TYPE_HOTKEYTRIGGERS:
		    case G_XIM_TYPE_HOTKEYSTATE:
		    case G_XIM_TYPE_STRINGCONVERSION:
		    case G_XIM_TYPE_PREEDITSTATE:
		    case G_XIM_TYPE_RESETSTATE:
			    goto nonsupported;
		    case G_XIM_TYPE_NESTEDLIST:
			    value = va_arg(args, gpointer);
			    priv->n_sent += g_xim_nested_list_put_to_stream((GXimNestedList *)value,
									    proto,
									    cancellable,
									    error);
			    break;
		    default:
		    nonsupported:
			    g_xim_message_warning(iface->message,
						  "Unknown/unsupported value type to send packets: %s",
						  g_xim_value_type_name(type));
#ifdef GNOME_ENABLE_DEBUG
			    abort();
#endif
			    break;
		}
		if (!g_queue_is_empty(priv->markerq)) {
			marker_data = g_queue_peek_tail(priv->markerq);
			if (marker_data->where == type) {
				goffset cur = g_seekable_tell(G_SEEKABLE (priv->base_send_ostream));
				gsize save_n_sent = priv->n_sent;

				g_seekable_seek(G_SEEKABLE (priv->base_send_ostream),
						marker_data->offset, G_SEEK_SET, NULL, NULL);
				g_xim_protocol_send_format(proto, cancellable, error, 1,
							   marker_data->type, cur - cur_pos);
				priv->n_sent = save_n_sent;
				g_seekable_seek(G_SEEKABLE (priv->base_send_ostream),
						cur, G_SEEK_SET, NULL, NULL);
				g_free(g_queue_pop_tail(priv->markerq));
			}
		}
		prev_size = priv->n_sent - cur_pos;
		if (*error) {
			if (!G_XIM_GERROR_IS_RECOVERABLE (*error))
				goto end;
			g_xim_message_gerror(iface->message, *error);
			g_clear_error(error);
		}
	}
  end:
	if (*error)
		return 0;

	return priv->n_sent - n_sent;
}

/**
 * g_xim_protocol_send_format:
 * @proto: a #GXimProtocol.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a location to store error, or %NULL.
 * @n_params: the number of a set of parameters.
 *
 * Converts the objects to the XIM protocol packets according to the format
 * and store it into the send buffer. you shouldn't use this function directly.
 * use g_xim_protocol_send() instead.
 *
 * Returns: the number of data stored.
 */
gsize
g_xim_protocol_send_format(GXimProtocol  *proto,
			   GCancellable  *cancellable,
			   GError       **error,
			   guint          n_params,
			   ...)
{
	va_list ap;
	gsize retval;

	va_start(ap, n_params);

	retval = g_xim_protocol_send_vformat(proto,
					     cancellable,
					     error,
					     n_params, ap);

	va_end(ap);

	return retval;
}

/**
 * g_xim_protocol_send:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @n_params: the number of a set of parameters.
 *
 * Sends XIM protocol according to the set of parameters. one is a parameter
 * type defined in #GXimValueType, and one is an object related to that.
 * See #GXimValueType documentation which objects are supposed to be specified
 * with them.
 *
 * Returns: %TRUE to be sent successfully.
 */
gboolean
g_xim_protocol_send(GXimProtocol  *proto,
		    GXimOpcode     major_opcode,
		    guint8         minor_opcode,
		    guint          n_params,
		    ...)
{
	GXimProtocolPrivate *priv;
	va_list ap;
	gpointer data;
	gboolean retval = FALSE;
	gsize osize;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, FALSE);

	va_start(ap, n_params);

	if (!g_xim_protocol_send_header(proto, major_opcode, minor_opcode,
					NULL, NULL))
		goto end;

	g_xim_protocol_send_vformat(proto, NULL, &error, n_params, ap);
	if (error) {
		g_xim_message_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				     error);
		if (!G_XIM_GERROR_IS_RECOVERABLE (error))
			goto end;
		g_clear_error(&error);
	}

	if ((osize = g_xim_protocol_send_fixate_size(proto, NULL, &error)) == 0)
		goto end;

	data = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM (priv->base_send_ostream));
	if (g_queue_get_length(priv->sendq) > 0) {
		GXimProtocolQueueNode *node = g_queue_peek_tail(priv->sendq);

		if (node->data == NULL) {
			/* if it's the queueing mode, do not send the packet */
			g_xim_message_debug(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "proto/event",
					    "Queued the packet for %s",
					    g_xim_protocol_name(major_opcode));
			node->data = g_new0(gchar, osize + 1);
			memcpy(node->data, data, osize);
			node->length = osize;
			retval = TRUE;
			goto end;
		}
	}
	retval = g_xim_protocol_send_packets(proto, data, osize);

  end:
	if (error)
		g_error_free(error);
	retval &= g_xim_protocol_send_terminate(proto, NULL, NULL);

	va_end(ap);

	return retval;
}

/**
 * g_xim_protocol_send_with_list:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @types: a list of #GXimValueType.
 * @values: a list of objects fitting to each value types in @types.
 *
 * Sends XIM protocol according to @types and @values.
 * See #GXimValueType documentation which objects are supposed to be specified
 * with them.
 *
 * Returns: %TRUE to be sent successfully.
 */
gboolean
g_xim_protocol_send_with_list(GXimProtocol  *proto,
			      GXimOpcode     major_opcode,
			      guint8         minor_opcode,
			      GSList        *types,
			      GSList        *values)
{
	GXimProtocolPrivate *priv;
	GSList *lt, *lv;
	gpointer data, value;
	gboolean retval = FALSE;
	gsize osize, prev_size = 0;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail (types != NULL, FALSE);
	g_return_val_if_fail (values != NULL, FALSE);
	g_return_val_if_fail (g_slist_length(types) == g_slist_length(values), FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (priv->byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, FALSE);

	if (!g_xim_protocol_send_header(proto, major_opcode, minor_opcode,
					NULL, NULL))
		goto end;

	for (lt = types, lv = values;
	     lt != NULL && lv != NULL;
	     lt = g_slist_next(lt), lv = g_slist_next(lv)) {
		value = lv->data;
		if (GPOINTER_TO_UINT (lt->data) == G_XIM_TYPE_AUTO_PADDING) {
			/* We have to deal with it here */
			value = GUINT_TO_POINTER (GPOINTER_TO_UINT (value) + prev_size);
		}
		prev_size = g_xim_protocol_send_format(proto, NULL, &error, 1, lt->data, value);
		if (GPOINTER_TO_UINT (lt->data) == G_XIM_TYPE_GSTRING) {
			/* XXX: hack to adjust the previous size... */
			prev_size -= 2;
		}
		if (error) {
			g_xim_message_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message, error);
			if (!G_XIM_GERROR_IS_RECOVERABLE (error))
				goto end;
			g_clear_error(&error);
		}
	}

	if ((osize = g_xim_protocol_send_fixate_size(proto, NULL, &error)) == 0)
		goto end;

	data = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM (priv->base_send_ostream));
	if (g_queue_get_length(priv->sendq) > 0) {
		GXimProtocolQueueNode *node = g_queue_peek_tail(priv->sendq);

		if (node->data == NULL) {
			/* if it's the queueing mode, do not send the packet */
			g_xim_message_debug(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "proto/event",
					    "Queued the packet for %s",
					    g_xim_protocol_name(major_opcode));
			node->data = g_new0(gchar, osize + 1);
			memcpy(node->data, data, osize);
			node->length = osize;
			retval = TRUE;
			goto end;
		}
	}
	retval = g_xim_protocol_send_packets(proto, data, osize);

  end:
	if (error)
		g_error_free(error);
	retval &= g_xim_protocol_send_terminate(proto, NULL, NULL);

	return retval;
}

/**
 * g_xim_protocol_start_queue:
 * @proto: a #GXimProtocol.
 *
 * Prepares the queue to not send a request immediately. this function is
 * useful if you need to wait for any requests but want to send it after
 * the pending requests is done.
 *
 * Returns: %TRUE to be ready to queue.
 */
gboolean
g_xim_protocol_start_queue(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;
	GXimProtocolQueueNode *node;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);

	node = g_new0(GXimProtocolQueueNode, 1);
	G_XIM_CHECK_ALLOC (node, FALSE);
	g_queue_push_tail(priv->sendq, node);

	return TRUE;
}

/**
 * g_xim_protocol_end_queue:
 * @proto: a #GXimProtocol.
 *
 * Terminates the queue.
 *
 * Returns: the #GXimProtocolQueueNode which queued the request this time.
 */
GXimProtocolQueueNode *
g_xim_protocol_end_queue(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), NULL);

	return  g_queue_peek_tail(priv->sendq);
}

/**
 * g_xim_protocol_cancel_queue:
 * @proto: a #GXimProtocol.
 *
 * Cancels queueing. Note that you have to invoke this function if you can't
 * proceed queueing process with any errors say. otherwise invalid node is kept
 * in the queue and you may get a trouble with it eveutnally.
 */
void
g_xim_protocol_cancel_queue(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;

	g_return_if_fail (G_IS_XIM_PROTOCOL (proto));
	g_return_if_fail ((priv = g_xim_protocol_get_private(proto)));
	g_return_if_fail (g_xim_protocol_is_queued(proto));

	g_free(g_queue_pop_tail(priv->sendq));
}

/**
 * g_xim_protocol_is_queued:
 * @proto: a #GXimProtocol.
 *
 * Checks if @proto is in queueing process.
 *
 * Returns: %TRUE to be in the queueing process. otherwise %FALSE.
 */
gboolean
g_xim_protocol_is_queued(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;
	GXimProtocolQueueNode *node;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);

	node = g_queue_peek_tail(priv->sendq);

	return node->data == NULL;
}

/**
 * g_xim_protocol_get_queue_length:
 * @proto: a #GXimProtocol.
 *
 * Obtains how many requests are stored in the queue now.
 *
 * Returns: the number of requests in the queue.
 */
guint
g_xim_protocol_get_queue_length(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), 0);

	return g_queue_get_length(priv->sendq);
}

/**
 * g_xim_protocol_read_vformat:
 * @proto: a #GXimProtocol.
 * @stream: a #GDataInputStream which contains XIM protocol packets.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a location to store error, or %NULL.
 * @n_params: the number of a set of parameters.
 * @args: a #va_list.
 *
 * Scans XIM protocol packets according to the set of parameters. if any,
 * objects generated against it is stored in the locations pointed to by
 * the pointer arguments. Each pointer argument must be of a type that is
 * appropriate for the value returned by the corresponding value type.
 * See #GXimValueType documentation which objects the value type is supposed
 * to be.
 *
 * Returns: %TRUE to be read successfully.
 */
gboolean
g_xim_protocol_read_vformat(GXimProtocol      *proto,
			    GDataInputStream  *stream,
			    GCancellable      *cancellable,
			    GError           **error,
			    guint              n_params,
			    va_list            args)
{
	GXimProtocolPrivate *priv;
	GXimProtocolIface *iface;
	guint i;
	GInputStream *istream;
	gboolean retval = TRUE;
	goffset base_pos;
	GXimProtocolMarker *marker_data = NULL;
	GQueue *markerq;
	GXimValueType vtype = G_XIM_TYPE_INVALID;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), FALSE);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	markerq = g_queue_new();
	G_XIM_GERROR_CHECK_ALLOC (markerq, error, G_XIM_PROTOCOL_ERROR, FALSE);
	istream = g_filter_input_stream_get_base_stream(G_FILTER_INPUT_STREAM (stream));
	base_pos = g_seekable_tell(G_SEEKABLE (istream));
	for (i = 0; i < n_params; i++) {
		GXimValueType mtype;
		gpointer value;
		gsize padding, sz, size;
		goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream));
		GType gtype;
		GXimAttr *attr;
		GdkNativeWindow nw;

		vtype = va_arg(args, GXimValueType);
		marker_data = g_xim_protocol_find_marker(markerq, vtype);
		switch (vtype) {
		    case G_XIM_TYPE_PADDING:
			    padding = va_arg(args, unsigned int);
		    padding:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (!g_seekable_seek(G_SEEKABLE (istream), padding,
						 G_SEEK_CUR, cancellable, error)) {
				    goto end;
			    }
			    break;
		    case G_XIM_TYPE_AUTO_PADDING:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    sz = va_arg(args, unsigned int);
			    padding = g_xim_protocol_n_pad4(cur_pos - base_pos + sz);
			    goto padding;
		    case G_XIM_TYPE_MARKER_N_BYTES_1:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, FALSE);

			    marker_data->type = G_XIM_TYPE_BYTE;
			    marker_data->where = mtype;
			    marker_data->value.b = g_data_input_stream_read_byte(stream, cancellable, error);
			    g_queue_push_tail(markerq, marker_data);
			    marker_data = NULL;
			    break;
		    case G_XIM_TYPE_MARKER_N_BYTES_2:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, FALSE);

			    marker_data->type = G_XIM_TYPE_WORD;
			    marker_data->where = mtype;
			    marker_data->value.w = g_data_input_stream_read_uint16(stream, cancellable, error);
			    g_queue_push_tail(markerq, marker_data);
			    marker_data = NULL;
			    break;
		    case G_XIM_TYPE_MARKER_N_BYTES_4:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, FALSE);

			    marker_data->type = G_XIM_TYPE_LONG;
			    marker_data->where = mtype;
			    marker_data->value.l = g_data_input_stream_read_uint32(stream, cancellable, error);
			    g_queue_push_tail(markerq, marker_data);
			    marker_data = NULL;
			    break;
		    case G_XIM_TYPE_MARKER_N_ITEMS_2:
			    if (marker_data) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"Invalid the length-of-data marker for value type %s",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    mtype = va_arg(args, GXimValueType);
			    marker_data = g_new0(GXimProtocolMarker, 1);
			    G_XIM_GERROR_CHECK_ALLOC (marker_data, error,
						      G_XIM_PROTOCOL_ERROR, FALSE);

			    marker_data->type = vtype;
			    marker_data->where = mtype;
			    marker_data->value.w = g_data_input_stream_read_uint16(stream, cancellable, error);
			    g_queue_push_tail(markerq, marker_data);
			    marker_data = NULL;
			    break;
		    case G_XIM_TYPE_STR:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimStr **)value) = g_xim_str_get_from_stream(stream, NULL, error);
			    break;
		    case G_XIM_TYPE_GSTRING:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    /* we don't want to include the length of characters for markers */
			    cur_pos += 2;
			    *((GString **)value) = g_xim_gstring_get_from_stream(stream, NULL, error);
			    break;
		    case G_XIM_TYPE_PREEDIT_CARET:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimPreeditCaret **)value) = g_xim_preedit_caret_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_PREEDIT_DRAW:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimPreeditDraw **)value) = g_xim_preedit_draw_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_GDKEVENT:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GdkEvent **)value) = g_xim_gdkevent_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_XIMTEXT:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimText **)value) = g_xim_text_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_HOTKEY_TRIGGER:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimHotkeyTrigger **)value) = g_xim_hotkey_trigger_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_PIXMAP:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    nw = g_data_input_stream_read_uint32(stream, cancellable, error);
			    *((GdkPixmap **)value) = g_xim_get_pixmap(g_xim_transport_get_display(G_XIM_TRANSPORT (proto)),
								      nw);
			    if (*((GdkPixmap **)value) == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_NO_DATA | G_XIM_NOTICE_ERROR,
						"Unable to get a valid pixmap for %p",
						G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
				    goto end;
			    }
			    break;
		    case G_XIM_TYPE_STATUS_DRAW:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimStatusDraw **)value) = g_xim_status_draw_get_from_stream(proto, stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_LIST_OF_IMATTR:
			    gtype = G_TYPE_XIM_IM_ATTR;
			    goto process_attr;
		    case G_XIM_TYPE_LIST_OF_ICATTR:
			    gtype = G_TYPE_XIM_IC_ATTR;
		    process_attr:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimRawAttr *a;

					    a = g_xim_raw_attr_get_from_stream(proto, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), a);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimRawAttr *a;

					    a = g_xim_raw_attr_get_from_stream(proto, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), a);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_IMATTRIBUTE:
			    if (!G_IS_XIM_IM_ATTR (G_XIM_CONNECTION (proto)->imattr)) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
						"Protocol order might be invalid. XIM_OPEN must be brought up first.");
				    goto end;
			    } else {
				    attr = G_XIM_ATTR (G_XIM_CONNECTION (proto)->imattr);
			    }
			    goto process_attribute;
		    case G_XIM_TYPE_LIST_OF_ICATTRIBUTE:
			    if (!G_IS_XIM_IC_ATTR (G_XIM_CONNECTION (proto)->default_icattr)) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
						"Protocol order might be invalid. XIM_OPEN must be brought up first.");
				    goto end;
			    } else {
				    attr = G_XIM_ATTR (G_XIM_CONNECTION (proto)->default_icattr);
			    }
		    process_attribute:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimAttribute *a;

					    a = g_xim_attr_get_attribute_from_stream(proto, attr, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), a);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimAttribute *a;

					    a = g_xim_attr_get_attribute_from_stream(proto, attr, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), a);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_EXT:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimExt *e;

					    e = g_xim_ext_get_from_stream(proto, stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), e);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    GXimExt *e;
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;

					    e = g_xim_ext_get_from_stream(proto, stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), e);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_STRING:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimString *s;

					    s = g_xim_string_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), s);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimString *s;

					    s = g_xim_string_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), s);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_STR:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimStr *s;

					    s = g_xim_str_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), s);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimStr *s;

					    s = g_xim_str_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), s);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_ENCODINGINFO:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimEncodingInfo *e;

					    e = g_xim_encodinginfo_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), e);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimEncodingInfo *e;

					    e = g_xim_encodinginfo_get_from_stream(stream, NULL, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), e);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_BYTE:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    /* assuming the length of item == the length of bytes */
			    size = G_XIM_GET_MARKER_DATA (marker_data);
			    g_free(marker_data);
			    marker_data = NULL;

			    *((GString **)value) = g_string_sized_new(size);
			    for (sz = 0; sz < size; sz++) {
				    gchar c;

				    c = g_data_input_stream_read_byte(stream, cancellable, error);
				    if (*error)
					    goto end;

				    g_string_append_c(*((GString **)value), c);
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_CARD16:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    guint16 n;

					    n = g_data_input_stream_read_uint16(stream, NULL, error);
					    if (*error)
						    goto end;
					    *((GSList **)value) = g_slist_append(*((GSList **)value), GUINT_TO_POINTER ((guint)n));
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    if ((size % 2) != 0) {
					    g_set_error(error, G_XIM_PROTOCOL_ERROR,
							G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
							"The length of bytes for value type %s has to be an even number: %" G_GSIZE_FORMAT,
							g_xim_value_type_name(vtype), size);
					    goto end;
				    }
				    while (size > 0) {
					    guint16 n;

					    n = g_data_input_stream_read_uint16(stream, NULL, error);
					    if (*error)
						    goto end;
					    *((GSList **)value) = g_slist_append(*((GSList **)value), GUINT_TO_POINTER ((guint)n));
					    size -= 2;
				    }
			    }
			    break;
		    case G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    for (sz = 0; sz < size; sz++) {
					    GXimHotkeyTrigger *v;

					    v = g_xim_hotkey_trigger_get_from_stream(proto, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), v);
				    }
			    } else {
				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    GXimHotkeyTrigger *v;

					    v = g_xim_hotkey_trigger_get_from_stream(proto, stream, cancellable, error);
					    if (*error) {
						    if (G_XIM_GERROR_IS_RECOVERABLE (*error)) {
							    g_prefix_error(error, "Error while processing a value type %s: ",
									   g_xim_value_type_name(vtype));
							    g_xim_message_gerror(iface->message, *error);
							    g_clear_error(error);
						    } else {
							    goto end;
						    }
					    }
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    goto end;
					    }
					    *((GSList **)value) = g_slist_append(*((GSList **)value), v);
					    size -= (pos - cur_pos);
				    }
			    }
			    break;
		    case G_XIM_TYPE_SEPARATOR:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimSepNestedList **)value) = g_xim_sep_nested_list_new();
			    break;
		    case G_XIM_TYPE_BYTE:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((gchar *)value) = g_data_input_stream_read_byte(stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_WORD:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((guint16 *)value) = g_data_input_stream_read_uint16(stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_LONG:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((guint32 *)value) = g_data_input_stream_read_uint32(stream, cancellable, error);
			    break;
		    case G_XIM_TYPE_CHAR:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    /* assuming the length of item == the length of bytes */
			    size = G_XIM_GET_MARKER_DATA (marker_data);
			    g_free(marker_data);
			    marker_data = NULL;

			    *((gchar **)value) = g_new0(gchar, size + 1);
			    for (sz = 0; sz < size; sz++) {
				    gchar c;

				    c = g_data_input_stream_read_byte(stream, cancellable, error);
				    if (*error)
					    goto end;

				    (*((gchar **)value))[sz] = c;
			    }
			    (*((gchar **)value))[sz] = 0;
			    break;
		    case G_XIM_TYPE_WINDOW:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    nw = g_data_input_stream_read_uint32(stream, cancellable, error);
			    if (nw) {
				    *((GdkWindow **)value) = g_xim_get_window(g_xim_transport_get_display(G_XIM_TRANSPORT (proto)),
									      nw);
				    if (*((GdkWindow **)value) == NULL) {
					    g_set_error(error, G_XIM_PROTOCOL_ERROR,
							G_XIM_PROTOCOL_ERROR_NO_DATA | G_XIM_NOTICE_ERROR,
							"Unable to get a valid window for %p",
							G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
					    goto end;
				    }
			    } else {
				    /* Window is possible to be set 0 */
				    *((GdkWindow **)value) = NULL;
			    }
			    break;
		    case G_XIM_TYPE_XIMSTYLES:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimStyles **)value) = g_xim_styles_get_from_stream(proto, stream, NULL, error);
			    break;
		    case G_XIM_TYPE_XRECTANGLE:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimRectangle **)value) = g_xim_rectangle_get_from_stream(proto, stream, NULL, error);
			    break;
		    case G_XIM_TYPE_XPOINT:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimPoint **)value) = g_xim_point_get_from_stream(proto, stream, NULL, error);
			    break;
		    case G_XIM_TYPE_XFONTSET:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    *((GXimFontSet **)value) = g_xim_fontset_get_from_stream(proto, stream, NULL, error);
			    break;
		    case G_XIM_TYPE_HOTKEYTRIGGERS:
		    case G_XIM_TYPE_HOTKEYSTATE:
		    case G_XIM_TYPE_STRINGCONVERSION:
		    case G_XIM_TYPE_PREEDITSTATE:
		    case G_XIM_TYPE_RESETSTATE:
			    goto nonsupported;
		    case G_XIM_TYPE_NESTEDLIST:
			    value = va_arg(args, gpointer);
			    if (value == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"value type %s requires an argument to store the result.",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (marker_data == NULL) {
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_ERROR,
						"No the markers for value type %s to determine how much the number of chunk available",
						g_xim_value_type_name(vtype));
				    goto end;
			    }
			    if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				    GXimNestedList *list;
				    GXimConnection *conn;

				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    conn = G_XIM_CONNECTION (proto);
				    /* XXX: NESTEDLIST is only used for IC attributes? */
				    list = g_xim_nested_list_new(G_XIM_ATTR (conn->default_icattr), size);
				    for (sz = 0; sz < size; sz++) {
					    gint16 attr_id;
					    guint16 n;
					    gpointer v;
					    GType gtype;
					    GXimValueType ntype;
					    gchar *name;

					    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
									    G_XIM_TYPE_WORD, &attr_id)) {
					      fail_nested_list_1:
						    g_prefix_error(error, "Error while processing arg %" G_GSIZE_FORMAT " of %" G_GSIZE_FORMAT " in a value type %s: ",
								   sz, size, g_xim_value_type_name(vtype));
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
						    n = g_data_input_stream_read_uint16(stream, cancellable, error);
						    g_seekable_seek(G_SEEKABLE (istream), n,
								    G_SEEK_CUR, cancellable, error);
						    if (*error)
							    goto end;
						    continue;
					    }
					    gtype = g_xim_attr_get_gtype_by_id(G_XIM_ATTR (conn->default_icattr), attr_id);
					    if (gtype == G_TYPE_INVALID) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to get the value type for attr_id %d",
								attr_id);
						    goto fail_nested_list_1;
					    }
					    ntype = g_xim_gtype_to_value_type(gtype);
					    if (ntype == G_XIM_TYPE_INVALID) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to convert the value type `%s' for attr_id %d",
								g_type_name(gtype), attr_id);
						    goto fail_nested_list_1;
					    }
					    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 2,
									    G_XIM_TYPE_MARKER_N_BYTES_2, ntype,
									    ntype, &v)) {
						    /* maybe hard to recover the error here */
						    goto end;
					    }
					    name = g_xim_attr_get_attribute_name(G_XIM_ATTR (conn->default_icattr), attr_id);
					    g_xim_nested_list_append(list, name, v);
					    g_free(name);
					    g_xim_free_by_gtype(gtype, v);
				    }
				    *((GXimNestedList **)value) = list;
			    } else {
				    GXimNestedList *list;
				    GXimConnection *conn;

				    size = G_XIM_GET_MARKER_DATA (marker_data);
				    g_free(marker_data);
				    marker_data = NULL;

				    conn = G_XIM_CONNECTION (proto);
				    /* XXX: NESTEDLIST is only used for IC attributes? */
				    list = g_xim_nested_list_new(G_XIM_ATTR (conn->default_icattr), size);
				    while (size > 0) {
					    goffset cur_pos = g_seekable_tell(G_SEEKABLE (istream)), pos;
					    gint16 attr_id;
					    guint16 n;
					    gpointer v;
					    GType gtype = G_TYPE_INVALID;
					    GXimValueType ntype;
					    gchar *name = NULL;

					    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
									    G_XIM_TYPE_WORD, &attr_id)) {
					      fail_nested_list_2:
						    g_prefix_error(error, "Error while processing a value type %s: ",
								   g_xim_value_type_name(vtype));
						    g_xim_message_gerror(iface->message, *error);
						    g_clear_error(error);
						    n = g_data_input_stream_read_uint16(stream, cancellable, error);
						    g_seekable_seek(G_SEEKABLE (istream), n,
								    G_SEEK_CUR, cancellable, error);
						    if (*error) {
							    g_free(name);
							    goto end;
						    }
						    goto size_check_nested_list;
					    }
					    gtype = g_xim_attr_get_gtype_by_id(G_XIM_ATTR (conn->default_icattr), attr_id);
					    if (gtype == G_TYPE_INVALID) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to get the value type for attr_id %d",
								attr_id);
						    goto fail_nested_list_2;
					    }
					    ntype = g_xim_gtype_to_value_type(gtype);
					    if (ntype == G_XIM_TYPE_INVALID) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to convert the value type `%s' for attr_id %d",
								g_type_name(gtype), attr_id);
						    goto fail_nested_list_2;
					    }

					    name = g_xim_attr_get_attribute_name(G_XIM_ATTR (conn->default_icattr), attr_id);
					    g_xim_message_debug(iface->message, "proto/attr",
								"    %d: Attribute: %s [%s]",
								attr_id, name, g_xim_value_type_name(ntype));

					    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 2,
									    G_XIM_TYPE_MARKER_N_BYTES_2, ntype,
									    ntype, &v)) {
						    /* maybe hard to recover the error here */
						    g_free(name);
						    /* however try to recover the existing data in the nested list as much as possible */
						    G_XIM_GERROR_RESET_NOTICE_FLAG (*error);
						    G_XIM_GERROR_SET_NOTICE_FLAG (*error, G_XIM_NOTICE_WARNING);
						    break;
					    }

				      size_check_nested_list:
					    pos = g_seekable_tell(G_SEEKABLE (istream));
					    if ((pos - cur_pos) > size) {
						    g_set_error(error, G_XIM_PROTOCOL_ERROR,
								G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
								"Unable to compose a value type %s with the remaining packet size: %" G_GSIZE_FORMAT " for %" G_GINT64_FORMAT,
								g_xim_value_type_name(vtype), size, pos - cur_pos);
						    g_free(name);
						    goto end;
					    }

					    if (name) {
						    g_xim_nested_list_append(list, name, v);
						    g_free(name);
						    name = NULL;
					    }
					    g_xim_free_by_gtype(gtype, v);

					    size -= (pos - cur_pos);
				    }
				    *((GXimNestedList **)value) = list;
			    }
			    break;
		    default:
		    nonsupported:
			    g_set_error(error, G_XIM_PROTOCOL_ERROR,
					G_XIM_STD_ERROR_UNSUPPORTED | G_XIM_NOTICE_ERROR,
					"Unknown/unsupported value type to read: %s",
					g_xim_value_type_name(vtype));
			    goto end;
		}
		if (marker_data) {
			goffset pos;

			if (G_XIM_MARKER_IS_ITEM_BASED (marker_data)) {
				g_set_error(error, G_XIM_PROTOCOL_ERROR,
					    G_XIM_STD_ERROR_UNSUPPORTED | G_XIM_NOTICE_ERROR,
					    "Value type %s doesn't support the number-of-item marker",
					    g_xim_value_type_name(vtype));
				goto end;
			}
			size = G_XIM_GET_MARKER_DATA (marker_data);
			pos = g_seekable_tell(G_SEEKABLE (istream));
			if ((pos - cur_pos) != size) {
				g_set_error(error, G_XIM_PROTOCOL_ERROR,
					    G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
					    "Unable to compose a value type %s properly due to the size difference: expected size: %" G_GSIZE_FORMAT ", actual size: %" G_GINT64_FORMAT,
					    g_xim_value_type_name(vtype), size, pos - cur_pos);
				goto end;
			}
			g_free(marker_data);
			marker_data = NULL;
		}
		base_pos = cur_pos;
		if (*error) {
			if (!G_XIM_GERROR_IS_RECOVERABLE (*error))
				goto end;
			g_prefix_error(error, "Error while processing a value type %s: ",
				       g_xim_value_type_name(vtype));
			g_xim_message_gerror(iface->message, *error);
			g_clear_error(error);
		}
	}

  end:
	if (marker_data) {
		/* to avoid the memory leak for the case broke out for errors */
		g_free(marker_data);
		marker_data = NULL;
	}
	if (*error) {
		g_prefix_error(error, "Error while processing a value type %s: ",
			       g_xim_value_type_name(vtype));
		retval = FALSE;
	}
	if (!g_queue_is_empty(markerq)) {
		g_xim_message_bug(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				  "Unused marker(s) given: %u is still in the queue for reading.",
				  g_queue_get_length(markerq));
		while (!g_queue_is_empty(markerq)) {
			marker_data = g_queue_pop_tail(markerq);
			g_xim_message_bug(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					  "  marker type: %s",
					  g_xim_value_type_name(marker_data->where));
			g_free(marker_data);
		}
	}
	g_queue_free(markerq);

	return retval;
}

/**
 * g_xim_protocol_read_format:
 * @proto: a #GXimProtocol.
 * @stream: a #GDataInputStream which contains XIM protocol packets.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a location to store error, or %NULL.
 * @n_params: the number of a set of parameters.
 *
 * Scans XIM protocol packets according to the set of parameters. if any,
 * objects generated against it is stored in the locations pointed to by
 * the pointer arguments. Each pointer argument must be of a type that is
 * appropriate for the value returned by the corresponding value type.
 * See #GXimValueType documentation which objects the value type is supposed
 * to be.
 *
 * Returns: %TRUE to be read successfully.
 */
gboolean
g_xim_protocol_read_format(GXimProtocol      *proto,
			   GDataInputStream  *stream,
			   GCancellable      *cancellable,
			   GError           **error,
			   guint              n_params,
			   ...)
{
	va_list ap;
	gboolean retval;

	va_start(ap, n_params);

	retval = g_xim_protocol_read_vformat(proto, stream, cancellable, error, n_params, ap);

	va_end(ap);

	return retval;
}

/**
 * g_xim_protocol_wait_for_reply:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @error: a location to store error, or %NULL.
 *
 * Waits for a response of @major_opcode and @minor_opcode or %G_XIM_ERROR.
 * this can be used if you need to send a request synchronously.  Note that
 * using this function in the server instance won't works due to how GObject
 * delivers signals and iterates it.
 *
 * Returns: %TRUE the request was successfully done.
 */
gboolean
g_xim_protocol_wait_for_reply(GXimProtocol *proto,
			      GXimOpcode    major_opcode,
			      guint8        minor_opcode,
			      GError      **error)
{
	GXimProtocolPrivate *priv;
	GXimProtocolSyncable *syncable;
	gboolean retval = TRUE;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	priv = g_xim_protocol_get_private(proto);
	syncable = g_new0(GXimProtocolSyncable, 1);
	G_XIM_GERROR_CHECK_ALLOC (syncable, error,
				  G_XIM_PROTOCOL_ERROR, FALSE);

	syncable->major_opcode = major_opcode;
	syncable->minor_opcode = minor_opcode;
	syncable->error        = NULL;

	G_LOCK (g_xim_protocol_syncable);
	g_queue_push_tail(priv->syncableq, syncable);
	G_UNLOCK (g_xim_protocol_syncable);

	while (g_queue_find(priv->syncableq, syncable))
		g_main_context_iteration(NULL, TRUE);

	retval = syncable->result;
	if (syncable->error) {
		retval = FALSE;
		*error = g_error_copy(syncable->error);
		g_error_free(syncable->error);
	}
	g_free(syncable);

	return retval;
}

/**
 * g_xim_protocol_raise_parser_error:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @imid: input-method ID.
 * @icid: input-context ID.
 *
 * Raises an appropriate error to the opposite direction on the connection
 * according to @major_opcode and @minor_opcode.
 *
 * Returns: %TRUE to be sent an error successfully.
 */
gboolean
g_xim_protocol_raise_parser_error(GXimProtocol *proto,
				  guint         major_opcode,
				  guint         minor_opcode,
				  guint         imid,
				  guint         icid)
{
	gboolean ret = FALSE;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), FALSE);

	g_signal_emit(proto, signals[PARSER_ERROR], 0,
		      major_opcode, minor_opcode, imid, icid, &ret);

	return ret;
}

/**
 * g_xim_protocol_add_protocol:
 * @proto: a #GXimProtocol.
 * @closure: a structure of closure.
 *
 * Registers a @closure to add the XIM protocol handler.
 */
void
g_xim_protocol_add_protocol(GXimProtocol        *proto,
			    GXimProtocolClosure *closure)
{
	GXimProtocolPrivate *priv;
	GXimProtocolClosure *c;

	g_return_if_fail (G_IS_XIM_PROTOCOL (proto));
	g_return_if_fail (closure != NULL);
	g_return_if_fail ((priv = g_xim_protocol_get_private(proto)));

	c = g_xim_protocol_lookup_protocol_by_id(proto,
						 closure->major_opcode.v2,
						 closure->minor_opcode);
	if (c != NULL) {
		g_xim_message_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				      "Protocol (major: 0x%x, minor: 0x%x) has already been registered. replacing\n  details: old: %s, new: %s",
				      c->major_opcode.v2, c->minor_opcode,
				      c->name, closure->name);
	}
	g_hash_table_replace(priv->proto_table__named_index,
			     g_ascii_strup(closure->name, -1),
			     closure);
	g_hash_table_replace(priv->proto_table__id_index,
			     G_XIM_OPCODE_KEY (closure->major_opcode.v2, closure->minor_opcode),
			     closure);
}

/**
 * g_xim_protocol_remove_protocol:
 * @proto: a #GXimProtocol.
 * @closure: a structure of closure.
 *
 * Gets rid of @closure to stop XIM protocol handling in @proto.
 */
void
g_xim_protocol_remove_protocol(GXimProtocol        *proto,
			       GXimProtocolClosure *closure)
{
	g_xim_protocol_remove_protocol_by_id(proto,
					     closure->major_opcode.v2,
					     closure->minor_opcode);
}

/**
 * g_xim_protocol_remove_protocol_by_id:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 *
 * Gets rid of XIM protocol handling to stop dealing with @major_opcode and
 * @minor_opcode in @proto.
 */
void
g_xim_protocol_remove_protocol_by_id(GXimProtocol *proto,
				     guint8        major_opcode,
				     guint8        minor_opcode)
{
	GXimProtocolClosure *c;
	GXimProtocolPrivate *priv;

	g_return_if_fail (G_IS_XIM_PROTOCOL (proto));
	g_return_if_fail ((priv = g_xim_protocol_get_private(proto)));

	c = g_xim_protocol_lookup_protocol_by_id(proto, major_opcode, minor_opcode);
	if (c == NULL) {
		g_xim_message_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				      "Protocol (major: 0x%x, minor: 0x%x) isn't yet registered.",
				      major_opcode, minor_opcode);
	} else {
		gchar *name = g_ascii_strup(c->name, -1);

		g_hash_table_remove(priv->proto_table__named_index,
				    name);
		g_hash_table_remove(priv->proto_table__id_index,
				    G_XIM_OPCODE_KEY (major_opcode, minor_opcode));
		g_free(name);
	}
}

/**
 * g_xim_protocol_lookup_protocol_by_name:
 * @proto: a #GXimProtocol.
 * @name: the name of XIM protocol event.
 *
 * Looks up the XIM protocol closure with @name which @proto is dealing with.
 *
 * Returns: a #GXimProtocolClosure corresponding to @name.
 */
GXimProtocolClosure *
g_xim_protocol_lookup_protocol_by_name(GXimProtocol *proto,
				       const gchar  *name)
{
	GXimProtocolPrivate *priv;
	GXimProtocolClosure *retval;
	gchar *s;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), NULL);

	s = g_ascii_strup(name, -1);
	retval = g_hash_table_lookup(priv->proto_table__named_index,
				     s);
	g_free(s);

	return retval;
}

/**
 * g_xim_protocol_lookup_protocol_by_id:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 *
 * Looks up the XIM protocol closure with @major_opcode and @minor_opcode which
 * @proto is dealing with.
 *
 * Returns: a #GXimProtocolClosure corresponding to @major_opcode and @minor_opcode.
 */
GXimProtocolClosure *
g_xim_protocol_lookup_protocol_by_id(GXimProtocol *proto,
				     guint8        major_opcode,
				     guint8        minor_opcode)
{
	GXimProtocolPrivate *priv;
	GXimProtocolClosure *retval;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), NULL);

	retval = g_hash_table_lookup(priv->proto_table__id_index,
				     G_XIM_OPCODE_KEY (major_opcode, minor_opcode));

	return retval;
}

/**
 * g_xim_protocol_connect_closure_by_id:
 * @proto: a #GXimProtocol.
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @func: the #GCallback to connect.
 * @data: data to pass to @func.
 *
 * Connects a #GCallback function to a signal for XIM protocol events.
 *
 * Returns: the handler id.
 */
gulong
g_xim_protocol_connect_closure_by_id(GXimProtocol *proto,
				     guint8        major_opcode,
				     guint8        minor_opcode,
				     GCallback     func,
				     gpointer      data)
{
	GXimProtocolClosure *closure;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);

	closure = g_xim_protocol_lookup_protocol_by_id(proto, major_opcode, minor_opcode);
	if (closure == NULL) {
		g_xim_message_bug(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				  "No closure for protocol major:0x%x, minor:0x%x",
				  major_opcode, minor_opcode);
		return 0;
	}

	return g_xim_protocol_closure_connect(closure, func, data);
}

/**
 * g_xim_protocol_connect_closure_by_name:
 * @proto: a #GXimProtocol.
 * @signal_name: a XIM protocol event name.
 * @func: the #GCallback to connect.
 * @data: data to pass to @func.
 *
 * Connects a #GCallback function to a signal for XIM protocol events.
 *
 * Returns: the handler id.
 */
gulong
g_xim_protocol_connect_closure_by_name(GXimProtocol *proto,
				       const gchar  *signal_name,
				       GCallback     func,
				       gpointer      data)
{
	GXimProtocolClosure *closure;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);

	closure = g_xim_protocol_lookup_protocol_by_name(proto, signal_name);
	if (closure == NULL) {
		g_xim_message_bug(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				  "No closure for protocol `%s'",
				  signal_name);
		return 0;
	}

	return g_xim_protocol_closure_connect(closure, func, data);
}

/**
 * g_xim_protocol_get_extensions:
 * @proto: a #GXimProtocol.
 *
 * Obtains a set of #GXimProtocolClosure registered as an extension. this is
 * helpful to deal with %G_XIM_QUERY_EXTENSION and so on.
 *
 * Returns: the #GSList which contains #GXimProtocolClosure.
 */
GSList *
g_xim_protocol_get_extensions(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv;
	GXimProtocolClosure *c;
	GHashTableIter iter;
	GSList *retval = NULL;
	gpointer key, val;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail ((priv = g_xim_protocol_get_private(proto)), NULL);

	g_hash_table_iter_init(&iter, priv->proto_table__id_index);
	while (g_hash_table_iter_next(&iter, &key, &val)) {
		c = val;

		if (g_xim_protocol_closure_is_an_extension(c))
			retval = g_slist_append(retval, val);
	}

	return retval;
}

/**
 * g_xim_protocol_closure_new:
 * @major_opcode: a major opcode in XIM protocol.
 * @minor_opcode: a minor opcode in XIM protocol.
 * @name: the XIM protocol name.
 * @is_an_extension: %TRUE to define an extension in XIM protocol, otherwise %FALSE.
 *
 * Creates an instance of the #GXimProtocolClosure.
 *
 * Returns: the #GXimProtocolClosure.
 */
GXimProtocolClosure *
g_xim_protocol_closure_new(guint8       major_opcode,
			   guint8       minor_opcode,
			   const gchar *name,
			   gboolean     is_an_extension)
{
	GClosure *closure;
	GXimProtocolClosure *retval;

	g_return_val_if_fail (name != NULL, NULL);

	closure = g_closure_new_simple(sizeof (GXimProtocolClosure), NULL);
	G_XIM_CHECK_ALLOC (closure, NULL);

	retval = (GXimProtocolClosure *)closure;
	retval->name = g_strdup(name);
	retval->major_opcode.v2 = major_opcode;
	retval->minor_opcode = minor_opcode;
	retval->is_an_extension = is_an_extension;

	g_closure_add_finalize_notifier(closure, NULL, g_xim_protocol_closure_free);

	return retval;
}

void
g_xim_protocol_closure_free(gpointer  data,
			    GClosure *closure)
{
	GXimProtocolClosure *c;

	g_return_if_fail (closure != NULL);

	c = (GXimProtocolClosure *)closure;
	g_free(c->param_types);
	g_free(c->name);
	g_slist_foreach(c->signal_handlers, (GFunc)g_free, NULL);
	g_slist_free(c->signal_handlers);
}

/**
 * g_xim_protocol_closure_is_an_extension:
 * @closure: a #GXimProtocolClosure.
 *
 * Checks if the @closure was created as an extension protocol in XIM.
 *
 * Returns: %TRUE to be an extension.
 */
gboolean
g_xim_protocol_closure_is_an_extension(GXimProtocolClosure *closure)
{
	g_return_val_if_fail (closure != NULL, FALSE);

	return closure->is_an_extension;
}

/**
 * g_xim_protocol_closure_set_marshal:
 * @closure: a #GXimProtocolClosure.
 * @func: the #GXimProtocolClosureFunc function to scan the XIM protocol packet
 *  and emits the proper signal.
 * @data: the data to store in the @data field of the #GClosure.
 *
 * Sets the marshaller of @closure which scans the XIM protocol packets and emit
 * the proper signal as needed.  Note that g_xim_protocol_closure_emit_signal()
 * has to be invoked in @func otherwise any signals added by
 * g_xim_protocol_closure_add_signal() won't do anything at all.
 */
void
g_xim_protocol_closure_set_marshal(GXimProtocolClosure     *closure,
				   GXimProtocolClosureFunc  func,
				   gpointer                 data)
{
	g_return_if_fail (closure != NULL);
	g_return_if_fail (func != NULL);

	((GCClosure *)closure)->callback = (gpointer)func;
	((GClosure *)closure)->data = data;
	g_closure_set_marshal((GClosure *)closure,
			      g_xim_protocol_closure_marshal_BOOLEAN__OBJECT_OBJECT_POINTER);
}

/**
 * g_xim_protocol_closure_add_signal:
 * @closure: a #GXimProtocolClosure.
 * @marshaller: the #GClosureMarshal to deal with the signal.
 * @n_params: the number of parameters for the signal.
 * @...: a list of types, one for each parameter.
 *
 * Adds a marshaller for signal.
 */
void
g_xim_protocol_closure_add_signal(GXimProtocolClosure *closure,
				  GClosureMarshal      marshaller,
				  guint                n_params,
				  ...)
{
	va_list ap;
	guint i;
	GType gtype;

	g_return_if_fail (closure != NULL);
	g_return_if_fail (marshaller != NULL);
	g_return_if_fail (closure->param_types == NULL);

	va_start(ap, n_params);

	closure->param_types = g_new0(GType, n_params);
	for (i = 0; i < n_params; i++) {
		gtype = va_arg(ap, GType);
		if (!G_TYPE_IS_VALUE (gtype & ~G_SIGNAL_TYPE_STATIC_SCOPE)) {
			const gchar *name;

			g_warning("parameter %d of type `%s' for protocol \"%s[0x%x:0x%x]\" is not a value type",
				  i + 1, gtype ? ((name = g_type_name(gtype)) ? name : "<unknown>") : "<invalid>",
				  closure->name, closure->major_opcode.v1, closure->minor_opcode);
			g_free(closure->param_types);
			closure->param_types = NULL;
			goto end;
		}
		closure->param_types[i] = gtype;
	}
	closure->signal_marshaller = marshaller;
	closure->n_params = n_params;
  end:

	va_end(ap);
}

/**
 * g_xim_protocol_closure_connect:
 * @closure: a #GXimProtocolClosure.
 * @func: the #GCallback to connect.
 * @data: data to pass to @func.
 *
 * Connects a closure to a signal.
 *
 * Returns: the handler id.
 */
gulong
g_xim_protocol_closure_connect(GXimProtocolClosure *closure,
			       GCallback            func,
			       gpointer             data)
{
	GXimProtocolClosureNode *node;

	g_return_val_if_fail (closure != NULL, 0);
	g_return_val_if_fail (closure->signal_marshaller != NULL, 0);
	g_return_val_if_fail (func != NULL, 0);

	node = g_new0(GXimProtocolClosureNode, 1);
	G_XIM_CHECK_ALLOC (node, 0);

	node->func = func;
	node->user_data = data;
	closure->signal_handlers = g_slist_prepend(closure->signal_handlers, node);

	return (gulong)node;
}

/**
 * g_xim_protocol_closure_disconnect:
 * @closure: a #GXimProtocolClosure.
 * @id: Handler id of the handler to be disconnected.
 *
 * Disconnects a handler from an instance so it will not be called during
 * any future or currently ongoing emissions of the signal if has been
 * connected to. The @id becomes invalid and may be reused.
 *
 * The @id has to be a valid signal handler id, connected to a
 * signal of @closure.
 */
void
g_xim_protocol_closure_disconnect(GXimProtocolClosure *closure,
				  gulong               id)
{
	GSList *l;

	g_return_if_fail (closure != NULL);
	g_return_if_fail (id != 0);

	l = g_slist_find(closure->signal_handlers, (gpointer)id);
	if (l) {
		g_free(l->data);
		closure->signal_handlers = g_slist_delete_link(closure->signal_handlers, l);
	}
}

/**
 * g_xim_protocol_closure_emit_signal:
 * @closure: a #GXimProtocolClosure.
 * @proto: the instance which wants to raise a signal with @closure.
 * @...: parameters to be passed to the signal.
 *
 * Emits a signal.
 *
 * Returns: %TRUE to be proceeded a signal in the handlers. %FALSE to be failed
 *  in any handlers or no handlers for this signal.
 */
gboolean
g_xim_protocol_closure_emit_signal(GXimProtocolClosure *closure,
				   GXimProtocol        *proto,
				   ...)
{
	va_list ap;
	GValue v = { 0, }, *pv;
	GClosure *c;
	GSList *l;
	gboolean ret = FALSE;
	guint i;
	const gchar *name;

	g_return_val_if_fail (closure != NULL, FALSE);
	g_return_val_if_fail (closure->signal_marshaller != NULL, FALSE);

	va_start(ap, proto);

	g_value_init(&v, G_TYPE_BOOLEAN);
	pv = g_new0(GValue, closure->n_params + 1);
	G_XIM_CHECK_ALLOC (pv, FALSE);

	g_value_init(&pv[0], G_TYPE_OBJECT);
	g_value_set_object(&pv[0], proto);

	for (i = 0; i < closure->n_params; i++) {
		if (g_type_is_a(closure->param_types[i], G_TYPE_OBJECT))
			g_value_init(&pv[i + 1], G_TYPE_OBJECT);
		else
			g_value_init(&pv[i + 1], closure->param_types[i]);
		switch (closure->param_types[i]) {
		    case G_TYPE_BOOLEAN:
			    g_value_set_boolean(&pv[i + 1], va_arg(ap, gboolean));
			    break;
		    case G_TYPE_CHAR:
			    g_value_set_char(&pv[i + 1], va_arg(ap, gint));
			    break;
		    case G_TYPE_UCHAR:
			    g_value_set_uchar(&pv[i + 1], va_arg(ap, guint));
			    break;
		    case G_TYPE_INT:
			    g_value_set_int(&pv[i + 1], va_arg(ap, gint));
			    break;
		    case G_TYPE_UINT:
			    g_value_set_uint(&pv[i + 1], va_arg(ap, guint));
			    break;
		    case G_TYPE_LONG:
			    g_value_set_long(&pv[i + 1], va_arg(ap, glong));
			    break;
		    case G_TYPE_ULONG:
			    g_value_set_ulong(&pv[i + 1], va_arg(ap, gulong));
			    break;
		    case G_TYPE_INT64:
			    g_value_set_int64(&pv[i + 1], va_arg(ap, gint64));
			    break;
		    case G_TYPE_UINT64:
			    g_value_set_uint64(&pv[i + 1], va_arg(ap, guint64));
			    break;
		    case G_TYPE_FLOAT:
			    g_value_set_float(&pv[i + 1], va_arg(ap, gdouble));
			    break;
		    case G_TYPE_DOUBLE:
			    g_value_set_double(&pv[i + 1], va_arg(ap, gdouble));
			    break;
		    case G_TYPE_STRING:
			    g_value_set_string(&pv[i + 1], va_arg(ap, gchar *));
			    break;
		    case G_TYPE_POINTER:
			    g_value_set_pointer(&pv[i + 1], va_arg(ap, gpointer));
			    break;
		    case G_TYPE_ENUM:
			    g_value_set_enum(&pv[i + 1], va_arg(ap, gint));
			    break;
		    case G_TYPE_FLAGS:
			    g_value_set_flags(&pv[i + 1], va_arg(ap, guint));
			    break;
		    case G_TYPE_PARAM:
			    g_value_set_param(&pv[i + 1], va_arg(ap, GParamSpec *));
			    break;
		    case G_TYPE_BOXED:
			    g_value_set_boxed(&pv[i + 1], va_arg(ap, gpointer));
			    break;
		    default:
			    if (g_type_is_a(closure->param_types[i], G_TYPE_GTYPE)) {
				    g_value_set_gtype(&pv[i + 1], va_arg(ap, GType));
			    } else if (g_type_is_a(closure->param_types[i], G_TYPE_OBJECT)) {
				    g_value_set_object(&pv[i + 1], va_arg(ap, gpointer));
			    } else if (G_TYPE_IS_VALUE (closure->param_types[i])) {
				    g_value_set_boxed(&pv[i + 1], va_arg(ap, gpointer));
			    } else {
				    g_xim_message_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
							  "Unsupported value type `%s' to emit a signal for %s",
							  closure->param_types[i] ? ((name = g_type_name(closure->param_types[i])) ? name : "<unknown>") : "<invalid>",
							  closure->name);
				    goto end;
			    }
			    break;
		}
	}

	for (l = closure->signal_handlers; l != NULL; l = g_slist_next(l)) {
		GXimProtocolClosureNode *node = l->data;

		c = g_cclosure_new(node->func, node->user_data, NULL);
		G_XIM_CHECK_ALLOC (c, FALSE);

		g_closure_set_marshal(c, closure->signal_marshaller);
		g_closure_invoke(c, &v, closure->n_params + 1, pv, NULL);

		ret = g_value_get_boolean(&v);

		g_closure_unref(c);
		if (ret)
			break;
	}
	for (i = 0; i < closure->n_params + 1; i++) {
		g_value_unset(&pv[i]);
	}

  end:
	g_free(pv);

	va_end(ap);

	return ret;
}
