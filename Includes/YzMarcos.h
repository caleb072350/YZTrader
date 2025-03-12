#pragma once
#include <limits.h>
#include <string.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
/* 这段代码检查 NOMINMAX 是否已被定义，如果未定义，则定义它。这确保了在包含 windows.h 时，min 和 max 宏不会被定义，从而避免与标准库函数的冲突。
需要注意的是，最好在包含 windows.h 之前定义 NOMINMAX，以确保宏定义生效 */

#define MAX_INSTRUMENT_LENGTH	32
#define MAX_EXCHANGE_LENGTH		16

#define STATIC_CONVERT(x,T)		static_cast<T>(x)

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e+308
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F        /* max value */
#endif

#ifdef _MSC_VER
#define INVALID_DOUBLE		DBL_MAX
#define INVALID_INT32		INT_MAX
#define INVALID_UINT32		UINT_MAX
#define INVALID_INT64		_I64_MAX
#define INVALID_UINT64		_UI64_MAX
#else
#define INVALID_DOUBLE		1.7976931348623158e+308 /* max value */
#define INVALID_INT32		2147483647
#define INVALID_UINT32		0xffffffffUL
#define INVALID_INT64		9223372036854775807LL
#define INVALID_UINT64		0xffffffffffffffffULL
#endif

#define NS_WTP_BEGIN	namespace wtp{
#define NS_WTP_END	}//namespace wtp
#define	USING_NS_WTP	using namespace wtp

#ifndef EXPORT_FLAG
#ifdef _MSC_VER
#	define EXPORT_FLAG __declspec(dllexport)
#else
#	define EXPORT_FLAG __attribute__((__visibility__("default")))
#endif
#endif
/* 这段代码用于定义一个名为 EXPORT_FLAG 的宏，以便在不同的编译器和平台上控制符号的导出，确保函数或变量在动态链接库（DLL）中可以被外部访问。
具体而言：
检查是否已定义 EXPORT_FLAG：如果未定义，则继续定义它。
根据编译器设置 EXPORT_FLAG：
Microsoft 编译器（MSVC）：如果检测到使用的是 Microsoft 编译器（通过检查 _MSC_VER 宏），则将 EXPORT_FLAG 定义为 __declspec(dllexport)。这是 MSVC 特定的关键字，用于指示符号应导出到 DLL 中。
其他编译器：对于非 MSVC 编译器，EXPORT_FLAG 被定义为 __attribute__((__visibility__("default")))。这是一种 GCC 和 Clang 编译器特有的属性，用于设置符号的可见性，使其在共享库中可见。
通过这种方式，EXPORT_FLAG 宏可以跨平台地控制符号导出，确保在不同的编译器和平台上，函数或变量都能被正确地导出并被外部代码访问 
*/

#ifndef PORTER_FLAG
#ifdef _MSC_VER
#	define PORTER_FLAG _cdecl
#else
#	define PORTER_FLAG 
#endif
#endif

/* 这段代码用于定义一个名为 PORTER_FLAG 的宏，以在不同的编译器环境下指定函数的调用约定。调用约定决定了函数在调用时参数的传递方式和栈的清理方式。
具体而言，这段代码的作用如下：
检查是否已定义 PORTER_FLAG：如果未定义，则继续执行后续代码。
根据编译器类型设置 PORTER_FLAG：
Microsoft 编译器（MSVC）：如果检测到使用的是 Microsoft 编译器（通过检查 _MSC_VER 宏），则将 PORTER_FLAG 定义为 _cdecl。_cdecl 是 MSVC 特有的关键字，用于指定 C 语言的调用约定，即函数参数从右到左入栈，调用者负责清理栈。
其他编译器：对于非 MSVC 编译器，PORTER_FLAG 被定义为空，即不指定特定的调用约定。
通过这种方式，PORTER_FLAG 宏可以在不同的编译器环境下确保函数使用适当的调用约定，确保代码的兼容性和正确性
*/

typedef unsigned int		WtUInt32;
typedef unsigned long long	WtUInt64;
typedef const char*			WtString;

#ifdef _MSC_VER
#define wt_stricmp _stricmp
#else
#define wt_stricmp strcasecmp
#endif

/*
 *	By Wesley @ 2022.03.17
 *	重写一个strcpy
 *	核心的要点就是不用strcpy
 *	字符串比较长的时候，会优于strcpy
 */
/*
 *	By Wesley @ 2023.10.09
 *	重新和strcpy进行了性能测试，发现性能上并没有提升，甚至还有一些下降
 *	可能和早期测试环境有很大关系，用到的地方很多，暂时先保留
 */
inline size_t wt_strcpy(char* des, const char* src, size_t len = 0)
{
	len = (len == 0) ? strlen(src) : len;
	memcpy(des, src, len);
	des[len] = '\0';
	return len;
}