#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
	if(nTokens==MAX_TOKENS)err("too many tokens");
	Token *tk=&tokens[nTokens];
	tk->code=code;
	tk->line=line;
	nTokens++;
	return tk;
	}

// copy in the dst buffer the string between [begin,end)
char *copyn(char *dst,const char *begin,const char *end){
	char *p=dst;
	if(end-begin>MAX_STR)err("string too long");
	while(begin!=end)*p++=*begin++;
	*p='\0';
	return dst;
	}

void tokenize(const char *pch){
	const char *start;
	Token *tk;
	char buf[MAX_STR+1];
	nTokens=0;
	line=1;
	for(;;){
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++;
			case '\n':
				line++;
				pch++;
				break;
			case '\0':addTk(FINISH);return;
			case ',':addTk(COMMA);pch++;break;
			case ':':addTk(COLON);pch++;break;
			case ';':addTk(SEMICOLON);pch++;break;
			case '(':addTk(LPAR);pch++;break;
			case ')':addTk(RPAR);pch++;break;
			case '+':addTk(ADD);pch++;break;
			case '-':addTk(SUB);pch++;break;
			case '*':addTk(MUL);pch++;break;
			case '<':addTk(LESS);pch++;break;
			case '/':
				if(pch[1]=='/'){
					for(pch+=2;*pch!='\n'&&*pch!='\r'&&*pch!='\0';pch++){}
					}else{
					addTk(DIV);
					pch++;
					}
				break;
			case '&':
				if(pch[1]=='&'){
					addTk(AND);
					pch+=2;
					}
				else err("line %d: invalid char after &: %c (%d)",line,pch[1],pch[1]);
				break;
			case '|':
				if(pch[1]=='|'){
					addTk(OR);
					pch+=2;
					}
				else err("line %d: invalid char after |: %c (%d)",line,pch[1],pch[1]);
				break;
			case '!':
				if(pch[1]=='='){
					addTk(NOTEQ);
					pch+=2;
					}else{
					addTk(NOT);
					pch++;
					}
				break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;
			case '>':
				if(pch[1]=='='){
					addTk(GREATEREQ);
					pch+=2;
					}else{
					addTk(GREATER);
					pch++;
					}
				break;
			case '"':
				tk=addTk(STR);
				for(start=++pch;*pch!='"';pch++){
					if(*pch=='\0')err("line %d: missing closing \" for string",tk->line);
					if(*pch=='\n'||(*pch=='\r'&&pch[1]!='\n'))line++;
					}
				copyn(tk->text,start,pch);
				pch++;
				break;
			default:
				if(isalpha((unsigned char)*pch)||*pch=='_'){
					for(start=pch++;isalnum((unsigned char)*pch)||*pch=='_';pch++){}
					char *text=copyn(buf,start,pch);
					if(strcmp(text,"var")==0)addTk(VAR);
					else if(strcmp(text,"function")==0)addTk(FUNCTION);
					else if(strcmp(text,"if")==0)addTk(IF);
					else if(strcmp(text,"else")==0)addTk(ELSE);
					else if(strcmp(text,"while")==0)addTk(WHILE);
					else if(strcmp(text,"end")==0)addTk(END);
					else if(strcmp(text,"return")==0)addTk(RETURN);
					else if(strcmp(text,"int")==0)addTk(TYPE_INT);
					else if(strcmp(text,"real")==0)addTk(TYPE_REAL);
					else if(strcmp(text,"str")==0)addTk(TYPE_STR);
					else{
						tk=addTk(ID);
						strcpy(tk->text,text);
						}
					}
				else if(isdigit((unsigned char)*pch)){
					for(start=pch++;isdigit((unsigned char)*pch);pch++){}
					if(*pch=='.'&&isdigit((unsigned char)pch[1])){
						for(pch++;isdigit((unsigned char)*pch);pch++){}
						tk=addTk(REAL);
						tk->r=atof(copyn(buf,start,pch));
						}else{
						tk=addTk(INT);
						tk->i=atoi(copyn(buf,start,pch));
						}
					}
				else err("line %d: invalid char: %c (%d)",line,*pch,*pch);
			}
		}
	}

static const char *tkNames[]={
	"ID"
	,"VAR","FUNCTION","IF","ELSE","WHILE","END","RETURN"
	,"TYPE_INT","TYPE_REAL","TYPE_STR"
	,"INT","REAL","STR"
	,"COMMA","COLON","SEMICOLON","LPAR","RPAR","FINISH"
	,"ADD","SUB","MUL","DIV","AND","OR","NOT"
	,"ASSIGN","EQUAL","NOTEQ","LESS","GREATER","GREATEREQ"
	};

void showTokens(){
	for(int i=0;i<nTokens;i++){
		Token *tk=&tokens[i];
		printf("%d %s",tk->line,tkNames[tk->code]);
		switch(tk->code){
			case ID:case STR:printf(":%s",tk->text);break;
			case INT:printf(":%d",tk->i);break;
			case REAL:printf(":%g",tk->r);break;
			}
		printf("\n");
		}
	}
