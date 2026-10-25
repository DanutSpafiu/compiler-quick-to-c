#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../LFTC_ALEX/lexer.h"
#include "parser.h"

static int iTk;
static Token *consumed;
static bool defVar(void), defFunc(void), block(void), instr(void), expr(void);

static _Noreturn void tkerr(const char *fmt,...){
    fprintf(stderr,"error in line %d: ",tokens[iTk].line);
    va_list va;
    va_start(va,fmt);
    vfprintf(stderr,fmt,va);
    va_end(va);
    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}

static bool consume(int code){
    if(iTk<nTokens && tokens[iTk].code==code){
        consumed=&tokens[iTk++];
        return true;
    }
    return false;
}

static bool baseType(void){
    return consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR);
}

static bool defVar(void){
    if(!consume(VAR))return false;
    if(!consume(ID))tkerr("lipseste numele variabilei dupa var");
    if(!consume(COLON))tkerr("lipseste : dupa numele variabilei");
    if(!baseType())tkerr("tip invalid: int, real sau str");
    if(!consume(SEMICOLON))tkerr("lipseste ; dupa declaratia variabilei");
    return true;
}

static bool funcParam(void){
    if(!consume(ID))return false;
    if(!consume(COLON))tkerr("lipseste : dupa numele parametrului");
    if(!baseType())tkerr("tip invalid pentru parametru: int, real sau str");
    return true;
}

static bool funcParams(void){
    if(!funcParam())return false;
    while(consume(COMMA)){
        if(!funcParam())tkerr("lipseste parametrul dupa virgula");
    }
    return true;
}

static bool defFunc(void){
    if(!consume(FUNCTION))return false;
    if(!consume(ID))tkerr("lipseste numele functiei dupa function");
    if(!consume(LPAR))tkerr("lipseste ( dupa numele functiei");
    funcParams();
    if(!consume(RPAR))tkerr("parametri invalizi sau lipseste )");
    if(!consume(COLON))tkerr("lipseste : inaintea tipului returnat");
    if(!baseType())tkerr("tip returnat invalid: int, real sau str");
    while(defVar()){}
    if(!block())tkerr("lipseste corpul functiei: cel putin o instructiune");
    if(!consume(END))tkerr("instructiune invalida sau lipseste end pentru functie");
    return true;
}

static bool factor(void){
    if(consume(INT) || consume(REAL) || consume(STR))return true;
    if(consume(LPAR)){
        if(!expr())tkerr("lipseste expresia dupa (");
        if(!consume(RPAR))tkerr("expresie invalida sau lipseste )");
        return true;
    }
    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                while(consume(COMMA)){
                    if(!expr())tkerr("lipseste argumentul dupa virgula");
                }
            }
            if(!consume(RPAR))tkerr("argumente invalide sau lipseste ) la apel");
        }
        return true;
    }
    return false;
}

static bool exprPrefix(void){
    if(consume(SUB) || consume(NOT)){
        if(!factor())tkerr("lipseste operandul dupa operatorul unar - sau !");
        return true;
    }
    return factor();
}

static bool exprMul(void){
    if(!exprPrefix())return false;
    while(consume(MUL) || consume(DIV)){
        if(!exprPrefix())tkerr("lipseste operandul dupa * sau /");
    }
    return true;
}

static bool exprAdd(void){
    if(!exprMul())return false;
    while(consume(ADD) || consume(SUB)){
        if(!exprMul())tkerr("lipseste operandul dupa + sau -");
    }
    return true;
}

static bool exprComp(void){
    if(!exprAdd())return false;
    if(consume(LESS) || consume(EQUAL)){
        if(!exprAdd())tkerr("lipseste operandul dupa < sau ==");
    }
    return true;
}

static bool exprAssign(void){
    int start=iTk;
    Token *previous=consumed;
    if(consume(ID) && consume(ASSIGN)){
        if(!exprComp())tkerr("lipseste expresia dupa =");
        return true;
    }
    iTk=start;
    consumed=previous;
    return exprComp();
}

static bool exprLogic(void){
    if(!exprAssign())return false;
    while(consume(AND) || consume(OR)){
        if(!exprAssign())tkerr("lipseste operandul dupa && sau ||");
    }
    return true;
}

static bool expr(void){ return exprLogic(); }

static bool instr(void){
    if(consume(IF)){
        if(!consume(LPAR))tkerr("lipseste ( dupa if");
        if(!expr())tkerr("lipseste conditia pentru if");
        if(!consume(RPAR))tkerr("conditie invalida pentru if sau lipseste )");
        if(!block())tkerr("lipseste cel putin o instructiune dupa if");
        if(consume(ELSE)){
            if(!block())tkerr("lipseste cel putin o instructiune dupa else");
        }
        if(!consume(END))tkerr("instructiune invalida sau lipseste end pentru if");
        return true;
    }
    if(consume(RETURN)){
        if(!expr())tkerr("lipseste expresia dupa return");
        if(!consume(SEMICOLON))tkerr("expresie invalida sau lipseste ; dupa return");
        return true;
    }
    if(consume(WHILE)){
        if(!consume(LPAR))tkerr("lipseste ( dupa while");
        if(!expr())tkerr("lipseste conditia pentru while");
        if(!consume(RPAR))tkerr("conditie invalida pentru while sau lipseste )");
        if(!block())tkerr("lipseste cel putin o instructiune dupa while");
        if(!consume(END))tkerr("instructiune invalida sau lipseste end pentru while");
        return true;
    }
    if(expr()){
        if(!consume(SEMICOLON))tkerr("expresie invalida sau lipseste ; dupa expresie");
        return true;
    }
    return consume(SEMICOLON);
}

static bool block(void){
    if(!instr())return false;
    while(instr()){}
    return true;
}

static bool program(void){
    for(;;){
        if(defVar()){}
        else if(defFunc()){}
        else if(block()){}
        else break;
    }
    if(!consume(FINISH))tkerr("eroare de sintaxa: se asteapta o declaratie sau o instructiune");
    return true;
}

void parse(void){
    if(nTokens<=0 || tokens[nTokens-1].code!=FINISH){
        fprintf(stderr,"error: parse necesita tokenurile produse de tokenize\n");
        exit(EXIT_FAILURE);
    }
    iTk=0;
    consumed=NULL;
    program();
}
