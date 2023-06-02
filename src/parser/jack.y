{%

	#include "jack.tab.h"

	#define YYERROR_VERBOSE

	int line = 0;

	int yyerror(const char* msg){
		printf("%s on line %d\n", msg, line);
		return 0;
	}

}%

%code requires{
	#include "scanner.h"
}

%token 