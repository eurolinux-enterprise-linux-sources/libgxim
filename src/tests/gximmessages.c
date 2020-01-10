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

#include <errno.h>
#include <string.h>
#include <glib/gstdio.h>
#include "libgxim/gximmessages.h"
#include "libgxim/gximmisc.h"
#include "main.h"


GXimMessages *o;

/************************************************************/
/* common functions                                         */
/************************************************************/
void
setup(void)
{
}

void
teardown(void)
{
}

/************************************************************/
/* Test cases                                               */
/************************************************************/
TDEF (g_xim_messages_new)
{
	o = g_xim_messages_new();
	TNUL (o);
	g_object_unref(o);
} TEND

TDEF (g_xim_messages_activate)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessages", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_messages_new();
	TNUL (o);
	g_xim_messages_activate(o, FALSE);
	g_xim_messages_set_filename(o, filename);

	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 1)");

	g_xim_messages_activate(o, TRUE);
	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "123") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_xim_finalize();
} TEND

TDEF (g_xim_messages_set_filename)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessages", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_messages_new();
	TNUL (o);
	g_xim_messages_activate(o, TRUE);
	g_xim_messages_set_filename(o, NULL);

	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 2)");

	g_xim_messages_set_filename(o, filename);
	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "123") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_xim_finalize();
} TEND

TDEF (g_xim_messages_clear_filter)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessages", NULL);
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_messages_new();
	TNUL (o);
	g_xim_messages_activate(o, TRUE);
	g_xim_messages_set_filename(o, filename);

	g_xim_messages_clear_filter(o);
	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 3)");
	if (fp)
		fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_xim_finalize();
} TEND

TDEF (g_xim_messages_enable_filter)
{
	GXimMessages *m;
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessages", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_messages_new();
	TNUL (o);
	m = g_xim_messages_new();
	TNUL (m);

	g_xim_messages_activate(o, TRUE);
	g_xim_messages_set_filename(o, filename);
	g_xim_messages_enable_filter(o, "foo");

	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "123") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_xim_message(m, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "123") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_object_unref(m);
	g_xim_finalize();
} TEND

TDEF (g_xim_messages_vprintf)
{
	/* nothing to test so far.
	 * this unit testing should be covered in other testcase.
	 */
} TEND

TDEF (g_xim_messages_printf)
{
	/* nothing to test so far.
	 * this unit testing should be covered in other testcase.
	 */
} TEND

TDEF (g_xim_message)
{
	/* nothing to test so far.
	 * this unit testing should be covered in other testcase.
	 */
} TEND

TDEF (g_xim_messages_info)
{
} TEND

TDEF (g_xim_messages_warning)
{
} TEND

TDEF (g_xim_messages_error)
{
} TEND

TDEF (g_xim_messages_critical)
{
} TEND

TDEF (g_xim_messages_debug)
{
} TEND

TDEF (g_xim_messages_bug)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessages", NULL);
	gchar buf[256];
	FILE *fp;
	gsize len;

	g_setenv("LIBGXIM_DEBUG", "", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_messages_new();
	TNUL (o);
	g_xim_messages_activate(o, TRUE);
	g_xim_messages_set_filename(o, filename);

	g_xim_messages_bug(o, "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	len = strlen(buf);
	fail_unless(strncmp(buf, "[BUG]", 5) == 0 && strncmp(&buf[len-4], "123\n", 4) == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_xim_finalize();
} TEND

TDEF (dbus)
{
} TEND

/************************************************************/
Suite *
g_xim_suite(void)
{
	Suite *s = suite_create("GXimMessages");
	TCase *tc = tcase_create("Generic Functionalities");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout(tc, 10);

	T (g_xim_messages_new);
	T (g_xim_messages_activate);
	T (g_xim_messages_set_filename);
	T (g_xim_messages_clear_filter);
	T (g_xim_messages_enable_filter);
	T (g_xim_messages_vprintf);
	T (g_xim_messages_printf);
	T (g_xim_message);
	T (g_xim_messages_info);
	T (g_xim_messages_warning);
	T (g_xim_messages_error);
	T (g_xim_messages_critical);
	T (g_xim_messages_debug);
	T (g_xim_messages_bug);
	T (dbus);

	suite_add_tcase(s, tc);

	return s;
}
