#ifndef __rdp__h
#define __rdp__h

#include <string>

struct rdp_val {
	int token;
	int ival;
	std::string sval;
	int type;
};

void rdp_parse();

#endif