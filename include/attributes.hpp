#ifndef __ATTRIBUTES_HPP_INCLUDED__
#define __ATTRIBUTES_HPP_INLCUDED__

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#define UNUSED __attribute__((unused))
#define INLINE __attribute__((always_inline)) inline
#define HIDDEN __attribute__((visibility("hidden")))
#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)
#else
#error Add attributes for different C++ compilers here
#endif

#endif