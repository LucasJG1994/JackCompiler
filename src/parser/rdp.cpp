#include "rdp.h"
#include "scanner.h"

#include <iostream>
#include <map>

extern "C" {
#include "jack.tab.h"
}

std::string class_name;
rdp_val current;
int rdp_line = 0;
int if_count = 0;
int while_count = 0;

static void block();
static void bin_expr();

static void adv() {
	current.token = yylex();
}

static bool match(int type) {
	return type == current.token;
}

static void consume(int type) {
	if(type == current.token) adv();
	else{ 
		std::cout << "Error on line " << rdp_line << std::endl;
		throw(0);
	}
}

static void var_decl() {
	int type;
	switch (current.token) {
		case TK_VAR: type = TYPE_VAR; break;
		case TK_FIELD: type = TYPE_FIELD; break;
		case TK_STATIC: type = TYPE_STATIC; break;
	}

	adv();
	consume(TK_ID);
	std::string name = current.sval;
	consume(TK_ID);

	scope_var_define(name.c_str(), type);

	while (match(TK_COMMA)) {
		adv();
		name = current.sval;
		consume(TK_ID);
		scope_var_define(name.c_str(), type);
	}

	consume(TK_SEMI);
}

static void factor() {
	std::cout << "factor\n";
	switch (current.token) {
		case TK_NUM: fw_write("push constant %d", current.ival); adv(); break;
		case TK_THIS: fw_write("push pointer 0"); adv(); break;
		case TK_TRUE:
			fw_write("push constant 0");
			fw_write("not");
			adv();
			break;
		case TK_FALSE: fw_write("push constant 0"); adv(); break;
		case TK_LP:
			adv();
			bin_expr();
			consume(TK_RP);
			break;
		case TK_ID:
			std::string name = current.sval;
			int args = 0;
			adv();

			if (match(TK_LB)) {
				adv();
				bin_expr();
				consume(TK_RB);
				break;
			}

			if (match(TK_LP)) {
				adv();

				if (match(TK_RP) == 0) {
					bin_expr();
					args++;

					while (match(TK_COMMA)) {
						adv();
						bin_expr();
						args++;
					}
				}

				consume(TK_RP);

				fw_write("call %s %d", name.c_str(), args);
				break;
			}

			if (match(TK_DOT)) {
				adv();
				std::string fname = current.sval;
				std::string arg_name;
				args = 1;
				consume(TK_ID);
				consume(TK_LP);

				switch (scope_var_resolve_type(name.c_str())) {
				case TYPE_VAR: fw_write("push local %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_STATIC: fw_write("push static %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_FIELD: fw_write("push this %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_ARG: fw_write("push argument %d", scope_var_resolve_index(name.c_str())); break;
				case -1: args = 0;
				}

				if(match(TK_RP) == 0){

					bin_expr();
					args++;

					while (match(TK_COMMA)) {
						adv();
						bin_expr();
						args++;
					}

				}

				consume(TK_RP);

				fw_write("call %s.%s %d", name.c_str(), fname.c_str(), args);
				break;
			}

			switch (scope_var_resolve_type(name.c_str())) {
				case TYPE_VAR: fw_write("push local %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_STATIC: fw_write("push static %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_FIELD: fw_write("push this %d", scope_var_resolve_index(name.c_str())); break;
				case TYPE_ARG: fw_write("push argument %d", scope_var_resolve_index(name.c_str())); break;
			}
			break;
	}
}

static void term() {
	std::cout << "term\n";
	factor();

	switch (current.token) {
		case TK_MULTIPLY: adv(); term(); fw_write("call Math.Multiply 2"); break;
		case TK_DIVIDE:   adv(); term(); fw_write("call Math.Divide 2"); break;
	}
}

static void expr() {
	std::cout << "expr\n";
	term();

	switch (current.token) {
		case TK_PLUS:  adv(); expr(); fw_write("add"); break;
		case TK_MINUS: adv(); expr(); fw_write("sub"); break;
	}
}

static void rel_expr() {
	std::cout << "rel_expr\n";
	expr();

	switch (current.token) {
		case TK_LT:     adv(); rel_expr(); fw_write("lt"); break;
		case TK_GT:     adv(); rel_expr(); fw_write("gt"); break;
		case TK_ASSIGN: adv(); rel_expr(); fw_write("eq"); break;
	}
}

static void bin_not() {
	std::cout << "bin_not\n";
	if (match(TK_NOT)) {
		adv();
		rel_expr();
		fw_write("not");
		return;
	}

	rel_expr();
}

static void bin_expr() {
	std::cout << "bin_expr\n";
	bin_not();

	switch (current.token) {
		case TK_AND: adv(); bin_expr(); fw_write("and"); break;
		case TK_OR:  adv(); bin_expr(); fw_write("or"); break;
	}
}

static void if_stmt() {
	std::cout << "if_stmt\n";
	int cur = if_count++;
	adv();
	consume(TK_LP);
	bin_expr();
	consume(TK_RP);
	consume(TK_LC);

	fw_write("if-goto ELSE%d", cur);

	block();
	consume(TK_RC);

	fw_write("goto ENDIF%d", cur);
	fw_write("label ELSE%d", cur);

	if (match(TK_ELSE)) {
		adv();
		consume(TK_LC);
		block();
		consume(TK_RC);
	}

	fw_write("label ENDIF%d", cur);
}

static void while_stmt() {
	std::cout << "while_stmt\n";
	int cur = while_count++;
	adv();
	consume(TK_LP);

	fw_write("label WHILE%d", cur);

	bin_expr();
	consume(TK_RP);
	consume(TK_LC);

	fw_write("if-goto WHILE_END%d", cur);

	block();
	consume(TK_RC);

	fw_write("goto WHILE%d", cur);
	fw_write("label WHILE_END%d", cur);
}

static void let_stmt() {
	std::cout << "let_stmt\n";
	adv();
	std::string name = current.sval;
	consume(TK_ID);

	if (match(TK_LB)) {
		adv();
		bin_expr();
		consume(TK_RB);
	}

	consume(TK_ASSIGN);
	bin_expr();

	consume(TK_SEMI);

	switch (scope_var_resolve_type(name.c_str())) {
	case TYPE_VAR: fw_write("pop local %d", scope_var_resolve_index(name.c_str())); break;
	case TYPE_STATIC: fw_write("pop static %d", scope_var_resolve_index(name.c_str())); break;
	case TYPE_FIELD: fw_write("pop this %d", scope_var_resolve_index(name.c_str())); break;
	case TYPE_ARG: fw_write("pop argument %d", scope_var_resolve_index(name.c_str())); break;
	}
}

static void do_stmt() {
	std::cout << "do_stmt\n";
	adv();
	std::string cname = current.sval;
	int args = 0;
	consume(TK_ID);
	if(match(TK_DOT)){
		consume(TK_DOT);
		std::string name = current.sval;
		std::string arg_name;
		args = 1;
		consume(TK_ID);
		consume(TK_LP);

		switch (scope_var_resolve_type(cname.c_str())) {
		case TYPE_VAR: fw_write("push local %d", scope_var_resolve_index(cname.c_str())); break;
		case TYPE_STATIC: fw_write("push static %d", scope_var_resolve_index(cname.c_str())); break;
		case TYPE_FIELD: fw_write("push this %d", scope_var_resolve_index(cname.c_str())); break;
		case TYPE_ARG: fw_write("push argument %d", scope_var_resolve_index(cname.c_str())); break;
		case -1: args = 0;
		}

		if(match(TK_RP) == 0){

			bin_expr();
			args++;

			while (match(TK_COMMA)) {
				adv();
				bin_expr();
				args++;
			}

		}

		consume(TK_RP);
		consume(TK_SEMI);

		fw_write("call %s.%s %d", cname.c_str(), name.c_str(), args);
		return;
	}

	consume(TK_LP);

	if (match(TK_RP) == 0) {
		bin_expr();
		args++;

		while (match(TK_COMMA)) {
			adv();
			bin_expr();
			args++;
		}
	}

	consume(TK_RP);
	consume(TK_SEMI);

	fw_write("call %s %d", cname.c_str(), args);
}

static void ret_stmt() {
	adv();

	if (match(TK_SEMI)) {
		adv();
		fw_write("return");
		return;
	}

	bin_expr();
	consume(TK_SEMI);
	fw_write("return");
}

static void block() {
	std::cout << "block\n";
	std::cout << current.token << std::endl;
	while(1)
	switch (current.token) {
		case TK_IF: if_stmt(); break;
		case TK_WHILE: while_stmt(); break;
		case TK_LET: let_stmt(); break;
		case TK_DO: do_stmt(); break;
		case TK_RETURN: ret_stmt(); break;
		case TK_VAR:
		case TK_STATIC:
		case TK_FIELD: var_decl(); break;
		default: return;
	}
}

static void constructor() {
	std::cout << "constructor_def\n";
	adv();

	if (match(TK_ID)) {
		if(class_name != current.sval) return;
		adv();

		std::string name = current.sval;
		int args = 0;

		consume(TK_ID);
		consume(TK_LP);

		if(match(TK_ID)){
			consume(TK_ID);

			scope_push(name.c_str());
			scope_var_define(current.sval.c_str(), TYPE_ARG);
			consume(TK_ID);
			args++;

			while (match(TK_COMMA)) {
				adv();
				consume(TK_ID);
				scope_var_define(current.sval.c_str(), TYPE_ARG);
				consume(TK_ID);
				args++;
			}

		}

		fw_write("function %s.%s %d", class_name.c_str(), name.c_str(), args);
		fw_write("push constant %d", args);
		fw_write("call Memory.alloc 1");
		fw_write("pop pointer 0");

		consume(TK_RP);
		consume(TK_LC);

		block();

		scope_pop();
		consume(TK_RC);
	}
}

static void functions() {
	std::cout << "function_def\n";
	adv();
	consume(TK_ID);
	std::string name = current.sval;
	int args = 0; 

	consume(TK_ID);
	consume(TK_LP);

	if(match(TK_ID)){
		consume(TK_ID);

		scope_push(name.c_str());
		scope_var_define(current.sval.c_str(), TYPE_ARG);
		consume(TK_ID);
		args++;

		while (match(TK_COMMA)) {
			adv();
			consume(TK_ID);
			scope_var_define(current.sval.c_str(), TYPE_ARG);
			consume(TK_ID);
			args++;
		}

	}

	fw_write("function %s.%s %d", class_name.c_str(), name.c_str(), args);
	fw_write("push argument 0");
	fw_write("pop pointer 0");

	consume(TK_RP);
	consume(TK_LC);

	block();

	scope_pop();
	consume(TK_RC);
}

static void clas_block() {
	std::cout << "class_block\n";
	while(match(TK_VAR) || match(TK_FIELD) || match(TK_STATIC)) var_decl();

	if(match(TK_CONSTRUCTOR)) constructor();
	while(match(TK_METHOD) || match(TK_FUNCTION)) functions();
}

static void clas() {
	std::cout << "class\n";
	consume(TK_CLASS);
	scope_new_class(current.sval.c_str());
	class_name = current.sval;
	consume(TK_ID);
	consume(TK_LC);
	clas_block();
	consume(TK_RC);
}

static void start() {
	std::cout << "start\n";
	clas();
}

void rdp_parse() {
	adv();
	try{
		start();
	}
	catch (int n) {

	}
}