/*!
** \file    /Source/CPPKeyword.h
** \brief   Definition of C++14 keywords
******************************************************************************/
#pragma once

// ============================================================================
enum Keywords
{
  cpp_alignas,
  cpp_alignof,
  cpp_asm,
  cpp_auto,
  cpp_bool,
  cpp_break,
  cpp_case,
  cpp_catch,
  cpp_char,
  cpp_char16_t,
  cpp_char32_t,
  cpp_class,
  cpp_const,
  cpp_constexpr,
  cpp_const_cast,
  cpp_continue,
  cpp_decltype,
  cpp_default,
  cpp_delete,
  cpp_do,
  cpp_double,
  cpp_dynamic_cast,
  cpp_else,
  cpp_enum,
  cpp_explicit,
  cpp_export,
  cpp_extern,
  cpp_false,
  cpp_float,
  cpp_for,
  cpp_friend,
  cpp_goto,
  cpp_if,
  cpp_inline,
  cpp_int,
  cpp_long,
  cpp_mutable,
  cpp_namespace,
  cpp_new,
  cpp_noexcept,
  cpp_nullptr,
  cpp_operator,
  cpp_private,
  cpp_protected,
  cpp_public,
  cpp_register,
  cpp_reinterpret_cast,
  cpp_return,
  cpp_short,
  cpp_signed,
  cpp_sizeof,
  cpp_static,
  cpp_static_assert,
  cpp_static_cast,
  cpp_struct,
  cpp_switch,
  cpp_template,
  cpp_this,
  cpp_thread_local,
  cpp_throw,
  cpp_true,
  cpp_try,
  cpp_typedef,
  cpp_typeid,
  cpp_typename,
  cpp_union,
  cpp_unsigned,
  cpp_using,
  cpp_virtual,
  cpp_void,
  cpp_volatile,
  cpp_wchar_t,
  cpp_while,
  cpp_cKeywords
};

// ============================================================================
extern const char* gstrKeywords[];

extern const Keywords gSimpleTypeSpecifier[];
extern const size_t gcSimpleTypeSpecifier;

extern const Keywords gStorageClassSpecifier[];
extern const size_t gcStorageClassSpecifier;

extern const Keywords gFunctionSpecifier[];
extern const size_t gcFunctionSpecifier;
