/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmessages.c
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include "gximacc.h"
#include "gximmarshal.h"
#include "gximmessages.h"

#define G_XIM_MESSAGES_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_MESSAGES, GXimMessagesPrivate))

/**
 * SECTION:gximmessages
 * @Title: GXimMessages
 * @Short_Description: Logging facility class
 *
 * GXimMessages provides a logging facility. this allows you to output messages
 * any time you want. you can manage it with DBus as well. so you don't even
 * restart processes to do something with this then.
 *
 * </para><refsect2 id="GXimMessages-DBus">
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
 *   as a parameter. See g_xim_messages_activate() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>SetFilename</term>
 *  <listitem><para>
 *   Sets the filename to be logged messages into. a string value is required
 *   as a parameter. See g_xim_messages_set_filename() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>RemoveAllFilters</term>
 *  <listitem><para>
 *   Resets filters to output. See g_xim_messages_clear_filter() for more details.
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry><term>AddFilter</term>
 *  <listitem><para>
 *   Adds a filter which you want to see a message. a string value is required
 *   as a parameter. See g_xim_messages_enable_filter() for more details.
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

struct _GXimMessagesPrivate {
	GDBusConnection *connection;
	GHashTable      *filter_table;
	gchar           *filename;
	guint            signal_id;
	guint            owner;
	gboolean         activate:1;
	gboolean         all_filters:1;
	gboolean         master:1;
};
typedef struct _GXimMessagesPrivateClass {
	GXimMessagesClass  g_klass;

	/*< private >*/
	/* internal use only */
	gboolean (* activated)        (GXimMessages *message,
				       gboolean     flag);
	gboolean (* filename_changed) (GXimMessages *message,
				       const gchar *filename);
	gboolean (* filter_cleared)   (GXimMessages *message);
	gboolean (* filter_added)     (GXimMessages *message,
				       const gchar *filter);
	void     (* created)          (GXimMessages *message,
				       GXimMessages *created_object);
} GXimMessagesPrivateClass;


static gboolean g_xim_messages_real_activated       (GXimMessages *message,
                                                    gboolean     flag);
static gboolean g_xim_messages_real_filename_changed(GXimMessages *message,
                                                    const gchar *filename);
static gboolean g_xim_messages_real_filter_cleared  (GXimMessages *message);
static gboolean g_xim_messages_real_filter_added    (GXimMessages *message,
                                                    const gchar *filter);
static void     g_xim_messages_real_created         (GXimMessages *message,
                                                    GXimMessages *created_object);


static guint signals[LAST_SIGNAL] = { 0 };
static gpointer g_xim_messages_parent_class = NULL;
static GXimMessages *master_message = NULL;

/*< private >*/
static void
g_xim_messages_bus_signal(GDBusConnection *connection,
			 const gchar     *sender,
			 const gchar     *object_path,
			 const gchar     *interface_name,
			 const gchar     *signal_name,
			 GVariant        *parameters,
			 gpointer         user_data)
{
	if (!master_message)
		return;

	d(g_print("%s: sender[%s] path[%s] iface[%s] method[%s]\n", __PRETTY_FUNCTION__, sender, object_path, interface_name, signal_name));

	if (g_strcmp0(signal_name, "Activate") == 0) {
		gboolean flag;

		g_variant_get(parameters, "(&b)",
			      &flag);
		g_xim_messages_activate(master_message, flag);
	} else if (g_strcmp0(signal_name, "SetFilename") == 0) {
		const gchar *filename;

		g_variant_get(parameters, "(&s)",
			      &filename);
		g_xim_messages_set_filename(master_message, filename);
	} else if (g_strcmp0(signal_name, "RemoveAllFilters") == 0) {
		g_xim_messages_clear_filter(master_message);
	} else if (g_strcmp0(signal_name, "AddFilter") == 0) {
		const gchar *filter;

		g_variant_get(parameters, "(&s)",
			      &filter);
		g_xim_messages_enable_filter(master_message, filter);
	}
}

static void
g_xim_messages_bus_on_name_acquired(GDBusConnection *connection,
				   const gchar     *name,
				   gpointer         user_data)
{
}

static void
g_xim_messages_bus_on_name_lost(GDBusConnection *connection,
			       const gchar     *name,
			       gpointer         user_data)
{
}

static void
g_xim_messages_real_set_property(GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GXimMessages *message = G_XIM_MESSAGES (object);
	GXimMessagesPrivate *priv = message->priv;
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
				    GError *err = NULL;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
				    g_print("%p: become a master messaging facility.\n", object);
#endif /* GNOME_ENABLE_DEBUG */
				    master_message = G_XIM_MESSAGES (object);
				    g_object_add_weak_pointer(G_OBJECT (master_message),
							      (gpointer *)&master_message);
				    g_signal_connect(master_message, "created",
						     G_CALLBACK (g_xim_messages_real_created),
						     NULL);

				    priv->connection = g_bus_get_sync(G_BUS_TYPE_SESSION,
								      NULL,
								      &err);
				    if (priv->connection == NULL) {
					    g_warning("Unable to establish a D-Bus session bus:\n  %s.",
						      err->message);
					    g_error_free(err);
				    } else {
					    guint flags = G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT|G_BUS_NAME_OWNER_FLAGS_REPLACE;

					    priv->signal_id = g_dbus_connection_signal_subscribe(priv->connection,
												 NULL /* sender */,
												 LIBGXIM_INTERFACE_DBUS,
												 NULL /* member */,
												 LIBGXIM_PATH_DBUS,
												 NULL /* arg0 */,
												 G_DBUS_SIGNAL_FLAGS_NONE,
												 g_xim_messages_bus_signal,
												 master_message,
												 NULL /* user_data_free_func */);
					    priv->owner = g_bus_own_name_on_connection(priv->connection,
										       LIBGXIM_SERVICE_DBUS,
										       flags,
										       g_xim_messages_bus_on_name_acquired,
										       g_xim_messages_bus_on_name_lost,
										       master_message,
										       NULL);
				    }
			    }
		    } else {
			    if (master_message == G_XIM_MESSAGES (object)) {
				    g_warning("[BUG] Unable to change the master message object.");
				    break;
			    }
		    }
		    priv->master = flag;
		    break;
	    case PROP_ALL_FILTERS:
		    flag = g_value_get_boolean(value);
		    g_xim_messages_enable_filter(G_XIM_MESSAGES (object),
						flag ? "all" : "noall");
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_messages_real_get_property(GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	GXimMessages *message = G_XIM_MESSAGES (object);
	GXimMessagesPrivate *priv = message->priv;

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
g_xim_messages_real_dispose(GObject *object)
{
	GXimMessages *message = G_XIM_MESSAGES (object);
	GXimMessagesPrivate *priv = message->priv;

	if (!priv->master && master_message) {
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
		g_print("%p: disconnecting from the master object.\n", object);
#endif /* GNOME_ENABLE_DEBUG */
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_messages_real_activated),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_messages_real_filename_changed),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_messages_real_filter_cleared),
						     object);
		g_signal_handlers_disconnect_by_func(master_message,
						     G_CALLBACK (g_xim_messages_real_filter_added),
						     object);
	}

	if (G_OBJECT_CLASS (g_xim_messages_parent_class)->dispose)
		(* G_OBJECT_CLASS (g_xim_messages_parent_class)->dispose) (object);
}

static void
g_xim_messages_real_finalize(GObject *object)
{
	GXimMessages *message = G_XIM_MESSAGES (object);
	GXimMessagesPrivate *priv = message->priv;

	if (priv->signal_id != 0)
		g_dbus_connection_signal_unsubscribe(priv->connection,
						     priv->signal_id);
	if (priv->owner != 0)
		g_bus_unown_name(priv->owner);
	if (priv->connection)
		g_object_unref(priv->connection);
	g_free(priv->filename);
	g_hash_table_destroy(priv->filter_table);

	if (G_OBJECT_CLASS (g_xim_messages_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_messages_parent_class)->finalize) (object);
}

static gboolean
g_xim_messages_real_activated(GXimMessages *message,
			     gboolean     flag)
{
	GXimMessagesPrivate *priv;

#ifdef GNOME_ENABLE_DEBUG
	g_print("%p: Messaging facility is %s\n", message, flag ? "activated" : "deactivated");
#endif /* GNOME_ENABLE_DEBUG */
	priv = message->priv;
	priv->activate = flag;

	return FALSE;
}

static gboolean
g_xim_messages_real_filename_changed(GXimMessages *message,
				    const gchar *filename)
{
	GXimMessagesPrivate *priv;

#ifdef GNOME_ENABLE_DEBUG
	g_print("%p: Logging file has been changed: %s\n", message, filename ? filename : "none");
#endif /* GNOME_ENABLE_DEBUG */
	priv = message->priv;
	g_free(priv->filename);
	priv->filename = g_strdup(filename);

	return FALSE;
}

static gboolean
g_xim_messages_real_filter_cleared(GXimMessages *message)
{
	GXimMessagesPrivate *priv;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Message filters has been cleared.\n", message);
#endif /* GNOME_ENABLE_DEBUG */
	priv = message->priv;
	g_hash_table_remove_all(priv->filter_table);
	priv->all_filters = FALSE;

	return FALSE;
}

static gboolean
g_xim_messages_real_filter_added(GXimMessages *message,
				const gchar *filter)
{
	GXimMessagesPrivate *priv;

	priv = message->priv;
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
g_xim_messages_real_created(GXimMessages *message,
			   GXimMessages *created_object)
{
	GXimMessagesPrivate *priv = message->priv;
	GHashTableIter iter;
	gpointer key, value;

#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Messaging object has been created: %p\n", message, created_object);
#endif /* GNOME_ENABLE_DEBUG */
	g_xim_messages_real_activated(created_object, priv->activate);
	g_xim_messages_real_filename_changed(created_object, priv->filename);
	g_xim_messages_real_filter_cleared(created_object);

	g_hash_table_iter_init(&iter, priv->filter_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		g_xim_messages_real_filter_added(created_object, key);
	}
	g_xim_messages_real_filter_added(created_object, priv->all_filters ? "all" : "noall");

	g_signal_connect_swapped(master_message, "activated",
				 G_CALLBACK (g_xim_messages_real_activated),
				 created_object);
	g_signal_connect_swapped(master_message, "filename_changed",
				 G_CALLBACK (g_xim_messages_real_filename_changed),
				 created_object);
	g_signal_connect_swapped(master_message, "filter_cleared",
				 G_CALLBACK (g_xim_messages_real_filter_cleared),
				 created_object);
	g_signal_connect_swapped(master_message, "filter_added",
				 G_CALLBACK (g_xim_messages_real_filter_added),
				 created_object);
#if defined(GNOME_ENABLE_DEBUG) && defined(MESSAGE_DEBUG)
	g_print("%p: Messaging object is up to date.\n", created_object);
#endif /* GNOME_ENABLE_DEBUG */
}

static void
g_xim_messages_class_init(GXimMessagesPrivateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_xim_messages_parent_class = g_type_class_peek_parent(klass);
	g_type_class_add_private(klass, sizeof (GXimMessagesPrivate));

	object_class->set_property = g_xim_messages_real_set_property;
	object_class->get_property = g_xim_messages_real_get_property;
	object_class->dispose      = g_xim_messages_real_dispose;
	object_class->finalize     = g_xim_messages_real_finalize;

	klass->activated        = g_xim_messages_real_activated;
	klass->filename_changed = g_xim_messages_real_filename_changed;
	klass->filter_cleared   = g_xim_messages_real_filter_cleared;
	klass->filter_added     = g_xim_messages_real_filter_added;

	/* properties */
	/**
	 * GXimMessages:master:
	 *
	 * %TRUE to be a master instance of #GXimMessages. %FALSE to be a slave instance of #GXimMessages.
	 */
	g_object_class_install_property(object_class, PROP_MASTER,
					g_param_spec_boolean("master",
							     _("Master"),
							     _("A flag to mark a master object."),
							     FALSE,
							     G_PARAM_READWRITE));
	/**
	 * GXimMessages:all-filters:
	 *
	 * %TRUE to enable all of the messages filters. %FALSE to deal with each filters added by #GXimMessages::filter-added signal.
	 */
	g_object_class_install_property(object_class, PROP_ALL_FILTERS,
					g_param_spec_boolean("all_filters",
							     _("All filters"),
							     _("A flag to enable all of the message filters."),
							     FALSE,
							     G_PARAM_READWRITE));

	/* signals */
	/**
	 * GXimMessages::activated:
	 * @message: the object which received the signal.
	 * @flag: %TRUE to enable the logging facility.
	 *
	 * The ::activated signal will be emitted when someone calls
	 * g_xim_messages_activate() or when someone emits Activate signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[ACTIVATED] = g_signal_new("activated",
					  G_OBJECT_CLASS_TYPE (klass),
					  G_SIGNAL_RUN_LAST,
					  G_STRUCT_OFFSET (GXimMessagesPrivateClass, activated),
					  _gxim_acc_signal_accumulator__BOOLEAN,
					  NULL,
					  gxim_marshal_BOOLEAN__BOOLEAN,
					  G_TYPE_BOOLEAN, 1,
					  G_TYPE_BOOLEAN);
	/**
	 * GXimMessages::filename-changed:
	 * @message: the object which received the signal.
	 * @filename: a filename to be logged into.
	 *
	 * The ::filename-changed signal will be emitted when someone calls
	 * g_xim_messages_set_filename() or when someone emits SetFilename signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILENAME_CHANGED] = g_signal_new("filename_changed",
						 G_OBJECT_CLASS_TYPE (klass),
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET (GXimMessagesPrivateClass, filename_changed),
						 _gxim_acc_signal_accumulator__BOOLEAN,
						 NULL,
						 gxim_marshal_BOOLEAN__STRING,
						 G_TYPE_BOOLEAN, 1,
						 G_TYPE_STRING);
	/**
	 * GXimMessages::filter-cleared:
	 * @message: the object which received the signal.
	 *
	 * The ::filter-cleared signal will be emitted when someone calls
	 * g_xim_messages_clear_filter() or when someone emits RemoveAllFilters
	 * signal through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILTER_CLEARED] = g_signal_new("filter_cleared",
					       G_OBJECT_CLASS_TYPE (klass),
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET (GXimMessagesPrivateClass, filter_cleared),
					       _gxim_acc_signal_accumulator__BOOLEAN,
					       NULL,
					       gxim_marshal_BOOLEAN__VOID,
					       G_TYPE_BOOLEAN, 0);
	/**
	 * GXimMessages::filter-added:
	 * @message: the object which received the signal.
	 * @filter: the filter name to enable logging.
	 *
	 * The ::filter-added signal will be emitted when someone calls
	 * g_xim_messages_enable_filter() or when someone emits AddFilter signal
	 * through DBus.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[FILTER_ADDED] = g_signal_new("filter_added",
					     G_OBJECT_CLASS_TYPE (klass),
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET (GXimMessagesPrivateClass, filter_added),
					     _gxim_acc_signal_accumulator__BOOLEAN,
					     NULL,
					     gxim_marshal_BOOLEAN__STRING,
					     G_TYPE_BOOLEAN, 1,
					     G_TYPE_STRING);
	/**
	 * GXimMessages::created:
	 * @message: the object which received the signal.
	 * @created_object: a #GXimMessages which newly created somewhere in the process.
	 *
	 * The ::created signal will be emitted when an instance of #GXimMessages
	 * is created. the master object only receives this signal.
	 */
	signals[CREATED] = g_signal_new("created",
					G_OBJECT_CLASS_TYPE (klass),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET (GXimMessagesPrivateClass, created),
					NULL, NULL,
					gxim_marshal_VOID__OBJECT,
					G_TYPE_NONE, 1,
					G_TYPE_XIM_MESSAGES);
}

static void
g_xim_messages_init(GXimMessages *message)
{
	GXimMessagesPrivate *priv = G_XIM_MESSAGES_GET_PRIVATE (message);

	message->priv = priv;
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
g_xim_messages_get_type(void)
{
	static volatile gsize type_id__volatile = 0;

	if (g_once_init_enter(&type_id__volatile)) {
		GType type_id;

		type_id = g_type_register_static_simple(G_TYPE_OBJECT,
							g_intern_static_string("GXimMessages"),
							sizeof (GXimMessagesPrivateClass),
							(GClassInitFunc)g_xim_messages_class_init,
							sizeof (GXimMessages),
							(GInstanceInitFunc)g_xim_messages_init,
							0);

		g_once_init_leave(&type_id__volatile, type_id);
	}

	return type_id__volatile;
}

/**
 * g_xim_messages_new:
 *
 * Creates an instance to provide you a logging facility. when an instance
 * is created, GXimMessages::created signal will be emitted.
 *
 * Return value: a #GXimMessages.
 */
GXimMessages *
g_xim_messages_new(void)
{
	GXimMessages *retval;

	retval = G_XIM_MESSAGES (g_object_new(G_TYPE_XIM_MESSAGES, NULL));
	if (master_message)
		g_signal_emit(master_message, signals[CREATED], 0,
			      retval);

	return retval;
}

/**
 * g_xim_messages_activate:
 * @message: a #GXimMessages.
 * @flag: %TRUE to enable logging. %FALSE to disable logging.
 *
 * Sets the activity of the logging facility.
 *
 * This affects all of the instance of #GXimMessages in the process.
 */
void
g_xim_messages_activate(GXimMessages *message,
		       gboolean     flag)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));

	if (master_message)
		g_signal_emit(master_message, signals[ACTIVATED], 0, flag, &ret);
	else
		g_signal_emit(message, signals[ACTIVATED], 0, flag, &ret);
}

/**
 * g_xim_messages_set_filename:
 * @message: a #GXimMessages.
 * @filename: the filename to be logged a message into.
 *
 * Sets the filename.
 *
 * This affects all of the instance of #GXimMessages in the process.
 */
void
g_xim_messages_set_filename(GXimMessages *message,
			   const gchar *filename)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));

	if (master_message)
		g_signal_emit(master_message, signals[FILENAME_CHANGED], 0, filename, &ret);
	else
		g_signal_emit(message, signals[FILENAME_CHANGED], 0, filename, &ret);
}

/**
 * g_xim_messages_clear_filter:
 * @message: a #GXimMessages.
 *
 * Resets all of the filters you wanted to allow a message.
 *
 * This affects all of the instance of #GXimMessages in the process.
 */
void
g_xim_messages_clear_filter(GXimMessages *message)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));

	if (master_message)
		g_signal_emit(master_message, signals[FILTER_CLEARED], 0, &ret);
	else
		g_signal_emit(message, signals[FILTER_CLEARED], 0, &ret);
}

/**
 * g_xim_messages_enable_filter:
 * @message: a #GXimMessages.
 * @filter_name: a unique filter name to be categorized.
 *
 * Allows logging a message categorized to @filter_name. "all" and "noall"
 * filter name is reserved. "all" to enable all of filters no matter what
 * filters are enabled. "noall" to enable filters specified by this function.
 *
 * This affects all of the instance of #GXimMessages in the process.
 */
void
g_xim_messages_enable_filter(GXimMessages *message,
			    const gchar *filter_name)
{
	gboolean ret;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));
	g_return_if_fail (filter_name != NULL);

	if (master_message)
		g_signal_emit(master_message, signals[FILTER_ADDED], 0, filter_name, &ret);
	else
		g_signal_emit(message, signals[FILTER_ADDED], 0, filter_name, &ret);
}

/**
 * g_xim_messages_vprintf:
 * @message: a #GXimMessages.
 * @filter: a unique filter name to be categorized.
 * @type: an urgency of the message.
 * @format: the message format. See the printf() documentation.
 * @args: a #va_list.
 *
 * Outputs a message. when %G_XIM_MESSAGES_WARNING, %G_XIM_MESSAGES_ERROR,
 * %G_XIM_MESSAGES_CRITICAL or %G_XIM_MESSAGES_BUG is specified to @type,
 * a message will be output regardless of what filter is given to @filter.
 * Otherwise if @filter isn't enabled, this just will be ignored.
 */
void
g_xim_messages_vprintf(GXimMessages     *message,
		      const gchar     *filter,
		      GXimMessagesType  type,
		      const gchar     *format,
		      va_list          args)
{
	GXimMessagesPrivate *priv;
	gchar *msg = NULL, *prefixed = NULL;
	gchar *f = NULL;
	GTimeVal val;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));
	g_return_if_fail (format != NULL);

	priv = message->priv;
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
	    case G_XIM_MESSAGES_MESSAGE:
		    prefixed = g_strdup(msg);
		    break;
	    case G_XIM_MESSAGES_INFO:
		    prefixed = g_strdup_printf("I[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGES_WARNING:
		    prefixed = g_strdup_printf("W[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGES_ERROR:
		    prefixed = g_strdup_printf("E[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGES_CRITICAL:
		    prefixed = g_strdup_printf("C[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGES_DEBUG:
		    prefixed = g_strdup_printf("D[% 10ld.%06ld]:%s%s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, f, msg);
		    break;
	    case G_XIM_MESSAGES_BUG:
		    prefixed = g_strdup_printf("[BUG][% 10ld.%06ld] %s\n", val.tv_sec, (val.tv_usec * 1000) / 1000, msg);
		    break;
	    default:
		    if (master_message) {
			    g_xim_messages_bug(master_message, "Unknown message type: %d\n", type);
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
 * g_xim_messages_printf:
 * @message: a #GXimMessages.
 * @filter: a unique filter name to be categorized.
 * @type: an urgency of the message.
 * @format: the message format. See the printf() documentation.
 * @...: arguments to @format.
 *
 * Outputs a message. when %G_XIM_MESSAGES_WARNING, %G_XIM_MESSAGES_ERROR,
 * %G_XIM_MESSAGES_CRITICAL or %G_XIM_MESSAGES_BUG is specified to @type,
 * a message will be output regardless of what filter is given to @filter.
 * Otherwise if @filter isn't enabled, this just will be ignored.
 */
void
g_xim_messages_printf(GXimMessages     *message,
		     const gchar     *filter,
		     GXimMessagesType  type,
		     const gchar     *format,
		     ...)
{
	GXimMessagesPrivate *priv;

	g_return_if_fail (G_IS_XIM_MESSAGES (message));
	g_return_if_fail (format != NULL);

	priv = message->priv;
	if (priv->activate ||
	    type >= G_XIM_MESSAGES_WARNING) {
		va_list ap;

		va_start(ap, format);
		g_xim_messages_vprintf(message, filter, type, format, ap);
		va_end(ap);
	}
}
