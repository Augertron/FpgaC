/* 
 * libgen.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * String functions.
 *
 */

 
#ifndef _LIBGEN_H_
#define _LIBGEN_H_

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

/* All the headers include this file. */
#include <_mingw.h>
#ifndef RC_INVOKED
#ifdef __cplusplus
extern "C" {
#endif

/*extern char * __cdecl _dirname (char *);*/
extern char * __cdecl _basename (char *);

/* If we don"t include these prototypes, only the underscored names
   will be available.  */
/*extern char * __cdecl dirname (char *)  __MINGW_ATTRIB_WEAK;*/
extern char * __cdecl basename (char *) /*__MINGW_ATTRIB_WEAK */;

#ifdef __cplusplus
}
#endif
#endif	/* Not RC_INVOKED */
#endif	/* Not _LIBGEN_H_ */
