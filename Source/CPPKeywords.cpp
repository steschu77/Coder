#include <Source/Headers.h>
#include "CPPKeywords.h"

// ============================================================================
const char* gstrKeywords[]
{
  "alignas",
  "alignof",
  "asm",
  "auto",
  "bool",
  "break",
  "case",
  "catch",
  "char",
  "char16_t",
  "char32_t",
  "class",
  "const",
  "constexpr",
  "const_cast",
  "continue",
  "decltype",
  "default",
  "delete",
  "do",
  "double",
  "dynamic_cast",
  "else",
  "enum",
  "explicit",
  "export",
  "extern",
  "false",
  "float",
  "for",
  "friend",
  "goto",
  "if",
  "inline",
  "int",
  "long",
  "mutable",
  "namespace",
  "new",
  "noexcept",
  "nullptr",
  "operator",
  "private",
  "protected",
  "public",
  "register",
  "reinterpret_cast",
  "return",
  "short",
  "signed",
  "sizeof",
  "static",
  "static_assert",
  "static_cast",
  "struct",
  "switch",
  "template",
  "this",
  "thread_local",
  "throw",
  "true",
  "try",
  "typedef",
  "typeid",
  "typename",
  "union",
  "unsigned",
  "using",
  "virtual",
  "void",
  "volatile",
  "wchar_t",
  "while"
};

// ============================================================================
const Keywords gSimpleTypeSpecifier[]
{
  cpp_auto,
  cpp_bool,
  cpp_char,
  cpp_char16_t,
  cpp_char32_t,
  cpp_double,
  cpp_float,
  cpp_int,
  cpp_long,
  cpp_short,
  cpp_signed,
  cpp_unsigned,
  cpp_void,
  cpp_wchar_t,
};

const size_t gcSimpleTypeSpecifier = countof(gSimpleTypeSpecifier);

// ============================================================================
const Keywords gStorageClassSpecifier[]
{
  cpp_extern,
  cpp_mutable,
  cpp_register,
  cpp_static,
  cpp_thread_local,
};

const size_t gcStorageClassSpecifier = countof(gStorageClassSpecifier);

// ============================================================================
const Keywords gFunctionSpecifier[]
{
  cpp_explicit,
  cpp_inline,
  cpp_virtual,
};

const size_t gcFunctionSpecifier = countof(gFunctionSpecifier);
