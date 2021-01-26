//////////////////////////////////////////////////////////////////////////////////////////////////
// author: exy.lang
//   date: 2020-12-09
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "src2char_stream.h"

#include "source.h"

namespace exy {
namespace src2tok_pass {
CharStream::CharStream(SourceFile &file) : pos(file.source.text), end(file.source.end()) {}

SourceChar CharStream::next() {
    Assert(pos <= end);
    SourceChar ch{ pos, line, col };
    if (pos < end) {
        if (*pos == '\n') {
            ++pos;
            ++line;
            col = 1;
        } else {
            ++pos;
            ++col;
        }
    }
    return ch;
}

bool CharStream::isEOF(const SourceChar &ch) {
    return ch.pos == end;
}
} // namespace src2tok_pass
} // namespace exy