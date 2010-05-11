//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class ostrstream

// char* str();

#include <strstream>
#include <cassert>

int main()
{
    {
        std::ostrstream out;
        out << 123 << ' ' << 4.5 << ' ' << "dog";
        assert(out.str() == std::string("123 4.5 dog"));
    }
}
