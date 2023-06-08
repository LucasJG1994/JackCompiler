#ifndef __scanner__h
#define __scanner__h

#if __cplusplus
extern "C" {
#endif

void yy_init(const char* path);
int yylex();

////////////////////////////
// Symbol Table Interface //
////////////////////////////

#define TYPE_VAR    1
#define TYPE_FIELD  2
#define TYPE_STATIC 3
#define TYPE_ARG    4

void scope_new_class(const char* name);
void scope_push(const char* name);
void scope_pop();
void scope_var_define(const char* name, int type);
int  scope_var_resolve_index(const char* name);
int  scope_var_resolve_type(const char* name);
int  scope_var_resolve_depth(const char* name);

////////////////////////
// File Writer Module //
////////////////////////

void fw_init(const char* path);
void fw_write(const char* fmt, ...);
void fw_close();

#if __cplusplus
}
#endif

#endif