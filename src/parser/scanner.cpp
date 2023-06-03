#include "scanner.h"

#include <iostream>
#include <map>
#include <string>

extern "C" {
#include "jack.tab.h"
#include "dir.h"
#include <stdio.h>
#include <stdlib.h>
extern int line;
}


static const char* src;
static const char* cur;
static const char* begin;
static std::map<std::string, int> keywords = {
	{ "class"      ,       TK_CLASS },
	{ "constructor", TK_CONSTRUCTOR },
	{ "method"     ,      TK_METHOD },
	{ "function"   ,    TK_FUNCTION },
	{ "int"        ,         TK_INT },
	{ "boolean"    ,     TK_BOOLEAN },
	{ "char"       ,        TK_CHAR },
	{ "void"       ,        TK_VOID },
	{ "var"        ,         TK_VAR },
	{ "static"     ,      TK_STATIC },
	{ "field"      ,       TK_FIELD },
	{ "let"        ,         TK_LET },
	{ "do"         ,          TK_DO },
	{ "if"         ,          TK_IF },
	{ "else"       ,        TK_ELSE },
	{ "while"      ,       TK_WHILE },
	{ "return"     ,      TK_RETURN },
	{ "true"       ,        TK_TRUE },
	{ "false"      ,       TK_FALSE },
	{ "null"       ,        TK_NULL },
	{ "this"       ,        TK_THIS }
};

static void adv() { cur++; }
static char peek() { return cur[1]; }
static bool match(char c) { return c == cur[0]; }
static bool end() { return cur[0] == 0; }

static std::string get_lex() {
	std::string self = std::string(begin, cur - begin);
	//self[cur - begin] = 0;
	return self;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
static bool is_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }

static bool is_number(std::string s) {
	if(s.length() == 0) return false;
	for (char c : s) {
		if(is_digit(c) == 0) return false;
	}

	return true;
}

static bool is_id(std::string s) {
	if(s.length() == 0) return false;
	if(is_alpha(s[0]) == 0) return false;
	return true;
}

extern "C" void yy_init(const char* path) {
	DIR_HANDLE d = DIR_OPEN(path);
	const char** files = DIR_GET_FILE_LIST(d);
	DIR_CLOSE(d);

	if(files == NULL) return;

	for (int i = 0; i < 0xFF; i++) {
		if(files[i] == NULL) continue;
		std::cout << files[i] << std::endl;
		std::string full_path = std::string(path);
		full_path += "/";
		full_path += std::string(files[i]);
		FILE* fp = fopen(full_path.c_str(), "rb");

		fseek(fp, 0L, SEEK_END);
		long len = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		char* buffer = new char[len + 1];
		if (buffer == nullptr) {
			fclose(fp);
			continue;
		}

		fread(buffer, sizeof(char), len, fp);
		fclose(fp);

		buffer[len] = 0;

		src = buffer;
		cur = buffer;
		begin = buffer;
		line = 1;

		yyparse();

		delete[] buffer;
	}

	for (int i = 0; i < 0xFF; i++) {
		if(files[i] == NULL) continue;
		free((void*)files[i]);
	}

	free(files);
}

extern "C" int yylex() {
	if(end()) return TK_EOF;

	while(end() == 0)
		switch (*cur) {
			case '+': { adv(); return TK_PLUS; }
			case '-': { adv(); return TK_MINUS; }
			case '*': { adv(); return TK_MULTIPLY; }
			case '&': { adv(); return TK_AND; }
			case '|': { adv(); return TK_OR; }
			case '~': { adv(); return TK_NOT; }
			case '<': { adv(); return TK_LT; }
			case '>': { adv(); return TK_GT; }
			case '(': { adv(); return TK_LP; }
			case ')': { adv(); return TK_RP; }
			case '[': { adv(); return TK_LB; }
			case ']': { adv(); return TK_RB; }
			case '{': { adv(); return TK_LC; }
			case '}': { adv(); return TK_RC; }
			case '.': { adv(); return TK_DOT; }
			case ',': { adv(); return TK_COMMA; }
			case '=': { adv(); return TK_ASSIGN; }
			case ';': { adv(); return TK_SEMI; }
			case '/': {
				adv();
				if (match('/')) {
					while (match('\n') == 0) {
						if (end()) break;
						adv();
					}
				}
				else if (match('*')) {
					adv();
					while (1) {
						if (end()) break;
						if (match('\n')) line++;
						if (match('*') && peek() == '/') {
							adv(); adv();
							break;
						}
						adv();
					}
				}
				else return TK_DIVIDE;
				break;
			}
			case ' ':
			case '\t': 
			case '\r': adv(); break;
			case '\n': line++; adv(); break;
			default: {
				begin = cur;
				while (is_alpha(*cur) || is_digit(*cur)) {
					if(end()) break;
					adv();
				}

				std::string lex = get_lex();

				if (is_number(lex)) {
					yylval.info.ival = std::atoi(lex.c_str());
					return TK_NUM;
				}

				if(keywords.find(lex) != keywords.end()) return keywords[lex];

				if (is_id(lex)) {
					char* buffer = (char*)calloc(lex.length() + 1, sizeof(char));
					if(buffer == NULL){
						std::cout << "Failed to allocate memory...\n";
						return TK_EOF;
					}

					memcpy(buffer, lex.c_str(), lex.length());
					yylval.info.sval = buffer;
					return TK_ID;
				}

				return TK_EOF;
			}
		}
	return TK_EOF;
}