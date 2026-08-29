/* Copyright 2026
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Header-only Android libc helpers selected from bionic libc.map.txt.
 *
 * This follows the linux_syscall_support.h style: definitions are static
 * inline by default, symbol names are prefixed by default, and callers may
 * override the prefix/inline policy before including the file.
 *
 * Implemented here: local byte memory, byte strings, ASCII ctype, integer
 * conversion/arithmetic, byte order, search/sort, a small deterministic PRNG,
 * float classification, and wide memory/string primitives.  Intentionally not
 * implemented: pthreads, syscalls, file/socket/stdio, malloc-backed helpers,
 * Android system properties, resolver/network database, locale databases,
 * timezone conversion, or formatting/parsing engines.
 *
 * Options:
 *   SELF_LIBC_INLINE          defaults to static inline
 *   SELF_LIBC_PREFIX          defaults to self_
 *   SELF_LIBC_ERRNO           optional errno l-value for ERANGE
 *   SELF_LIBC_REMAP_STANDARD_NAMES maps memcpy/strcmp/... to prefixed helpers
 */
#ifndef SELF_LIBC_SUPPORT_H_
#define SELF_LIBC_SUPPORT_H_

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wchar.h>

#ifdef wcswcs
#undef wcswcs
#endif
#ifndef EOF
#define EOF (-1)
#endif

#ifndef SELF_LIBC_INLINE
#define SELF_LIBC_INLINE static inline
#endif
#ifndef SELF_LIBC_PREFIX
#define SELF_LIBC_PREFIX self_
#endif
#define SELF_LIBC_CONCAT_INNER(a, b) a##b
#define SELF_LIBC_CONCAT(a, b) SELF_LIBC_CONCAT_INNER(a, b)
#define SELF_LIBC_NAME(name) SELF_LIBC_CONCAT(SELF_LIBC_PREFIX, name)
#if defined(__cplusplus)
#define SELF_LIBC_RESTRICT
#else
#define SELF_LIBC_RESTRICT restrict
#endif
#if defined(__has_builtin)
#define SELF_LIBC_HAS_BUILTIN(x) __has_builtin(x)
#else
#define SELF_LIBC_HAS_BUILTIN(x) 0
#endif
#ifdef SELF_LIBC_ERRNO
#define SELF_LIBC_SET_ERRNO(v) (SELF_LIBC_ERRNO = (v))
#else
#define SELF_LIBC_SET_ERRNO(v) ((void)0)
#endif
#if defined(__GNUC__) || defined(__clang__)
#define SELF_LIBC_TRAP() __builtin_trap()
#else
#define SELF_LIBC_TRAP() do { *(volatile int*)0 = 0; } while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

SELF_LIBC_INLINE int SELF_LIBC_NAME(internal_isspace)(int c) {
  return c == ' ' || (c >= '\t' && c <= '\r');
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(internal_tolower)(int c) {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(internal_toupper)(int c) {
  return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(internal_digit)(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'z') return c - 'a' + 10;
  if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
  return -1;
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(internal_swap)(unsigned char* a,
                                                    unsigned char* b,
                                                    size_t n) {
  while (n-- != 0) {
    unsigned char t = *a;
    *a++ = *b;
    *b++ = t;
  }
}

/* Memory. */
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memcpy)(
    void* SELF_LIBC_RESTRICT dst, const void* SELF_LIBC_RESTRICT src,
    size_t n) {
  unsigned char* d = (unsigned char*)dst;
  const unsigned char* s = (const unsigned char*)src;
  while (n-- != 0) *d++ = *s++;
  return dst;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(mempcpy)(
    void* SELF_LIBC_RESTRICT dst, const void* SELF_LIBC_RESTRICT src,
    size_t n) {
  SELF_LIBC_NAME(memcpy)(dst, src, n);
  return (unsigned char*)dst + n;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memmove)(void* dst, const void* src,
                                               size_t n) {
  unsigned char* d = (unsigned char*)dst;
  const unsigned char* s = (const unsigned char*)src;
  uintptr_t da = (uintptr_t)d;
  uintptr_t sa = (uintptr_t)s;
  if (d == s || n == 0) return dst;
  if (da - sa >= n) {
    while (n-- != 0) *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n-- != 0) *--d = *--s;
  }
  return dst;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memset)(void* dst, int c, size_t n) {
  unsigned char* d = (unsigned char*)dst;
  while (n-- != 0) *d++ = (unsigned char)c;
  return dst;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memset_explicit)(void* dst, int c,
                                                       size_t n) {
  volatile unsigned char* d = (volatile unsigned char*)dst;
  while (n-- != 0) *d++ = (unsigned char)c;
  return dst;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(memcmp)(const void* lhs, const void* rhs,
                                            size_t n) {
  const unsigned char* l = (const unsigned char*)lhs;
  const unsigned char* r = (const unsigned char*)rhs;
  while (n-- != 0) {
    if (*l != *r) return (int)*l - (int)*r;
    ++l;
    ++r;
  }
  return 0;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memchr)(const void* s, int c, size_t n) {
  const unsigned char* p = (const unsigned char*)s;
  unsigned char ch = (unsigned char)c;
  while (n-- != 0) {
    if (*p == ch) return (void*)p;
    ++p;
  }
  return NULL;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memrchr)(const void* s, int c,
                                               size_t n) {
  const unsigned char* p = (const unsigned char*)s + n;
  unsigned char ch = (unsigned char)c;
  while (n-- != 0) {
    --p;
    if (*p == ch) return (void*)p;
  }
  return NULL;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memccpy)(void* SELF_LIBC_RESTRICT dst,
                                               const void* SELF_LIBC_RESTRICT src,
                                               int c, size_t n) {
  unsigned char* d = (unsigned char*)dst;
  const unsigned char* s = (const unsigned char*)src;
  unsigned char ch = (unsigned char)c;
  while (n-- != 0) {
    *d = *s;
    if (*s == ch) return d + 1;
    ++d;
    ++s;
  }
  return NULL;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(memmem)(const void* h, size_t hlen,
                                              const void* n, size_t nlen) {
  const unsigned char* hay = (const unsigned char*)h;
  const unsigned char* needle = (const unsigned char*)n;
  size_t i;
  if (nlen == 0) return (void*)hay;
  if (hlen < nlen) return NULL;
  for (i = 0; i <= hlen - nlen; ++i) {
    if (hay[i] == needle[0] &&
        SELF_LIBC_NAME(memcmp)(hay + i, needle, nlen) == 0) {
      return (void*)(hay + i);
    }
  }
  return NULL;
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(bcopy)(const void* src, void* dst,
                                            size_t n) {
  SELF_LIBC_NAME(memmove)(dst, src, n);
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(bzero)(void* dst, size_t n) {
  SELF_LIBC_NAME(memset)(dst, 0, n);
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(memswap)(void* a, void* b, size_t n) {
  SELF_LIBC_NAME(internal_swap)((unsigned char*)a, (unsigned char*)b, n);
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(swab)(const void* SELF_LIBC_RESTRICT from,
                                           void* SELF_LIBC_RESTRICT to,
                                           ssize_t n) {
  const unsigned char* f = (const unsigned char*)from;
  unsigned char* t = (unsigned char*)to;
  while (n > 1) {
    unsigned char c = f[0];
    t[0] = f[1];
    t[1] = c;
    f += 2;
    t += 2;
    n -= 2;
  }
}
#define SELF_LIBC_AEABI_COPY(name, target)                                      \
  SELF_LIBC_INLINE void SELF_LIBC_NAME(name)(void* d, const void* s, size_t n) { \
    SELF_LIBC_NAME(target)(d, s, n);                                             \
  }
SELF_LIBC_AEABI_COPY(__aeabi_memcpy, memcpy)
SELF_LIBC_AEABI_COPY(__aeabi_memcpy4, memcpy)
SELF_LIBC_AEABI_COPY(__aeabi_memcpy8, memcpy)
SELF_LIBC_AEABI_COPY(__aeabi_memmove, memmove)
SELF_LIBC_AEABI_COPY(__aeabi_memmove4, memmove)
SELF_LIBC_AEABI_COPY(__aeabi_memmove8, memmove)
#undef SELF_LIBC_AEABI_COPY
#define SELF_LIBC_AEABI_SET(name)                                    \
  SELF_LIBC_INLINE void SELF_LIBC_NAME(name)(void* d, size_t n, int c) { \
    SELF_LIBC_NAME(memset)(d, c, n);                                  \
  }
SELF_LIBC_AEABI_SET(__aeabi_memset)
SELF_LIBC_AEABI_SET(__aeabi_memset4)
SELF_LIBC_AEABI_SET(__aeabi_memset8)
#undef SELF_LIBC_AEABI_SET
#define SELF_LIBC_AEABI_CLR(name)                                \
  SELF_LIBC_INLINE void SELF_LIBC_NAME(name)(void* d, size_t n) { \
    SELF_LIBC_NAME(memset)(d, 0, n);                              \
  }
SELF_LIBC_AEABI_CLR(__aeabi_memclr)
SELF_LIBC_AEABI_CLR(__aeabi_memclr4)
SELF_LIBC_AEABI_CLR(__aeabi_memclr8)
#undef SELF_LIBC_AEABI_CLR

/* Byte strings. */
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strlen)(const char* s) {
  const char* p = s;
  while (*p != '\0') ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strnlen)(const char* s, size_t max_len) {
  size_t n = 0;
  while (n < max_len && s[n] != '\0') ++n;
  return n;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strcmp)(const char* lhs, const char* rhs) {
  const unsigned char* l = (const unsigned char*)lhs;
  const unsigned char* r = (const unsigned char*)rhs;
  while (*l != '\0' && *l == *r) {
    ++l;
    ++r;
  }
  return (int)*l - (int)*r;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strncmp)(const char* lhs, const char* rhs,
                                             size_t n) {
  const unsigned char* l = (const unsigned char*)lhs;
  const unsigned char* r = (const unsigned char*)rhs;
  while (n-- != 0) {
    if (*l != *r) return (int)*l - (int)*r;
    if (*l == '\0') return 0;
    ++l;
    ++r;
  }
  return 0;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(stpcpy)(char* SELF_LIBC_RESTRICT dst,
                                              const char* SELF_LIBC_RESTRICT src) {
  while ((*dst = *src) != '\0') {
    ++dst;
    ++src;
  }
  return dst;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strcpy)(char* SELF_LIBC_RESTRICT dst,
                                              const char* SELF_LIBC_RESTRICT src) {
  SELF_LIBC_NAME(stpcpy)(dst, src);
  return dst;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(stpncpy)(char* SELF_LIBC_RESTRICT dst,
                                               const char* SELF_LIBC_RESTRICT src,
                                               size_t n) {
  char* d = dst;
  while (n != 0 && *src != '\0') {
    *d++ = *src++;
    --n;
  }
  if (n == 0) return d;
  *d++ = '\0';
  while (--n != 0) *d++ = '\0';
  return d - 1;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strncpy)(char* SELF_LIBC_RESTRICT dst,
                                               const char* SELF_LIBC_RESTRICT src,
                                               size_t n) {
  SELF_LIBC_NAME(stpncpy)(dst, src, n);
  return dst;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strcat)(char* SELF_LIBC_RESTRICT dst,
                                              const char* SELF_LIBC_RESTRICT src) {
  SELF_LIBC_NAME(stpcpy)(dst + SELF_LIBC_NAME(strlen)(dst), src);
  return dst;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strncat)(char* SELF_LIBC_RESTRICT dst,
                                               const char* SELF_LIBC_RESTRICT src,
                                               size_t n) {
  char* d = dst + SELF_LIBC_NAME(strlen)(dst);
  while (n-- != 0 && *src != '\0') *d++ = *src++;
  *d = '\0';
  return dst;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strchr)(const char* s, int c) {
  char ch = (char)c;
  for (;;) {
    if (*s == ch) return (char*)s;
    if (*s == '\0') return NULL;
    ++s;
  }
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strchrnul)(const char* s, int c) {
  char ch = (char)c;
  while (*s != '\0' && *s != ch) ++s;
  return (char*)s;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(index)(const char* s, int c) {
  return SELF_LIBC_NAME(strchr)(s, c);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strrchr)(const char* s, int c) {
  const char* last = NULL;
  char ch = (char)c;
  do {
    if (*s == ch) last = s;
  } while (*s++ != '\0');
  return (char*)last;
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strspn)(const char* s,
                                               const char* accept) {
  const char* p = s;
  while (*p != '\0' && SELF_LIBC_NAME(strchr)(accept, *p) != NULL) ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strcspn)(const char* s,
                                                const char* reject) {
  const char* p = s;
  while (*p != '\0' && SELF_LIBC_NAME(strchr)(reject, *p) == NULL) ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strpbrk)(const char* s,
                                               const char* accept) {
  while (*s != '\0') {
    if (SELF_LIBC_NAME(strchr)(accept, *s) != NULL) return (char*)s;
    ++s;
  }
  return NULL;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strstr)(const char* haystack,
                                              const char* needle) {
  size_t nlen;
  if (*needle == '\0') return (char*)haystack;
  nlen = SELF_LIBC_NAME(strlen)(needle);
  while ((haystack = SELF_LIBC_NAME(strchr)(haystack, *needle)) != NULL) {
    if (SELF_LIBC_NAME(strncmp)(haystack, needle, nlen) == 0) {
      return (char*)haystack;
    }
    ++haystack;
  }
  return NULL;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strcasecmp)(const char* lhs,
                                                const char* rhs) {
  const unsigned char* l = (const unsigned char*)lhs;
  const unsigned char* r = (const unsigned char*)rhs;
  while (*l != '\0') {
    int lc = SELF_LIBC_NAME(internal_tolower)(*l);
    int rc = SELF_LIBC_NAME(internal_tolower)(*r);
    if (lc != rc) return lc - rc;
    ++l;
    ++r;
  }
  return SELF_LIBC_NAME(internal_tolower)(*l) -
         SELF_LIBC_NAME(internal_tolower)(*r);
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strncasecmp)(const char* lhs,
                                                 const char* rhs, size_t n) {
  const unsigned char* l = (const unsigned char*)lhs;
  const unsigned char* r = (const unsigned char*)rhs;
  while (n-- != 0) {
    int lc = SELF_LIBC_NAME(internal_tolower)(*l);
    int rc = SELF_LIBC_NAME(internal_tolower)(*r);
    if (lc != rc) return lc - rc;
    if (*l == '\0') return 0;
    ++l;
    ++r;
  }
  return 0;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strcasecmp_l)(const char* lhs,
                                                  const char* rhs,
                                                  const void* locale) {
  (void)locale;
  return SELF_LIBC_NAME(strcasecmp)(lhs, rhs);
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(strncasecmp_l)(const char* lhs,
                                                   const char* rhs, size_t n,
                                                   const void* locale) {
  (void)locale;
  return SELF_LIBC_NAME(strncasecmp)(lhs, rhs, n);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strcasestr)(const char* haystack,
                                                  const char* needle) {
  size_t nlen;
  if (*needle == '\0') return (char*)haystack;
  nlen = SELF_LIBC_NAME(strlen)(needle);
  while (*haystack != '\0') {
    if (SELF_LIBC_NAME(internal_tolower)((unsigned char)*haystack) ==
            SELF_LIBC_NAME(internal_tolower)((unsigned char)*needle) &&
        SELF_LIBC_NAME(strncasecmp)(haystack, needle, nlen) == 0) {
      return (char*)haystack;
    }
    ++haystack;
  }
  return NULL;
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strlcpy)(char* SELF_LIBC_RESTRICT dst,
                                                const char* SELF_LIBC_RESTRICT src,
                                                size_t dst_size) {
  const char* s = src;
  size_t left = dst_size;
  if (left != 0) {
    while (--left != 0 && *s != '\0') *dst++ = *s++;
    if (left == 0) {
      if (dst_size != 0) *dst = '\0';
      while (*s++ != '\0') {}
    } else {
      *dst = '\0';
    }
  } else {
    while (*s++ != '\0') {}
  }
  return (size_t)((s - src) - 1);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(strlcat)(char* SELF_LIBC_RESTRICT dst,
                                                const char* SELF_LIBC_RESTRICT src,
                                                size_t dst_size) {
  size_t dlen = SELF_LIBC_NAME(strnlen)(dst, dst_size);
  size_t slen = SELF_LIBC_NAME(strlen)(src);
  if (dlen == dst_size) return dst_size + slen;
  if (slen < dst_size - dlen) {
    SELF_LIBC_NAME(memcpy)(dst + dlen, src, slen + 1);
  } else {
    SELF_LIBC_NAME(memcpy)(dst + dlen, src, dst_size - dlen - 1);
    dst[dst_size - 1] = '\0';
  }
  return dlen + slen;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strsep)(char** stringp,
                                              const char* delim) {
  char* s;
  char* end;
  if (stringp == NULL || *stringp == NULL) return NULL;
  s = *stringp;
  end = s + SELF_LIBC_NAME(strcspn)(s, delim);
  if (*end == '\0') {
    *stringp = NULL;
  } else {
    *end++ = '\0';
    *stringp = end;
  }
  return s;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strtok_r)(char* SELF_LIBC_RESTRICT s,
                                                const char* SELF_LIBC_RESTRICT delim,
                                                char** SELF_LIBC_RESTRICT saveptr) {
  char* token;
  if (s == NULL) s = *saveptr;
  if (s == NULL) return NULL;
  s += SELF_LIBC_NAME(strspn)(s, delim);
  if (*s == '\0') {
    *saveptr = NULL;
    return NULL;
  }
  token = s;
  s = SELF_LIBC_NAME(strpbrk)(token, delim);
  if (s == NULL) {
    *saveptr = NULL;
  } else {
    *s++ = '\0';
    *saveptr = s;
  }
  return token;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(strtok)(char* s, const char* delim) {
  static char* state;
  return SELF_LIBC_NAME(strtok_r)(s, delim, &state);
}

SELF_LIBC_INLINE char* SELF_LIBC_NAME(__gnu_basename)(const char* path) {
  const char* last;
  if (path == NULL) return (char*)"";
  last = SELF_LIBC_NAME(strrchr)(path, '/');
  return (char*)(last == NULL ? path : last + 1);
}

SELF_LIBC_INLINE char* SELF_LIBC_NAME(basename)(char* path) {
  char* end;
  char* slash;
  if (path == NULL || *path == '\0') return (char*)".";
  end = path + SELF_LIBC_NAME(strlen)(path) - 1;
  while (end > path && *end == '/') *end-- = '\0';
  if (end == path && *end == '/') return path;
  slash = SELF_LIBC_NAME(strrchr)(path, '/');
  return slash == NULL ? path : slash + 1;
}

SELF_LIBC_INLINE char* SELF_LIBC_NAME(dirname)(char* path) {
  char* end;
  char* slash;
  if (path == NULL || *path == '\0') return (char*)".";
  end = path + SELF_LIBC_NAME(strlen)(path) - 1;
  while (end > path && *end == '/') *end-- = '\0';
  slash = SELF_LIBC_NAME(strrchr)(path, '/');
  if (slash == NULL) return (char*)".";
  while (slash > path && *slash == '/') --slash;
  if (slash == path && *slash == '/') {
    slash[1] = '\0';
  } else {
    slash[1] = '\0';
  }
  return path;
}

/* Checked fortify-like helpers. */
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__memcpy_chk)(void* d, const void* s,
                                                    size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(memcpy)(d, s, n);
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__memmove_chk)(void* d, const void* s,
                                                     size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(memmove)(d, s, n);
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__memset_chk)(void* d, int c, size_t n,
                                                    size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(memset)(d, c, n);
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__mempcpy_chk)(void* d, const void* s,
                                                     size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(mempcpy)(d, s, n);
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__memchr_chk)(const void* s, int c,
                                                    size_t n, size_t slen) {
  if (n > slen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(memchr)(s, c, n);
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(__memrchr_chk)(const void* s, int c,
                                                     size_t n, size_t slen) {
  if (n > slen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(memrchr)(s, c, n);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(__strlen_chk)(const char* s,
                                                     size_t slen) {
  size_t i;
  for (i = 0; i < slen; ++i) if (s[i] == '\0') return i;
  SELF_LIBC_TRAP();
  return 0;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strcpy_chk)(char* d, const char* s,
                                                    size_t dlen) {
  if (SELF_LIBC_NAME(strlen)(s) + 1 > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(strcpy)(d, s);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__stpcpy_chk)(char* d, const char* s,
                                                    size_t dlen) {
  if (SELF_LIBC_NAME(strlen)(s) + 1 > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(stpcpy)(d, s);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strncpy_chk)(char* d, const char* s,
                                                     size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(strncpy)(d, s, n);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__stpncpy_chk)(char* d, const char* s,
                                                     size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(stpncpy)(d, s, n);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strncpy_chk2)(char* d, const char* s,
                                                      size_t n, size_t dlen,
                                                      size_t slen) {
  size_t i;
  if (n > dlen) SELF_LIBC_TRAP();
  for (i = 0; i < n; ++i) {
    if (i >= slen) SELF_LIBC_TRAP();
    d[i] = s[i];
    if (s[i] == '\0') {
      while (++i < n) d[i] = '\0';
      return d;
    }
  }
  return d;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strcat_chk)(char* d, const char* s,
                                                    size_t dlen) {
  size_t dl = SELF_LIBC_NAME(strlen)(d);
  size_t sl = SELF_LIBC_NAME(strlen)(s);
  if (dl + sl + 1 > dlen) SELF_LIBC_TRAP();
  SELF_LIBC_NAME(memcpy)(d + dl, s, sl + 1);
  return d;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strncat_chk)(char* d, const char* s,
                                                     size_t n, size_t dlen) {
  size_t dl = SELF_LIBC_NAME(strlen)(d);
  size_t sl = SELF_LIBC_NAME(strnlen)(s, n);
  if (dl + sl + 1 > dlen) SELF_LIBC_TRAP();
  SELF_LIBC_NAME(memcpy)(d + dl, s, sl);
  d[dl + sl] = '\0';
  return d;
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(__strlcpy_chk)(char* d, const char* s,
                                                      size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(strlcpy)(d, s, n);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(__strlcat_chk)(char* d, const char* s,
                                                      size_t n, size_t dlen) {
  if (n > dlen) SELF_LIBC_TRAP();
  return SELF_LIBC_NAME(strlcat)(d, s, n);
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strchr_chk)(const char* s, int c,
                                                    size_t slen) {
  size_t i;
  for (i = 0; i < slen; ++i) {
    if (s[i] == (char)c) return (char*)(s + i);
    if (s[i] == '\0') return NULL;
  }
  SELF_LIBC_TRAP();
  return NULL;
}
SELF_LIBC_INLINE char* SELF_LIBC_NAME(__strrchr_chk)(const char* s, int c,
                                                     size_t slen) {
  const char* last = NULL;
  size_t i;
  for (i = 0; i < slen; ++i) {
    if (s[i] == (char)c) last = s + i;
    if (s[i] == '\0') return (char*)last;
  }
  SELF_LIBC_TRAP();
  return NULL;
}
/* ASCII ctype.  Locale variants ignore locale, matching Android's C locale. */
SELF_LIBC_INLINE int SELF_LIBC_NAME(isascii)(int c) { return (unsigned)c <= 0x7fU; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isblank)(int c) { return c == ' ' || c == '\t'; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(iscntrl)(int c) { return (c >= 0 && c < 0x20) || c == 0x7f; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isdigit)(int c) { return c >= '0' && c <= '9'; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(islower)(int c) { return c >= 'a' && c <= 'z'; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isupper)(int c) { return c >= 'A' && c <= 'Z'; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isalpha)(int c) { return SELF_LIBC_NAME(islower)(c) || SELF_LIBC_NAME(isupper)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isalnum)(int c) { return SELF_LIBC_NAME(isalpha)(c) || SELF_LIBC_NAME(isdigit)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isspace)(int c) { return SELF_LIBC_NAME(internal_isspace)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isxdigit)(int c) { return SELF_LIBC_NAME(isdigit)(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isgraph)(int c) { return c >= 0x21 && c <= 0x7e; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isprint)(int c) { return c >= 0x20 && c <= 0x7e; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(ispunct)(int c) { return SELF_LIBC_NAME(isgraph)(c) && !SELF_LIBC_NAME(isalnum)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(toascii)(int c) { return c & 0x7f; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(tolower)(int c) { return SELF_LIBC_NAME(internal_tolower)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(toupper)(int c) { return SELF_LIBC_NAME(internal_toupper)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(_tolower)(int c) { return SELF_LIBC_NAME(internal_tolower)(c); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(_toupper)(int c) { return SELF_LIBC_NAME(internal_toupper)(c); }
#define SELF_LIBC_CTYPE_L(name) \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(name##_l)(int c, const void* l) { \
    (void)l; \
    return SELF_LIBC_NAME(name)(c); \
  }
SELF_LIBC_CTYPE_L(isalnum) SELF_LIBC_CTYPE_L(isalpha) SELF_LIBC_CTYPE_L(isblank)
SELF_LIBC_CTYPE_L(iscntrl) SELF_LIBC_CTYPE_L(isdigit) SELF_LIBC_CTYPE_L(isgraph)
SELF_LIBC_CTYPE_L(islower) SELF_LIBC_CTYPE_L(isprint) SELF_LIBC_CTYPE_L(ispunct)
SELF_LIBC_CTYPE_L(isspace) SELF_LIBC_CTYPE_L(isupper) SELF_LIBC_CTYPE_L(isxdigit)
SELF_LIBC_CTYPE_L(tolower) SELF_LIBC_CTYPE_L(toupper)
#undef SELF_LIBC_CTYPE_L

/* Integer conversion and arithmetic. */
SELF_LIBC_INLINE uintmax_t SELF_LIBC_NAME(internal_parse_int)(
    const char* nptr, char** endptr, int base, int* negative,
    uintmax_t pos_limit, uintmax_t neg_limit, int* overflow) {
  const char* orig = nptr;
  const char* s = nptr;
  uintmax_t limit;
  uintmax_t value = 0;
  int neg = 0;
  int any = 0;
  int ov = 0;
  while (SELF_LIBC_NAME(internal_isspace)((unsigned char)*s)) ++s;
  if (*s == '+' || *s == '-') neg = (*s++ == '-');
  if (base == 0) {
    if (s[0] == '0') {
      if ((s[1] == 'x' || s[1] == 'X') && SELF_LIBC_NAME(internal_digit)((unsigned char)s[2]) >= 0 && SELF_LIBC_NAME(internal_digit)((unsigned char)s[2]) < 16) {
        base = 16;
        s += 2;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  } else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X') && SELF_LIBC_NAME(internal_digit)((unsigned char)s[2]) >= 0 && SELF_LIBC_NAME(internal_digit)((unsigned char)s[2]) < 16) {
    s += 2;
  }
  if (base < 2 || base > 36) {
    if (endptr != NULL) *endptr = (char*)orig;
    if (negative != NULL) *negative = 0;
    if (overflow != NULL) *overflow = 0;
    return 0;
  }
  limit = neg ? neg_limit : pos_limit;
  for (;;) {
    int d = SELF_LIBC_NAME(internal_digit)((unsigned char)*s);
    if (d < 0 || d >= base) break;
    any = 1;
    if (value > (limit - (uintmax_t)d) / (uintmax_t)base) {
      ov = 1;
      value = limit;
    } else if (!ov) {
      value = value * (uintmax_t)base + (uintmax_t)d;
    }
    ++s;
  }
  if (!any) {
    if (endptr != NULL) *endptr = (char*)orig;
    neg = 0;
    ov = 0;
    value = 0;
  } else if (endptr != NULL) {
    *endptr = (char*)s;
  }
  if (negative != NULL) *negative = neg;
  if (overflow != NULL) *overflow = ov;
  if (ov) SELF_LIBC_SET_ERRNO(ERANGE);
  return value;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(abs)(int n) { return n < 0 ? -n : n; }
SELF_LIBC_INLINE long SELF_LIBC_NAME(labs)(long n) { return n < 0 ? -n : n; }
SELF_LIBC_INLINE long long SELF_LIBC_NAME(llabs)(long long n) { return n < 0 ? -n : n; }
SELF_LIBC_INLINE intmax_t SELF_LIBC_NAME(imaxabs)(intmax_t n) { return n < 0 ? -n : n; }
SELF_LIBC_INLINE div_t SELF_LIBC_NAME(div)(int n, int d) { div_t r; r.quot = n / d; r.rem = n % d; return r; }
SELF_LIBC_INLINE ldiv_t SELF_LIBC_NAME(ldiv)(long n, long d) { ldiv_t r; r.quot = n / d; r.rem = n % d; return r; }
SELF_LIBC_INLINE lldiv_t SELF_LIBC_NAME(lldiv)(long long n, long long d) { lldiv_t r; r.quot = n / d; r.rem = n % d; return r; }
SELF_LIBC_INLINE imaxdiv_t SELF_LIBC_NAME(imaxdiv)(intmax_t n, intmax_t d) { imaxdiv_t r; r.quot = n / d; r.rem = n % d; return r; }
SELF_LIBC_INLINE uintmax_t SELF_LIBC_NAME(strtoumax)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, UINTMAX_MAX, UINTMAX_MAX, &ov);
  if (ov) return UINTMAX_MAX;
  return neg ? (uintmax_t)(0 - v) : v;
}
SELF_LIBC_INLINE intmax_t SELF_LIBC_NAME(strtoimax)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t nl = (uintmax_t)INTMAX_MAX + 1U;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, (uintmax_t)INTMAX_MAX, nl, &ov);
  if (ov) return neg ? INTMAX_MIN : INTMAX_MAX;
  if (!neg) return (intmax_t)v;
  return v == nl ? INTMAX_MIN : -(intmax_t)v;
}
SELF_LIBC_INLINE unsigned long long SELF_LIBC_NAME(strtoull)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, (uintmax_t)ULLONG_MAX, (uintmax_t)ULLONG_MAX, &ov);
  if (ov) return ULLONG_MAX;
  return neg ? (unsigned long long)(0ULL - (unsigned long long)v) : (unsigned long long)v;
}
SELF_LIBC_INLINE long long SELF_LIBC_NAME(strtoll)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t nl = (uintmax_t)LLONG_MAX + 1U;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, (uintmax_t)LLONG_MAX, nl, &ov);
  if (ov) return neg ? LLONG_MIN : LLONG_MAX;
  if (!neg) return (long long)v;
  return v == nl ? LLONG_MIN : -(long long)v;
}
SELF_LIBC_INLINE unsigned long SELF_LIBC_NAME(strtoul)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, (uintmax_t)ULONG_MAX, (uintmax_t)ULONG_MAX, &ov);
  if (ov) return ULONG_MAX;
  return neg ? (unsigned long)(0UL - (unsigned long)v) : (unsigned long)v;
}
SELF_LIBC_INLINE long SELF_LIBC_NAME(strtol)(const char* s, char** e, int b) {
  int neg, ov;
  uintmax_t nl = (uintmax_t)LONG_MAX + 1U;
  uintmax_t v = SELF_LIBC_NAME(internal_parse_int)(s, e, b, &neg, (uintmax_t)LONG_MAX, nl, &ov);
  if (ov) return neg ? LONG_MIN : LONG_MAX;
  if (!neg) return (long)v;
  return v == nl ? LONG_MIN : -(long)v;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(atoi)(const char* s) { return (int)SELF_LIBC_NAME(strtol)(s, NULL, 10); }
SELF_LIBC_INLINE long SELF_LIBC_NAME(atol)(const char* s) { return SELF_LIBC_NAME(strtol)(s, NULL, 10); }
SELF_LIBC_INLINE long long SELF_LIBC_NAME(atoll)(const char* s) { return SELF_LIBC_NAME(strtoll)(s, NULL, 10); }
SELF_LIBC_INLINE long SELF_LIBC_NAME(strtol_l)(const char* s, char** e, int b, const void* l) { (void)l; return SELF_LIBC_NAME(strtol)(s, e, b); }
SELF_LIBC_INLINE unsigned long SELF_LIBC_NAME(strtoul_l)(const char* s, char** e, int b, const void* l) { (void)l; return SELF_LIBC_NAME(strtoul)(s, e, b); }
SELF_LIBC_INLINE long long SELF_LIBC_NAME(strtoll_l)(const char* s, char** e, int b, const void* l) { (void)l; return SELF_LIBC_NAME(strtoll)(s, e, b); }
SELF_LIBC_INLINE unsigned long long SELF_LIBC_NAME(strtoull_l)(const char* s, char** e, int b, const void* l) { (void)l; return SELF_LIBC_NAME(strtoull)(s, e, b); }
SELF_LIBC_INLINE uint16_t SELF_LIBC_NAME(htons)(uint16_t x) { return (uint16_t)((x << 8) | (x >> 8)); }
SELF_LIBC_INLINE uint16_t SELF_LIBC_NAME(ntohs)(uint16_t x) { return SELF_LIBC_NAME(htons)(x); }
SELF_LIBC_INLINE uint32_t SELF_LIBC_NAME(htonl)(uint32_t x) { return ((x & 0xffU) << 24) | ((x & 0xff00U) << 8) | ((x & 0xff0000U) >> 8) | ((x & 0xff000000U) >> 24); }
SELF_LIBC_INLINE uint32_t SELF_LIBC_NAME(ntohl)(uint32_t x) { return SELF_LIBC_NAME(htonl)(x); }
#define SELF_LIBC_FFS(type, utype, name)                    \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(name)(type v) {       \
    utype x = (utype)v;                                     \
    int bit = 1;                                            \
    if (x == 0) return 0;                                   \
    while ((x & (utype)1) == 0) { x >>= 1; ++bit; }          \
    return bit;                                             \
  }
SELF_LIBC_FFS(int, unsigned int, ffs)
SELF_LIBC_FFS(long, unsigned long, ffsl)
SELF_LIBC_FFS(long long, unsigned long long, ffsll)
#undef SELF_LIBC_FFS

/* Search/sort. */
SELF_LIBC_INLINE void* SELF_LIBC_NAME(bsearch)(const void* key, const void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*)) {
  const unsigned char* b = (const unsigned char*)base;
  while (nmemb != 0) {
    size_t mid = nmemb / 2;
    const unsigned char* e = b + mid * size;
    int r = cmp(key, e);
    if (r == 0) return (void*)e;
    if (r > 0) { b = e + size; nmemb -= mid + 1; } else { nmemb = mid; }
  }
  return NULL;
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(qsort)(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*)) {
  unsigned char* b = (unsigned char*)base;
  size_t i;
  if (size == 0) return;
  for (i = 1; i < nmemb; ++i) {
    size_t j = i;
    while (j > 0 && cmp(b + (j - 1) * size, b + j * size) > 0) {
      SELF_LIBC_NAME(internal_swap)(b + (j - 1) * size, b + j * size, size);
      --j;
    }
  }
}
SELF_LIBC_INLINE void SELF_LIBC_NAME(qsort_r)(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*, void*), void* arg) {
  unsigned char* b = (unsigned char*)base;
  size_t i;
  if (size == 0) return;
  for (i = 1; i < nmemb; ++i) {
    size_t j = i;
    while (j > 0 && cmp(b + (j - 1) * size, b + j * size, arg) > 0) {
      SELF_LIBC_NAME(internal_swap)(b + (j - 1) * size, b + j * size, size);
      --j;
    }
  }
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(lfind)(const void* key, const void* base, size_t* nmemb, size_t size, int (*cmp)(const void*, const void*)) {
  const unsigned char* b = (const unsigned char*)base;
  size_t i;
  for (i = 0; i < *nmemb; ++i) if (cmp(key, b + i * size) == 0) return (void*)(b + i * size);
  return NULL;
}
SELF_LIBC_INLINE void* SELF_LIBC_NAME(lsearch)(const void* key, void* base, size_t* nmemb, size_t size, int (*cmp)(const void*, const void*)) {
  void* found = SELF_LIBC_NAME(lfind)(key, base, nmemb, size, cmp);
  unsigned char* e;
  if (found != NULL) return found;
  e = (unsigned char*)base + (*nmemb) * size;
  SELF_LIBC_NAME(memcpy)(e, key, size);
  ++*nmemb;
  return e;
}

/* Small deterministic PRNG. */
SELF_LIBC_INLINE unsigned int* SELF_LIBC_NAME(internal_rand_state)(void) { static unsigned int state = 1; return &state; }
SELF_LIBC_INLINE void SELF_LIBC_NAME(srand)(unsigned int seed) { *SELF_LIBC_NAME(internal_rand_state)() = seed; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(rand_r)(unsigned int* seedp) {
  unsigned int x = *seedp;
  int r;
  x = x * 1103515245U + 12345U; r = (int)((x >> 16) & 0x7ffU);
  x = x * 1103515245U + 12345U; r = (r << 10) ^ (int)((x >> 16) & 0x3ffU);
  x = x * 1103515245U + 12345U; r = (r << 10) ^ (int)((x >> 16) & 0x3ffU);
  *seedp = x;
  return r & 0x7fffffff;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(rand)(void) { return SELF_LIBC_NAME(rand_r)(SELF_LIBC_NAME(internal_rand_state)()); }
SELF_LIBC_INLINE void SELF_LIBC_NAME(srandom)(unsigned int seed) { SELF_LIBC_NAME(srand)(seed); }
SELF_LIBC_INLINE long SELF_LIBC_NAME(random)(void) { return (long)SELF_LIBC_NAME(rand)(); }

/* Floating classification; use compiler builtins when available. */
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnan)(double x) { return x != x; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnanf)(float x) { return x != x; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnanl)(long double x) { return x != x; }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isinf)(double x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isinf)
  return __builtin_isinf(x);
#else
  return !SELF_LIBC_NAME(isnan)(x) && x != 0.0 && x + x == x;
#endif
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(isinff)(float x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isinf)
  return __builtin_isinf(x);
#else
  return !SELF_LIBC_NAME(isnanf)(x) && x != 0.0f && x + x == x;
#endif
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(isinfl)(long double x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isinf)
  return __builtin_isinf(x);
#else
  return !SELF_LIBC_NAME(isnanl)(x) && x != 0.0L && x + x == x;
#endif
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(isfinite)(double x) { return !SELF_LIBC_NAME(isnan)(x) && !SELF_LIBC_NAME(isinf)(x); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isfinitef)(float x) { return !SELF_LIBC_NAME(isnanf)(x) && !SELF_LIBC_NAME(isinff)(x); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isfinitel)(long double x) { return !SELF_LIBC_NAME(isnanl)(x) && !SELF_LIBC_NAME(isinfl)(x); }
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnormal)(double x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isnormal)
  return __builtin_isnormal(x);
#else
  return SELF_LIBC_NAME(isfinite)(x) && x != 0.0;
#endif
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnormalf)(float x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isnormal)
  return __builtin_isnormal(x);
#else
  return SELF_LIBC_NAME(isfinitef)(x) && x != 0.0f;
#endif
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(isnormall)(long double x) {
#if SELF_LIBC_HAS_BUILTIN(__builtin_isnormal)
  return __builtin_isnormal(x);
#else
  return SELF_LIBC_NAME(isfinitel)(x) && x != 0.0L;
#endif
}
#define SELF_LIBC_FP_ALIAS(name) \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(__##name)(double x) { return SELF_LIBC_NAME(name)(x); } \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(__##name##f)(float x) { return SELF_LIBC_NAME(name##f)(x); } \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(__##name##l)(long double x) { return SELF_LIBC_NAME(name##l)(x); }
SELF_LIBC_FP_ALIAS(isfinite) SELF_LIBC_FP_ALIAS(isinf) SELF_LIBC_FP_ALIAS(isnan) SELF_LIBC_FP_ALIAS(isnormal)
#undef SELF_LIBC_FP_ALIAS
/* Wide-character memory and strings.  Wide case/ctype are ASCII-only. */
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wmemcpy)(wchar_t* d, const wchar_t* s,
                                                  size_t n) {
  wchar_t* r = d;
  while (n-- != 0) *d++ = *s++;
  return r;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wmempcpy)(wchar_t* d,
                                                   const wchar_t* s,
                                                   size_t n) {
  SELF_LIBC_NAME(wmemcpy)(d, s, n);
  return d + n;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wmemmove)(wchar_t* d,
                                                   const wchar_t* s,
                                                   size_t n) {
  return (wchar_t*)SELF_LIBC_NAME(memmove)(d, s, n * sizeof(wchar_t));
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wmemset)(wchar_t* d, wchar_t c,
                                                  size_t n) {
  wchar_t* r = d;
  while (n-- != 0) *d++ = c;
  return r;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wmemcmp)(const wchar_t* a,
                                             const wchar_t* b, size_t n) {
  while (n-- != 0) {
    if (*a != *b) return (*a > *b) - (*a < *b);
    ++a;
    ++b;
  }
  return 0;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wmemchr)(const wchar_t* s, wchar_t c,
                                                  size_t n) {
  while (n-- != 0) {
    if (*s == c) return (wchar_t*)s;
    ++s;
  }
  return NULL;
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcslen)(const wchar_t* s) {
  const wchar_t* p = s;
  while (*p != L'\0') ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcsnlen)(const wchar_t* s,
                                                size_t max_len) {
  size_t n = 0;
  while (n < max_len && s[n] != L'\0') ++n;
  return n;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcscmp)(const wchar_t* a,
                                            const wchar_t* b) {
  while (*a != L'\0' && *a == *b) {
    ++a;
    ++b;
  }
  return (*a > *b) - (*a < *b);
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcsncmp)(const wchar_t* a,
                                             const wchar_t* b, size_t n) {
  while (n-- != 0) {
    if (*a != *b) return (*a > *b) - (*a < *b);
    if (*a == L'\0') return 0;
    ++a;
    ++b;
  }
  return 0;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcpcpy)(wchar_t* d,
                                                 const wchar_t* s) {
  while ((*d = *s) != L'\0') {
    ++d;
    ++s;
  }
  return d;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcscpy)(wchar_t* d,
                                                 const wchar_t* s) {
  SELF_LIBC_NAME(wcpcpy)(d, s);
  return d;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcpncpy)(wchar_t* d,
                                                  const wchar_t* s, size_t n) {
  wchar_t* r = d;
  while (n != 0 && *s != L'\0') {
    *d++ = *s++;
    --n;
  }
  if (n == 0) return d;
  *d++ = L'\0';
  while (--n != 0) *d++ = L'\0';
  (void)r;
  return d - 1;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcsncpy)(wchar_t* d,
                                                  const wchar_t* s, size_t n) {
  SELF_LIBC_NAME(wcpncpy)(d, s, n);
  return d;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcscat)(wchar_t* d,
                                                 const wchar_t* s) {
  SELF_LIBC_NAME(wcpcpy)(d + SELF_LIBC_NAME(wcslen)(d), s);
  return d;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcsncat)(wchar_t* d,
                                                  const wchar_t* s, size_t n) {
  wchar_t* p = d + SELF_LIBC_NAME(wcslen)(d);
  while (n-- != 0 && *s != L'\0') *p++ = *s++;
  *p = L'\0';
  return d;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcschr)(const wchar_t* s, wchar_t c) {
  for (;;) {
    if (*s == c) return (wchar_t*)s;
    if (*s == L'\0') return NULL;
    ++s;
  }
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcsrchr)(const wchar_t* s,
                                                  wchar_t c) {
  const wchar_t* last = NULL;
  do {
    if (*s == c) last = s;
  } while (*s++ != L'\0');
  return (wchar_t*)last;
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcsspn)(const wchar_t* s,
                                               const wchar_t* accept) {
  const wchar_t* p = s;
  while (*p != L'\0' && SELF_LIBC_NAME(wcschr)(accept, *p) != NULL) ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcscspn)(const wchar_t* s,
                                                const wchar_t* reject) {
  const wchar_t* p = s;
  while (*p != L'\0' && SELF_LIBC_NAME(wcschr)(reject, *p) == NULL) ++p;
  return (size_t)(p - s);
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcspbrk)(const wchar_t* s,
                                                  const wchar_t* accept) {
  while (*s != L'\0') {
    if (SELF_LIBC_NAME(wcschr)(accept, *s) != NULL) return (wchar_t*)s;
    ++s;
  }
  return NULL;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcsstr)(const wchar_t* h,
                                                 const wchar_t* n) {
  size_t nlen;
  if (*n == L'\0') return (wchar_t*)h;
  nlen = SELF_LIBC_NAME(wcslen)(n);
  while ((h = SELF_LIBC_NAME(wcschr)(h, *n)) != NULL) {
    if (SELF_LIBC_NAME(wcsncmp)(h, n, nlen) == 0) return (wchar_t*)h;
    ++h;
  }
  return NULL;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcswcs)(const wchar_t* h,
                                                 const wchar_t* n) {
  return SELF_LIBC_NAME(wcsstr)(h, n);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcslcpy)(wchar_t* d, const wchar_t* s,
                                                size_t dsize) {
  const wchar_t* start = s;
  size_t left = dsize;
  if (left != 0) {
    while (--left != 0 && *s != L'\0') *d++ = *s++;
    if (left == 0) {
      if (dsize != 0) *d = L'\0';
      while (*s++ != L'\0') {}
    } else {
      *d = L'\0';
    }
  } else {
    while (*s++ != L'\0') {}
  }
  return (size_t)((s - start) - 1);
}
SELF_LIBC_INLINE size_t SELF_LIBC_NAME(wcslcat)(wchar_t* d, const wchar_t* s,
                                                size_t dsize) {
  size_t dl = SELF_LIBC_NAME(wcsnlen)(d, dsize);
  size_t sl = SELF_LIBC_NAME(wcslen)(s);
  if (dl == dsize) return dsize + sl;
  if (sl < dsize - dl) {
    SELF_LIBC_NAME(wmemcpy)(d + dl, s, sl + 1);
  } else {
    SELF_LIBC_NAME(wmemcpy)(d + dl, s, dsize - dl - 1);
    d[dsize - 1] = L'\0';
  }
  return dl + sl;
}
SELF_LIBC_INLINE wchar_t* SELF_LIBC_NAME(wcstok)(wchar_t* s,
                                                 const wchar_t* delim,
                                                 wchar_t** saveptr) {
  wchar_t* token;
  if (s == NULL) s = *saveptr;
  if (s == NULL) return NULL;
  s += SELF_LIBC_NAME(wcsspn)(s, delim);
  if (*s == L'\0') { *saveptr = NULL; return NULL; }
  token = s;
  s = SELF_LIBC_NAME(wcspbrk)(token, delim);
  if (s == NULL) {
    *saveptr = NULL;
  } else {
    *s++ = L'\0';
    *saveptr = s;
  }
  return token;
}
SELF_LIBC_INLINE wint_t SELF_LIBC_NAME(towlower)(wint_t wc) {
  return (wc >= L'A' && wc <= L'Z') ? wc + (L'a' - L'A') : wc;
}
SELF_LIBC_INLINE wint_t SELF_LIBC_NAME(towupper)(wint_t wc) {
  return (wc >= L'a' && wc <= L'z') ? wc - (L'a' - L'A') : wc;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcscasecmp)(const wchar_t* a,
                                                const wchar_t* b) {
  while (*a != L'\0') {
    wint_t ac = SELF_LIBC_NAME(towlower)(*a);
    wint_t bc = SELF_LIBC_NAME(towlower)(*b);
    if (ac != bc) return (ac > bc) - (ac < bc);
    ++a;
    ++b;
  }
  return (SELF_LIBC_NAME(towlower)(*a) > SELF_LIBC_NAME(towlower)(*b)) -
         (SELF_LIBC_NAME(towlower)(*a) < SELF_LIBC_NAME(towlower)(*b));
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcsncasecmp)(const wchar_t* a,
                                                 const wchar_t* b, size_t n) {
  while (n-- != 0) {
    wint_t ac = SELF_LIBC_NAME(towlower)(*a);
    wint_t bc = SELF_LIBC_NAME(towlower)(*b);
    if (ac != bc) return (ac > bc) - (ac < bc);
    if (*a == L'\0') return 0;
    ++a;
    ++b;
  }
  return 0;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcscasecmp_l)(const wchar_t* a,
                                                  const wchar_t* b,
                                                  const void* l) {
  (void)l;
  return SELF_LIBC_NAME(wcscasecmp)(a, b);
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wcsncasecmp_l)(const wchar_t* a,
                                                   const wchar_t* b, size_t n,
                                                   const void* l) {
  (void)l;
  return SELF_LIBC_NAME(wcsncasecmp)(a, b, n);
}
SELF_LIBC_INLINE wint_t SELF_LIBC_NAME(btowc)(int c) {
  return c == EOF ? WEOF : (wint_t)(unsigned char)c;
}
SELF_LIBC_INLINE int SELF_LIBC_NAME(wctob)(wint_t wc) {
  return (wc >= 0 && wc <= UCHAR_MAX) ? (int)wc : EOF;
}
#define SELF_LIBC_WCTYPE(name, narrow) \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(name)(wint_t wc) { \
    return (wc >= 0 && wc <= UCHAR_MAX) ? SELF_LIBC_NAME(narrow)((int)wc) : 0; \
  } \
  SELF_LIBC_INLINE int SELF_LIBC_NAME(name##_l)(wint_t wc, const void* l) { \
    (void)l; \
    return SELF_LIBC_NAME(name)(wc); \
  }
SELF_LIBC_WCTYPE(iswalnum, isalnum) SELF_LIBC_WCTYPE(iswalpha, isalpha)
SELF_LIBC_WCTYPE(iswblank, isblank) SELF_LIBC_WCTYPE(iswcntrl, iscntrl)
SELF_LIBC_WCTYPE(iswdigit, isdigit) SELF_LIBC_WCTYPE(iswgraph, isgraph)
SELF_LIBC_WCTYPE(iswlower, islower) SELF_LIBC_WCTYPE(iswprint, isprint)
SELF_LIBC_WCTYPE(iswpunct, ispunct) SELF_LIBC_WCTYPE(iswspace, isspace)
SELF_LIBC_WCTYPE(iswupper, isupper) SELF_LIBC_WCTYPE(iswxdigit, isxdigit)
#undef SELF_LIBC_WCTYPE
SELF_LIBC_INLINE wint_t SELF_LIBC_NAME(towlower_l)(wint_t wc, const void* l) { (void)l; return SELF_LIBC_NAME(towlower)(wc); }
SELF_LIBC_INLINE wint_t SELF_LIBC_NAME(towupper_l)(wint_t wc, const void* l) { (void)l; return SELF_LIBC_NAME(towupper)(wc); }

#ifdef __cplusplus
}  /* extern "C" */
#endif
#ifdef SELF_LIBC_REMAP_STANDARD_NAMES
/* Some host C libraries expose common libc names as macros. */
#undef isalnum
#undef isalpha
#undef isascii
#undef isblank
#undef iscntrl
#undef isdigit
#undef isfinite
#undef isgraph
#undef isinf
#undef islower
#undef isnan
#undef isnormal
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef iswalnum
#undef iswalpha
#undef iswblank
#undef iswcntrl
#undef iswdigit
#undef iswgraph
#undef iswlower
#undef iswprint
#undef iswpunct
#undef iswspace
#undef iswupper
#undef iswxdigit
#undef isxdigit
#undef strcasecmp
#undef strncasecmp
#undef towlower
#undef towupper
#undef wcscasecmp
#undef wcsncasecmp
#undef wcswcs
#undef basename
#undef dirname
#undef __gnu_basename
#define abs SELF_LIBC_NAME(abs)
#define atoi SELF_LIBC_NAME(atoi)
#define atol SELF_LIBC_NAME(atol)
#define atoll SELF_LIBC_NAME(atoll)
#define basename SELF_LIBC_NAME(basename)
#define bcopy SELF_LIBC_NAME(bcopy)
#define bsearch SELF_LIBC_NAME(bsearch)
#define btowc SELF_LIBC_NAME(btowc)
#define bzero SELF_LIBC_NAME(bzero)
#define div SELF_LIBC_NAME(div)
#define dirname SELF_LIBC_NAME(dirname)
#define ffs SELF_LIBC_NAME(ffs)
#define ffsl SELF_LIBC_NAME(ffsl)
#define ffsll SELF_LIBC_NAME(ffsll)
#define htonl SELF_LIBC_NAME(htonl)
#define htons SELF_LIBC_NAME(htons)
#define imaxabs SELF_LIBC_NAME(imaxabs)
#define imaxdiv SELF_LIBC_NAME(imaxdiv)
#define index SELF_LIBC_NAME(index)
#define isalnum SELF_LIBC_NAME(isalnum)
#define isalnum_l SELF_LIBC_NAME(isalnum_l)
#define isalpha SELF_LIBC_NAME(isalpha)
#define isalpha_l SELF_LIBC_NAME(isalpha_l)
#define isascii SELF_LIBC_NAME(isascii)
#define isblank SELF_LIBC_NAME(isblank)
#define isblank_l SELF_LIBC_NAME(isblank_l)
#define iscntrl SELF_LIBC_NAME(iscntrl)
#define iscntrl_l SELF_LIBC_NAME(iscntrl_l)
#define isdigit SELF_LIBC_NAME(isdigit)
#define isdigit_l SELF_LIBC_NAME(isdigit_l)
#define isfinite SELF_LIBC_NAME(isfinite)
#define isfinitef SELF_LIBC_NAME(isfinitef)
#define isfinitel SELF_LIBC_NAME(isfinitel)
#define isgraph SELF_LIBC_NAME(isgraph)
#define isgraph_l SELF_LIBC_NAME(isgraph_l)
#define isinf SELF_LIBC_NAME(isinf)
#define isinff SELF_LIBC_NAME(isinff)
#define isinfl SELF_LIBC_NAME(isinfl)
#define islower SELF_LIBC_NAME(islower)
#define islower_l SELF_LIBC_NAME(islower_l)
#define isnan SELF_LIBC_NAME(isnan)
#define isnanf SELF_LIBC_NAME(isnanf)
#define isnanl SELF_LIBC_NAME(isnanl)
#define isnormal SELF_LIBC_NAME(isnormal)
#define isnormalf SELF_LIBC_NAME(isnormalf)
#define isnormall SELF_LIBC_NAME(isnormall)
#define isprint SELF_LIBC_NAME(isprint)
#define isprint_l SELF_LIBC_NAME(isprint_l)
#define ispunct SELF_LIBC_NAME(ispunct)
#define ispunct_l SELF_LIBC_NAME(ispunct_l)
#define isspace SELF_LIBC_NAME(isspace)
#define isspace_l SELF_LIBC_NAME(isspace_l)
#define isupper SELF_LIBC_NAME(isupper)
#define isupper_l SELF_LIBC_NAME(isupper_l)
#define iswalnum SELF_LIBC_NAME(iswalnum)
#define iswalnum_l SELF_LIBC_NAME(iswalnum_l)
#define iswalpha SELF_LIBC_NAME(iswalpha)
#define iswalpha_l SELF_LIBC_NAME(iswalpha_l)
#define iswblank SELF_LIBC_NAME(iswblank)
#define iswblank_l SELF_LIBC_NAME(iswblank_l)
#define iswcntrl SELF_LIBC_NAME(iswcntrl)
#define iswcntrl_l SELF_LIBC_NAME(iswcntrl_l)
#define iswdigit SELF_LIBC_NAME(iswdigit)
#define iswdigit_l SELF_LIBC_NAME(iswdigit_l)
#define iswgraph SELF_LIBC_NAME(iswgraph)
#define iswgraph_l SELF_LIBC_NAME(iswgraph_l)
#define iswlower SELF_LIBC_NAME(iswlower)
#define iswlower_l SELF_LIBC_NAME(iswlower_l)
#define iswprint SELF_LIBC_NAME(iswprint)
#define iswprint_l SELF_LIBC_NAME(iswprint_l)
#define iswpunct SELF_LIBC_NAME(iswpunct)
#define iswpunct_l SELF_LIBC_NAME(iswpunct_l)
#define iswspace SELF_LIBC_NAME(iswspace)
#define iswspace_l SELF_LIBC_NAME(iswspace_l)
#define iswupper SELF_LIBC_NAME(iswupper)
#define iswupper_l SELF_LIBC_NAME(iswupper_l)
#define iswxdigit SELF_LIBC_NAME(iswxdigit)
#define iswxdigit_l SELF_LIBC_NAME(iswxdigit_l)
#define isxdigit SELF_LIBC_NAME(isxdigit)
#define isxdigit_l SELF_LIBC_NAME(isxdigit_l)
#define labs SELF_LIBC_NAME(labs)
#define ldiv SELF_LIBC_NAME(ldiv)
#define lfind SELF_LIBC_NAME(lfind)
#define lldiv SELF_LIBC_NAME(lldiv)
#define llabs SELF_LIBC_NAME(llabs)
#define lsearch SELF_LIBC_NAME(lsearch)
#define memccpy SELF_LIBC_NAME(memccpy)
#define memchr SELF_LIBC_NAME(memchr)
#define memcmp SELF_LIBC_NAME(memcmp)
#define memcpy SELF_LIBC_NAME(memcpy)
#define memmem SELF_LIBC_NAME(memmem)
#define memmove SELF_LIBC_NAME(memmove)
#define mempcpy SELF_LIBC_NAME(mempcpy)
#define memrchr SELF_LIBC_NAME(memrchr)
#define memset SELF_LIBC_NAME(memset)
#define memset_explicit SELF_LIBC_NAME(memset_explicit)
#define memswap SELF_LIBC_NAME(memswap)
#define ntohl SELF_LIBC_NAME(ntohl)
#define ntohs SELF_LIBC_NAME(ntohs)
#define qsort SELF_LIBC_NAME(qsort)
#define qsort_r SELF_LIBC_NAME(qsort_r)
#define rand SELF_LIBC_NAME(rand)
#define rand_r SELF_LIBC_NAME(rand_r)
#define random SELF_LIBC_NAME(random)
#define srand SELF_LIBC_NAME(srand)
#define srandom SELF_LIBC_NAME(srandom)
#define stpcpy SELF_LIBC_NAME(stpcpy)
#define stpncpy SELF_LIBC_NAME(stpncpy)
#define strcasecmp SELF_LIBC_NAME(strcasecmp)
#define strcasecmp_l SELF_LIBC_NAME(strcasecmp_l)
#define strcasestr SELF_LIBC_NAME(strcasestr)
#define strcat SELF_LIBC_NAME(strcat)
#define strchr SELF_LIBC_NAME(strchr)
#define strchrnul SELF_LIBC_NAME(strchrnul)
#define strcmp SELF_LIBC_NAME(strcmp)
#define strcpy SELF_LIBC_NAME(strcpy)
#define strcspn SELF_LIBC_NAME(strcspn)
#define strlcat SELF_LIBC_NAME(strlcat)
#define strlcpy SELF_LIBC_NAME(strlcpy)
#define strlen SELF_LIBC_NAME(strlen)
#define strncasecmp SELF_LIBC_NAME(strncasecmp)
#define strncasecmp_l SELF_LIBC_NAME(strncasecmp_l)
#define strncat SELF_LIBC_NAME(strncat)
#define strncmp SELF_LIBC_NAME(strncmp)
#define strncpy SELF_LIBC_NAME(strncpy)
#define strnlen SELF_LIBC_NAME(strnlen)
#define strpbrk SELF_LIBC_NAME(strpbrk)
#define strrchr SELF_LIBC_NAME(strrchr)
#define strsep SELF_LIBC_NAME(strsep)
#define strspn SELF_LIBC_NAME(strspn)
#define strstr SELF_LIBC_NAME(strstr)
#define strtok SELF_LIBC_NAME(strtok)
#define strtok_r SELF_LIBC_NAME(strtok_r)
#define strtoimax SELF_LIBC_NAME(strtoimax)
#define strtol SELF_LIBC_NAME(strtol)
#define strtol_l SELF_LIBC_NAME(strtol_l)
#define strtoll SELF_LIBC_NAME(strtoll)
#define strtoll_l SELF_LIBC_NAME(strtoll_l)
#define strtoul SELF_LIBC_NAME(strtoul)
#define strtoul_l SELF_LIBC_NAME(strtoul_l)
#define strtoull SELF_LIBC_NAME(strtoull)
#define strtoull_l SELF_LIBC_NAME(strtoull_l)
#define strtoumax SELF_LIBC_NAME(strtoumax)
#define swab SELF_LIBC_NAME(swab)
#define toascii SELF_LIBC_NAME(toascii)
#define tolower SELF_LIBC_NAME(tolower)
#define tolower_l SELF_LIBC_NAME(tolower_l)
#define toupper SELF_LIBC_NAME(toupper)
#define toupper_l SELF_LIBC_NAME(toupper_l)
#define towlower SELF_LIBC_NAME(towlower)
#define towlower_l SELF_LIBC_NAME(towlower_l)
#define towupper SELF_LIBC_NAME(towupper)
#define towupper_l SELF_LIBC_NAME(towupper_l)
#define wcpcpy SELF_LIBC_NAME(wcpcpy)
#define wcpncpy SELF_LIBC_NAME(wcpncpy)
#define wcscasecmp SELF_LIBC_NAME(wcscasecmp)
#define wcscasecmp_l SELF_LIBC_NAME(wcscasecmp_l)
#define wcscat SELF_LIBC_NAME(wcscat)
#define wcschr SELF_LIBC_NAME(wcschr)
#define wcscmp SELF_LIBC_NAME(wcscmp)
#define wcscpy SELF_LIBC_NAME(wcscpy)
#define wcscspn SELF_LIBC_NAME(wcscspn)
#define wcslcat SELF_LIBC_NAME(wcslcat)
#define wcslcpy SELF_LIBC_NAME(wcslcpy)
#define wcslen SELF_LIBC_NAME(wcslen)
#define wcsncasecmp SELF_LIBC_NAME(wcsncasecmp)
#define wcsncasecmp_l SELF_LIBC_NAME(wcsncasecmp_l)
#define wcsncat SELF_LIBC_NAME(wcsncat)
#define wcsncmp SELF_LIBC_NAME(wcsncmp)
#define wcsncpy SELF_LIBC_NAME(wcsncpy)
#define wcsnlen SELF_LIBC_NAME(wcsnlen)
#define wcspbrk SELF_LIBC_NAME(wcspbrk)
#define wcsrchr SELF_LIBC_NAME(wcsrchr)
#define wcsspn SELF_LIBC_NAME(wcsspn)
#define wcsstr SELF_LIBC_NAME(wcsstr)
#define wcstok SELF_LIBC_NAME(wcstok)
#define wcswcs SELF_LIBC_NAME(wcswcs)
#define wctob SELF_LIBC_NAME(wctob)
#define wmemchr SELF_LIBC_NAME(wmemchr)
#define wmemcmp SELF_LIBC_NAME(wmemcmp)
#define wmemcpy SELF_LIBC_NAME(wmemcpy)
#define wmemmove SELF_LIBC_NAME(wmemmove)
#define wmempcpy SELF_LIBC_NAME(wmempcpy)
#define wmemset SELF_LIBC_NAME(wmemset)
#define _tolower SELF_LIBC_NAME(_tolower)
#define _toupper SELF_LIBC_NAME(_toupper)
#define __aeabi_memclr SELF_LIBC_NAME(__aeabi_memclr)
#define __aeabi_memclr4 SELF_LIBC_NAME(__aeabi_memclr4)
#define __aeabi_memclr8 SELF_LIBC_NAME(__aeabi_memclr8)
#define __aeabi_memcpy SELF_LIBC_NAME(__aeabi_memcpy)
#define __aeabi_memcpy4 SELF_LIBC_NAME(__aeabi_memcpy4)
#define __aeabi_memcpy8 SELF_LIBC_NAME(__aeabi_memcpy8)
#define __aeabi_memmove SELF_LIBC_NAME(__aeabi_memmove)
#define __aeabi_memmove4 SELF_LIBC_NAME(__aeabi_memmove4)
#define __aeabi_memmove8 SELF_LIBC_NAME(__aeabi_memmove8)
#define __aeabi_memset SELF_LIBC_NAME(__aeabi_memset)
#define __aeabi_memset4 SELF_LIBC_NAME(__aeabi_memset4)
#define __aeabi_memset8 SELF_LIBC_NAME(__aeabi_memset8)
#define __gnu_basename SELF_LIBC_NAME(__gnu_basename)
#define __isfinite SELF_LIBC_NAME(__isfinite)
#define __isfinitef SELF_LIBC_NAME(__isfinitef)
#define __isfinitel SELF_LIBC_NAME(__isfinitel)
#define __isinf SELF_LIBC_NAME(__isinf)
#define __isinff SELF_LIBC_NAME(__isinff)
#define __isinfl SELF_LIBC_NAME(__isinfl)
#define __isnan SELF_LIBC_NAME(__isnan)
#define __isnanf SELF_LIBC_NAME(__isnanf)
#define __isnanl SELF_LIBC_NAME(__isnanl)
#define __isnormal SELF_LIBC_NAME(__isnormal)
#define __isnormalf SELF_LIBC_NAME(__isnormalf)
#define __isnormall SELF_LIBC_NAME(__isnormall)
#define __memchr_chk SELF_LIBC_NAME(__memchr_chk)
#define __memcpy_chk SELF_LIBC_NAME(__memcpy_chk)
#define __memmove_chk SELF_LIBC_NAME(__memmove_chk)
#define __mempcpy_chk SELF_LIBC_NAME(__mempcpy_chk)
#define __memrchr_chk SELF_LIBC_NAME(__memrchr_chk)
#define __memset_chk SELF_LIBC_NAME(__memset_chk)
#define __stpcpy_chk SELF_LIBC_NAME(__stpcpy_chk)
#define __stpncpy_chk SELF_LIBC_NAME(__stpncpy_chk)
#define __strcat_chk SELF_LIBC_NAME(__strcat_chk)
#define __strchr_chk SELF_LIBC_NAME(__strchr_chk)
#define __strcpy_chk SELF_LIBC_NAME(__strcpy_chk)
#define __strlcat_chk SELF_LIBC_NAME(__strlcat_chk)
#define __strlcpy_chk SELF_LIBC_NAME(__strlcpy_chk)
#define __strlen_chk SELF_LIBC_NAME(__strlen_chk)
#define __strncat_chk SELF_LIBC_NAME(__strncat_chk)
#define __strncpy_chk SELF_LIBC_NAME(__strncpy_chk)
#define __strncpy_chk2 SELF_LIBC_NAME(__strncpy_chk2)
#define __strrchr_chk SELF_LIBC_NAME(__strrchr_chk)
#endif  /* SELF_LIBC_REMAP_STANDARD_NAMES */

#endif  /* SELF_LIBC_SUPPORT_H_ */
