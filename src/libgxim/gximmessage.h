/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmessage.h
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
#ifndef __G_XIM_MESSAGE_H__
#define __G_XIM_MESSAGE_H__

#include <glib.h>
#include <glib-object.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_MESSAGE		(g_xim_message_get_type())
#define G_XIM_MESSAGE(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_MESSAGE, GXimMessage))
#define G_XIM_MESSAGE_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_MESSAGE, GXimMessageClass))
#define G_IS_XIM_MESSAGE(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_MESSAGE))
#define G_IS_XIM_MESSAGE_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_MESSAGE))
#define G_XIM_MESSAGE_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_MESSAGE, GXimMessageClass))

/**
 * g_xim_message_gerror:
 * @_i_: a #GXimMessage.
 * @_e_: a #GError to output.
 *
 * Outputs a #GError with g_xim_message_printf().
 *
 * This is a convenience macro to output a message. the message type depends
 * on the error code which would be logically added with #GXimErrorType.
 */
#define g_xim_message_gerror(_i_,_e_)					\
	G_STMT_START {							\
		if ((_e_)->code & G_XIM_NOTICE_CRITICAL) {		\
			g_xim_message_critical((_i_), "%s", (_e_)->message); \
		} else if ((_e_)->code & G_XIM_NOTICE_ERROR) {		\
			g_xim_message_error((_i_), "%s", (_e_)->message); \
		} else if ((_e_)->code & G_XIM_NOTICE_BUG) {		\
			g_xim_message_bug((_i_), "%s", (_e_)->message);	\
		} else if ((_e_)->code & G_XIM_NOTICE_WARNING) {	\
			g_xim_message_warning((_i_), "%s", (_e_)->message); \
		} else {						\
			g_xim_message_error((_i_), "%s", (_e_)->message); \
		}							\
	} G_STMT_END

/**
 * g_xim_message:
 * @_i_: a #GXimMessage.
 * @_f_: a unique filter name.
 * @...: arguments include the format string
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_MESSAGE.
 */
/**
 * g_xim_message_info:
 * @_i_: a #GXimMessage.
 * @_f_: a unique filter name.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_INFO.
 */
/**
 * g_xim_message_warning:
 * @_i_: a #GXimMessage.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_WARNING.
 */
/**
 * g_xim_message_error:
 * @_i_: a #GXimMessage.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_ERROR.
 */
/**
 * g_xim_message_critical:
 * @_i_: a #GXimMessage.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_CRITICAL.
 */
/**
 * g_xim_message_debug:
 * @_i_: a #GXimMessage.
 * @_f_: a unique filter name.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_DEBUG.
 */
/**
 * g_xim_message_bug:
 * @_i_: a #GXimMessage.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_message_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGE_BUG.
 */
#ifdef G_HAVE_ISO_VARARGS
#define g_xim_message(_i_,_f_,...)		g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_MESSAGE, \
								     __VA_ARGS__)
#define g_xim_message_info(_i_,_f_,...)		g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_INFO, \
								     __VA_ARGS__)
#define g_xim_message_warning(_i_,...)		g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_WARNING, \
								     __VA_ARGS__)
#define g_xim_message_error(_i_,...)		g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_ERROR, \
								     __VA_ARGS__)
#define g_xim_message_critical(_i_,...)		g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_CRITICAL, \
								     __VA_ARGS__)
#define g_xim_message_debug(_i_,_f_,...)	g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_DEBUG, \
								     __VA_ARGS__)
#define g_xim_message_bug(_i_,...)		g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_BUG,	\
								     __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_xim_message(_i_,_f_,format...)	g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_MESSAGE, \
								     format)
#define g_xim_message_info(_i_,_f_,format...)	g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_INFO, \
								     format)
#define g_xim_message_warning(_i_,format...)	g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_WARNING, \
								     format)
#define g_xim_message_error(_i_,format...)	g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_ERROR, \
								     format)
#define g_xim_message_critical(_i_,format...)	g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_CRITICAL, \
								     format)
#define g_xim_message_debug(_i_,_f_,format...)	g_xim_message_printf((_i_), (_f_), \
								     G_XIM_MESSAGE_DEBUG, \
								     format)
#define g_xim_message_bug(_i_,format__)		g_xim_message_printf((_i_), NULL, \
								     G_XIM_MESSAGE_BUG, \
								     format)
#else /* no varargs macros */
static void
g_xim_message(GXimMessage *message,
	      const gchar *filter,
	      const gchar *format,
	      ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, filter, G_XIM_MESSAGE_MESSAGE, format, ap);
	va_end(ap);
}
static void
g_xim_message_info(GXimMessage *message,
		   const gchar *filter,
		   const gchar *format,
		   ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, filter, G_XIM_MESSAGE_INFO, format, ap);
	va_end(ap);
}
static void
g_xim_message_warning(GXimMessage *message,
		      const gchar *format,
		      ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, NULL, G_XIM_MESSAGE_WARNING, format, ap);
	va_end(ap);
}
static void
g_xim_message_error(GXimMessage *message,
		    const gchar *format,
		    ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, NULL, G_XIM_MESSAGE_ERROR, format, ap);
	va_end(ap);
}
static void
g_xim_message_critical(GXimMessage *message,
		       const gchar *format,
		       ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, NULL, G_XIM_MESSAGE_CRITICAL, format, ap);
	va_end(ap);
}
static void
g_xim_message_debug(GXimMessage *message,
		    const gchar *filter,
		    const gchar *format,
		    ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, filter, G_XIM_MESSAGE_DEBUG, format, ap);
	va_end(ap);
}
static void
g_xim_message_bug(GXimMessage *message,
		  const gchar *format,
		  ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_message_vprintf(message, NULL, G_XIM_MESSAGE_BUG, format, ap);
	va_end(ap);
}
#endif


/**
 * GXimMessageType:
 * @G_XIM_MESSAGE_MESSAGE: a message type for usual messages, see g_xim_message().
 * @G_XIM_MESSAGE_DEBUG: a message type for debug, see g_xim_message_debug().
 * @G_XIM_MESSAGE_INFO: a message type for information, see g_xim_message_info().
 * @G_XIM_MESSAGE_WARNING: a message type for warnings, see g_xim_message_warning().
 * @G_XIM_MESSAGE_ERROR: a message type for errors, see g_xim_message_error().
 * @G_XIM_MESSAGE_CRITICAL: a message type for critical errors, see g_xim_message_critical().
 * @G_XIM_MESSAGE_BUG: a message type for bugs, see g_xim_message_bug().
 *
 * Flags specifying the type of messages.
 */
typedef enum {
	G_XIM_MESSAGE_MESSAGE,
	G_XIM_MESSAGE_DEBUG,
	G_XIM_MESSAGE_INFO,
	G_XIM_MESSAGE_WARNING,
	G_XIM_MESSAGE_ERROR,
	G_XIM_MESSAGE_CRITICAL,
	G_XIM_MESSAGE_BUG
} GXimMessageType;

/**
 * GXimMessage:
 * @parent: a #GObject.
 *
 * An implementation of message handler class
 **/
typedef struct _GXimMessageClass	GXimMessageClass;

struct _GXimMessage {
	GObject  parent_instance;

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

struct _GXimMessageClass {
	GObjectClass  parent_class;

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

GType        g_xim_message_get_type    (void) G_GNUC_CONST;
GXimMessage *g_xim_message_new          (void);
void         g_xim_message_activate     (GXimMessage     *message,
                                         gboolean         flag);
void         g_xim_message_set_filename (GXimMessage     *message,
                                         const gchar     *filename);
void         g_xim_message_clear_filter (GXimMessage     *message);
void         g_xim_message_enable_filter(GXimMessage     *message,
                                         const gchar     *filter_name);
void         g_xim_message_vprintf      (GXimMessage     *message,
                                         const gchar     *filter,
                                         GXimMessageType  type,
                                         const gchar     *format,
                                         va_list          args);
void         g_xim_message_printf       (GXimMessage     *message,
                                         const gchar     *filter,
                                         GXimMessageType  type,
                                         const gchar     *format,
					 ...) G_GNUC_PRINTF (4, 5);

G_END_DECLS

#endif /* __G_XIM_MESSAGE_H__ */
