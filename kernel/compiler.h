#ifndef KERNEL_COMPILER_H
#define KERNEL_COMPILER_H

#if defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
# define __gnu_exts 1
#endif

#if defined(__gnu_exts)
# define __likely(x)		__builtin_expect(!!(x), 1)
# define __unlikely(x)		__builtin_expect(!!(x), 0)
# define __wur			__attribute__ ((warn_unused_result))
# define __noinline		__attribute__ ((noinline))
# define __really_inline	inline __attribute__ ((always_inline))
# define __unused		__attribute__ ((unused))
# define __fmt(...)		__attribute__ ((format (printf, ##__VA_ARGS__)))
# define __packed		__attribute__ ((packed))
# define __aligned(x)		__attribute__ ((aligned(x)))
#else
# define __likely(x)
# define __unlikely(x)
# define __wur
# define __noinline
# define __really_inline
# define __unused
# define __fmt(...)
# define __packed
# define __aligned(x)
#endif

#define __to_string(x) #x

#define __array_size(x) (sizeof ((x)) / sizeof (*(x)))

#endif /* KERNEL_COMPILER_H */