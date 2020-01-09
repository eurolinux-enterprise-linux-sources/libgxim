#! /usr/bin/env ruby
# mkacc.rb
# Copyright (C) 2008 Akira TAGOH
#
# Authors:
#   Akira TAGOH  <akira@tagoh.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
require 'optparse'

options = {}
opts = OptionParser.new do |opts|
  opts.banner = sprintf("Usage: %s [options] <marshal list file>", File.basename($0))
  opts.on("-p", "--prefix PREFIX", String, "Prefix") do |arg|
    options[:prefix] = arg
  end
  opts.on("-o", "--output FILE", String, "Output filename") do |arg|
    options[:output] = File.open(arg, "rw")
  end
  opts.on("-m", "--mode MODE", ["header", "body"], "Output mode [MODE=(header|body)]") do |arg|
    options[:mode] = arg
  end
  opts.on("-e", "--evalfunc FILE", String, "Use own file to evaluate value") do |arg|
    options[:eval] = sprintf("#include \"%s\"", arg)
  end
end
opts.parse!

options[:output] = $stdout if options[:output].nil?
options[:prefix] = '' if options[:prefix].nil?
options[:eval] = '' if options[:eval].nil?
if options[:mode].nil? ||
    ARGV.empty? then
  puts opts
  exit
end
header_tmpl = ''

if options[:mode] == 'body' then
  header_tmpl = <<_EOT_
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

__EVAL__

#ifndef G_XIM_ACC_EVAL_BOOLEAN
#define G_XIM_ACC_EVAL_BOOLEAN(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_CHAR
#define G_XIM_ACC_EVAL_CHAR(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_UCHAR
#define G_XIM_ACC_EVAL_UCHAR(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_INT
#define G_XIM_ACC_EVAL_INT(_v_)		(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_UINT
#define G_XIM_ACC_EVAL_UINT(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_LONG
#define G_XIM_ACC_EVAL_LONG(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_ULONG
#define G_XIM_ACC_EVAL_ULONG(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_ENUM
#define G_XIM_ACC_EVAL_ENUM(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_FLAGS
#define G_XIM_ACC_EVAL_FLAGS(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_FLOAT
#define G_XIM_ACC_EVAL_FLOAT(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_DOUBLE
#define G_XIM_ACC_EVAL_DOUBLE(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_STRING
#define G_XIM_ACC_EVAL_STRING(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_PARAM
#define G_XIM_ACC_EVAL_PARAM(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_BOXED
#define G_XIM_ACC_EVAL_BOXED(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_POINTER
#define G_XIM_ACC_EVAL_POINTER(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_OBJECT
#define G_XIM_ACC_EVAL_OBJECT(_v_)	(_v_)
#endif

_EOT_
elsif options[:mode] == 'header' then
  header_tmpl = <<_EOT_
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <glib.h>
#include <glib-object.h>
_EOT_
end

param2typeval = {
  'VOID' => 'void',
  'BOOLEAN' => 'gboolean',
  'CHAR' => 'gchar',
  'UCHAR' => 'guchar',
  'INT' => 'gint',
  'UINT' => 'guint',
  'LONG' => 'glong',
  'ULONG' => 'gulong',
  'ENUM' => 'gint',
  'FLAGS' => 'guint',
  'FLOAT' => 'gfloat',
  'DOUBLE' => 'gdouble',
  'STRING' => 'const gchar *',
  'PARAM' => 'const GParamSpec *',
  'BOXED' => 'const GBoxed *',
  'POINTER' => 'gpointer',
  'OBJECT' => 'GObject *',
  'NONE' => 'void',
  'BOOL' => 'gboolean'
}
param2type = {
  'VOID' => 'void',
  'BOOLEAN' => 'gboolean',
  'CHAR' => 'gchar',
  'UCHAR' => 'guchar',
  'INT' => 'gint',
  'UINT' => 'guint',
  'LONG' => 'glong',
  'ULONG' => 'gulong',
  'ENUM' => 'gint',
  'FLAGS' => 'guint',
  'FLOAT' => 'gfloat',
  'DOUBLE' => 'gdouble',
  'STRING' => 'gchar *',
  'PARAM' => 'GParamSpec *',
  'BOXED' => 'GBoxed *',
  'POINTER' => 'gpointer',
  'OBJECT' => 'GObject *',
  'NONE' => 'void',
  'BOOL' => 'gboolean'
}

ARGV.each do |file|
  File.open(file) do |f|
    duplicated = {}
    data = ''
    n = 0
    f.each_line do |line|
      n += 1
      retval, args = line.gsub(/#.*/, '').gsub(/\n/, '').split(':')
      next if retval.nil?
      retval = 'BOOLEAN' if retval.upcase == 'BOOL'
      next if param2typeval[retval.upcase] == 'void'
      next if duplicated[retval.upcase]
      args = args.split(',')
      tmpl = ''

      duplicated[retval.upcase] = true
      if options[:mode] == 'body' then
        tmpl = <<_EOT_
/* __TYPE__:__ARGS__ (__FILE__:__LINE__) */
gboolean
__PREFIX___signal_accumulator____TYPE__(GSignalInvocationHint *hints,
__PREFIXTAB__                     __TYPETAB__ GValue                *return_accu,
__PREFIXTAB__                     __TYPETAB__ const GValue          *handler_return,
__PREFIXTAB__                     __TYPETAB__ gpointer               data)
{
	__TYPEVAL__ ret = g_value_get___TYPE_SMALLLETTER__(handler_return);

	g_value_set___TYPE_SMALLLETTER__(return_accu, ret);

	return !G_XIM_ACC_EVAL___TYPE__(ret);
}

_EOT_
      elsif options[:mode] == 'header' then
	tmpl = <<_EOT_
/* __TYPE__:__ARGS__ (__FILE__:__LINE__) */
extern gboolean __PREFIX___signal_accumulator____TYPE__(GSignalInvocationHint *hints,
                __PREFIXTAB__                     __TYPETAB__ GValue                *return_accu,
                __PREFIXTAB__                     __TYPETAB__ const GValue          *handler_return,
                __PREFIXTAB__                     __TYPETAB__ gpointer               data);

_EOT_
      end

      tmpl.gsub!(/__PREFIX__/, options[:prefix])
      tmpl.gsub!(/__PREFIXTAB__/, " "*options[:prefix].length)
      tmpl.gsub!(/__TYPE__/, retval.upcase)
      tmpl.gsub!(/__TYPETAB__/, " "*retval.length)
      tmpl.gsub!(/__TYPEVAL__/, param2typeval[retval.upcase])
      tmpl.gsub!(/__TYPE_SMALLLETTER__/, retval.downcase)
      tmpl.gsub!(/__ARGS__/, args.join(','))
      tmpl.gsub!(/__FILE__/, file)
      tmpl.gsub!(/__LINE__/, n.to_s)
      data << tmpl
    end

    options[:output].print header_tmpl.gsub(/__EVAL__/, options[:eval])
    options[:output].print data
  end
end
