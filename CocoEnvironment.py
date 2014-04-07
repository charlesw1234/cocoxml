# -*- python -*-
# Author: Charles Wang <charlesw123456@gmail.com>
# License: GPLv2

import os
import sys
import SCons.Builder
import SCons.Node.FS
import SCons.Util
from SCons.Environment import Environment

def CheckFunc0(context, func_call, headers):
    context.Message('Checking for %s ...' % func_call)
    ret = context.TryLink("""%s
int main(void) { %s; return 0; }
""" % (headers, func_call), '.c')
    context.Result(ret)
    return ret

def CheckJava(context):
    context.Message('Checking for java ...')
    # Dirty hack, context.env.Jar shall not present
    # when the jre is not present ...
    ret = 1
    try:
        jar = context.env.Jar
    except AttributeError:
        ret = 0
#    ret = context.TryBuild(context.env.Java,
#                           """import java.io.File;
#class conftest_2 {
#    public static void main(String args[]) { System.out.println("hello"); }
#}
#""", '.java')
    context.Result(ret)
    return ret

def CheckMono(context):
    context.Message('Checking for mono ...')
    ret = context.TryBuild(context.env.CLIProgram,
                           """using System;
public class hello { static void Main() { Console.WriteLine("hello"); } }
""", '.cs')
    context.Result(ret)
    return ret

def MonoSetup(env):
    csccom = "$CSC $CSCFLAGS -out:${TARGET.abspath} $SOURCES"
    csclibcom = "$CSC -t:library $CSCLIBFLAGS $_CSCLIBPATH $_CSCLIBS -out:${TARGET.abspath} $SOURCES"

    McsBuilder = SCons.Builder.Builder(action = '$CSCCOM',
                                       source_factory = SCons.Node.FS.default_fs.Entry,
                                       suffix = '.exe',
                                       src_suffix = '.cs')
    McsLibBuilder = SCons.Builder.Builder(action = '$CSCLIBCOM',
                                          source_factory = SCons.Node.FS.default_fs.Entry,
                                          suffix = '.dll',
                                          src_suffix = '.cs')
    env['BUILDERS']['CLIProgram'] = McsBuilder
    env['BUILDERS']['CLILibrary'] = McsLibBuilder

    env['CSC'] = 'gmcs'
    env['_CSCLIBS'] = "${_stripixes('-r:', CILLIBS, '', '-r', '', __env__)}"
    env['_CSCLIBPATH'] = "${_stripixes('-lib:', CILLIBPATH, '', '-r', '', __env__)}"
    env['CSCFLAGS'] = SCons.Util.CLVar('')
    env['CSCCOM'] = SCons.Action.Action(csccom)
    env['CSCLIBCOM'] = SCons.Action.Action(csclibcom)

def CocoEnvironment(**kwargs):
    if sys.platform == 'win32':
        # The following Environment setup is used with MSVC in Win32.
        kwargs['ENV'] = os.environ
        #kwargs['tools'] = ['gcc', 'g++', 'gnulink', 'gas', 'ar']
        #kwargs['CCFLAGS'] = ['-g', '-Wall']
    elif sys.platform == 'linux2':
        kwargs['CCFLAGS'] = ['-g', '-Wall']
    env = Environment(**kwargs)
    MonoSetup(env)
    conf = env.Configure(config_h = env['config_h'],
                         custom_tests = { 'CheckFunc0' : CheckFunc0,
                                          'CheckJava' : CheckJava,
                                          'CheckMono' : CheckMono })
    conf.env['COCO_FEATURES'] = []
    if conf.CheckFunc0('readdir_r((void *)0, (void *)0, (void *)0)',
                       '#include <dirent.h>'):
        conf.Define('HAVE_READDIR_R', 1)
    if conf.CheckLibWithHeader('expat', 'expat.h', 'c',
                               'XML_ParserCreate(NULL);', autoadd=0):
        conf.Define('HAVE_EXPAT_H', 1)
        conf.env['COCO_FEATURES'].append('expat')
    if conf.CheckJava():
        conf.env['COCO_FEATURES'].append('java')
    if conf.CheckMono():
        conf.env['COCO_FEATURES'].append('mono')
    env = conf.Finish()
    return env
