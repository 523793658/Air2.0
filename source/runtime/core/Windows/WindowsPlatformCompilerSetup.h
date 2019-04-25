#pragma once
#if !PLATFORM_WINDOWS
#error PLATFORM_WINDOWS not defined
#endif


#pragma warning(disable: 4091) // 'keyword' : ignored on left of 'type' when no variable is declared								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4091

// Unwanted VC++ level 4 warnings to disable.
#pragma warning(disable: 4100) // 'identifier' : unreferenced formal parameter														https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4100
#pragma warning(disable: 4121) // 'symbol' : alignment of a member was sensitive to packing											https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4121
#pragma warning(disable: 4127) // Conditional expression is constant																https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4127
#pragma warning(disable: 4180) // qualifier applied to function type has no meaning; ignored										https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4180
#pragma warning(disable: 4189) // 'identifier': local variable is initialized but not referenced									https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4000-through-c4199

#pragma warning(disable: 4200) // Zero-length array item at end of structure, a VC-specific extension								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-levels-2-and-4-c4200
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union												https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4201
#pragma warning(disable: 4217) // 'operator' : member template functions cannot be used for copy-assignment or copy-construction	// No docs
#pragma warning(disable: 4244) // 'argument' : conversion from 'type1' to 'type2', possible loss of data							https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4201
#pragma warning(disable: 4245) // 'initializing': conversion from 'type' to 'type', signed/unsigned mismatch						https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4245
#pragma warning(disable: 4251) // 'type' needs to have dll-interface to be used by clients of 'type'								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
#pragma warning(disable: 4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data									https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4267
#pragma warning(disable: 4275) // non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4275
#pragma warning(disable: 4291) // typedef-name '' used as synonym for class-name ''													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4291

#pragma warning(disable: 4307) // '': integral constant overflow																	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4307
#pragma warning(disable: 4315) // 'classname': 'this' pointer for member 'member' may not be aligned 'alignment' as expected by the constructor	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4200-through-c4399
#pragma warning(disable: 4316) // 'identifier': object allocated on the heap may not be aligned 'alignment'							https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4200-through-c4399
#pragma warning(disable: 4324) // structure was padded due to __declspec(align())													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4324
#pragma warning(disable: 4347) // behavior change: 'function template' is called instead of 'function								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4200-through-c4399
#pragma warning(disable: 4351) // new behavior: elements of array 'array' will be default initialized								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4200-through-c4399
#pragma warning(disable: 4355) // this used in base initializer list																https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-c4355
#pragma warning(disable: 4366) // The result of the unary 'operator' operator may be unaligned										https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4366
#pragma warning(disable: 4373) // '%$S': virtual function overrides '%$pS', previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4373
#pragma warning(disable: 4389) // signed/unsigned mismatch																			https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-c4355

#pragma warning(disable: 4464) // relative include path contains '..'																https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c4464
#pragma warning(disable: 4482) // nonstandard extension used: enum 'enumeration' used in qualified name								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4400-through-c4599

#pragma warning(disable: 4505) // 'function' : unreferenced local function has been removed											https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4505
#pragma warning(disable: 4511) // 'class' : copy constructor could not be generated													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4511
#pragma warning(disable: 4512) // 'class' : assignment operator could not be generated												https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4512
#pragma warning(disable: 4514) // 'function' : unreferenced inline function has been removed										https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4514
#if UE_BUILD_DEBUG
// xstring.h causes this warning in debug builds
#pragma warning(disable: 4548) // expression before comma has no effect; expected expression with side-effect						https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4548
#endif
#pragma warning(disable: 4592) // 'function': 'constexpr' call evaluation failed; function will be called at run-time				https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4400-through-c4599
#pragma warning(disable: 4599) // 'flag path': command line argument number number does not match precompiled header				https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4400-through-c4599

#pragma warning(disable: 4605) // '/Dmacro' specified on current command line, but was not specified when precompiled header was built	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4600-through-c4799
#pragma warning(disable: 4623) // 'derived class' : default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4623
#pragma warning(disable: 4625) // 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4625
#pragma warning(disable: 4626) // 'derived class' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4626
#pragma warning(disable: 4640) // 'instance' : construction of local static object is not thread-safe								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4640
#pragma warning(disable: 4699) // creating precompiled header																		// No docs

#pragma warning(disable: 4702) // unreachable code																					https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4702
#pragma warning(disable: 4710) // 'function' : function not inlined																	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4710
#pragma warning(disable: 4711) // function selected for automatic inlining															https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4711
#pragma warning(disable: 4714) // function 'function' marked as __forceinline not inlined											https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4714
#pragma warning(disable: 4748) // /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function	// No docs
#pragma warning(disable: 4768) // __declspec attributes before linkage specification are ignored									https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4600-through-c4799
// NOTE: _mm_cvtpu8_ps will generate this falsely if it doesn't get inlined
#pragma warning(disable: 4799) // Warning: function 'ident' has no EMMS instruction													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4799

// NOTE: https://answers.unrealengine.com/questions/701635/warning-c4828.html
#pragma warning(disable: 4828) // The file contains a character starting at offset ... that is illegal in the current source character set(codepage ...).	// No docs
#pragma warning(disable: 4868) // 'file(line_number)' compiler may not enforce left-to-right evaluation order in braced initializer list	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-c4868

// NOTE: ocid.h breaks this
#pragma warning(disable: 4917) // 'declarator' : a GUID can only be associated with a class, interface or namespace 				https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4917
#if WINVER == 0x0502
// NOTE: WinXP hits deprecated versions of stdio across the board
#pragma warning(disable: 4995) // 'function': name was marked as #pragma deprecated													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4995
#endif

//
// All of the /Wall warnings that we are able to enable
// @todo: https://docs.microsoft.com/en-us/cpp/preprocessor/compiler-warnings-that-are-off-by-default
// NOTE: This is currently just overriding the error versions above, removing these will cause them to be errors!
//

#pragma warning(default: 4191) // 'operator/operation': unsafe conversion from 'type_of_expression' to 'type_required'				https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4000-through-c4199

#pragma warning(default: 4255) // 'function' : no function prototype given: converting '()' to '(void)'								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4255
#pragma warning(default: 4263) // 'function' : member function does not override any base class virtual member function				https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4263
#pragma warning(default: 4264) // 'virtual_function' : no override available for virtual member function from base 'class'; function is hidden	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4264
#pragma warning(3:       4265) // 'class' : class has virtual functions, but destructor is not virtual								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4265
#pragma warning(default: 4287) // 'operator' : unsigned/negative constant mismatch													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4264
#pragma warning(default: 4289) // nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4289

//#pragma warning(disable : 4339) // 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
#pragma warning(disable: 4345) // behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized

#pragma warning(disable: 4514) // unreferenced inline/local function has been removed
#pragma warning(default: 4529) // 'member_name' : forming a pointer-to-member requires explicit use of the address-of operator ('&') and a qualified name	// No docs
#pragma warning(default: 4536) // 'type name' : type-name exceeds meta-data limit of 'limit' characters								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4536
#pragma warning(default: 4545) // expression before comma evaluates to a function which is missing an argument list					https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4545
#pragma warning(default: 4546) // function call before comma missing argument list													https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4546
#pragma warning(default: 4557) // '__assume' contains effect 'effect'																https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4557
#pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. Specify /EHsc

#pragma warning(default: 4628) // digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4289
#pragma warning(default: 4682) // 'parameter' : no directional parameter attribute specified, defaulting to [in]					https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4682
#pragma warning(default: 4686) // 'user-defined type' : possible change in behavior, change in UDT return calling convention		https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4686

#pragma warning(disable: 4710) // 'function' : function not inlined / The given function was selected for inline expansion, but the compiler did not perform the inlining.
#pragma warning(default: 4786) // 'identifier' : identifier was truncated to 'number' characters in the debug information			// No docs
#pragma warning(default: 4793) // 'function' : function is compiled as native code: 'reason'										https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-and-3-c4793

#pragma warning(default: 4905) // wide string literal cast to 'LPSTR'																https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4905
#pragma warning(default: 4906) // string literal cast to 'LPWSTR'																	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4906
#pragma warning(default: 4928) // illegal copy-initialization; more than one user-defined conversion has been implicitly applied	https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4928
#pragma warning(default: 4931) // we are assuming the type library was built for number-bit pointers								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4931
#pragma warning(default: 4946) // reinterpret_cast used between related classes: 'class1' and 'class2'								https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4946

#pragma warning(default: 4996)

// interesting ones to turn on and off at times
//#pragma warning(disable : 4266) // '' : no override available for virtual member function from base ''; function is hidden
//#pragma warning(disable : 4296) // 'operator' : expression is always true / false
//#pragma warning(disable : 4820) // 'bytes' bytes padding added after member 'member'
// Mixing MMX/SSE intrinsics will cause this warning, even when it's done correctly.
//#pragma warning(disable : 4730) //mixing _m64 and floating point expressions may result in incorrect code

// If C++ exception handling is disabled, force guarding to be off.
#if !defined(_CPPUNWIND) && !defined(__INTELLISENSE__) && !defined(HACK_HEADER_GENERATOR)
#error "Bad VCC option: C++ exception handling must be enabled" //lint !e309 suppress as lint doesn't have this defined
#endif

// Make sure characters are unsigned.
#ifdef _CHAR_UNSIGNED
#error "Bad VC++ option: Characters must be signed" //lint !e309 suppress as lint doesn't have this defined
#endif