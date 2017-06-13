#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

import locale
import os
import platform
import pkgutil
import pipes
import re
import sys

from libcxx.test.executor import *
from libcxx.test.tracing import *
import libcxx.util

def loadSiteConfig(lit_config, config, param_name, env_name):
    # We haven't loaded the site specific configuration (the user is
    # probably trying to run on a test file directly, and either the site
    # configuration hasn't been created by the build system, or we are in an
    # out-of-tree build situation).
    site_cfg = lit_config.params.get(param_name,
                                     os.environ.get(env_name))
    if not site_cfg:
        lit_config.warning('No site specific configuration file found!'
                           ' Running the tests in the default configuration.')
    elif not os.path.isfile(site_cfg):
        lit_config.fatal(
            "Specified site configuration file does not exist: '%s'" %
            site_cfg)
    else:
        lit_config.note('using site specific configuration at %s' % site_cfg)
        ld_fn = lit_config.load_config

        # Null out the load_config function so that lit.site.cfg doesn't
        # recursively load a config even if it tries.
        # TODO: This is one hell of a hack. Fix it.
        def prevent_reload_fn(*args, **kwargs):
            pass
        lit_config.load_config = prevent_reload_fn
        ld_fn(config, site_cfg)
        lit_config.load_config = ld_fn

class Configuration(object):
    # pylint: disable=redefined-outer-name
    def __init__(self, lit_config, config):
        self.lit_config = lit_config
        self.config = config
        self.is_windows = platform.system() == 'Windows'
        self.cxx = None
        self.cxx_stdlib_under_test = None
        self.project_obj_root = None
        self.libcxx_src_root = None
        self.libcxx_obj_root = None
        self.cxx_library_root = None
        self.cxx_runtime_root = None
        self.abi_library_root = None
        self.link_shared = self.get_lit_bool('enable_shared', default=True)
        self.debug_build = self.get_lit_bool('debug_build',   default=False)
        self.exec_env = dict(os.environ)
        self.use_target = False
        self.use_system_cxx_lib = False
        self.use_clang_verify = False
        self.long_tests = None
        self.execute_external = False

    def get_lit_conf(self, name, default=None):
        val = self.lit_config.params.get(name, None)
        if val is None:
            val = getattr(self.config, name, None)
            if val is None:
                val = default
        return val

    def get_lit_bool(self, name, default=None, env_var=None):
        def check_value(value, var_name):
            if value is None:
                return default
            if isinstance(value, bool):
                return value
            if not isinstance(value, str):
                raise TypeError('expected bool or string')
            if value.lower() in ('1', 'true'):
                return True
            if value.lower() in ('', '0', 'false'):
                return False
            self.lit_config.fatal(
                "parameter '{}' should be true or false".format(var_name))

        conf_val = self.get_lit_conf(name)
        if env_var is not None and env_var in os.environ and \
                os.environ[env_var] is not None:
            val = os.environ[env_var]
            if conf_val is not None:
                self.lit_config.warning(
                    'Environment variable %s=%s is overriding explicit '
                    '--param=%s=%s' % (env_var, val, name, conf_val))
            return check_value(val, env_var)
        return check_value(conf_val, name)

    def make_static_lib_name(self, name):
        """Return the full filename for the specified library name"""
        if self.is_windows:
            assert name == 'c++'  # Only allow libc++ to use this function for now.
            return 'lib' + name + '.lib'
        else:
            return 'lib' + name + '.a'

    def configure(self):
        self.configure_executor()
        self.configure_use_system_cxx_lib()
        self.configure_target_info()
        self.configure_cxx()
        self.configure_triple()
        self.configure_deployment()
        self.configure_availability()
        self.configure_src_root()
        self.configure_obj_root()
        self.configure_cxx_stdlib_under_test()
        self.cxx.configure_cxx_library_root(self)
        self.configure_use_clang_verify()
        self.cxx.configure_use_thread_safety(self)
        self.configure_execute_external()
        self.cxx.configure_ccache(self)
        self.cxx.configure_compile_flags(self)
        self.configure_filesystem_compile_flags()
        self.cxx.configure_link_flags(self)
        self.configure_env()
        self.cxx.configure_color_diagnostics(self)
        self.configure_debug_mode()
        self.cxx.configure_warnings(self)
        self.cxx.configure_sanitizer(self)
        self.cxx.configure_coverage(self)
        self.cxx.configure_modules(self)
        self.cxx.configure_coroutines(self)
        self.configure_substitutions()
        self.configure_features()

    def print_config_info(self):
        # Print the final compile and link flags.
        self.cxx.print_config_info(self)
        # Print as list to prevent "set([...])" from being printed.
        self.lit_config.note('Using available_features: %s' %
                             list(self.config.available_features))
        show_env_vars = {}
        for k,v in self.exec_env.items():
            if k not in os.environ or os.environ[k] != v:
                show_env_vars[k] = v
        self.lit_config.note('Adding environment variables: %r' % show_env_vars)
        sys.stderr.flush()  # Force flushing to avoid broken output on Windows

    def get_test_format(self):
        from libcxx.test.format import LibcxxTestFormat
        return LibcxxTestFormat(
            self.cxx,
            self.use_clang_verify,
            self.execute_external,
            self.executor,
            exec_env=self.exec_env)

    def configure_executor(self):
        exec_str = self.get_lit_conf('executor', "None")
        te = eval(exec_str)
        if te:
            self.lit_config.note("Using executor: %r" % exec_str)
            if self.lit_config.useValgrind:
                # We have no way of knowing where in the chain the
                # ValgrindExecutor is supposed to go. It is likely
                # that the user wants it at the end, but we have no
                # way of getting at that easily.
                selt.lit_config.fatal("Cannot infer how to create a Valgrind "
                                      " executor.")
        else:
            te = LocalExecutor()
            if self.lit_config.useValgrind:
                te = ValgrindExecutor(self.lit_config.valgrindArgs, te)
        self.executor = te

    def configure_target_info(self):
        from libcxx.test.target_info import make_target_info
        self.target_info = make_target_info(self)

    def configure_cxx(self):
        from libcxx.compiler import make_compiler
        self.cxx = make_compiler(self)

    def configure_src_root(self):
        self.libcxx_src_root = self.get_lit_conf(
            'libcxx_src_root', os.path.dirname(self.config.test_source_root))

    def configure_obj_root(self):
        self.project_obj_root = self.get_lit_conf('project_obj_root')
        self.libcxx_obj_root = self.get_lit_conf('libcxx_obj_root')
        if not self.libcxx_obj_root and self.project_obj_root is not None:
            possible_root = os.path.join(self.project_obj_root, 'projects', 'libcxx')
            if os.path.isdir(possible_root):
                self.libcxx_obj_root = possible_root
            else:
                self.libcxx_obj_root = self.project_obj_root

    def configure_use_system_cxx_lib(self):
        # This test suite supports testing against either the system library or
        # the locally built one; the former mode is useful for testing ABI
        # compatibility between the current headers and a shipping dynamic
        # library.
        # Default to testing against the locally built libc++ library.
        self.use_system_cxx_lib = self.get_lit_conf('use_system_cxx_lib')
        if self.use_system_cxx_lib == 'true':
            self.use_system_cxx_lib = True
        elif self.use_system_cxx_lib == 'false':
            self.use_system_cxx_lib = False
        elif self.use_system_cxx_lib:
            assert os.path.isdir(self.use_system_cxx_lib)
        self.lit_config.note(
            "inferred use_system_cxx_lib as: %r" % self.use_system_cxx_lib)

    def configure_availability(self):
        # See http://llvm.org/docs/AvailabilityMarkup.html
        self.with_availability = self.get_lit_bool('with_availability', False)
        self.lit_config.note(
            "inferred with_availability as: %r" % self.with_availability)

    def configure_cxx_stdlib_under_test(self):
        self.cxx_stdlib_under_test = self.get_lit_conf(
            'cxx_stdlib_under_test', 'libc++')
        if self.cxx_stdlib_under_test not in \
                ['libc++', 'libstdc++', 'msvc', 'cxx_default']:
            self.lit_config.fatal(
                'unsupported value for "cxx_stdlib_under_test": %s'
                % self.cxx_stdlib_under_test)
        self.config.available_features.add(self.cxx_stdlib_under_test)
        if self.cxx_stdlib_under_test == 'libstdc++':
            self.config.available_features.add('libstdc++')
            # Manually enable the experimental and filesystem tests for libstdc++
            # if the options aren't present.
            # FIXME this is a hack.
            if self.get_lit_conf('enable_experimental') is None:
                self.config.enable_experimental = 'true'
            if self.get_lit_conf('enable_filesystem') is None:
                self.config.enable_filesystem = 'true'

    def configure_use_clang_verify(self):
        '''If set, run clang with -verify on failing tests.'''
        if self.with_availability:
            self.use_clang_verify = False
            return
        self.use_clang_verify = self.get_lit_bool('use_clang_verify')
        if self.use_clang_verify is None:
            # NOTE: We do not test for the -verify flag directly because
            #   -verify will always exit with non-zero on an empty file.
            self.use_clang_verify = self.cxx.isVerifySupported()
            self.lit_config.note(
                "inferred use_clang_verify as: %r" % self.use_clang_verify)
        if self.use_clang_verify:
                self.config.available_features.add('verify-support')

    def configure_execute_external(self):
        # Choose between lit's internal shell pipeline runner and a real shell.
        # If LIT_USE_INTERNAL_SHELL is in the environment, we use that as the
        # default value. Otherwise we ask the target_info.
        use_lit_shell_default = os.environ.get('LIT_USE_INTERNAL_SHELL')
        if use_lit_shell_default is not None:
            use_lit_shell_default = use_lit_shell_default != '0'
        else:
            use_lit_shell_default = self.target_info.use_lit_shell_default()
        # Check for the command line parameter using the default value if it is
        # not present.
        use_lit_shell = self.get_lit_bool('use_lit_shell',
                                          use_lit_shell_default)
        self.execute_external = not use_lit_shell

    def add_deployment_feature(self, feature):
        (arch, name, version) = self.config.deployment
        self.config.available_features.add('%s=%s-%s' % (feature, arch, name))
        self.config.available_features.add('%s=%s' % (feature, name))
        self.config.available_features.add('%s=%s%s' % (feature, name, version))

    def configure_features(self):
        additional_features = self.get_lit_conf('additional_features')
        if additional_features:
            for f in additional_features.split(','):
                self.config.available_features.add(f.strip())
        self.target_info.add_locale_features(self.config.available_features)

        target_platform = self.target_info.platform()

        # Write an "available feature" that combines the triple when
        # use_system_cxx_lib is enabled. This is so that we can easily write
        # XFAIL markers for tests that are known to fail with versions of
        # libc++ as were shipped with a particular triple.
        if self.use_system_cxx_lib:
            self.config.available_features.add('with_system_cxx_lib')
            self.config.available_features.add(
                'with_system_cxx_lib=%s' % self.config.target_triple)

            # Add subcomponents individually.
            target_components = self.config.target_triple.split('-')
            for component in target_components:
                self.config.available_features.add(
                    'with_system_cxx_lib=%s' % component)

            # Add available features for more generic versions of the target
            # triple attached to  with_system_cxx_lib.
            if self.use_deployment:
                self.add_deployment_feature('with_system_cxx_lib')

        # Configure the availability markup checks features.
        if self.with_availability:
            self.config.available_features.add('availability_markup')
            self.add_deployment_feature('availability_markup')

        if self.use_system_cxx_lib or self.with_availability:
            self.config.available_features.add('availability')
            self.add_deployment_feature('availability')

        if platform.system() == 'Darwin':
            self.config.available_features.add('apple-darwin')

        # Insert the platform name into the available features as a lower case.
        self.config.available_features.add(target_platform)

        # Simulator testing can take a really long time for some of these tests
        # so add a feature check so we can REQUIRES: long_tests in them
        self.long_tests = self.get_lit_bool('long_tests')
        if self.long_tests is None:
            # Default to running long tests.
            self.long_tests = True
            self.lit_config.note(
                "inferred long_tests as: %r" % self.long_tests)

        if self.long_tests:
            self.config.available_features.add('long_tests')

        self.cxx.add_features(self.config.available_features, self)

        if self.get_lit_bool('has_libatomic', False):
            self.config.available_features.add('libatomic')

        if self.is_windows:
            self.config.available_features.add('windows')
            if self.cxx_stdlib_under_test == 'libc++':
                # LIBCXX-WINDOWS-FIXME is the feature name used to XFAIL the
                # initial Windows failures until they can be properly diagnosed
                # and fixed. This allows easier detection of new test failures
                # and regressions. Note: New failures should not be suppressed
                # using this feature. (Also see llvm.org/PR32730)
                self.config.available_features.add('LIBCXX-WINDOWS-FIXME')

    def configure_filesystem_compile_flags(self):
        enable_fs = self.get_lit_bool('enable_filesystem', default=False)
        if not enable_fs:
            return
        enable_experimental = self.get_lit_bool('enable_experimental', default=False)
        if not enable_experimental:
            self.lit_config.fatal(
                'filesystem is enabled but libc++experimental.a is not.')
        self.config.available_features.add('c++filesystem')
        static_env = os.path.join(self.libcxx_src_root, 'test', 'std',
                                  'experimental', 'filesystem', 'Inputs', 'static_test_env')
        static_env = os.path.realpath(static_env)
        assert os.path.isdir(static_env)
        self.cxx.add_pp_string_flag('LIBCXX_FILESYSTEM_STATIC_TEST_ROOT', static_env)

        dynamic_env = os.path.join(self.config.test_exec_root,
                                   'filesystem', 'Output', 'dynamic_env')
        dynamic_env = os.path.realpath(dynamic_env)
        if not os.path.isdir(dynamic_env):
            os.makedirs(dynamic_env)
        self.cxx.add_pp_string_flag('LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT', dynamic_env)
        self.exec_env['LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT'] = ("%s" % dynamic_env)

        dynamic_helper = os.path.join(self.libcxx_src_root, 'test', 'support',
                                      'filesystem_dynamic_test_helper.py')
        assert os.path.isfile(dynamic_helper)

        self.cxx.add_pp_string_flag('LIBCXX_FILESYSTEM_DYNAMIC_TEST_HELPER', '%s %s'
                                   % (sys.executable, dynamic_helper))

    def configure_debug_mode(self):
        debug_level = self.get_lit_conf('debug_level', None)
        if not debug_level:
            return
        if debug_level not in ['0', '1']:
            self.lit_config.fatal('Invalid value for debug_level "%s".'
                                  % debug_level)
        self.cxx.add_pp_int_flag('_LIBCPP_DEBUG', debug_level)

    def configure_substitutions(self):
        sub = self.config.substitutions
        self.cxx.configure_substitutions(sub)

        # Configure exec prefix substitutions.
        # Configure run env substitution.
        sub.append(('%run', '%t.exe'))
        # Configure not program substitutions
        not_py = os.path.join(self.libcxx_src_root, 'utils', 'not.py')
        not_str = '%s %s ' % (pipes.quote(sys.executable), pipes.quote(not_py))
        sub.append(('not ', not_str))

    def can_use_deployment(self):
        # Check if the host is on an Apple platform using clang.
        if not self.target_info.platform() == "darwin":
            return False
        if not self.target_info.is_host_macosx():
            return False
        if not self.cxx.type.endswith('clang'):
            return False
        return True

    def configure_triple(self):
        # Get or infer the target triple.
        target_triple = self.get_lit_conf('target_triple')
        self.use_target = self.get_lit_bool('use_target', False)
        if self.use_target and target_triple:
            self.lit_config.warning('use_target is true but no triple is specified')

        # Use deployment if possible.
        self.use_deployment = not self.use_target and self.can_use_deployment()
        if self.use_deployment:
            return

        # Save the triple (and warn on Apple platforms).
        self.config.target_triple = target_triple
        if self.use_target and 'apple' in target_triple:
            self.lit_config.warning('consider using arch and platform instead'
                                    ' of target_triple on Apple platforms')

        # If no target triple was given, try to infer it from the compiler
        # under test.
        if not self.config.target_triple:
            target_triple = self.cxx.getTriple()
            # Drop sub-major version components from the triple, because the
            # current XFAIL handling expects exact matches for feature checks.
            # Example: x86_64-apple-darwin14.0.0 -> x86_64-apple-darwin14
            # The 5th group handles triples greater than 3 parts
            # (ex x86_64-pc-linux-gnu).
            target_triple = re.sub(r'([^-]+)-([^-]+)-([^.]+)([^-]*)(.*)',
                                   r'\1-\2-\3\5', target_triple)
            # linux-gnu is needed in the triple to properly identify linuxes
            # that use GLIBC. Handle redhat and opensuse triples as special
            # cases and append the missing `-gnu` portion.
            if (target_triple.endswith('redhat-linux') or
                target_triple.endswith('suse-linux')):
                target_triple += '-gnu'
            self.config.target_triple = target_triple
            self.lit_config.note(
                "inferred target_triple as: %r" % self.config.target_triple)

    def configure_deployment(self):
        assert not self.use_deployment is None
        assert not self.use_target is None
        if not self.use_deployment:
            # Warn about ignored parameters.
            if self.get_lit_conf('arch'):
                self.lit_config.warning('ignoring arch, using target_triple')
            if self.get_lit_conf('platform'):
                self.lit_config.warning('ignoring platform, using target_triple')
            return

        assert not self.use_target
        assert self.target_info.is_host_macosx()

        # Always specify deployment explicitly on Apple platforms, since
        # otherwise a platform is picked up from the SDK.  If the SDK version
        # doesn't match the system version, tests that use the system library
        # may fail spuriously.
        arch = self.get_lit_conf('arch')
        if not arch:
            arch = self.cxx.getTriple().split('-', 1)[0]
            self.lit_config.note("inferred arch as: %r" % arch)

        inferred_platform, name, version = self.target_info.get_platform()
        if inferred_platform:
            self.lit_config.note("inferred platform as: %r" % (name + version))
        self.config.deployment = (arch, name, version)

        # Set the target triple for use by lit.
        self.config.target_triple = arch + '-apple-' + name + version
        self.lit_config.note(
            "computed target_triple as: %r" % self.config.target_triple)

    def configure_env(self):
        self.target_info.configure_env(self.exec_env)

    def add_path(self, dest_env, new_path):
        if 'PATH' not in dest_env:
            dest_env['PATH'] = new_path
        else:
            split_char = ';' if self.is_windows else ':'
            dest_env['PATH'] = '%s%s%s' % (new_path, split_char,
                                           dest_env['PATH'])
