/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmessages.h
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
#ifndef __G_XIM_MESSAGES_H__
#define __G_XIM_MESSAGES_H__

#include <glib.h>
#include <glib-object.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_TYPE_XIM_MESSAGES		(g_xim_messages_get_type())
#define G_XIM_MESSAGES(_o_)		(G_TYPE_CHECK_INSTANCE_CAST ((_o_), G_TYPE_XIM_MESSAGES, GXimMessages))
#define G_XIM_MESSAGES_CLASS(_c_)	(G_TYPE_CHECK_CLASS_CAST ((_c_), G_TYPE_XIM_MESSAGES, GXimMessagesClass))
#define G_IS_XIM_MESSAGES(_o_)		(G_TYPE_CHECK_INSTANCE_TYPE ((_o_), G_TYPE_XIM_MESSAGES))
#define G_IS_XIM_MESSAGES_CLASS(_c_)	(G_TYPE_CHECK_CLASS_TYPE ((_c_), G_TYPE_XIM_MESSAGES))
#define G_XIM_MESSAGES_GET_CLASS(_o_)	(G_TYPE_INSTANCE_GET_CLASS ((_o_), G_TYPE_XIM_MESSAGES, GXimMessagesClass))

/**
 * LIBGXIM_SERVICE_DBUS:
 *
 * A service name for libgxim used in DBus.
 */
#define LIBGXIM_SERVICE_DBUS		"org.tagoh.libgxim"
/**
 * LIBGXIM_PATH_DBUS:
 *
 * A path name for libgxim used in DBus.
 */
#define LIBGXIM_PATH_DBUS		"/org/tagoh/libgxim"
/**
 * LIBGXIM_INTERFACE_DBUS:
 *
 * An interface name for libgxim used in DBus.
 */
#define LIBGXIM_INTERFACE_DBUS		"org.tagoh.libgxim"

/**
 * g_xim_messages_gerror:
 * @_i_: a #GXimMessages.
 * @_e_: a #GError to output.
 *
 * Outputs a #GError with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message. the message type depends
 * on the error code which would be logically added with #GXimErrorType.
 */
#define g_xim_messages_gerror(_i_,_e_)					\
	G_STMT_START {							\
		if ((_e_)->code & G_XIM_NOTICE_CRITICAL) {		\
			g_xim_messages_critical((_i_), "%s", (_e_)->message); \
		} else if ((_e_)->code & G_XIM_NOTICE_ERROR) {		\
			g_xim_messages_error((_i_), "%s", (_e_)->message); \
		} else if ((_e_)->code & G_XIM_NOTICE_BUG) {		\
			g_xim_messages_bug((_i_), "%s", (_e_)->message);	\
		} else if ((_e_)->code & G_XIM_NOTICE_WARNING) {	\
			g_xim_messages_warning((_i_), "%s", (_e_)->message); \
		} else {						\
			g_xim_messages_error((_i_), "%s", (_e_)->message); \
		}							\
	} G_STMT_END

/**
 * g_xim_message:
 * @_i_: a #GXimMessages.
 * @_f_: a unique filter name.
 * @...: arguments include the format string
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_MESSAGE.
 */
/**
 * g_xim_messages_info:
 * @_i_: a #GXimMessages.
 * @_f_: a unique filter name.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_INFO.
 */
/**
 * g_xim_messages_warning:
 * @_i_: a #GXimMessages.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_WARNING.
 */
/**
 * g_xim_messages_error:
 * @_i_: a #GXimMessages.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_ERROR.
 */
/**
 * g_xim_messages_critical:
 * @_i_: a #GXimMessages.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_CRITICAL.
 */
/**
 * g_xim_messages_debug:
 * @_i_: a #GXimMessages.
 * @_f_: a unique filter name.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_DEBUG.
 */
/**
 * g_xim_messages_bug:
 * @_i_: a #GXimMessages.
 * @...: arguments include the format string.
 *
 * Outputs a message with g_xim_messages_printf().
 *
 * This is a convenience macro to output a message with %G_XIM_MESSAGES_BUG.
 */
#ifdef G_HAVE_ISO_VARARGS
#define g_xim_message(_i_,_f_,...)		g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_MESSAGE, \
								     __VA_ARGS__)
#define g_xim_messages_info(_i_,_f_,...)		g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_INFO, \
								     __VA_ARGS__)
#define g_xim_messages_warning(_i_,...)		g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_WARNING, \
								     __VA_ARGS__)
#define g_xim_messages_error(_i_,...)		g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_ERROR, \
								     __VA_ARGS__)
#define g_xim_messages_critical(_i_,...)		g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_CRITICAL, \
								     __VA_ARGS__)
#define g_xim_messages_debug(_i_,_f_,...)	g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_DEBUG, \
								     __VA_ARGS__)
#define g_xim_messages_bug(_i_,...)		g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_BUG,	\
								     __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_xim_message(_i_,_f_,format...)	g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_MESSAGE, \
								     format)
#define g_xim_messages_info(_i_,_f_,format...)	g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_INFO, \
								     format)
#define g_xim_messages_warning(_i_,format...)	g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_WARNING, \
								     format)
#define g_xim_messages_error(_i_,format...)	g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_ERROR, \
								     format)
#define g_xim_messages_critical(_i_,format...)	g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_CRITICAL, \
								     format)
#define g_xim_messages_debug(_i_,_f_,format...)	g_xim_messages_printf((_i_), (_f_), \
								     G_XIM_MESSAGES_DEBUG, \
								     format)
#define g_xim_messages_bug(_i_,format__)		g_xim_messages_printf((_i_), NULL, \
								     G_XIM_MESSAGES_BUG, \
								     format)
#else /* no varargs macros */
static void
g_xim_message(GXimMessages *message,
	      const gchar *filter,
	      const gchar *format,
	      ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, filter, G_XIM_MESSAGES_MESSAGE, format, ap);
	va_end(ap);
}
static void
g_xim_messages_info(GXimMessages *message,
		   const gchar *filter,
		   const gchar *format,
		   ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, filter, G_XIM_MESSAGES_INFO, format, ap);
	va_end(ap);
}
static void
g_xim_messages_warning(GXimMessages *message,
		      const gchar *format,
		      ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, NULL, G_XIM_MESSAGES_WARNING, format, ap);
	va_end(ap);
}
static void
g_xim_messages_error(GXimMessages *message,
		    const gchar *format,
		    ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, NULL, G_XIM_MESSAGES_ERROR, format, ap);
	va_end(ap);
}
static void
g_xim_messages_critical(GXimMessages *message,
		       const gchar *format,
		       ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, NULL, G_XIM_MESSAGES_CRITICAL, format, ap);
	va_end(ap);
}
static void
g_xim_messages_debug(GXimMessages *message,
		    const gchar *filter,
		    const gchar *format,
		    ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, filter, G_XIM_MESSAGES_DEBUG, format, ap);
	va_end(ap);
}
static void
g_xim_messages_bug(GXimMessages *message,
		  const gchar *format,
		  ...)
{
	va_list ap;

	va_start(ap, format);
	g_xim_messages_vprintf(message, NULL, G_XIM_MESSAGES_BUG, format, ap);
	va_end(ap);
}
#endif


/**
 * GXimMessagesType:
 * @G_XIM_MESSAGES_MESSAGE: a message type for usual messages, see g_xim_message().
 * @G_XIM_MESSAGES_DEBUG: a message type for debug, see g_xim_messages_debug().
 * @G_XIM_MESSAGES_INFO: a message type for information, see g_xim_messages_info().
 * @G_XIM_MESSAGES_WARNING: a message type for warnings, see g_xim_messages_warning().
 * @G_XIM_MESSAGES_ERROR: a message type for errors, see g_xim_messages_error().
 * @G_XIM_MESSAGES_CRITICAL: a message type for critical errors, see g_xim_messages_critical().
 * @G_XIM_MESSAGES_BUG: a message type for bugs, see g_xim_messages_bug().
 *
 * Flags specifying the type of messages.
 */
typedef enum {
	G_XIM_MESSAGES_MESSAGE,
	G_XIM_MESSAGES_DEBUG,
	G_XIM_MESSAGES_INFO,
	G_XIM_MESSAGES_WARNING,
	G_XIM_MESSAGES_ERROR,
	G_XIM_MESSAGES_CRITICAL,
	G_XIM_MESSAGES_BUG
} GXimMessagesType;

/**
 * GXimMessages:
 * @parent: a #GObject.
 *
 * An implementation of message handler class
 **/
typedef struct _GXimMessagesClass	GXimMessagesClass;
typedef struct _GXimMessagesPrivate	GXimMessagesPrivate;

struct _GXimMessages {
	GObject              parent_instance;
	GXimMessagesPrivate *priv;
};

struct _GXimMessagesClass {
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

GType         g_xim_messages_get_type     (void) G_GNUC_CONST;
GXimMessages *g_xim_messages_new          (void);
void          g_xim_messages_activate     (GXimMessages     *message,
					   gboolean          flag);
void          g_xim_messages_set_filename (GXimMessages     *message,
					   const gchar      *filename);
void          g_xim_messages_clear_filter (GXimMessages     *message);
void          g_xim_messages_enable_filter(GXimMessages     *message,
					   const gchar      *filter_name);
void          g_xim_messages_vprintf      (GXimMessages     *message,
					   const gchar      *filter,
					   GXimMessagesType  type,
					   const gchar      *format,
					   va_list           args);
void          g_xim_messages_printf       (GXimMessages     *message,
					   const gchar      *filter,
					   GXimMessagesType  type,
					   const gchar      *format,
					   ...) G_GNUC_PRINTF (4, 5);

G_END_DECLS

#endif /* __G_XIM_MESSAGES_H__ */
