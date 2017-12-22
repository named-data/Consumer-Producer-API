# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

from waflib import Logs, Utils, Context
import os

VERSION = '0.1'
APPNAME = 'consumerproducer'

def options(opt):
    opt.load(['compiler_c', 'compiler_cxx', 'gnu_dirs'])
    opt.load(['boost', 'default-compiler-flags', 'coverage', 'sanitizers'],
             tooldir=['.waf-tools'])

    syncopt = opt.add_option_group ("Consumer-Producer-API Options")
    syncopt.add_option('--with-examples', action='store_true', default=False, dest='_examples',
                       help='''build examples''')

def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs', 'boost', 'default-compiler-flags'])

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)

    boost_libs = 'system thread iostreams'
    conf.check_boost(lib=boost_libs, mt=True)

    if conf.options._examples:
        conf.env['Consumer_Producer_API_HAVE_EXAMPLES'] = 1
        conf.define('Consumer_Producer_API_HAVE_EXAMPLES', 1);

    # Loading "late" to prevent tests to be compiled with profiling flags
    conf.load('coverage')

    conf.load('sanitizers')

    conf.write_config_header('config.hpp')

def build(bld):
    libconsumerproducer = bld(
        target='consumerproducer',
        features=['cxx', 'cxxshlib'],
        source =  bld.path.ant_glob(['src/**/*.cpp', 'src/**/*.proto']),
        use = 'BOOST NDN_CXX LOG4CXX',
        includes = ['src', '.'],
        export_includes=['src', '.'],
        )

    # Examples
    if bld.env["Consumer_Producer_API_HAVE_EXAMPLES"]:
        bld.recurse('examples')

    bld.install_files(
        dest = '%s/Consumer-Producer-API' % bld.env['INCLUDEDIR'],
        files = bld.path.ant_glob(['src/**/*.hpp', 'src/**/*.h']),
        cwd = bld.path.find_dir("src"),
        relative_trick = False,
        )

    bld.install_files(
        dest = "%s/Consumer-Producer-API" % bld.env['INCLUDEDIR'],
        files = bld.path.get_bld().ant_glob(['src/**/*.hpp', 'src/**/*.h', 'config.hpp']),
        cwd = bld.path.get_bld().find_dir("src"),
        relative_trick = False,
        )

    pc = bld(
        features = "subst",
        source='Consumer-Producer-API.pc.in',
        target='Consumer-Producer-API.pc',
        install_path = '${LIBDIR}/pkgconfig',
        PREFIX       = bld.env['PREFIX'],
        INCLUDEDIR   = "%s/Consumer-Producer-API" % bld.env['INCLUDEDIR'],
        VERSION      = VERSION,
        )

def version(ctx):
    if getattr(Context.g_module, 'VERSION_BASE', None):
        return

    Context.g_module.VERSION_BASE = Context.g_module.VERSION
    Context.g_module.VERSION_SPLIT = [v for v in VERSION_BASE.split('.')]

    try:
        cmd = ['git', 'describe', '--match', 'Consumer-Producer-API-*']
        p = Utils.subprocess.Popen(cmd, stdout=Utils.subprocess.PIPE,
                                   stderr=None, stdin=None)
        out = p.communicate()[0].strip()
        if p.returncode == 0 and out != "":
            Context.g_module.VERSION = out[11:]
    except:
        pass

def dist(ctx):
    version(ctx)

def distcheck(ctx):
    version(ctx)
