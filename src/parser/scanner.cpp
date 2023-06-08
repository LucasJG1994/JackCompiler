#include "scanner.h"
#include "rdp.h"

#include <iostream>
#include <map>
#include <string>

extern "C" {
#include "jack.tab.h"
#include "dir.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
extern int line;
}
extern rdp_val current;
extern int rdp_line;

static const char* src;
static const char* cur;
static const char* begin;
static std::map<std::string, int> keywords = {
	{ "class"      ,       TK_CLASS },
	{ "constructor", TK_CONSTRUCTOR },
	{ "method"     ,      TK_METHOD },
	{ "function"   ,    TK_FUNCTION },
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
		
		std::string full_path = std::string(path);
		std::string file = std::string(files[i]);
		std::string ext = ".jack";

		full_path += "/";
		std::string out_path = full_path + file.substr(0, file.length() - ext.length());
		out_path += ".vm";
		full_path += std::string(files[i]);

		if(ext != file.substr(file.length() - ext.length(), ext.length())) continue;

		std::cout << files[i] << std::endl;

		fw_init(out_path.c_str());
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
		rdp_line = 1;

		//yyparse();
		rdp_parse();

		delete[] buffer;

		fw_close();
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
			case '\n': rdp_line++; adv(); break;
			default: {
				begin = cur;
				while (is_alpha(*cur) || is_digit(*cur)) {
					if(end()) break;
					adv();
				}

				std::string lex = get_lex();

				if (is_number(lex)) {
					current.ival = std::atoi(lex.c_str());
					return TK_NUM;
				}

				if(keywords.find(lex) != keywords.end()) return keywords[lex];

				if (is_id(lex)) {
					current.sval = lex;
					return TK_ID;
				}

				return TK_EOF;
			}
		}
	return TK_EOF;
}

/////////////////////////
// Symbol Table Module //
/////////////////////////

struct symbol {
	int type;
	std::string name;
	int depth;
	int index;
};

struct scope {
	std::string name;
	std::map<std::string, scope*> sub_scope;
	std::map<std::string, symbol> vars;
	int depth;
	int index;
	scope* prev;
};

static std::map<std::string, scope*> classes = {
	{"int", nullptr},
	{"char", nullptr},
	{"boolean", nullptr},
	{"void", nullptr}
};
static scope* cur_scope;

extern "C" void scope_new_class(const char* name) {
	scope* new_scope = new scope;
	if(new_scope == nullptr) return;

	new_scope->name = std::string(name);
	new_scope->depth = 0;
	new_scope->index = 0;
	new_scope->prev = nullptr;

	classes[new_scope->name] = new_scope;
	cur_scope = new_scope;
}

extern "C" void scope_push(const char* name) {
	scope* new_scope = new scope;
	if(new_scope == nullptr) return;

	new_scope->name = std::string(name);
	new_scope->depth = cur_scope->depth + 1;
	new_scope->index = 0;
	new_scope->prev = cur_scope;
	cur_scope->sub_scope[new_scope->name] = new_scope;
	cur_scope = new_scope;
}

extern "C" void scope_pop() {
	if(cur_scope->prev == nullptr) return;
	cur_scope = cur_scope->prev;
}

extern "C" void scope_var_define(const char* name, int type) {
	std::string n = std::string(name);
	cur_scope->vars[n] = {type, n, cur_scope->depth, cur_scope->index++};
}

extern "C" int scope_var_resolve_index(const char* name) {
	std::string n = std::string(name);
	scope* cur = cur_scope;

	while (cur != nullptr) {
		if (cur->vars.find(n) != cur->vars.end()) return cur->vars[n].index;
		else cur = cur->prev;
	}

	return -1;
}

extern "C" int scope_var_resolve_type(const char* name) {
	std::string n = std::string(name);
	scope* cur = cur_scope;

	while (cur != nullptr) {
		if (cur->vars.find(n) != cur->vars.end()) return cur->vars[n].type;
		else cur = cur->prev;
	}

	return -1;
}

extern "C" int scope_var_resolve_depth(const char* name) {
	std::string n = std::string(name);
	scope* cur = cur_scope;

	while (cur != nullptr) {
		if(cur->vars.find(n) != cur->vars.end()) return cur->vars[n].depth;
		else cur = cur->prev;
	}

	return -1;
}

////////////////////////
// File Writer Module //
////////////////////////

static FILE* fp;

extern "C" void fw_init(const char* path) {
	if(fopen_s(&fp, path, "wb")!= 0){
		std::cout << "Failed to open file...\n";
		fp = nullptr;
		return;
	}
}

extern "C" void fw_write(const char* fmt, ...) {
	if(fp == nullptr) return;

	va_list arg;
	va_start(arg, fmt);
	long len = vsnprintf(NULL, 0, fmt, arg);
	char* buffer = new char[len + 1];
	if (buffer == NULL) {
		va_end(arg);
		return;
	}

	vsprintf(buffer, fmt, arg);
	va_end(arg);

	std::string tmp = std::string(buffer);
	tmp += "\n";

	delete[] buffer;
	fwrite(tmp.c_str(), sizeof(char), tmp.length(), fp);
}

extern "C" void fw_close() {
	if(fp == nullptr) return;
	fclose(fp);
}