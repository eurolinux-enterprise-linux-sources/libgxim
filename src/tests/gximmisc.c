/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmisc.c
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
#include <unistd.h>
#include <glib/gstdio.h>
#include "libgxim/gximmessages.h"
#include "libgxim/gximmisc.h"
#include "main.h"

GMainLoop *loop;

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
static gboolean
_test_g_xim_init(gpointer data)
{
	gchar *filename = g_build_filename(g_get_tmp_dir(), "gximmisc", NULL);
	gchar buf[256];
	GXimMessages *m;
	FILE *fp;

	g_setenv("LIBGXIM_DEBUG", "foo,bar", TRUE);
	g_xim_init();

	if (g_unlink(filename) == -1) {
		fail_if(errno != ENOENT, "Unable to clean up the log file");
	}
	m = g_xim_messages_new();
	g_xim_messages_activate(m, TRUE);
	g_xim_messages_set_filename(m, filename);

	g_xim_message(m, "foo", "123");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "123") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_xim_message(m, "bar", "456");
	fail_unless((fp = fopen(filename, "r")) != NULL, "Unable to open a log file");
	fgets(buf, 255, fp);
	fail_unless(strcmp(buf, "456") == 0, "Unexpected output in the log");
	fclose(fp);
	g_unlink(filename);

	g_free(filename);

	g_main_loop_quit(data);

	return FALSE;
}

TDEF (g_xim_init)
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);
	g_timeout_add_seconds(1, _test_g_xim_init, loop);
	g_main_loop_run(loop);
	g_main_loop_unref(loop);
} TEND

/************************************************************/
Suite *
g_xim_suite(void)
{
	Suite *s = suite_create("Miscellaneous");
	TCase *tc = tcase_create("Generic Functionalities");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout(tc, 10);

	T (g_xim_init);

	suite_add_tcase(s, tc);

	return s;
}
