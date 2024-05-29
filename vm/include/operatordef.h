#ifndef __NEO_VM_OPERATORDEF__
#define __NEO_VM_OPERATORDEF__
enum NEO_OPERATOR_TYPE {
  NEO_OPERATOR_ADD = 1,
  NEO_OPERATOR_SUBTRACT,
  NEO_OPERATOR_MULTIPLY,
  NEO_OPERATOR_DIVIDE,
  NEO_OPERATOR_MODULU,
  NEO_OPERATOR_INCREMENT,
  NEO_OPERATOR_DECREMENT,
  NEO_OPERATOR_EQUAL,
  NEO_OPERATOR_NOT_EQUAL,
  NEO_OPERATOR_GREATER,
  NEO_OPERATOR_GREATER_AND_EQUAL,
  NEO_OPERATOR_LESS,
  NEO_OPERATOR_LESS_AND_EQUL,
  NEO_OPERATOR_LOGIC_AND,
  NEO_OPERATOR_LOGIC_OR,
  NEO_OPERATOR_LOGIC_NOT,
  NEO_OPERATOR_AND,
  NEO_OPERATOR_OR,
  NEO_OPERATOR_XOR,
  NEO_OPERATOR_NOT,
  NEO_OPERATOR_LEFT_SHIFT,
  NEO_OPERATOR_RIGHT_SHIFT,
  NEO_OPERATOR_SET,
  NEO_OPERATOR_ADD_AND_SET,
  NEO_OPERATOR_SUBTRACT_AND_SET,
  NEO_OPERATOR_DIVIDE_AND_SET,
  NEO_OPERATOR_MULTIPLY_AND_SET,
  NEO_OPERATOR_MODULU_AND_SET,
  NEO_OPERATOR_AND_AND_SET,
  NEO_OPERATOR_OR_AND_SET,
  NEO_OPERATOR_XOR_AND_SET,
  NEO_OPERATOR_LEFT_SHIFT_AND_SET,
  NEO_OPERATOR_RIGHT_SHIFT_AND_SET,
  NEO_OPERATOR_COMMA
};
#endif