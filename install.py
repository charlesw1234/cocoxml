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

import glob
import os.path
import shutil
import string
import sys

cfgfile = open('config.status')
cfgmap = eval(cfgfile.read())
cfgfile.close()

destroot = ''
for arg in sys.argv:
    if arg[:9] == 'DESTROOT=': destroot = arg[9:]

def execname(name):
    bname = os.path.basename(name)
    if cfgmap['program-prefix']: bname = cfgmap['program-prefix'] + bname
    if cfgmap['program-suffix']: bname = bname + cfgmap['program-suffix']
    return os.path.join(os.path.dirname(name), bname)

def rootjoin(destroot, destpath):
    if destroot == '': return destpath
    if destpath[0] == '/': destpath = destpath[1:]
    return os.path.join(destroot, destpath)

def install_lines(destroot, tgtfile, lines):
    tgtfile = rootjoin(destroot, tgtfile)
    tgtdir = os.path.dirname(tgtfile)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtf = open(tgtfile, 'w')
    tgtf.write(string.join(lines, '\n'))
    tgtf.close()

def install(destroot, tgtdir, srcfile):
    tgtdir = rootjoin(destroot, tgtdir)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtfile = os.path.join(tgtdir, os.path.basename(srcfile))
    shutil.copyfile(srcfile, tgtfile)
    shutil.copystat(srcfile, tgtfile)

def install_temp(destroot, tgtdir, srcfile,
                 prefix = '/*---- ', suffix = ' ----*/'):
    def get_command(ln, prefix, suffix):
        b = 0; e = len(ln) - 1
        while b <= e:
            if ln[b].isspace(): b = b + 1
            elif ln[e].isspace(): e = e - 1
            else: break
        if b > e: return False
        if e - b < len(prefix) + len(suffix): return False
        e = e + 1
        return ln[b:b+len(prefix)] == prefix and \
            ln[e-len(suffix):e] == suffix
    tgtdir = rootjoin(destroot, tgtdir)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtfile = os.path.join(tgtdir, os.path.basename(srcfile))
    srcf = open(srcfile, 'r')
    tgtf = open(tgtfile, 'w')
    ln = srcf.readline()
    dump = True
    while ln:
        if dump:
            tgtf.write(ln)
            cmd = get_command(ln, prefix, suffix)
            if cmd:
                dump = False
        else:
            cmd = get_command(ln, prefix, suffix)
            if cmd:
                dump = True
                tgtf.write(ln)
        ln = srcf.readline()
    tgtf.close()
    srcf.close()
    shutil.copystat(srcfile, tgtfile)

# Real installations.
install(destroot, cfgmap['bindir'], execname('Coco'))
install(destroot, cfgmap['libdir'], 'libcoco.a')
for hdr in ['CDefs.h', 'ErrorPool.h', 'Buffer.h', 'Token.h', 'Position.h',
            'IncPathList.h', 'Indent.h', 'ScanInput.h']:
    install(destroot,
            os.path.join(cfgmap['includedir'], 'Coco', 'c'),
            os.path.join('schemes', 'c', hdr))
for hdr in ['XmlScanOper.h']:
    install(destroot,
            os.path.join(cfgmap['includedir'], 'Coco', 'cxml'),
            os.path.join('schemes', 'cxml', hdr))
pclines = []
pclines.append('prefix=%s' % cfgmap['prefix'])
pclines.append('includedir=%s' % cfgmap['includedir'])
pclines.append('')
pclines.append('Name: CocoXml')
pclines.append('Description: Coco/R & CocoXml in C.')
pclines.append('Version: 0.9.1')
pclines.append('Libs: -lcoco')
pclines.append('Cflags: -I${includedir}/Coco')
install_lines(destroot,
              os.path.join(cfgmap['libdir'], 'pkgconfig', 'cocoxml.pc'),
              pclines)

install(destroot, cfgmap['docdir'], 'README')

tempdir = os.path.join(cfgmap['datadir'], 'Coco')

tgtdir = os.path.join(tempdir, 'dump')
for f in glob.glob(os.path.join('schemes', 'dump', '*.html')):
    install_temp(destroot, tgtdir, f, '<!---- ', ' ---->')

for scheme in [('c', ['Scanner', 'Parser'], ['.h', '.c']),
               ('cxml', ['Scanner4Xml', 'Parser4Xml'], ['.h', '.c']),
               ('csharp', ['Buffer', 'ErrorPool', 'Position',
                           'Token', 'Scanner', 'Parser'], ['.cs']),
               ('csharpxml', [], ['.cs'])]:
    tgtdir = os.path.join(tempdir, scheme[0])
    for f in scheme[1]:
        for e in scheme[2]:
            install_temp(destroot, tgtdir,
                         os.path.join('schemes', scheme[0], f + e))
