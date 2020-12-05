//////////////////////////////////////////////////////////////////////////////////////////////////
// author: exy.lang
//   date: 2020-12-05
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef PARSER_H
#define PARSER_H

namespace exy {
//--Begin forward declarations
struct SyntaxFile;
//----End forward declarations

namespace syn_pass {
void parse(SyntaxFile &file);
} // namespace syn_pass
} // namespace exy

#endif // SYNTAX_H