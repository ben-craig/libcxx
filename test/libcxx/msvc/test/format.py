#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

import errno
import os
import time
import random
import re
from lit.util import to_bytes, to_string

import lit.Test        # pylint: disable=import-error
import lit.TestRunner  # pylint: disable=import-error
from lit.TestRunner import ParserKind, IntegratedTestKeywordParser  \
    # pylint: disable=import-error
import lit.util        # pylint: disable=import-error


import libcxx.util
from libcxx.msvc.test.compiler import CXXCompiler


class LibcxxTestFormat(object):
    """
    Custom test format handler for use with the test format use by libc++.

    Tests fall into two categories:
      FOO.pass.cpp - Executable test which should compile, run, and exit with
                     code 0.
      FOO.fail.cpp - Negative test case which is expected to fail compilation.
      FOO.sh.cpp   - A test that uses LIT's ShTest format.
    """

    def __init__(self):
        self.test_cxx = CXXCompiler(None)
        pass

    def _add_header_requirements(self, test):
        includes_re = re.compile(to_bytes(r"#include\s+<([^>]*)>"))

        with open(test.getSourcePath(), 'rb') as f:
            # Read the entire file contents.
            data = f.read()
            # Ensure the data ends with a newline.
            if not data.endswith(to_bytes('\n')):
                data = data + to_bytes('\n')

            for match in includes_re.finditer(data):
                match_position = match.start()

                # Convert the keyword and line to UTF-8 strings and yield the
                # command. Note that we take care to return regular strings in
                # Python 2, to avoid other code having to differentiate between the
                # str and unicode types.
                header = match.group(1)
                header = header.replace("/", "_")
                test.requires.append(to_string(('header.'+header).decode('utf-8')))

    # TODO: Move this into lit's FileBasedTest
    def getTestsInDirectory(self, testSuite, path_in_suite,
                            litConfig, localConfig):
        source_path = testSuite.getSourcePath(path_in_suite)
        for filename in os.listdir(source_path):
            # Ignore dot files and excluded tests.
            if filename.startswith('.') or filename in localConfig.excludes:
                continue

            filepath = os.path.join(source_path, filename)
            if not os.path.isdir(filepath):
                if any([filename.endswith(ext)
                        for ext in localConfig.suffixes]):
                    yield lit.Test.Test(testSuite, path_in_suite + (filename,),
                                        localConfig)

    def execute(self, test, lit_config):
        while True:
            try:
                return self._execute(test, lit_config)
            except OSError as oe:
                if oe.errno != errno.ETXTBSY:
                    raise
                time.sleep(0.1)

    def _execute(self, test, lit_config):
        name = test.path_in_suite[-1]
        name_root, name_ext = os.path.splitext(name)
        is_libcxx_test = test.path_in_suite[0] == 'libcxx'
        is_sh_test = name_root.endswith('.sh')
        is_pass_test = name.endswith('.pass.cpp')
        is_fail_test = name.endswith('.fail.cpp')
        assert is_sh_test or name_ext == '.cpp', 'non-cpp file must be sh test'

        if test.config.unsupported:
            return (lit.Test.UNSUPPORTED,
                    "A lit.local.cfg marked this unsupported")
        self._add_header_requirements(test)

        script = lit.TestRunner.parseIntegratedTestScript(
            test, require_script=is_sh_test)
        # Check if a result for the test was returned. If so return that
        # result.
        if isinstance(script, lit.Test.Result):
            return script
        if lit_config.noExecute:
            return lit.Test.Result(lit.Test.PASS)

        # Check that we don't have run lines on tests that don't support them.
        if not is_sh_test and len(script) != 0:
            lit_config.fatal('Unsupported RUN line found in test %s' % name)

        tmpDir, tmpBase = lit.TestRunner.getTempPaths(test)
        substitutions = lit.TestRunner.getDefaultSubstitutions(test, tmpDir,
                                                               tmpBase)
        script = lit.TestRunner.applySubstitutions(script, substitutions)
        test_cxx = CXXCompiler(None)
        # Dispatch the test based on its suffix.
        if is_sh_test:
            return (lit.Test.UNSUPPORTED, 'ShTest format not yet supported')
        elif is_fail_test:
            return self._evaluate_fail_test(test, test_cxx)
        elif is_pass_test:
            return self._evaluate_pass_test(test, tmpBase, test_cxx)
        else:
            # No other test type is supported
            assert False

    def _clean(self, exec_path):  # pylint: disable=no-self-use
        libcxx.util.cleanFile(exec_path)

    def _evaluate_pass_test(self, test, tmpBase, test_cxx):
        execDir = os.path.dirname(test.getExecPath())
        source_path = test.getSourcePath()
        exec_path = tmpBase + '.exe'
        object_path = tmpBase + '.obj'
        # Create the output directory if it does not already exist.
        lit.util.mkdir_p(os.path.dirname(tmpBase))
        try:
            # Compile the test
            cmd, out, err, rc = test_cxx.compileLinkTwoSteps(
                source_path, out=exec_path, object_file=object_path,
                cwd=execDir)
            compile_cmd = cmd
            if rc != 0:
                report = libcxx.util.makeReport(cmd, out, err, rc)
                report += "Compilation failed unexpectedly!"
                return (lit.Test.FAIL, report)
            return (lit.Test.PASS, '')
            # TODO run the test
        finally:
            # Note that cleanup of exec_file happens in `_clean()`. If you
            # override this, cleanup is your reponsibility.
            libcxx.util.cleanFile(object_path)
            self._clean(exec_path)

    def _evaluate_fail_test(self, test, test_cxx):
        source_path = test.getSourcePath()
        # FIXME: lift this detection into LLVM/LIT.
        with open(source_path, 'r') as f:
            contents = f.read()
        cmd, out, err, rc = test_cxx.compile(source_path, out=os.devnull)
        expected_rc = 2
        if rc == expected_rc:
            return (lit.Test.PASS, '')
        else:
            report = libcxx.util.makeReport(cmd, out, err, rc)
            report_msg = 'Expected compilation to fail!'
            return (lit.Test.FAIL, report + report_msg + '\n')
