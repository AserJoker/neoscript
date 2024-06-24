#ifndef __NEO_VM_ASTDEF__
#define __NEO_VM_ASTDEF__
enum NEO_AST_TYPE {
  NEO_AST_TYPE_UNKNOWN = 0,
  NEO_AST_TYPE_UNDEFINED,
  NEO_AST_TYPE_NULL,
  NEO_AST_TYPE_BOOLEAN,
  NEO_AST_TYPE_NUMBER,
  NEO_AST_TYPE_STRING,
  NEO_AST_TYPE_SYMBOL,
  NEO_AST_TYPE_REGEX,
  NEO_AST_TYPE_THIS,
  NEO_AST_TYPE_SUPER,
  NEO_AST_TYPE_TEMPLATE_STRING,
  NEO_AST_TYPE_ASYNC_FUNCTION,
  NEO_AST_TYPE_FUNCTION,
  NEO_AST_TYPE_FUNCTION_DECLARATION,
  NEO_AST_TYPE_FUNCTION_META,
  NEO_AST_TYPE_ASYNC_GENERATOR_FUNCTION,
  NEO_AST_TYPE_GENERATOR_FUNCTION,
  NEO_AST_TYPE_ASYNC_LAMBDA,
  NEO_AST_TYPE_LAMBDA,
  NEO_AST_TYPE_ARG_DEF,
  NEO_AST_TYPE_REST,
  NEO_AST_TYPE_DECORATOR,
  NEO_AST_TYPE_CLASS,
  NEO_AST_TYPE_CLASS_META,
  NEO_AST_TYPE_CLASS_FIELD,
  NEO_AST_TYPE_CLASS_COMPUTED_FIELD,
  NEO_AST_TYPE_IMPORT_CALL,
  NEO_AST_TYPE_CLASS_GET,
  NEO_AST_TYPE_CLASS_SET,
  NEO_AST_TYPE_CLASS_FUNCTION,
  NEO_AST_TYPE_CLASS_STATIC,
  NEO_AST_TYPE_CLASS_STATIC_BLOCK,
  NEO_AST_TYPE_ARRAY,
  NEO_AST_TYPE_ARRAY_DESTRUCT,
  NEO_AST_TYPE_ARRAY_DESTRUCT_FIELD,
  NEO_AST_TYPE_OBJECT,
  NEO_AST_TYPE_OBJECT_DESTRUCT,
  NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD,
  NEO_AST_TYPE_OBJECT_COMPUTED_FIELD,
  NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD_NAME,
  NEO_AST_TYPE_OBJECT_FIELD,
  NEO_AST_TYPE_OBJECT_FUNCTION,
  NEO_AST_TYPE_OBJECT_GET,
  NEO_AST_TYPE_OBJECT_SET,
  NEO_AST_TYPE_EXPRESSION,
  NEO_AST_TYPE_MEMBER,
  NEO_AST_TYPE_OPTIONAL_MEMBER,
  NEO_AST_TYPE_CALL,
  NEO_AST_TYPE_OPTIONAL_CALL,
  NEO_AST_TYPE_PAIR,
  NEO_AST_TYPE_PROGRAM,
  NEO_AST_TYPE_STATEMENTS,
  NEO_AST_TYPE_VAR_STATEMENT,
  NEO_AST_TYPE_VAR,
  NEO_AST_TYPE_RETURN,
  NEO_AST_TYPE_THROW,
  NEO_AST_TYPE_EMPTY,
  NEO_AST_TYPE_IF_STATEMENT,
  NEO_AST_TYPE_ELSE_STATEMENT,
  NEO_AST_TYPE_BREAK,
  NEO_AST_TYPE_CONTINUE,
  NEO_AST_TYPE_LABEL,
  NEO_AST_TYPE_SWITCH_STATEMENT,
  NEO_AST_TYPE_SWITCH_CASE,
  NEO_AST_TYPE_SWITCH_DEFAULT,
  NEO_AST_TYPE_FOR_STATEMENT,
  NEO_AST_TYPE_FOR_OF_STATEMENT,
  NEO_AST_TYPE_FOR_IN_STATEMENT,
  NEO_AST_TYPE_FOR_META,
  NEO_AST_TYPE_TRY_STATEMENT,
  NEO_AST_TYPE_CATCH_STATEMENT,
  NEO_AST_TYPE_BLOCK_STATEMENT,
  NEO_AST_TYPE_WHILE_STATEMENT,
  NEO_AST_TYPE_WITH_STATEMENT,
  NEO_AST_TYPE_EXPORT,
  NEO_AST_TYPE_EXPORT_DEFAULT,
  NEO_AST_TYPE_BRACKETS,
  NEO_AST_TYPE_TERNARY,
  NEO_AST_TYPE_TERNARY_BODY,
  NEO_AST_TYPE_MAX
};
#endif
