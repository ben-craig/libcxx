#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

import platform
import os
import shlex
import libcxx.util

def make_compiler(full_config):
    # Gather various compiler parameters.
    cxx_path = full_config.get_lit_conf('cxx_under_test')

    cxx_is_clang_cl = cxx_path is not None and \
                           os.path.basename(cxx_path) == 'clang-cl.exe'
    # If no specific cxx_under_test was given, attempt to infer it as
    # clang++.
    if cxx_path is None or cxx_is_clang_cl:
        search_paths = full_config.config.environment['PATH']
        if cxx_path is not None and os.path.isabs(cxx_path):
            search_paths = os.path.dirname(cxx_path)
        clangxx = libcxx.util.which('clang++', search_paths)
        if clangxx:
            cxx_path = clangxx
            full_config.lit_config.note(
                "inferred cxx_under_test as: %r" % cxx_path)
        elif cxx_is_clang_cl:
            full_config.lit_config.fatal('Failed to find clang++ substitution for'
                                  ' clang-cl')
    if not cxx_path:
        full_config.lit_config.fatal('must specify user parameter cxx_under_test '
                              '(e.g., --param=cxx_under_test=clang++)')

    if cxx_is_clang_cl:
        cxx = make_clang_cl(cxx_path, full_config)
    else:
        cxx = CXXCompiler(cxx_path)
    cxx.compile_env = dict(os.environ)

    return cxx

def make_clang_cl(cxx_conf, full_config):
    def _split_env_var(var):
        return [p.strip() for p in os.environ.get(var, '').split(';') if p.strip()]

    def _prefixed_env_list(var, prefix):
        from itertools import chain
        return list(chain.from_iterable((prefix, path) for path in _split_env_var(var)))

    flags = []
    compile_flags = _prefixed_env_list('INCLUDE', '-isystem')
    link_flags = _prefixed_env_list('LIB', '-L')
    for path in _split_env_var('LIB'):
        full_config.add_path(full_config.exec_env, path)
    return CXXCompiler(cxx_conf, flags=flags,
                       compile_flags=compile_flags,
                       link_flags=link_flags)

class CXXCompilerInterface(object):
    def add_pp_flag(self, name, value=None): pass
    def print_config_info(self, full_config): pass
    def configure_use_thread_safety(self, full_config): pass
    def configure_ccache(self, full_config): pass
    def add_features(self, features, full_config): pass
    def configure_compile_flags(self, full_config): pass
    def configure_link_flags(self, full_config): pass
    def isVerifySupported(self): return False
    def hasCompileFlag(self, flag): return False
    def dumpMacros(self, source_files=None, flags=[], cwd=None): return {}
    def addFlagIfSupported(self, flag): return False
    def useCCache(self, value=True): pass
    def useWarnings(self, value=True): pass
    def hasWarningFlag(self, flag): return False
    def addWarningFlagIfSupported(self, flag): return False
    def useModules(self, value=True): pass
    def getTriple(self): return None

    def compileLinkTwoSteps(self, source_file, out=None, object_file=None,
                            flags=[], cwd=None):
        cmd, out, err, rc = ("", "", "", -1)
        return cmd, out, err, rc

    def compile(self, source_files, out=None, flags=[], cwd=None):
        cmd, out, err, rc = ("", "", "", -1)
        return cmd, out, err, rc

class CXXCompiler(CXXCompilerInterface):
    CM_Default = 0
    CM_PreProcess = 1
    CM_Compile = 2
    CM_Link = 3

    def __init__(self, path, flags=None, compile_flags=None, link_flags=None,
                 warning_flags=None, verify_supported=None,
                 verify_flags=None, use_verify=False,
                 modules_flags=None, use_modules=False,
                 use_ccache=False, use_warnings=False, compile_env=None,
                 cxx_type=None, cxx_version=None):
        self.source_lang = 'c++'
        self.path = path
        self.flags = list(flags or [])
        self.compile_flags = list(compile_flags or [])
        self.link_flags = list(link_flags or [])
        self.warning_flags = list(warning_flags or [])
        self.verify_supported = verify_supported
        self.use_verify = use_verify
        self.verify_flags = list(verify_flags or [])
        assert not use_verify or verify_supported
        assert not use_verify or verify_flags is not None
        self.modules_flags = list(modules_flags or [])
        self.use_modules = use_modules
        assert not use_modules or modules_flags is not None
        self.use_ccache = use_ccache
        self.use_warnings = use_warnings
        if compile_env is not None:
            self.compile_env = dict(compile_env)
        else:
            self.compile_env = None
        self.type = cxx_type
        self.version = cxx_version
        if self.type is None or self.version is None:
            self._initTypeAndVersion()

    def add_pp_string_flag(self, name, value=None):
        if value is None:
            self.compile_flags += ['-D%s' % name]
        else:
            self.compile_flags += ['-D%s="%s"' % (name, value)]

    def print_config_info(self, full_config):
        full_config.lit_config.note('Using compiler: %s' % self.path)
        full_config.lit_config.note('Using flags: %s' % self.flags)
        if self.use_modules:
            full_config.lit_config.note('Using modules flags: %s' %
                                 self.modules_flags)
        full_config.lit_config.note('Using compile flags: %s'
                             % self.compile_flags)
        if len(self.warning_flags):
            full_config.lit_config.note('Using warnings: %s' % self.warning_flags)
        full_config.lit_config.note('Using link flags: %s' % self.link_flags)

    def configure_use_thread_safety(self, full_config):
        has_thread_safety = self.hasCompileFlag('-Werror=thread-safety')
        if has_thread_safety:
            self.compile_flags += ['-Werror=thread-safety']
            full_config.config.available_features.add('thread-safety')
            full_config.lit_config.note("enabling thread-safety annotations")

    def configure_ccache(self, full_config):
        use_ccache_default = os.environ.get('LIBCXX_USE_CCACHE') is not None
        use_ccache = full_config.get_lit_bool('use_ccache', use_ccache_default)
        if use_ccache:
            # 'CCACHE_CPP2' prevents ccache from stripping comments while
            # preprocessing. This is required to prevent stripping of '-verify'
            # comments.
            self.compile_env['CCACHE_CPP2'] = '1'

            self.use_ccache = True
            full_config.lit_config.note('enabling ccache')

    def add_features(self, features, full_config):
        if self.type is not None:
            assert self.version is not None
            maj_v, min_v, _ = self.version
            features.add(self.type)
            features.add('%s-%s' % (self.type, maj_v))
            features.add('%s-%s.%s' % (self.type, maj_v, min_v))
        # Run a compile test for the -fsized-deallocation flag. This is needed
        # in test/std/language.support/support.dynamic/new.delete
        if self.hasCompileFlag('-fsized-deallocation'):
            features.add('fsized-deallocation')

        if self.hasCompileFlag('-faligned-allocation'):
            features.add('-faligned-allocation')
        else:
            # FIXME remove this once more than just clang-4.0 support
            # C++17 aligned allocation.
            features.add('no-aligned-allocation')

        macros = self.dumpMacros()
        if '__cpp_if_constexpr' not in macros:
            features.add('libcpp-no-if-constexpr')

        if '__cpp_structured_bindings' not in macros:
            features.add('libcpp-no-structured-bindings')

        if '__cpp_deduction_guides' not in macros:
            features.add('libcpp-no-deduction-guides')

        # Attempt to detect the glibc version by querying for __GLIBC__
        # in 'features.h'.
        macros = self.dumpMacros(flags=['-include', 'features.h'])
        if macros is not None and '__GLIBC__' in macros:
            maj_v, min_v = (macros['__GLIBC__'], macros['__GLIBC_MINOR__'])
            features.add('glibc')
            features.add('glibc-%s' % maj_v)
            features.add('glibc-%s.%s' % (maj_v, min_v))

        # Support Objective-C++ only on MacOS and if the compiler supports it.
        if full_config.target_info.platform() == "darwin" and \
           full_config.target_info.is_host_macosx() and \
           self.hasCompileFlag(["-x", "objective-c++", "-fobjc-arc"]):
            features.add("objective-c++")

    def isVerifySupported(self):
        if self.verify_supported is None:
            self.verify_supported = self.hasCompileFlag(['-Xclang',
                                        '-verify-ignore-unexpected'])
            if self.verify_supported:
                self.verify_flags = [
                    '-Xclang', '-verify',
                    '-Xclang', '-verify-ignore-unexpected=note',
                    '-ferror-limit=1024'
                ]
        return self.verify_supported

    def useVerify(self, value=True):
        self.use_verify = value
        assert not self.use_verify or self.verify_flags is not None

    def useModules(self, value=True):
        self.use_modules = value
        assert not self.use_modules or self.modules_flags is not None

    def useCCache(self, value=True):
        self.use_ccache = value

    def useWarnings(self, value=True):
        self.use_warnings = value

    def _initTypeAndVersion(self):
        # Get compiler type and version
        macros = self.dumpMacros()
        if macros is None:
            return
        compiler_type = None
        major_ver = minor_ver = patchlevel = None
        if '__clang__' in macros.keys():
            compiler_type = 'clang'
            # Treat apple's llvm fork differently.
            if '__apple_build_version__' in macros.keys():
                compiler_type = 'apple-clang'
            major_ver = macros['__clang_major__']
            minor_ver = macros['__clang_minor__']
            patchlevel = macros['__clang_patchlevel__']
        elif '__GNUC__' in macros.keys():
            compiler_type = 'gcc'
            major_ver = macros['__GNUC__']
            minor_ver = macros['__GNUC_MINOR__']
            patchlevel = macros['__GNUC_PATCHLEVEL__']
        self.type = compiler_type
        self.version = (major_ver, minor_ver, patchlevel)

    def _basicCmd(self, source_files, out, mode=CM_Default, flags=[],
                  input_is_cxx=False):
        cmd = []
        if self.use_ccache \
                and not mode == self.CM_Link \
                and not mode == self.CM_PreProcess:
            cmd += ['ccache']
        cmd += [self.path]
        if out is not None:
            cmd += ['-o', out]
        if input_is_cxx:
            cmd += ['-x', self.source_lang]
        if isinstance(source_files, list):
            cmd += source_files
        elif isinstance(source_files, str):
            cmd += [source_files]
        else:
            raise TypeError('source_files must be a string or list')
        if mode == self.CM_PreProcess:
            cmd += ['-E']
        elif mode == self.CM_Compile:
            cmd += ['-c']
        cmd += self.flags
        if self.use_verify:
            cmd += self.verify_flags
            assert mode in [self.CM_Default, self.CM_Compile]
        if self.use_modules:
            cmd += self.modules_flags
        if mode != self.CM_Link:
            cmd += self.compile_flags
            if self.use_warnings:
                cmd += self.warning_flags
        if mode != self.CM_PreProcess and mode != self.CM_Compile:
            cmd += self.link_flags
        cmd += flags
        return cmd

    def preprocessCmd(self, source_files, out=None, flags=[]):
        return self._basicCmd(source_files, out, flags=flags,
                             mode=self.CM_PreProcess,
                             input_is_cxx=True)

    def compileCmd(self, source_files, out=None, flags=[]):
        return self._basicCmd(source_files, out, flags=flags,
                             mode=self.CM_Compile,
                             input_is_cxx=True) + ['-c']

    def linkCmd(self, source_files, out=None, flags=[]):
        return self._basicCmd(source_files, out, flags=flags,
                              mode=self.CM_Link)

    def compileLinkCmd(self, source_files, out=None, flags=[]):
        return self._basicCmd(source_files, out, flags=flags)

    def preprocess(self, source_files, out=None, flags=[], cwd=None):
        cmd = self.preprocessCmd(source_files, out, flags)
        out, err, rc = libcxx.util.executeCommand(cmd, env=self.compile_env,
                                                  cwd=cwd)
        return cmd, out, err, rc

    def compile(self, source_files, out=None, flags=[], cwd=None):
        cmd = self.compileCmd(source_files, out, flags)
        out, err, rc = libcxx.util.executeCommand(cmd, env=self.compile_env,
                                                  cwd=cwd)
        return cmd, out, err, rc

    def link(self, source_files, out=None, flags=[], cwd=None):
        cmd = self.linkCmd(source_files, out, flags)
        out, err, rc = libcxx.util.executeCommand(cmd, env=self.compile_env,
                                                  cwd=cwd)
        return cmd, out, err, rc

    def compileLink(self, source_files, out=None, flags=[],
                    cwd=None):
        cmd = self.compileLinkCmd(source_files, out, flags)
        out, err, rc = libcxx.util.executeCommand(cmd, env=self.compile_env,
                                                  cwd=cwd)
        return cmd, out, err, rc

    def compileLinkTwoSteps(self, source_file, out=None, object_file=None,
                            flags=[], cwd=None):
        if not isinstance(source_file, str):
            raise TypeError('This function only accepts a single input file')
        if object_file is None:
            # Create, use and delete a temporary object file if none is given.
            with_fn = lambda: libcxx.util.guardedTempFilename(suffix='.o')
        else:
            # Otherwise wrap the filename in a context manager function.
            with_fn = lambda: libcxx.util.nullContext(object_file)
        with with_fn() as object_file:
            cc_cmd, cc_stdout, cc_stderr, rc = self.compile(
                source_file, object_file, flags=flags, cwd=cwd)
            if rc != 0:
                return cc_cmd, cc_stdout, cc_stderr, rc

            link_cmd, link_stdout, link_stderr, rc = self.link(
                object_file, out=out, flags=flags, cwd=cwd)
            return (cc_cmd + ['&&'] + link_cmd, cc_stdout + link_stdout,
                    cc_stderr + link_stderr, rc)

    def dumpMacros(self, source_files=None, flags=[], cwd=None):
        if source_files is None:
            source_files = os.devnull
        flags = ['-dM'] + flags
        cmd, out, err, rc = self.preprocess(source_files, flags=flags, cwd=cwd)
        if rc != 0:
            return None
        parsed_macros = {}
        lines = [l.strip() for l in out.split('\n') if l.strip()]
        for l in lines:
            assert l.startswith('#define ')
            l = l[len('#define '):]
            macro, _, value = l.partition(' ')
            parsed_macros[macro] = value
        return parsed_macros

    def getTriple(self):
        cmd = [self.path] + self.flags + ['-dumpmachine']
        return libcxx.util.capture(cmd).strip()

    def hasCompileFlag(self, flag):
        if isinstance(flag, list):
            flags = list(flag)
        else:
            flags = [flag]
        # Add -Werror to ensure that an unrecognized flag causes a non-zero
        # exit code. -Werror is supported on all known compiler types.
        if self.type is not None:
            flags += ['-Werror', '-fsyntax-only']
        cmd, out, err, rc = self.compile(os.devnull, out=os.devnull,
                                         flags=flags)
        return rc == 0

    def addFlagIfSupported(self, flag):
        if isinstance(flag, list):
            flags = list(flag)
        else:
            flags = [flag]
        if self.hasCompileFlag(flags):
            self.flags += flags
            return True
        else:
            return False

    def addCompileFlagIfSupported(self, flag):
        if isinstance(flag, list):
            flags = list(flag)
        else:
            flags = [flag]
        if self.hasCompileFlag(flags):
            self.compile_flags += flags
            return True
        else:
            return False

    def hasWarningFlag(self, flag):
        """
        hasWarningFlag - Test if the compiler supports a given warning flag.
        Unlike addCompileFlagIfSupported, this function detects when
        "-Wno-<warning>" flags are unsupported. If flag is a
        "-Wno-<warning>" GCC will not emit an unknown option diagnostic unless
        another error is triggered during compilation.
        """
        assert isinstance(flag, str)
        assert flag.startswith('-W')
        if not flag.startswith('-Wno-'):
            return self.hasCompileFlag(flag)
        flags = ['-Werror', flag]
        old_use_warnings = self.use_warnings
        self.useWarnings(False)
        cmd = self.compileCmd('-', os.devnull, flags)
        self.useWarnings(old_use_warnings)
        # Remove '-v' because it will cause the command line invocation
        # to be printed as part of the error output.
        # TODO(EricWF): Are there other flags we need to worry about?
        if '-v' in cmd:
            cmd.remove('-v')
        out, err, rc = libcxx.util.executeCommand(
            cmd, input=libcxx.util.to_bytes('#error\n'))

        assert rc != 0
        if flag in err:
            return False
        return True

    def addWarningFlagIfSupported(self, flag):
        if self.hasWarningFlag(flag):
            if flag not in self.warning_flags:
                self.warning_flags += [flag]
            return True
        return False

    def configure_compile_flags(self, full_config):
        no_default_flags = full_config.get_lit_bool('no_default_flags', False)
        if not no_default_flags:
            self.configure_default_compile_flags(full_config)
        # This include is always needed so add so add it regardless of
        # 'no_default_flags'.
        support_path = os.path.join(full_config.libcxx_src_root, 'test/support')
        self.compile_flags += ['-I' + support_path]
        # Configure extra flags
        compile_flags_str = full_config.get_lit_conf('compile_flags', '')
        self.compile_flags += shlex.split(compile_flags_str)
        if full_config.is_windows:
            # FIXME: Can we remove this?
            self.compile_flags += ['-D_CRT_SECURE_NO_WARNINGS']
            # Required so that tests using min/max don't fail on Windows,
            # and so that those tests don't have to be changed to tolerate
            # this insanity.
            self.compile_flags += ['-DNOMINMAX']

    def configure_default_compile_flags(self, full_config):
        # Try and get the std version from the command line. Fall back to
        # default given in lit.site.cfg is not present. If default is not
        # present then force c++11.
        std = full_config.get_lit_conf('std')
        if not std:
            # Choose the newest possible language dialect if none is given.
            possible_stds = ['c++1z', 'c++14', 'c++11', 'c++03']
            if self.type == 'gcc':
                maj_v, _, _ = self.version
                maj_v = int(maj_v)
                if maj_v < 7:
                    possible_stds.remove('c++1z')
                # FIXME: How many C++14 tests actually fail under GCC 5 and 6?
                # Should we XFAIL them individually instead?
                if maj_v <= 6:
                    possible_stds.remove('c++14')
            for s in possible_stds:
                if self.hasCompileFlag('-std=%s' % s):
                    std = s
                    full_config.lit_config.note(
                        'inferred language dialect as: %s' % std)
                    break
            if not std:
                full_config.lit_config.fatal(
                    'Failed to infer a supported language dialect from one of %r'
                    % possible_stds)
        self.compile_flags += ['-std={0}'.format(std)]
        full_config.config.available_features.add(std.replace('gnu++', 'c++'))
        # Configure include paths
        self.configure_compile_flags_header_includes(full_config)
        full_config.target_info.add_cxx_compile_flags(self.compile_flags)
        # Configure feature flags.
        self.configure_compile_flags_exceptions(full_config)
        self.configure_compile_flags_rtti(full_config)
        self.configure_compile_flags_abi_version(full_config)
        enable_32bit = full_config.get_lit_bool('enable_32bit', False)
        if enable_32bit:
            self.flags += ['-m32']
        # Use verbose output for better errors
        self.flags += ['-v']
        sysroot = full_config.get_lit_conf('sysroot')
        if sysroot:
            self.flags += ['--sysroot', sysroot]
        gcc_toolchain = full_config.get_lit_conf('gcc_toolchain')
        if gcc_toolchain:
            self.flags += ['-gcc-toolchain', gcc_toolchain]
        # NOTE: the _DEBUG definition must preceed the triple check because for
        # the Windows build of libc++, the forced inclusion of a header requires
        # that _DEBUG is defined.  Incorrect ordering will result in -target
        # being elided.
        if full_config.is_windows and full_config.debug_build:
            self.compile_flags += ['-D_DEBUG']
        if full_config.use_target:
            if not self.addFlagIfSupported(
                    ['-target', full_config.config.target_triple]):
                full_config.lit_config.warning('use_target is true but -target is '\
                        'not supported by the compiler')
        if full_config.use_deployment:
            arch, name, version = full_config.config.deployment
            self.flags += ['-arch', arch]
            self.flags += ['-m' + name + '-version-min=' + version]

        # Disable availability unless explicitely requested
        if not full_config.with_availability:
            self.flags += ['-D_LIBCPP_DISABLE_AVAILABILITY']

    def configure_compile_flags_header_includes(self, full_config):
        support_path = os.path.join(full_config.libcxx_src_root, 'test', 'support')
        self.configure_config_site_header(full_config)
        if full_config.cxx_stdlib_under_test != 'libstdc++' and \
           not full_config.is_windows:
            self.compile_flags += [
                '-include', os.path.join(support_path, 'nasty_macros.hpp')]
        if full_config.cxx_stdlib_under_test == 'msvc':
            self.compile_flags += [
                '-include', os.path.join(support_path,
                                         'msvc_stdlib_force_include.hpp')]
            pass
        if full_config.is_windows and full_config.debug_build and \
                full_config.cxx_stdlib_under_test != 'msvc':
            self.compile_flags += [
                '-include', os.path.join(support_path,
                                         'set_windows_crt_report_mode.h')
            ]
        cxx_headers = full_config.get_lit_conf('cxx_headers')
        if cxx_headers == '' or (cxx_headers is None
                                 and full_config.cxx_stdlib_under_test != 'libc++'):
            full_config.lit_config.note('using the system cxx headers')
            return
        self.compile_flags += ['-nostdinc++']
        if cxx_headers is None:
            cxx_headers = os.path.join(full_config.libcxx_src_root, 'include')
        if not os.path.isdir(cxx_headers):
            full_config.lit_config.fatal("cxx_headers='%s' is not a directory."
                                  % cxx_headers)
        self.compile_flags += ['-I' + cxx_headers]
        if full_config.libcxx_obj_root is not None:
            cxxabi_headers = os.path.join(full_config.libcxx_obj_root, 'include',
                                          'c++build')
            if os.path.isdir(cxxabi_headers):
                self.compile_flags += ['-I' + cxxabi_headers]

    def configure_config_site_header(self, full_config):
        # Check for a possible __config_site in the build directory. We
        # use this if it exists.
        if full_config.libcxx_obj_root is None:
            return
        config_site_header = os.path.join(full_config.libcxx_obj_root, '__config_site')
        if not os.path.isfile(config_site_header):
            return
        contained_macros = self.parse_config_site_and_add_features(
            config_site_header,
            full_config)
        full_config.lit_config.note('Using __config_site header %s with macros: %r'
            % (config_site_header, contained_macros))
        # FIXME: This must come after the call to
        # 'parse_config_site_and_add_features(...)' in order for it to work.
        self.compile_flags += ['-include', config_site_header]

    def parse_config_site_and_add_features(self, header, full_config):
        """ parse_config_site_and_add_features - Deduce and add the test
            features that that are implied by the #define's in the __config_site
            header. Return a dictionary containing the macros found in the
            '__config_site' header.
        """
        # Parse the macro contents of __config_site by dumping the macros
        # using 'c++ -dM -E' and filtering the predefines.
        predefines = self.dumpMacros()
        macros = self.dumpMacros(header)
        feature_macros_keys = set(macros.keys()) - set(predefines.keys())
        feature_macros = {}
        for k in feature_macros_keys:
            feature_macros[k] = macros[k]
        # We expect the header guard to be one of the definitions
        assert '_LIBCPP_CONFIG_SITE' in feature_macros
        del feature_macros['_LIBCPP_CONFIG_SITE']
        # The __config_site header should be non-empty. Otherwise it should
        # have never been emitted by CMake.
        assert len(feature_macros) > 0
        # Transform each macro name into the feature name used in the tests.
        # Ex. _LIBCPP_HAS_NO_THREADS -> libcpp-has-no-threads
        for m in feature_macros:
            if m == '_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS':
                continue
            if m == '_LIBCPP_ABI_VERSION':
                full_config.config.available_features.add('libcpp-abi-version-v%s'
                    % feature_macros[m])
                continue
            assert m.startswith('_LIBCPP_HAS_') or m == '_LIBCPP_ABI_UNSTABLE'
            m = m.lower()[1:].replace('_', '-')
            full_config.config.available_features.add(m)
        return feature_macros

    def configure_compile_flags_exceptions(self, full_config):
        enable_exceptions = full_config.get_lit_bool('enable_exceptions', True)
        if not enable_exceptions:
            full_config.config.available_features.add('libcpp-no-exceptions')
            self.compile_flags += ['-fno-exceptions']

    def configure_compile_flags_rtti(self, full_config):
        enable_rtti = full_config.get_lit_bool('enable_rtti', True)
        if not enable_rtti:
            full_config.config.available_features.add('libcpp-no-rtti')
            self.compile_flags += ['-fno-rtti', '-D_LIBCPP_NO_RTTI']

    def configure_compile_flags_abi_version(self, full_config):
        abi_version = full_config.get_lit_conf('abi_version', '').strip()
        abi_unstable = full_config.get_lit_bool('abi_unstable')
        # Only add the ABI version when it is non-default.
        # FIXME(EricWF): Get the ABI version from the "__config_site".
        if abi_version and abi_version != '1':
          self.compile_flags += ['-D_LIBCPP_ABI_VERSION=' + abi_version]
        if abi_unstable:
          full_config.config.available_features.add('libcpp-abi-unstable')
          self.compile_flags += ['-D_LIBCPP_ABI_UNSTABLE']

    def configure_link_flags(self, full_config):
        no_default_flags = full_config.get_lit_bool('no_default_flags', False)
        if not no_default_flags:
            # Configure library path
            self.configure_link_flags_cxx_library_path(full_config)
            self.configure_link_flags_abi_library_path(full_config)

            # Configure libraries
            if full_config.cxx_stdlib_under_test == 'libc++':
                self.link_flags += ['-nodefaultlibs']
                # FIXME: Handle MSVCRT as part of the ABI library handling.
                if full_config.is_windows:
                    self.link_flags += ['-nostdlib']
                self.configure_link_flags_cxx_library(full_config)
                self.configure_link_flags_abi_library(full_config)
                self.configure_extra_library_flags(full_config)
            elif full_config.cxx_stdlib_under_test == 'libstdc++':
                enable_fs = full_config.get_lit_bool('enable_filesystem',
                                              default=False)
                if enable_fs:
                    full_config.config.available_features.add('c++experimental')
                    self.link_flags += ['-lstdc++fs']
                self.link_flags += ['-lm', '-pthread']
            elif full_config.cxx_stdlib_under_test == 'msvc':
                # FIXME: Correctly setup debug/release flags here.
                pass
            elif full_config.cxx_stdlib_under_test == 'cxx_default':
                self.link_flags += ['-pthread']
            else:
                full_config.lit_config.fatal(
                    'unsupported value for "use_stdlib_type": %s'
                    %  use_stdlib_type)

        link_flags_str = full_config.get_lit_conf('link_flags', '')
        self.link_flags += shlex.split(link_flags_str)

    def configure_link_flags_cxx_library_path(self, full_config):
        if not full_config.use_system_cxx_lib:
            if full_config.cxx_library_root:
                self.link_flags += ['-L' + full_config.cxx_library_root]
                if full_config.is_windows and full_config.link_shared:
                    full_config.add_path(self.compile_env, full_config.cxx_library_root)
            if full_config.cxx_runtime_root:
                if not full_config.is_windows:
                    self.link_flags += ['-Wl,-rpath,' +
                                            full_config.cxx_runtime_root]
                elif full_config.is_windows and full_config.link_shared:
                    full_config.add_path(full_config.exec_env, full_config.cxx_runtime_root)
        elif os.path.isdir(str(full_config.use_system_cxx_lib)):
            self.link_flags += ['-L' + full_config.use_system_cxx_lib]
            if not full_config.is_windows:
                self.link_flags += ['-Wl,-rpath,' +
                                        full_config.use_system_cxx_lib]
            if full_config.is_windows and full_config.link_shared:
                full_config.add_path(self.compile_env, full_config.use_system_cxx_lib)

    def configure_link_flags_abi_library_path(self, full_config):
        # Configure ABI library paths.
        full_config.abi_library_root = full_config.get_lit_conf('abi_library_path')
        if full_config.abi_library_root:
            self.link_flags += ['-L' + full_config.abi_library_root]
            if not full_config.is_windows:
                self.link_flags += ['-Wl,-rpath,' + full_config.abi_library_root]
            else:
                full_config.add_path(full_config.exec_env, full_config.abi_library_root)

    def configure_link_flags_cxx_library(self, full_config):
        libcxx_experimental = full_config.get_lit_bool('enable_experimental', default=False)
        if libcxx_experimental:
            full_config.config.available_features.add('c++experimental')
            self.link_flags += ['-lc++experimental']
        if full_config.link_shared:
            self.link_flags += ['-lc++']
        else:
            cxx_library_root = full_config.get_lit_conf('cxx_library_root')
            if cxx_library_root:
                libname = full_config.make_static_lib_name('c++')
                abs_path = os.path.join(cxx_library_root, libname)
                assert os.path.exists(abs_path) and \
                       "static libc++ library does not exist"
                self.link_flags += [abs_path]
            else:
                self.link_flags += ['-lc++']

    def configure_link_flags_abi_library(self, full_config):
        cxx_abi = full_config.get_lit_conf('cxx_abi', 'libcxxabi')
        if cxx_abi == 'libstdc++':
            self.link_flags += ['-lstdc++']
        elif cxx_abi == 'libsupc++':
            self.link_flags += ['-lsupc++']
        elif cxx_abi == 'libcxxabi':
            if full_config.target_info.allow_cxxabi_link():
                libcxxabi_shared = full_config.get_lit_bool('libcxxabi_shared', default=True)
                if libcxxabi_shared:
                    self.link_flags += ['-lc++abi']
                else:
                    cxxabi_library_root = full_config.get_lit_conf('abi_library_path')
                    if cxxabi_library_root:
                        libname = full_config.make_static_lib_name('c++abi')
                        abs_path = os.path.join(cxxabi_library_root, libname)
                        self.link_flags += [abs_path]
                    else:
                        self.link_flags += ['-lc++abi']
        elif cxx_abi == 'libcxxrt':
            self.link_flags += ['-lcxxrt']
        elif cxx_abi == 'vcruntime':
            debug_suffix = 'd' if full_config.debug_build else ''
            self.link_flags += ['-l%s%s' % (lib, debug_suffix) for lib in
                                    ['vcruntime', 'ucrt', 'msvcrt']]
        elif cxx_abi == 'none' or cxx_abi == 'default':
            if full_config.is_windows:
                debug_suffix = 'd' if full_config.debug_build else ''
                self.link_flags += ['-lmsvcrt%s' % debug_suffix]
        else:
            full_config.lit_config.fatal(
                'C++ ABI setting %s unsupported for tests' % cxx_abi)

    def configure_extra_library_flags(self, full_config):
        if full_config.get_lit_bool('cxx_ext_threads', default=False):
            self.link_flags += ['-lc++external_threads']
        full_config.target_info.add_cxx_link_flags(self.link_flags)

