/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmessage.c
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

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib/gi18n-lib.h>
#include "gximacc.h"
#include "gximmarshal.h"
#include "gximmessage.h"

#define G_XIM_MESSAGE_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_MESSAGE, GXimMessagePrivate))

#define LIBGXIM_PATH_DBUS		"/org/tagoh/libgxim"
#define LIBGXIM_INTERFACE_DBUS		"org.tagoh.libgxim"


/**
 * SECTION:gximmessage
 * @Title: GXimMessage
 * @Short_Description: Logging facility class
 *
 * GXimMessage provides a logging facility. this allows you to output messages
 * any time you want. you can manage it with DBus as well. so you don't even
 * restart processes to do something with this then.
 *
 * </para><refsect2 id="GXimMessage-DBus">
 * <title>DBus signals</title><para>
 * Right now the following signals are available to manage the logging facility.
 * you can emits signals any time as following:
 *
 * |[
 * $ dbus-send --session --type=signal /org/tagoh/libgxim org.tagoh.libgxim.<replaceable>SignalName</replaceable> type:<replaceable>value</replaceable>
 * ]|
 *
 * <replaceable>SignalName</replaceable> will be replaced with following signal names. See manpage for
 * <command>dbus-send</command> to learn usage for others.
 *
 * </para><variablelist>
 * <varlistentry><term>Activate</term>
 *  <listitem><para>
 *   Sets the activity of the logging facility. a boolean value is required
 *   as a parameter. See g_xim_message_activate() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>SetFilename</term>
 *  <listitem><para>
 *   Sets the filename to be logged messages into. a string value is required
 *   as a parameter. See g_xim_message_set_filename() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>RemoveAllFilters</term>
 *  <listitem><para>
 *   Resets filters to output. See g_xim_message_clear_filter() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>AddFilter</term>
 *  <listitem><para>
 *   Adds a filter which you want to see a message. a string value is required
 *   as a parameter. See g_xim_message_enable_filter() for more details.
 *  </para></listitem>
 * </varlistentry>
 * </variablelist>
 * </refsect2><para>
 */

enum {
	PROP_0,
	PROP_MASTER,
	PROP_ALL_FILTERS,
	LAST_PROP
};
enum {
	SIGNAL_0,
	ACTIVATED,
	FILENAME_CHANGED,
	FILTER_CLEARED,
	FILTER_ADDED,
	CREATED,
	LAST_SIGNAL
};

typedef struct _GXimMessagePrivate {
	DBusConnection *dbus_conn;
	GHashTable     *filter_table;
	gchar          *filename;
	gboolean        activate;
	gboolean        all_filters;
	gboolean        master;
} GXimMessagePrivate;
typedef struct _GXimMessagePrivateClass {
	GXimMessageClass  g_klass;

	/*< private >*/
	/* internal use only */
	gboolean (* activated)        (GXimMessage *message,
				       gboolean     flag);
	gboolean (* filename_changed) (GXimMessage *message,
				       const gchar *filename);
	gboolean (* filter_cleared)   (GXimMessage *message);
	gboolean (* filter_added)     (GXimMessage *message,
				       const gchar *filter);
	void     (* created)          (GXimMessage *message,
				       GXimMessage *created_object);
} GXimMessagePrivateClass;


static gboolean          g_xim_message_real_activated       (GXimMessage    *message,
                                                             gboolean        flag);
static gboolean          g_xim_message_real_filename_changed(GXimMessage    *message,
                                                             const gchar    *filename);
static gboolean          g_xim_message_real_filter_cleared  (GXimMessage    *message);
static gboolean          g_xim_message_real_filter_added    (GXimMessage    *message,
                                                             const gchar    *filter);
static void              g_xim_message_real_created         (GXimMessage    *message,
                                                             GXimMessage    *created_object);
static DBusHandlerResult _g_xim_message_dbus_message_filter (DBusConnection *connection,
                                                             DBusMessage    *message,
                                                             void           *data);


static guint signals[LAST_SIGNAL] = { 0 };
static gpointer g_xim_message_parent_class = NULL;
static GXimMessage *master_message = NULL;

/*
 * Private functions
 */
static void
g_xim_message_real_set_property(GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GXimMessage *message = G_XIM_MESSAGE (object);
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	gboolean flag;

	switch (prop_id) {
	    case PROP_MASTER:
		    flag = g_value_get_boolean(value);
		    if (flag) {
			    /* XXX: I have no idea to inherit the objects
			     * that connected to the 'created' signal.
			     * without it, we can't deliver the changes
			     * to all of the objects anymore then.
			     */
			    if (master_message) {
				    g_warning("[BUG] Unable to change the master message object.");
				    break;
			    } else {
				    DBusError derror;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
				    g_print("%p: become a master messaging facility.\n", object);
#endif /* GNOME_ENABLE_DEBUG */
				    master_message = G_XIM_MESSAGE (object);
				    g_signal_connect(master_message, "created",
						     G_CALLBACK (g_xim_message_real_created),
						     NULL);

				    dbus_error_init(&derror);
				    priv->dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &derror);
				    if (priv->dbus_conn == NULL) {
					    g_warning("Unable to establish a D-Bus session bus:\n  %s.",
						      derror.message);
					    dbus_error_free(&derror);
				    } else {
					    dbus_connection_setup_with_g_main(priv->dbus_conn, NULL);
					    dbus_bus_add_match(priv->dbus_conn,
							       "type='signal',"
							       "interface='" LIBGXIM_INTERFACE_DBUS "'",
							       &derror);
					    if (dbus_error_is_set(&derror)) {
						    g_warning("%s: %s", derror.name, derror.message);
						    dbus_error_free(&derror);
					    } else {
						    dbus_connection_add_filter(priv->dbus_conn,
									       _g_xim_message_dbus_message_filter,
									       NULL, NULL);
					    }
				    }
			    }
		    } else {
			    if (master_message == G_XIM_MESSAGE (object)) {
				    g_warning("[BUG] Unable to change the master message object.");
				    break;
			    }
		    }
		    priv->master = flag;
		    break;
	    case PROP_ALL_FILTERS:
		    flag = g_value_get_boolean(value);
		    g_xim_message_enable_filter(G_XIM_MESSAGE (object),
						flag ? "all" : "noall");
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_message_real_get_property(GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	GXimMessage *message = G_XIM_MESSAGE (object);
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (message);

	switch (prop_id) {
	    case PROP_MASTER:
		    g_value_set_boolean(value, priv->master);
		    break;
	    case PROP_ALL_FILTERS:
		    g_value_set_boolean(value, priv->all_filters);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_message_real_dispose(GObject *object)
{
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (object);

	if (!priv->master && master_message) {
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
		g_print("%p: disconnecting from the master object.\n", object);
#endif /* GNOME_ENABLE_DEBUG */
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_message_real_activated),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_message_real_filename_changed),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_message_real_filter_cleared),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_message_real_filter_added),
						     object);
	}

	if (G_OBJECT_CLASS (g_xim_message_parent_class)->dispose)
		(* G_OBJECT_CLASS (g_xim_message_parent_class)->dispose) (object);
}

static void
g_xim_message_real_finalize(GObject *object)
{
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (object);

	if (priv->dbus_conn)
		dbus_connection_unref(priv->dbus_conn);
	g_free(priv->filename);
	g_hash_table_destroy(priv->filter_table);

	if (G_OBJECT_CLASS (g_xim_message_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_message_parent_class)->finalize) (object);
}

static gboolean
g_xim_message_real_activated(GXimMessage *message,
			     gboolean     flag)
{
	GXimMessagePrivate *priv;

#ifdef GNOME_ENABLE_DEBUG
	g_print("%p: Messaging facility is %s\n", message, flag ? "activated" : "deactivated");
#endif /* GNOME_ENABLE_DEBUG */
	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	priv->activate = flag;

	return FALSE;
}

static gboolean
g_xim_message_real_filename_changed(GXimMessage *message,
				    const gchar *filename)
{
	GXimMessagePrivate *priv;

#ifdef GNOME_ENABLE_DEBUG
	g_print("%p: Logging file has been changed: %s\n", message, filename ? filename : "none");
#endif /* GNOME_ENABLE_DEBUG */
	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	g_free(priv->filename);
	priv->filename = g_strdup(filename);

	return FALSE;
}

static gboolean
g_xim_message_real_filter_cleared(GXimMessage *message)
{
	GXimMessagePrivate *priv;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Message filters has been cleared.\n", message);
#endif /* GNOME_ENABLE_DEBUG */
	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	g_hash_table_remove_all(priv->filter_table);
	priv->all_filters = FALSE;

	return FALSE;
}

static gboolean
g_xim_message_real_filter_added(GXimMessage *message,
				const gchar *filter)
{
	GXimMessagePrivate *priv;

	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	if (g_ascii_strcasecmp(filter, "all") == 0) {
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
		g_print("%p: All filters has been enabled.\n", message);
#endif /* GNOME_ENABLE_DEBUG */
		priv->all_filters = TRUE;
	} else if (g_ascii_strcasecmp(filter, "noall") == 0) {
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
		g_print("%p: Enabling the specified filters only\n", message);
#endif /* GNOME_ENABLE_DEBUG */
		priv->all_filters = FALSE;
	} else {
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
		g_print("%p: Enabling filter `%s'\n", message, filter);
#endif /* GNOME_ENABLE_DEBUG */
		g_hash_table_replace(priv->filter_table,
				     g_strdup(filter),
				     GUINT_TO_POINTER (1));
	}

	return FALSE;
}

static void
g_xim_message_real_created(GXimMessage *message,
			   GXimMessage *created_object)
{
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	GHashTableIter iter;
	gpointer key, value;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Messaging object has been created: %p\n", message, created_object);
#endif /* GNOME_ENABLE_DEBUG */
	g_xim_message_real_activated(created_object, priv->activate);
	g_xim_message_real_filename_changed(created_object, priv->filename);
	g_xim_message_real_filter_cleared(created_object);

	g_hash_table_iter_init(&iter, priv->filter_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		g_xim_message_real_filter_added(created_object, key);
	}
	g_xim_message_real_filter_added(created_object, priv->all_filters ? "all" : "noall");

	g_signal_connect_swapped(master_message, "activated",
				 G_CALLBACK (g_xim_message_real_activated),
				 created_object);
	g_signal_connect_swapped(master_message, "filename_changed",
				 G_CALLBACK (g_xim_message_real_filename_changed),
				 created_object);
	g_signal_connect_swapped(master_message, "filter_cleared",
				 G_CALLBACK (g_xim_message_real_filter_cleared),
				 created_object);
	g_signal_connect_swapped(master_message, "filter_added",
				 G_CALLBACK (g_xim_message_real_filter_added),
				 created_object);
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Messaging object is up to date.\n", created_object);
#endif /* GNOME_ENABLE_DEBUG */
}

static void
_g_xim_message_warn_dbus_signal(const gchar *signal,
				DBusError   *error)
{
	g_warning("Unable to deal with `%s' signal:\n  %s",
		  signal, error->message);
}

static DBusHandlerResult
_g_xim_message_dbus_message_filter(DBusConnection *connection,
				   DBusMessage    *message,
				   void           *data)
{
	DBusError derror;

	if (master_message == NULL)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	dbus_error_init(&derror);

	if (dbus_message_is_signal(message, LIBGXIM_INTERFACE_DBUS, "Activate")) {
		gboolean flag;

		if (dbus_message_get_args(message, &derror,
					  DBUS_TYPE_BOOLEAN, &flag,
					  DBUS_TYPE_INVALID)) {
			g_xim_message_activate(master_message, flag);

			return DBUS_HANDLER_RESULT_HANDLED;
		} else {
			_g_xim_message_warn_dbus_signal("Activate", &derror);
		}
	} else if (dbus_message_is_signal(message, LIBGXIM_INTERFACE_DBUS, "SetFilename")) {
		const gchar *filename;

		if (dbus_message_get_args(message, &derror,
					  DBUS_TYPE_STRING, &filename,
					  DBUS_TYPE_INVALID)) {
			g_xim_message_set_filename(master_message, filename);

			return DBUS_HANDLER_RESULT_HANDLED;
		} else {
			_g_xim_message_warn_dbus_signal("SetFilename", &derror);
		}
	} else if (dbus_message_is_signal(message, LIBGXIM_INTERFACE_DBUS, "RemoveAllFilters")) {
		g_xim_message_clear_filter(master_message);

		return DBUS_HANDLER_RESULT_HANDLED;
	} else if (dbus_message_is_signal(message, LIBGXIM_INTERFACE_DBUS, "AddFilter")) {
		const gchar *filter;

		if (dbus_message_get_args(message, &derror,
					  DBUS_TYPE_STRING, &filter,
					  DBUS_TYPE_INVALID)) {
			g_xim_message_enable_filter(master_message, filter);

			return DBUS_HANDLER_RESULT_HANDLED;
		} else {
			_g_xim_message_warn_dbus_signal("AddFilter", &derror);
		}
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
g_xim_message_class_init(GXimMessagePrivateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_xim_message_parent_class = g_type_class_peek_parent(klass);
	g_type_class_add_private(klass, sizeof (GXimMessagePrivate));

	object_class->set_property = g_xim_message_real_set_property;
	object_class->get_property = g_xim_message_real_get_property;
	object_class->dispose      = g_xim_message_real_dispose;
	object_class->finalize     = g_xim_message_real_finalize;

	klass->activated        = g_xim_message_real_activated;
	klass->filename_changed = g_xim_message_real_filename_changed;
	klass->filter_cleared   = g_xim_message_real_filter_cleared;
	klass->filter_added     = g_xim_message_real_filter_added;

	/* properties */
	/**
	 * GXimMessage:master:
	 *
	 * %TRUE to be a master instance of #GXimMessage. %FALSE to be a slave instance of #GXimMessage.
	 */
	g_object_class_install_property(object_class, PROP_MASTER,
					g_param_spec_boolean("master",
							     _("Master"),
							     _("A flag to mark a master object."),
							     FALSE,
							     G_PARAM_READWRITE));
	/**
	 * GXimMessage:all-filters:
	 *
	 * %TRUE to enable all of the message filters. %FALSE to deal with each filters added by #GXimMessage::filter-added signal.
	 */
	g_object_class_install_property(object_class, PROP_ALL_FILTERS,
					g_param_spec_boolean("all_filters",
							     _("All filters"),
							     _("A flag to enable all of the message filters."),
							     FALSE,
							     G_PARAM_READWRITE));

	/* signals */
	/**
	 * GXimMessage::activated:
	 * @message: the object which received the signal.
	 * @flag: %TRUE to enable the logging facility.
	 *
	 * The ::activated signal will be emitted when someone calls
	 * g_xim_message_activate() or when someone emits Activate signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[ACTIVATED] = g_signal_new("activated",
					  G_OBJECT_CLASS_TYPE (klass),
					  G_SIGNAL_RUN_LAST,
					  G_STRUCT_OFFSET (GXimMessagePrivateClass, activated),
					  _gxim_acc_signal_accumulator__BOOLEAN,
					  NULL,
					  gxim_marshal_BOOLEAN__BOOLEAN,
					  G_TYPE_BOOLEAN, 1,
					  G_TYPE_BOOLEAN);
	/**
	 * GXimMessage::filename-changed:
	 * @message: the object which received the signal.
	 * @filename: a filename to be logged into.
	 *
	 * The ::filename-changed signal will be emitted when someone calls
	 * g_xim_message_set_filename() or when someone emits SetFilename signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILENAME_CHANGED] = g_signal_new("filename_changed",
						 G_OBJECT_CLASS_TYPE (klass),
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET (GXimMessagePrivateClass, filename_changed),
						 _gxim_acc_signal_accumulator__BOOLEAN,
						 NULL,
						 gxim_marshal_BOOLEAN__STRING,
						 G_TYPE_BOOLEAN, 1,
						 G_TYPE_STRING);
	/**
	 * GXimMessage::filter-cleared:
	 * @message: the object which received the signal.
	 *
	 * The ::filter-cleared signal will be emitted when someone calls
	 * g_xim_message_clear_filter() or when someone emits RemoveAllFilters
	 * signal through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILTER_CLEARED] = g_signal_new("filter_cleared",
					       G_OBJECT_CLASS_TYPE (klass),
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET (GXimMessagePrivateClass, filter_cleared),
					       _gxim_acc_signal_accumulator__BOOLEAN,
					       NULL,
					       gxim_marshal_BOOLEAN__VOID,
					       G_TYPE_BOOLEAN, 0);
	/**
	 * GXimMessage::filter-added:
	 * @message: the object which received the signal.
	 * @filter: the filter name to enable logging.
	 *
	 * The ::filter-added signal will be emitted when someone calls
	 * g_xim_message_enable_filter() or when someone emits AddFilter signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILTER_ADDED] = g_signal_new("filter_added",
					     G_OBJECT_CLASS_TYPE (klass),
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET (GXimMessagePrivateClass, filter_added),
					     _gxim_acc_signal_accumulator__BOOLEAN,
					     NULL,
					     gxim_marshal_BOOLEAN__STRING,
					     G_TYPE_BOOLEAN, 1,
					     G_TYPE_STRING);
	/**
	 * GXimMessage::created:
	 * @message: the object which received the signal.
	 * @created_object: a #GXimMessage which newly created somewhere in the process.
	 *
	 * The ::created signal will be emitted when an instance of #GXimMessage
	 * is created. the master object only receives this signal.
	 */
	signals[CREATED] = g_signal_new("created",
					G_OBJECT_CLASS_TYPE (klass),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET (GXimMessagePrivateClass, created),
					NULL, NULL,
					gxim_marshal_VOID__OBJECT,
					G_TYPE_NONE, 1,
					G_TYPE_XIM_MESSAGE);
}

static void
g_xim_message_init(GXimMessage *message)
{
	GXimMessagePrivate *priv = G_XIM_MESSAGE_GET_PRIVATE (message);

	if (!g_threads_got_initialized)
		g_thread_init(NULL);

	priv->filename = NULL;
	priv->activate = FALSE;
	priv->filter_table = g_hash_table_new_full(g_str_hash,
						   g_str_equal,
						   g_free,
						   NULL);
	priv->all_filters = FALSE;
}

/*
 * Public functions
 */
GType
g_xim_message_get_type(void)
{
	static volatile gsize type_id__volatile = 0;

	if (g_once_init_enter(&type_id__volatile)) {
		GType type_id;

		type_id = g_type_register_static_simple(G_TYPE_OBJECT,
							g_intern_static_string("GXimMessage"),
							sizeof (GXimMessagePrivateClass),
							(GClassInitFunc)g_xim_message_class_init,
							sizeof (GXimMessage),
							(GInstanceInitFunc)g_xim_message_init,
							0);

		g_once_init_leave(&type_id__volatile, type_id);
	}

	return type_id__volatile;
}

/**
 * g_xim_message_new:
 *
 * Creates an instance to provide you a logging facility. when an instance
 * is created, GXimMessage::created signal will be emitted.
 *
 * Return value: a #GXimMessage.
 */
GXimMessage *
g_xim_message_new(void)
{
	GXimMessage *retval;

	retval = G_XIM_MESSAGE (g_object_new(G_TYPE_XIM_MESSAGE, NULL));
	if (master_message)
		g_signal_emit(master_message, signals[CREATED], 0,
			      retval);

	return retval;
}

/**
 * g_xim_message_activate:
 * @message: a #GXimMessage.
 * @flag: %TRUE to enable logging. %FALSE to disable logging.
 *
 * Sets the activity of the logging facility.
 *
 * This affects all of the instance of #GXimMessage in the process.
 */
void
g_xim_message_activate(GXimMessage *message,
		       gboolean     flag)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));

	if (master_message)
		g_signal_emit(master_message, signals[ACTIVATED], 0, flag, &ret);
	else
		g_signal_emit(message, signals[ACTIVATED], 0, flag, &ret);
}

/**
 * g_xim_message_set_filename:
 * @message: a #GXimMessage.
 * @filename: the filename to be logged a message into.
 *
 * Sets the filename.
 *
 * This affects all of the instance of #GXimMessage in the process.
 */
void
g_xim_message_set_filename(GXimMessage *message,
			   const gchar *filename)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));

	if (master_message)
		g_signal_emit(master_message, signals[FILENAME_CHANGED], 0, filename, &ret);
	else
		g_signal_emit(message, signals[FILENAME_CHANGED], 0, filename, &ret);
}

/**
 * g_xim_message_clear_filter:
 * @message: a #GXimMessage.
 *
 * Resets all of the filters you wanted to allow a message.
 *
 * This affects all of the instance of #GXimMessage in the process.
 */
void
g_xim_message_clear_filter(GXimMessage *message)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));

	if (master_message)
		g_signal_emit(master_message, signals[FILTER_CLEARED], 0, &ret);
	else
		g_signal_emit(message, signals[FILTER_CLEARED], 0, &ret);
}

/**
 * g_xim_message_enable_filter:
 * @message: a #GXimMessage.
 * @filter_name: a unique filter name to be categorized.
 *
 * Allows logging a message categorized to @filter_name. "all" and "noall"
 * filter name is reserved. "all" to enable all of filters no matter what
 * filters are enabled. "noall" to enable filters specified by this function.
 *
 * This affects all of the instance of #GXimMessage in the process.
 */
void
g_xim_message_enable_filter(GXimMessage *message,
			    const gchar *filter_name)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));
	g_return_if_fail (filter_name != NULL);

	if (master_message)
		g_signal_emit(master_message, signals[FILTER_ADDED], 0, filter_name, &ret);
	else
		g_signal_emit(message, signals[FILTER_ADDED], 0, filter_name, &ret);
}

/**
 * g_xim_message_vprintf:
 * @message: a #GXimMessage.
 * @filter: a unique filter name to be categorized.
 * @type: an urgency of the message.
 * @format: the message format. See the printf() documentation.
 * @args: a #va_list.
 *
 * Outputs a message. when %G_XIM_MESSAGE_WARNING, %G_XIM_MESSAGE_ERROR,
 * %G_XIM_MESSAGE_CRITICAL or %G_XIM_MESSAGE_BUG is specified to @type,
 * a message will be output regardless of what filter is given to @filter.
 * Otherwise if @filter isn't enabled, this just will be ignored.
 */
void
g_xim_message_vprintf(GXimMessage     *message,
		      const gchar     *filter,
		      GXimMessageType  type,
		      const gchar     *format,
		      va_list          args)
{
	GXimMessagePrivate *priv;
	gchar *msg = NULL, *prefixed = NULL;
	gchar *f = NULL;
	GTimeVal val;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));
	g_return_if_fail (format != NULL);

	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	if (filter != NULL &&
	    !priv->all_filters &&
	    g_hash_table_lookup(priv->filter_table, filter) == NULL)
		return;

	if (filter)
		f = g_strdup_printf("[%s] ", filter);
	else
		f = g_strdup("");
	msg = g_strdup_vprintf(format, args);
	g_get_current_time(&val);
	switch (type) {
	    case G_XIM_MESSAGE_MESSAGE:
		    prefixed = g_strdup(msg);
		    break;
	    case G_XIM_MESSAGE_INFO:
		    prefixed = g_strdup_printf("I[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGE_WARNING:
		    prefixed = g_strdup_printf("W[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGE_ERROR:
		    prefixed = g_strdup_printf("E[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGE_CRITICAL:
		    prefixed = g_strdup_printf("C[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGE_DEBUG:
		    prefixed = g_strdup_printf("D[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGE_BUG:
		    prefixed = g_strdup_printf("[BUG][% 10ld.%06ld] %s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, msg);
		    break;
	    default:
		    if (master_message) {
			    g_xim_message_bug(master_message, "Unknown message type: %d\n", type);
		    } else {
			    g_warning("[BUG] Unknown message type: %d\n", type);
		    }
		    goto end;
	}
	if (priv->filename == NULL) {
		/* output to stderr */
		fputs(prefixed, stderr);
	} else {
		FILE *fp;

		if ((fp = fopen(priv->filename, "a")) == NULL) {
			g_warning("Unable to output a message to `%s'", priv->filename);
		} else {
			GTimeVal val;

			g_get_current_time(&val);
			fputs(prefixed, fp);
			fclose(fp);
		}
	}
  end:
	g_free(f);
	g_free(prefixed);
	g_free(msg);
}

/**
 * g_xim_message_printf:
 * @message: a #GXimMessage.
 * @filter: a unique filter name to be categorized.
 * @type: an urgency of the message.
 * @format: the message format. See the printf() documentation.
 * @...: arguments to @format.
 *
 * Outputs a message. when %G_XIM_MESSAGE_WARNING, %G_XIM_MESSAGE_ERROR,
 * %G_XIM_MESSAGE_CRITICAL or %G_XIM_MESSAGE_BUG is specified to @type,
 * a message will be output regardless of what filter is given to @filter.
 * Otherwise if @filter isn't enabled, this just will be ignored.
 */
void
g_xim_message_printf(GXimMessage     *message,
		     const gchar     *filter,
		     GXimMessageType  type,
		     const gchar     *format,
		     ...)
{
	GXimMessagePrivate *priv;

	g_return_if_fail (G_IS_XIM_MESSAGE (message));
	g_return_if_fail (format != NULL);

	priv = G_XIM_MESSAGE_GET_PRIVATE (message);
	if (priv->activate ||
	    type >= G_XIM_MESSAGE_WARNING) {
		va_list ap;

		va_start(ap, format);
		g_xim_message_vprintf(message, filter, type, format, ap);
		va_end(ap);
	}
}
