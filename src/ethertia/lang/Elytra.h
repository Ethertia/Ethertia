//
// Created by Dreamtowards on 2022/5/5.
//

#ifndef ETHERTIA_ELYTRA_H
#define ETHERTIA_ELYTRA_H

#include <ethertia/lang/parser/Parser.h>
#include <ethertia/client/Loader.h>

void et() {

    Lexer lx;
    lx.src = Loader::loadAssetsStr("elytra/main.et");
    auto a = Parser::parseCompilationUnit(&lx);

}

// AstModifiable Modifiers, Annotation, Template

// Keyword Removement.

#endif //ETHERTIA_ELYTRA_H