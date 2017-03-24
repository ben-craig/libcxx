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
import lit.util
import libcxx.util


class CXXCompiler(object):
    CM_Default = 0
    CM_PreProcess = 1
    CM_Compile = 2
    CM_Link = 3

    def __init__(self, path, flags=None, compile_flags=None, link_flags=None,
                 warning_flags=None, compile_env=None):
        # TODO: HACK populate these
        #self.path = path
        self.path = r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\x86_amd64\cl.exe'
        self.flags = list(flags or [])
        #self.compile_flags = list(compile_flags or [])
        self.compile_flags = [
            r'/IC:\src\libcxx\include',
            r'/IC:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\km\crt',
            r'/IC:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\km',
            r'/IC:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\shared',
            r'/IC:\Program Files (x86)\Windows Kits\10\Include\wdf\kmdf\1.15',
            #"/Zi",
            "/nologo",
            "/Od",
            "/Oi",
            "/Oy-",
            "/D _WIN64",
            "/D _AMD64_",
            "/D AMD64",
            "/D _WIN64",
            "/D _AMD64_",
            "/D AMD64",
            "/D DEPRECATE_DDK_FUNCTIONS=1",
            "/D MSC_NOOPT",
            "/D _WIN32_WINNT=0x0A00",
            "/D WINVER=0x0A00",
            "/D WINNT=1",
            "/D NTDDI_VERSION=0x0A000002",
            "/D DBG=1",
            "/D KMDF_VERSION_MAJOR=1",
            "/D KMDF_VERSION_MINOR=15",
            "/GF",
            "/Gm-",
            "/Zp8",
            "/GS",
            "/Gy",
            "/fp:precise",
            #"/Zc:wchar_t-",
            "/Zc:forScope",
            "/Zc:inline",
            "/GR-",
            #"/Fo\"..\x64\Debug\\\"",
            #"/Fd\"..\x64\Debug\vc140.pdb\"",
            "/Gz",
            #"/FI\"C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\shared\warning.h\"",
            "/errorReport:none",
            "/kernel",
            "-cbstring",
            "-d2epilogunwind",
            #"/d1import_no_registry",
            #"/d2AllowCompatibleILVersions",
            #"/d2Zi+"
        ]
        #self.link_flags = list(link_flags or [])
        self.link_flags = list(link_flags or [])
        #self.warning_flags = list(warning_flags or [])
        self.warning_flags = [
            "/W4", 
            "/WX",
            r'/wd4748', 
            r'/wd4603', 
            r'/wd4627', 
            r'/wd4986', 
            r'/wd4987', 
            r'/wd4996', 
        ]
        #if compile_env is not None:
        #    self.compile_env = dict(compile_env)
        #else:
        #    self.compile_env = None
        self.compile_env = os.environ.copy()
        self.compile_env["PATH"] = r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin;' + self.compile_env["PATH"]

    def copy(self):
        new_cxx = CXXCompiler(
            self.path, flags=self.flags, compile_flags=self.compile_flags,
            link_flags=self.link_flags, warning_flags=self.warning_flags,
            compile_env=self.compile_env)
        return new_cxx

    def _basicCmd(self, source_files, out, mode=CM_Default, flags=[],
                  input_is_cxx=False):
        cmd = [self.path]
        if out is not None:
            if mode == self.CM_PreProcess:
                cmd += ['/Fi' + os.path.dirname(out)]
            elif mode == self.CM_Compile:
                cmd += ['/Fo' + os.path.dirname(out)]
            elif mode == self.CM_Link:
                cmd += ["/OUT:\"" + os.path.dirname(out) + "\""]
        if input_is_cxx:
            cmd += ['/TP']
        if isinstance(source_files, list):
            cmd += source_files
        elif isinstance(source_files, str):
            cmd += [source_files]
        else:
            raise TypeError('source_files must be a string or list')
        if mode == self.CM_PreProcess:
            cmd += ['/P']
        elif mode == self.CM_Compile:
            cmd += ['/c']
        cmd += self.flags
        if mode != self.CM_Link:
            cmd += self.compile_flags
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
        out, err, rc = lit.util.executeCommand(cmd, env=self.compile_env,
                                               cwd=cwd)
        return cmd, out, err, rc

    def compile(self, source_files, out=None, flags=[], cwd=None):
        cmd = self.compileCmd(source_files, out, flags)
        out, err, rc = lit.util.executeCommand(cmd, env=self.compile_env,
                                               cwd=cwd)
        return cmd, out, err, rc

    def link(self, source_files, out=None, flags=[], cwd=None):
        cmd = self.linkCmd(source_files, out, flags)
        out, err, rc = lit.util.executeCommand(cmd, env=self.compile_env,
                                               cwd=cwd)
        return cmd, out, err, rc

    def compileLink(self, source_files, out=None, flags=[],
                    cwd=None):
        cmd = self.compileLinkCmd(source_files, out, flags)
        out, err, rc = lit.util.executeCommand(cmd, env=self.compile_env,
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
            #if rc != 0:
            return cc_cmd, cc_stdout, cc_stderr, rc

            link_cmd, link_stdout, link_stderr, rc = self.link(
                object_file, out=out, flags=flags, cwd=cwd)
            return (cc_cmd + ['&&'] + link_cmd, cc_stdout + link_stdout,
                    cc_stderr + link_stderr, rc)
