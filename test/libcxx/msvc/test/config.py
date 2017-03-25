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
import re
import shlex
import shutil
import sys

import lit.Test  # pylint: disable=import-error,no-name-in-module
import lit.util  # pylint: disable=import-error,no-name-in-module

from libcxx.msvc.test.format import LibcxxTestFormat

# configuration = config_module.Configuration(lit_config, config)
# configuration.configure()
# configuration.print_config_info()
# config.test_format = configuration.get_test_format()

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
        self.lig_config = lit_config
        self.config = config
        pass

    def configure(self):
        header_subset = [
            "float.h",
            #"iso646.h",
            "limits.h",
            #"stdalign.h",
            "stdarg.h",
            #"stdbool.h",
            "stddef.h",
            "stdint.h",
            #"stdnoreturn.h",

            "algorithm", #
            "array", #
            "atomic",
            "cfloat",
            "ciso646",
            "climits",
            #"cstdalign",
            "cstdarg",
            "cstdbool",
            "cstddef",
            "cstdint",
            "cstdlib",
            "initializer_list",
            "limits",
            "memory", #
            "new",
            "tuple", #
            "type_traits",
            "utility", #
        ]
        for header in header_subset: # self.target_info.header_subset():
            self.config.available_features.add('header.{0}'.format(header))
        self.config.available_features.add('fsized-deallocation')
        self.config.available_features.add('-faligned-allocation')
        self.config.available_features.add('no-aligned-allocation')
        self.config.available_features.add('libcpp-no-if-constexpr')
        self.config.available_features.add('libcpp-no-structured-bindings')
        self.config.available_features.add('libcpp-no-exceptions')
        self.config.available_features.add('libcpp-no-rtti')
        self.config.available_features.add('libcpp-abi-unstable')
        self.config.available_features.add('c++14')
        
    def print_config_info(self):
        pass
        
    def get_test_format(self):
        return LibcxxTestFormat()
