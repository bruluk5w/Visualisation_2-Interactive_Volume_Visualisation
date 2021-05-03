#pragma once // (c) 2020 Lukas Brunner

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#error When using MSVC compile with /Zc:preprocessor to use a standard conformant preprocessor
#endif


#define BRWL_STRIP_PARENTHESIS_(...) __VA_ARGS__
// transforms a parameter like "(a, b)" to "a, b"
#define BRWL_STRIP_PARENTHESIS(group) BRWL_STRIP_PARENTHESIS_ group

#define BRWL_APPEND_ARGS_(...) , ## __VA_ARGS__
// transforms "arg_group" from "()" or "(a, b)" to "" or ", a, b" respectively
#define BRWL_APPEND_ARGS(arg_group) BRWL_APPEND_ARGS_ arg_group

//#define SWITCH_MACRO(_0, NAME, ...) NAME

// returns the first argument from __VA_ARGS__
#define BRWL_FIRST_ARG(first, ...) first

// does nothing
#define BRWL_NOP do { } while(0)

// BRWL_IF_DEBUG(MY_MACRO) gets replaced with the contents of MY_MACRO_ if _DEBUG is defined else a null operation is emitted
// BRWL_IF_NDEBUG works the same for the inverse case
#ifdef _DEBUG
#define BRWL_IF_DEBUG(m, ...) m##_(__VA_ARGS__)
#define BRWL_IF_NDEBUG(m) BRWL_NOP
#else
	#define BRWL_IF_DEBUG(m) BRWL_NOP
	#define BRWL_IF_NDEBUG(m) m##_
#endif
