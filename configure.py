#!/usr/bin/python
#  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the 
#  Free Software Foundation; either version 2, or (at your option) any 
#  later version.
#
#  This program is distributed in the hope that it will be useful, but 
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
#  for more details.
#
#  You should have received a copy of the GNU General Public License along 
#  with this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import os.path
import sys

package = 'Coco'
default_prefix = '/usr'

arglist = [('prefix', [default_prefix], 'PREFIX',
            'install architecture-independent files in PREFIX [%s]' % default_prefix),
           ('exec-prefix', ['@prefix'], 'EXEC_PREFIX',
            'install architecture-dependent files in EPREFIX [PREFIX]'),
           ('bindir', ['@exec-prefix', 'bin'], 'BINDIR',
            'user executables [EPREFIX/bin]'),
           ('sbindir', ['@exec-prefix', 'sbin'], 'SBINDIR',
            'system admin executables [EPREFIX/sbin]'),
           ('libexecdir', ['@exec-prefix', 'libexec'], 'LIBEXECDIR',
            'program executables [EPREFIX/libexec]'),
           ('sysconfdir', ['@prefix', 'etc'], 'SYSCONFDIR',
            'read-only single-machine data [PREFIX/etc]'),
           ('sharedstatedir', ['@prefix', 'com'], 'SHAREDSTATEDIR',
            'modifiable architecture-independent data [PREFIX/com]'),
           ('localstatedir', ['@prefix', 'var'], 'LOCALSTATEDIR',
            'modifiable single-machine data [PREFIX/var]'),
           ('libdir', ['@exec-prefix', 'lib'], 'LIBDIR',
            'object code libraries [EPREFIX/lib]'),
           ('includedir', ['@prefix', 'include'], 'INCLUDEDIR',
            'C header files [PREFIX/include]'),
           ('datarootdir', ['@prefix', 'share'], 'DATAROOTDIR',
            'read-only arch-independent data root [PREFIX/share]'),
           ('datadir', ['@datarootdir'], 'DATADIR',
            'read-only architecture-independent data root [DATAROOTDIR]'),
           ('infodir', ['@datarootdir', 'info'], 'INFODIR',
            'info documentation [DATAROOTDIR/info]'),
           ('localedir', ['@datarootdir', 'locale'], 'LOCALEDIR',
            'locale-dependent data [DATAROOTDIR/locale]'),
           ('mandir', ['@datarootdir', 'man'], 'MANDIR',
            'man documentation [DATAROOTDIR/man]'),
           ('docdir', ['@datarootdir', 'doc', package], 'DOCDIR',
            'documentation root [DATAROOTDIR/doc/PACKAGE]'),
           ('htmldir', ['@docdir'], 'HTMLDIR', 'html documentation [DOCDIR]'),
           ('dvidir', ['@docdir'], 'DVIDIR', 'dvi documentation [DOCDIR]'),
           ('pdfdir', ['@docdir'], 'PDFDIR', 'pdf documentation [PDFDIR]'),
           ('psdir', ['@docdir'], 'PSDIR', 'ps documentation [PSDIR]'),
           ('program-prefix', None, 'PROGRAM_PREFIX',
            'prepend to installed program names'),
           ('program-suffix', None, 'PROGRAM_SUFFIX',
            'append to installed program names')]

argmap = {}
for v in arglist:
    argmap[v[0]] = v[1]

valuemap = {}

def show_helpmsg():
    print 'Possible options:'
    for v in arglist:
        if v[0] == 'prefix': value = 'PREFIX'
        elif v[0] == 'exec-prefix': value = 'EPREFIX'
        elif v[0] == 'program-prefix': value = 'PREFIX'
        elif v[0] == 'program-suffix': value = 'SUFFIX'
        else: value = 'DIR'
        value = '  --%s=%s' % (v[0], value)
        while len(value) < 28: value = value + ' '
        print '%s%s' % (value, v[3])

for arg in sys.argv:
    if arg == '--help':
        show_helpmsg()
        sys.exit(0)
    if arg[:2] != '--': continue
    argsplit = arg[2:].split('=')
    if len(argsplit) != 2: continue
    if argsplit[0] not in argmap: continue
    valuemap[argsplit[0]] = argsplit[1]

def fillvalue(arg):
    value = argmap[arg]
    if value == None:
        valuemap[arg] = None
        return
    if value[0][0] == '@':
        refname = value[0][1:]
        if refname not in valuemap: fillvalue(refname)
        value = [valuemap[refname]] + value[1:]
    valuemap[arg] = os.path.join(*value)

for arg in argmap.keys():
    if arg in valuemap: continue
    fillvalue(arg)

output = open('config.status', 'w')
output.write(repr(valuemap))
output.close()

output = open('config.h', 'w')
output.write("""#ifndef CONFIG_H
#define CONFIG_H

#ifndef ACCONFIG_H_SEEN
#include "acconfig.h"
#endif

#define PACKAGE "%s"
""" % (package))
for arg in arglist:
    value = valuemap[arg[0]]
    if value is None: value = ''
    output.write('#define %s "%s"\n' % (arg[2], repr(value)[1:-1]))
output.write("""
#endif
""")
