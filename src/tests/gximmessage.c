/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmessage.c
 * Copyright (C) 2008 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <string.h>
#include <glib/gstdio.h>
#include "libgxim/gximmessage.h"
#include "libgxim/gximmisc.h"
#include "main.h"


GXimMessage *o;

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
TDEF (g_xim_message_new)
{
	o = g_xim_message_new();
	TNUL (o);
	g_object_unref(o);
} TEND

TDEF (g_xim_message_activate)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessage", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_message_new();
	TNUL (o);
	g_xim_message_set_filename(o, filename);

	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 1)");

	g_xim_message_activate(o, TRUE);
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

TDEF (g_xim_message_set_filename)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessage", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_message_new();
	TNUL (o);
	g_xim_message_activate(o, TRUE);
	g_xim_message_set_filename(o, NULL);

	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 2)");

	g_xim_message_set_filename(o, filename);
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

TDEF (g_xim_message_clear_filter)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessage", NULL);
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "all", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_message_new();
	TNUL (o);
	g_xim_message_activate(o, TRUE);
	g_xim_message_set_filename(o, filename);

	g_xim_message_clear_filter(o);
	g_xim_message(o, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) == NULL, "The log file shouldn't exists. (take 3)");
	if (fp)
		fclose(fp);
	g_unlink(filename);

	g_free(filename);
	g_object_unref(o);
	g_xim_finalize();
} TEND

TDEF (g_xim_message_enable_filter)
{
	GXimMessage *m;
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessage", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_message_new();
	TNUL (o);
	m = g_xim_message_new();
	TNUL (m);

	g_xim_message_activate(o, TRUE);
	g_xim_message_set_filename(o, filename);
	g_xim_message_enable_filter(o, "foo");

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

TDEF (g_xim_message_vprintf)
{
	/* nothing to test so far.
	 * this unit testing should be covered in other testcase.
	 */
} TEND

TDEF (g_xim_message_printf)
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

TDEF (g_xim_message_info)
{
} TEND

TDEF (g_xim_message_warning)
{
} TEND

TDEF (g_xim_message_error)
{
} TEND

TDEF (g_xim_message_critical)
{
} TEND

TDEF (g_xim_message_debug)
{
} TEND

TDEF (g_xim_message_bug)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmessage", NULL);
	gchar buf[256];
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	o = g_xim_message_new();
	TNUL (o);
	g_xim_message_activate(o, TRUE);
	g_xim_message_set_filename(o, filename);

	g_xim_message_bug(o, "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "[BUG] 123\n") == 0, "Unexpected output in the log");
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
	Suite *s = suite_create("GXimMessage");
	TCase *tc = tcase_create("Generic Functionalities");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout(tc, 10);

	T (g_xim_message_new);
	T (g_xim_message_activate);
	T (g_xim_message_set_filename);
	T (g_xim_message_clear_filter);
	T (g_xim_message_enable_filter);
	T (g_xim_message_vprintf);
	T (g_xim_message_printf);
	T (g_xim_message);
	T (g_xim_message_info);
	T (g_xim_message_warning);
	T (g_xim_message_error);
	T (g_xim_message_critical);
	T (g_xim_message_debug);
	T (g_xim_message_bug);
	T (dbus);

	suite_add_tcase(s, tc);

	return s;
}
