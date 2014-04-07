# -*- python -*-
# Author: Charles Wang <charlesw123456@gmail.com>
# License: GPLv2

import os.path
from CocoEnvironment import CocoEnvironment

topsrc = os.getcwd()
cpppath = [topsrc] + map(lambda subdir: os.path.join(topsrc, subdir),
                         ['core', 'algorithm', 'schemes'])

env = CocoEnvironment(CPPPATH = cpppath, config_h = 'acconfig.h')

Export('env')

libobjs = []
cocosrc_libobjs = []
Export('libobjs', 'cocosrc_libobjs')

Export('env')
SConscript(os.path.join('core', 'SConscript'))
SConscript(os.path.join('algorithm', 'SConscript'))
SConscript(os.path.join('schemes', 'SConscript'))

lib = env.Library('coco', libobjs)
cocosrc_lib = env.Library('cocosrc', cocosrc_libobjs)
Export('lib', 'cocosrc_lib')

cocoexe = env.Program('Coco', ['Coco.c', cocosrc_lib, lib])
Export('cocoexe')

SConscript(os.path.join('applications', 'SConscript'))
SConscript(os.path.join('tests', 'SConscript'))
