//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class istrstream

// strstreambuf* rdbuf() const;

#include <strstream>
#include <cassert>

int main()
{
    {
        const char buf[] = "123 4.5 dog";
        const std::istrstream in(buf);
        std::strstreambuf* sb = in.rdbuf();
        assert(sb->sgetc() == '1');
    }
}
