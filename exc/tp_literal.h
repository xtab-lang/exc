//////////////////////////////////////////////////////////////////////////////////////////////////
// author: exy.lang
//   date: 2020-12-18
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef TP_LITERAL_H_
#define TP_LITERAL_H_

namespace exy {
namespace typ_pass {
struct Literal {
    Typer &tp;

    Literal(Typer *tp) : tp(*tp) {}

    AstConstant* visit(SyntaxLiteral*);
};
} // namespace typ_pass
} // namespace exy

#endif // TP_LITERAL_H_