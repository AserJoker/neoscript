#ifndef __NEO_VM_AST__
#define __NEO_VM_AST__
#include "astdef.h"
#include "common/include/cstring.h"
#include "token.h"
#include <stdint.h>

typedef struct _neo_ast *neo_ast;
typedef struct _neo_ast neo_ast_meta;
typedef struct _neo_ast_identifier *neo_ast_identifier;
typedef struct _neo_ast_private_name *neo_ast_private_name;
typedef struct _neo_ast_regex_literal *neo_ast_regex_literal;
typedef struct _neo_ast_null_literal *neo_ast_null_literal;
typedef struct _neo_ast_string_literal *neo_ast_string_literal;
typedef struct _neo_ast_boolean_literal *neo_ast_boolean_literal;
typedef struct _neo_ast_number_literal *neo_ast_number_literal;
typedef struct _neo_ast_bigint_literal *neo_ast_bigint_literal;
typedef struct _neo_ast_decimal_literal *neo_ast_decimal_literal;
typedef struct _neo_ast_program *neo_ast_program;
typedef struct _neo_ast_function *neo_ast_function;
typedef struct _neo_ast_block_statement *neo_ast_block_statement;
typedef struct _neo_ast_empty_statement *neo_ast_empty_statement;
typedef struct _neo_ast_debugger_statement *neo_ast_debugger_statement;
typedef struct _neo_ast_with_statement *neo_ast_with_statement;
typedef struct _neo_ast_return_statement *neo_ast_return_statement;
typedef struct _neo_ast_labeled_statement *neo_ast_labeled_statement;
typedef struct _neo_ast_break_statement *neo_ast_break_statement;
typedef struct _neo_ast_continue_statement *neo_ast_continue_statement;
typedef struct _neo_ast_if_statement neo_ast_if_statement;
typedef struct _neo_ast_switch_statement *neo_ast_switch_statement;
typedef struct _neo_ast_switch_case_statement *neo_ast_switch_case_statement;
typedef struct _neo_ast_throw_statement *neo_ast_throw_statement;
typedef struct _neo_ast_try_statement *neo_ast_try_statement;
typedef struct _neo_ast_catch_clause *neo_ast_catch_clause;
typedef struct _neo_ast_while_statement *neo_ast_while_statement;
typedef struct _neo_ast_do_while_statement *neo_ast_do_while_statement;
typedef struct _neo_ast_for_statement *neo_ast_for_statement;
typedef struct _neo_ast_for_in_statement *neo_ast_for_in_statement;
typedef struct _neo_ast_for_of_statement *neo_ast_for_of_statement;
typedef struct _neo_ast_function *neo_ast_function_declaration;
typedef struct _neo_ast_variable_declaration *neo_ast_variable_declaration;
typedef struct _neo_ast_variable_declarator *neo_ast_variable_declarator;
typedef struct _neo_ast_decorator *neo_ast_decorator;
typedef struct _neo_ast_directive *neo_ast_directive;
typedef neo_ast_string_literal neo_ast_interpreter_directive;
typedef neo_ast_string_literal neo_ast_directive_literal;
typedef struct _neo_ast_super *neo_ast_super;
typedef struct _neo_ast_import *neo_ast_import;
typedef struct _neo_ast_this_expression *neo_ast_this_expression;
typedef struct _neo_ast_function *neo_ast_arrow_function;
typedef struct _neo_ast_yield_expression *neo_ast_yield_expression;
typedef struct _neo_ast_await_expression *neo_ast_await_expression;
typedef struct _neo_ast_array_expression *neo_ast_array_expression;
typedef struct _neo_ast_object_expression *neo_ast_object_expression;
typedef struct _neo_ast_object_property *neo_ast_object_property;
typedef struct _neo_ast_object_method *neo_ast_object_method;
typedef struct _neo_ast_record_expression *neo_ast_record_expression;
typedef struct _neo_ast_tuple_expression *neo_ast_tuple_expression;
typedef struct _neo_ast_function *neo_ast_function_expression;
typedef struct _neo_ast_unary_expression *neo_ast_unary_expression;
typedef struct _neo_ast_update_expression *neo_ast_update_expression;
typedef struct _neo_ast_binary_expression *neo_ast_binary_expression;
typedef struct _neo_ast_assignment_expression *neo_ast_assignment_expression;
typedef struct _neo_ast_logic_expression *neo_ast_logic_expression;
typedef struct _neo_ast_spread_element *neo_ast_spread_element;
typedef struct _neo_ast_argument_placeholder *neo_ast_argument_placeholder;
typedef struct _neo_ast_member_expression *neo_ast_member_expression;
typedef struct _neo_ast_optional_member_expression
    *neo_ast_optional_member_expression;
typedef struct _neo_ast_bind_expression *neo_ast_bind_expression;
typedef struct _neo_ast_condition_expression *neo_ast_condition_expression;
typedef struct _neo_ast_call_expression *neo_ast_call_expression;
typedef struct _neo_ast_optional_call_expression
    *neo_ast_optional_call_expression;
typedef struct _neo_ast_call_expression *neo_ast_new_expression;
typedef struct _neo_ast_sequence_expression *neo_ast_sequence_expression;
typedef struct _neo_ast_parenthesized_expression
    *neo_ast_parenthesized_expression;
typedef struct _neo_ast_do_expression *neo_ast_do_expression;
typedef struct _neo_ast_module_expression *neo_ast_module_expression;
typedef struct _neo_ast_topic_expression *neo_ast_topic_expression;
typedef struct _neo_ast_template_literal *neo_ast_template_literal;
typedef struct _neo_ast_tagged_template_expression
    *neo_ast_tagged_template_expression;
typedef struct _neo_ast_template_element *neo_ast_template_element;
typedef struct _neo_ast_object_property *neo_ast_assignment_property;
typedef struct _neo_ast_object_pattern *neo_ast_object_pattern;
typedef struct _neo_ast_array_pattern *neo_ast_array_pattern;
typedef struct _neo_ast_rest_pattern *neo_ast_rest_pattern;
typedef struct _neo_ast_assignment_pattern *neo_ast_assignment_pattern;
typedef struct _neo_ast_class *neo_ast_class;
typedef struct _neo_ast_class_body *neo_ast_class_body;
typedef struct _neo_ast_class_method *neo_ast_class_method;
typedef struct _neo_ast_class_private_method *neo_ast_class_private_method;
typedef struct _neo_ast_class_property *neo_ast_class_property;
typedef struct _neo_ast_class_private_property *neo_ast_class_private_property;
typedef struct _neo_ast_class_accessor_property
    *neo_ast_class_accessor_property;
typedef struct _neo_ast_static_block *neo_ast_static_block;
typedef struct _neo_ast_class *neo_ast_class_declaration;
typedef struct _neo_ast_class *neo_ast_class_expression;
typedef struct _neo_ast_meta_property *neo_ast_meta_property;
typedef struct _neo_ast_import_declaration *neo_ast_import_declaration;
typedef struct _neo_ast_import_specifier *neo_ast_import_specifier;
typedef struct _neo_ast_import_default_specifier
    *neo_ast_import_default_specifier;
typedef struct _neo_ast_import_namespace_specifier
    *neo_ast_import_namespace_specifier;
typedef struct _neo_ast_import_attribute *neo_ast_import_attribute;
typedef struct _neo_ast_export_named_declaration
    *neo_ast_export_named_declaration;
typedef struct _neo_ast_export_specifier *neo_ast_export_specifier;
typedef struct _neo_ast_export_namespace_specifier
    *neo_ast_export_namespace_specifier;
typedef struct _neo_ast_export_default_declaration
    *neo_ast_export_default_declaration;
typedef struct _neo_ast_pipeline_body *neo_ast_pipeline_body;
typedef struct _neo_ast_bare_function_pipeline_body
    *neo_ast_bare_function_pipeline_body;
typedef struct _neo_ast_bare_constructor_pipeline_body
    *neo_ast_bare_constructor_pipeline_body;
typedef struct _neo_ast_awaited_function_pipeline_body
    *neo_ast_awaited_function_pipeline_body;
typedef struct _neo_ast_topic_pipeline_body *neo_ast_topic_pipeline_body;
struct _neo_ast {
  enum NEO_AST_TYPE type;
  neo_token start;
  neo_token end;
  int32_t level;
};

struct _neo_ast_identifier {
  neo_ast_meta meta;
  cstring name;
};

struct _neo_ast_private_name {
  neo_ast_meta meta;
  neo_ast_identifier id;
};

struct _neo_ast_regex_literal {
  neo_ast_meta meta;
  cstring pattern;
  cstring flags;
};

struct _neo_ast_null_literal {
  neo_ast_meta meta;
};

struct _neo_ast_string_literal {
  neo_ast_meta meta;
  cstring value;
};

struct _neo_ast_boolean_literal {
  neo_ast_meta meta;
  int8_t value;
};

struct _neo_ast_number_literal {
  neo_ast_meta meta;
  double value;
};

struct _neo_ast_bigint_literal {
  neo_ast_meta meta;
  cstring value;
};

struct _neo_ast_decimal_literal {
  neo_ast_meta meta;
  cstring value;
};

struct _neo_ast_program {
  neo_ast_meta meta;
  neo_ast_interpreter_directive interpreter;
  neo_ast *body;
  neo_ast_directive *directives;
  int8_t kind;
};

struct _neo_ast_function {
  neo_ast_meta meta;
  neo_ast_identifier id;
  neo_ast *params;
  neo_ast body;
  int8_t generator;
  int8_t async;
};

struct _neo_ast_block_statement {
  neo_ast_meta meta;
  neo_ast *body;
  neo_ast_directive *directives;
};

struct _neo_ast_empty_statement {
  neo_ast_meta meta;
};

struct _neo_ast_debugger_statement {
  neo_ast_meta meta;
};

struct _neo_ast_with_statement {
  neo_ast_meta meta;
  neo_ast object;
  neo_ast body;
};

struct _neo_ast_return_statement {
  neo_ast_meta meta;
  neo_ast argument;
};

struct _neo_ast_labeled_statement {
  neo_ast_meta meta;
  neo_ast_identifier id;
  neo_ast body;
};

struct _neo_ast_break_statement {
  neo_ast_meta meta;
  neo_ast_identifier label;
};

struct _neo_ast_continue_statement {
  neo_ast_meta meta;
  neo_ast_identifier label;
};

struct _neo_ast_if_statement {
  neo_ast_meta meta;
  neo_ast test;
  neo_ast consequent;
  neo_ast alternate;
};

struct _neo_ast_switch_statement {
  neo_ast_meta meta;
  neo_ast discriminant;
  neo_ast_switch_case_statement *cases;
};

struct _neo_ast_switch_case_statement {
  neo_ast_meta meta;
  neo_ast test;
  neo_ast *consequent;
};

struct _neo_ast_throw_statement {
  neo_ast_meta meta;
  neo_ast argument;
};

struct _neo_ast_try_statement {
  neo_ast_meta meta;
  neo_ast_block_statement block;
  neo_ast_catch_clause handler;
  neo_ast_block_statement finalizer;
};

struct _neo_ast_catch_clause {
  neo_ast_meta meta;
  neo_ast_identifier param;
  neo_ast_block_statement body;
};

struct _neo_ast_while_statement {
  neo_ast_meta meta;
  neo_ast test;
  neo_ast body;
};

struct _neo_ast_do_while_statement {
  neo_ast_meta meta;
  neo_ast body;
  neo_ast test;
};

struct _neo_ast_for_statement {
  neo_ast_meta meta;
  neo_ast init;
  neo_ast test;
  neo_ast update;
  neo_ast body;
};

struct _neo_ast_for_in_statement {
  neo_ast_meta meta;
  neo_ast left;
  neo_ast right;
  neo_ast body;
};

struct _neo_ast_for_of_statement {
  neo_ast_meta meta;
  neo_ast left;
  neo_ast right;
  neo_ast body;
  int8_t await;
};

struct _neo_ast_variable_declaration {
  neo_ast_meta meta;
  neo_ast_variable_declarator *declarations;
  int8_t kind;
};

struct _neo_ast_variable_declarator {
  neo_ast_meta meta;
  neo_ast_identifier id;
  neo_ast init;
};

struct _neo_ast_decorator {
  neo_ast_meta meta;
  neo_ast expression;
};

struct _neo_ast_directive {
  neo_ast_meta meta;
  neo_ast_directive_literal value;
};

struct _neo_ast_super {
  neo_ast_meta meta;
};

struct _neo_ast_import {
  neo_ast_meta meta;
};

struct _neo_ast_this_expression {
  neo_ast_meta meta;
};

struct _neo_ast_yield_expression {
  neo_ast_meta meta;
  neo_ast argument;
  int8_t delegate;
};

struct _neo_ast_await_expression {
  neo_ast_meta meta;
  neo_ast argument;
};

struct _neo_ast_array_expression {
  neo_ast_meta meta;
  neo_ast *elements;
};

struct _neo_ast_object_expression {
  neo_ast_meta meta;
  neo_ast *properties;
};

struct _neo_ast_object_property {
  neo_ast_meta meta;
  int8_t computed;
  neo_ast key;
  neo_ast_decorator *decorators;
  int8_t shorthand;
  neo_ast value;
};

struct _neo_ast_object_method {
  neo_ast_meta meta;
  int8_t computed;
  neo_ast key;
  neo_ast_decorator *decorators;
  int8_t kind;
};

struct _neo_ast_record_expression {
  neo_ast_meta meta;
  neo_ast *properties;
};

struct _neo_ast_tuple_expression {
  neo_ast_meta meta;
  neo_ast *elements;
};

struct _neo_ast_unary_expression {
  neo_ast_meta meta;
  int8_t prefix;
  neo_ast argument;
  neo_token operator;
};

struct _neo_ast_update_expression {
  neo_ast_meta meta;
  int8_t prefix;
  neo_ast argument;
  neo_token operator;
};

struct _neo_ast_binary_expression {
  neo_ast_meta meta;
  neo_token operator;
  neo_ast left;
  neo_ast right;
};

struct _neo_ast_assignment_expression {
  neo_ast_meta meta;
  neo_token operator;
  neo_ast left;
  neo_ast right;
};

struct _neo_ast_logic_expression {
  neo_ast_meta meta;
  neo_token operator;
  neo_ast left;
  neo_ast right;
};

struct _neo_ast_spread_element {
  neo_ast_meta meta;
  neo_ast argument;
};

struct _neo_ast_argument_placeholder {
  neo_ast_meta meta;
};

struct _neo_ast_member_expression {
  neo_ast_meta meta;
  neo_ast object;
  neo_ast property;
  int8_t computed;
};

struct _neo_ast_optional_member_expression {
  neo_ast_meta meta;
  neo_ast object;
  neo_ast property;
  int8_t computed;
  int8_t optional;
};

struct _neo_ast_bind_expression {
  neo_ast_meta meta;
  neo_ast object;
  neo_ast callee;
};

struct _neo_ast_condition_expression {
  neo_ast_meta meta;
  neo_ast test;
  neo_ast consequent;
  neo_ast alternate;
};

struct _neo_ast_call_expression {
  neo_ast_meta meta;
  neo_ast callee;
  neo_ast *arguments;
};

struct _neo_ast_optional_call_expression {
  neo_ast_meta meta;
  neo_ast callee;
  neo_ast *arguments;
  int8_t optional;
};

struct _neo_ast_sequence_expression {
  neo_ast_meta meta;
  neo_ast *expressions;
};

struct _neo_ast_parenthesized_expression {
  neo_ast_meta meta;
  neo_ast expression;
};

struct _neo_ast_do_expression {
  neo_ast_meta meta;
  neo_ast body;
};

struct _neo_ast_module_expression {
  neo_ast_meta meta;
  neo_ast_program body;
};

struct _neo_ast_topic_expression {
  neo_ast_meta meta;
};

struct _neo_ast_template_literal {
  neo_ast_meta meta;
  neo_ast_template_element *quasis;
  neo_ast *expressions;
};

struct _neo_ast_tagged_template_expression {
  neo_ast_meta meta;
  neo_ast tag;
  neo_ast_template_literal quasi;
};

struct _neo_ast_template_element {
  neo_ast_meta meta;
  int8_t tail;
  struct {
    cstring cooked;
    cstring raw;
  } value;
};

struct _neo_ast_object_pattern {
  neo_ast_meta meta;
  neo_ast *properties;
};

struct _neo_ast_array_pattern {
  neo_ast_meta meta;
  neo_ast *elements;
};

struct _neo_ast_rest_pattern {
  neo_ast_meta meta;
  neo_ast argument;
};

struct _neo_ast_assignment_pattern {
  neo_ast_meta meta;
  neo_ast left;
  neo_ast right;
};

struct _neo_ast_class {
  neo_ast_meta meta;
  neo_ast_identifier id;
  neo_ast super;
  neo_ast_class_body body;
  neo_ast_decorator *decorators;
};

struct _neo_ast_class_body {
  neo_ast_meta meta;
  neo_ast *body;
};

struct _neo_ast_class_method {
  neo_ast_meta meta;
  neo_ast id;
  neo_ast *params;
  neo_ast body;
  int8_t generator;
  int8_t async;
  int8_t computed;
  int8_t static_;
  neo_ast_decorator *decorators;
};

struct _neo_ast_class_private_method {
  neo_ast_meta meta;
  neo_ast_private_name id;
  neo_ast *params;
  neo_ast body;
  int8_t generator;
  int8_t async;
  int8_t static_;
  neo_ast_decorator *decorators;
};

struct _neo_ast_class_property {
  neo_ast_meta meta;
  neo_ast id;
  neo_ast value;
  int8_t computed;
  int8_t static_;
};

struct _neo_ast_class_private_property {
  neo_ast_meta meta;
  neo_ast_private_name id;
  neo_ast value;
  int8_t static_;
};

struct _neo_ast_class_accessor_property {
  neo_ast_meta meta;
  neo_ast id;
  neo_ast value;
  int8_t computed;
  int8_t static_;
};

struct _neo_ast_static_block {
  neo_ast_meta meta;
  neo_ast *body;
};

struct _neo_ast_meta_property {
  neo_ast_meta meta;
  neo_ast_identifier id;
  neo_ast_identifier property;
};

struct _neo_ast_import_declaration {
  neo_ast_meta meta;
  int8_t kind;
  neo_ast *specifiers;
  neo_ast_string_literal source;
  neo_ast_import_attribute *assertions;
};

struct _neo_ast_import_specifier {
  neo_ast_meta meta;
  neo_ast_identifier local;
  neo_ast imported;
};

struct _neo_ast_import_default_specifier {
  neo_ast_meta meta;
  neo_ast_identifier local;
};

struct _neo_ast_import_namespace_specifier {
  neo_ast_meta meta;
  neo_ast_identifier local;
};

struct _neo_ast_import_attribute {
  neo_ast_meta meta;
  neo_ast_identifier key;
  neo_ast_string_literal value;
};

struct _neo_ast_export_named_declaration {
  neo_ast_meta meta;
  neo_ast declaration;
  neo_ast *specifiers;
  neo_ast_string_literal source;
  neo_ast_import_attribute *assertions;
};

struct _neo_ast_export_specifier {
  neo_ast_meta meta;
  neo_ast exported;
  neo_ast local;
};

struct _neo_ast_export_namespace_specifier {
  neo_ast_meta meta;
  neo_ast exported;
};

struct _neo_ast_export_default_declaration {
  neo_ast_meta meta;
  neo_ast declartion;
};

struct _neo_ast_pipeline_body {
  neo_ast_meta meta;
};

struct _neo_ast_bare_function_pipeline_body {
  neo_ast_meta meta;
  neo_ast callee;
};

struct _neo_ast_bare_constructor_pipeline_body {
  neo_ast_meta meta;
  neo_ast callee;
};

struct _neo_ast_awaited_function_pipeline_body {
  neo_ast_meta meta;
  neo_ast callee;
};

struct _neo_ast_topic_pipeline_body {
  neo_ast_meta meta;
  neo_ast expression;
};

neo_ast_meta make_neo_ast(enum NEO_AST_TYPE type, neo_token start,
                          neo_token end);
#endif
