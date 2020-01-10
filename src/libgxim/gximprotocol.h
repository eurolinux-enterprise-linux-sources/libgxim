/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximprotocol.h
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
#ifndef __G_XIM_PROTOCOL_H__
#define __G_XIM_PROTOCOL_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_PROTOCOL		(g_xim_protocol_get_type())
#define G_XIM_PROTOCOL(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_PROTOCOL, GXimProtocol))
#define G_IS_XIM_PROTOCOL(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_PROTOCOL))
#define G_XIM_PROTOCOL_GET_IFACE(_o_)	(G_TYPE_INSTANCE_GET_INTERFACE ((_o_), G_TYPE_XIM_PROTOCOL, GXimProtocolIface))

/**
 * G_XIM_PROTOCOL_ERROR:
 *
 * Error domain for #GXimProtocol. Errors in this domain will be from
 * the #GXimProtocolError or #GXimStandardError enumeration.
 * See #GError for more information on error domains.
 */
#define G_XIM_PROTOCOL_ERROR		(g_xim_protocol_get_error_quark())

/**
 * g_xim_protocol_n_pad4:
 * @_n_: a number
 *
 * Generates a number aligned to 4 bytes as needed.
 */
#define g_xim_protocol_n_pad4(_n_)	((4 - ((_n_) % 4)) % 4)
/**
 * G_XIM_OPCODE_KEY:
 * @_m_: a major opcode in XIM protocol.
 * @_n_: a minor opcode in XIM protocol.
 *
 * Generates a key from opcodes.
 */
#define G_XIM_OPCODE_KEY(_m_,_n_)	GUINT_TO_POINTER ((_m_) << 8 | ((_n_) & 0xff))


/**
 * GXimProtocolError:
 * @G_XIM_PROTOCOL_ERROR_BEGIN: Unused.
 * @G_XIM_PROTOCOL_ERROR_UNKNOWN_ENDIAN: Packets is sent with the unknown endian.
 * @G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED: Not parserable packets received.
 * @G_XIM_PROTOCOL_ERROR_DELIVERY_FAILURE: Unable to deliver the signal properly.
 * @G_XIM_PROTOCOL_ERROR_NO_PARSER: No handlers available to parse packets.
 * @G_XIM_PROTOCOL_ERROR_NO_DATA: No data received in a request.
 *
 * Error codes returned by #GXimProtocol functions.
 */
typedef enum {
	G_XIM_PROTOCOL_ERROR_BEGIN = 128,
	G_XIM_PROTOCOL_ERROR_UNKNOWN_ENDIAN,
	G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED,
	G_XIM_PROTOCOL_ERROR_DELIVERY_FAILURE,
	G_XIM_PROTOCOL_ERROR_NO_PARSER,
	G_XIM_PROTOCOL_ERROR_NO_DATA,
} GXimProtocolError;

typedef struct _GXimProtocolIface		GXimProtocolIface;
typedef struct _GXimProtocolPrivate		GXimProtocolPrivate;
typedef struct _GXimProtocolQueueNode		GXimProtocolQueueNode;

/**
 * GXimProtocolIface:
 * @parent_iface: a parent.
 * @message: a #GXimMessages.
 * @atom_xim_protocol: a #GdkAtom for %_XIM_PROTOCOL.
 * @atom_xim_moredata: a #GdkAtom for %_XIM_MOREDATA.
 *
 * An interface of XIM protocol.
 */
struct _GXimProtocolIface {
	GTypeInterface  parent_iface;

	GXimMessages   *message;
	GdkAtom         atom_xim_protocol;
	GdkAtom         atom_xim_moredata;

	/*< private >*/
	gboolean (* parser_error) (GXimProtocol *proto,
				   guint         major_opcode,
				   guint         minor_opcode,
				   guint         imid,
				   guint         icid);
	void     (* reserved1)    (void);
	void     (* reserved2)    (void);
	void     (* reserved3)    (void);
	void     (* reserved4)    (void);
};

struct _GXimProtocolPrivate {
	/*< protected >*/
	GOutputStream        *base_send_ostream;
	GOutputStream        *base_recv_ostream;
	GDataOutputStream    *send_ostream;
	GDataOutputStream    *recv_ostream;
	GHashTable           *proto_table__named_index;
	GHashTable           *proto_table__id_index;
	GQueue               *markerq;
	GQueue               *syncableq;
	GQueue               *sendq;
	gulong                base_signal_ids[LAST_XIM_EVENTS];
	gulong                signal_ids[LAST_XIM_EVENTS];
	GDataStreamByteOrder  byte_order;
	gboolean              is_disconnected;
	/* XXX: workaround for failing to get the number of bytes written */
	gssize                n_sent;
	gssize                n_received;
};

/**
 * GXimProtocolQueueNode:
 * @data: the packet data received.
 * @length: the number of bytes of @data.
 * @major_opcode: the major opcode in XIM protocol
 * @minor_opcode: the minor opcode in XIM protocol
 * @imid: the input-method ID
 * @icid: the input-context ID
 * @is_sent:
 *
 */
struct _GXimProtocolQueueNode {
	gchar    *data;
	gsize     length;
	guint16   major_opcode;
	guint16   minor_opcode;
	guint16   imid;
	guint16   icid;
	gboolean  is_sent;
};


GType                  g_xim_protocol_get_type               (void) G_GNUC_CONST;
void                   g_xim_protocol_init                   (GXimProtocol        *proto);
void                   g_xim_protocol_dispose                (GObject             *object);
void                   g_xim_protocol_finalize               (GObject             *object);
GXimProtocolPrivate   *g_xim_protocol_get_private            (GXimProtocol        *proto);
GQuark                 g_xim_protocol_get_error_quark        (void);
gboolean               g_xim_protocol_process_event          (GXimProtocol        *proto,
                                                              GdkEventClient      *event,
                                                              GError              **error);
gboolean               g_xim_protocol_translate              (GXimProtocol        *proto,
                                                              gpointer             data,
                                                              gssize               length,
                                                              GError              **error);
gboolean               g_xim_protocol_send_packets           (GXimProtocol        *proto,
                                                              const gchar         *data,
                                                              gsize                length);
gsize                  g_xim_protocol_send_vformat           (GXimProtocol        *proto,
                                                              GCancellable        *cancellable,
                                                              GError              **error,
                                                              guint                n_params,
                                                              va_list              args);
gsize                  g_xim_protocol_send_format            (GXimProtocol        *proto,
                                                              GCancellable        *cancellable,
                                                              GError              **error,
                                                              guint                n_params,
                                                              ...);
gboolean               g_xim_protocol_send                   (GXimProtocol        *proto,
                                                              GXimOpcode           major_opcode,
                                                              guint8               minor_opcode,
                                                              guint                n_params,
                                                              ...);
gboolean               g_xim_protocol_send_with_list         (GXimProtocol        *proto,
                                                              GXimOpcode           major_opcode,
                                                              guint8               minor_opcode,
                                                              GSList              *types,
                                                              GSList              *values);
gboolean               g_xim_protocol_start_queue            (GXimProtocol        *proto);
GXimProtocolQueueNode *g_xim_protocol_end_queue              (GXimProtocol        *proto);
void                   g_xim_protocol_cancel_queue           (GXimProtocol        *proto);
gboolean               g_xim_protocol_is_queued              (GXimProtocol        *proto);
guint                  g_xim_protocol_get_queue_length       (GXimProtocol        *proto);
gboolean               g_xim_protocol_read_vformat           (GXimProtocol        *proto,
                                                              GDataInputStream    *stream,
                                                              GCancellable        *cancellable,
                                                              GError              **error,
                                                              guint                n_params,
                                                              va_list              args);
gboolean               g_xim_protocol_read_format            (GXimProtocol        *proto,
                                                              GDataInputStream    *stream,
                                                              GCancellable        *cancellable,
                                                              GError              **error,
                                                              guint                n_params,
                                                              ...);
gboolean               g_xim_protocol_wait_for_reply         (GXimProtocol        *proto,
                                                              GXimOpcode           major_opcode,
                                                              guint8               minor_opcode,
                                                              GError              **error);
gboolean               g_xim_protocol_raise_parser_error     (GXimProtocol        *proto,
							      guint                major_opcode,
							      guint                minor_opcode,
							      guint                imid,
							      guint                icid);
void                   g_xim_protocol_add_protocol           (GXimProtocol        *proto,
                                                              GXimProtocolClosure *closure);
void                   g_xim_protocol_remove_protocol        (GXimProtocol        *proto,
                                                              GXimProtocolClosure *closure);
void                   g_xim_protocol_remove_protocol_by_id  (GXimProtocol        *proto,
                                                              guint8               major_opcode,
                                                              guint8               minor_opcode);
GXimProtocolClosure   *g_xim_protocol_lookup_protocol_by_name(GXimProtocol        *proto,
                                                              const gchar         *name);
GXimProtocolClosure   *g_xim_protocol_lookup_protocol_by_id  (GXimProtocol        *proto,
                                                              guint8               major_opcode,
                                                              guint8               minor_opcode);
gulong                 g_xim_protocol_connect_closure_by_id  (GXimProtocol        *proto,
                                                              guint8               major_opcode,
                                                              guint8               minor_opcode,
                                                              GCallback            func,
                                                              gpointer             data);
gulong                 g_xim_protocol_connect_closure_by_name(GXimProtocol        *proto,
                                                              const gchar         *signal_name,
                                                              GCallback            func,
                                                              gpointer             data);
GSList                *g_xim_protocol_get_extensions         (GXimProtocol        *proto);


/**
 * GXimProtocolClosure:
 *
 **/
typedef gboolean (* GXimProtocolClosureFunc)	(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data);

struct _GXimProtocolClosure {
	GCClosure        closure;
	GClosureMarshal  signal_marshaller;
	guint            n_params;
	GType           *param_types;
	GSList          *signal_handlers;
	gchar           *name;
	union {
		GXimOpcode v1;
		guint8     v2;
	} major_opcode;
	guint8           minor_opcode;
	gboolean         is_an_extension;
};

GXimProtocolClosure *g_xim_protocol_closure_new            (guint8                   major_opcode,
							    guint8                   minor_opcode,
							    const gchar             *name,
							    gboolean                 is_an_extension);
void                 g_xim_protocol_closure_free           (gpointer                 data,
							    GClosure                *closure);
gboolean             g_xim_protocol_closure_is_an_extension(GXimProtocolClosure     *closure);
void                 g_xim_protocol_closure_set_marshal    (GXimProtocolClosure     *closure,
							    GXimProtocolClosureFunc  func,
							    gpointer                 data);
void                 g_xim_protocol_closure_add_signal     (GXimProtocolClosure     *closure,
							    GClosureMarshal          marshaller,
							    guint                    n_params,
							    ...);
gulong               g_xim_protocol_closure_connect        (GXimProtocolClosure     *closure,
							    GCallback                func,
							    gpointer                 data);
void                 g_xim_protocol_closure_disconnect     (GXimProtocolClosure     *closure,
							    gulong                   id);
gboolean             g_xim_protocol_closure_emit_signal    (GXimProtocolClosure     *closure,
							    GXimProtocol            *proto,
							    ...) G_GNUC_WARN_UNUSED_RESULT;

/**
 * GXimAttr:
 *
 **/
gsize    g_xim_attr_put_attribute_to_stream  (GXimProtocol      *proto,
                                              GXimAttr          *attr,
                                              guint              attribute_id,
                                              GCancellable      *cancellable,
                                              GError           **error);
gpointer g_xim_attr_get_attribute_from_stream(GXimProtocol      *proto,
                                              GXimAttr          *attr,
                                              GDataInputStream  *stream,
                                              GCancellable      *cancellable,
                                              GError           **error);


G_END_DECLS

#endif /* __G_XIM_PROTOCOL_H__ */
