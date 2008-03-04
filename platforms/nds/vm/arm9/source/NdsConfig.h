 /*
 *   Copyright (C) 1996-2005 by Ian Piumarta and other authors/contributors
 *                              listed elsewhere in this file.
 *
 * This program is free software.  You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * See the file COPYRIGHT for more details.
 */

#ifndef __sq_config_h
#define __sq_config_h

/* explicit image width */

#define	HAVE_INTERP_H 1

/* package options */

#undef	USE_X11
#undef	USE_X11_GLX
#undef	USE_QUARTZ
#undef	USE_QUARTZ_CGL
#undef	USE_RFB

/* libraries */

#undef	HAVE_LIBX11
#undef	HAVE_LIBXEXT
#undef	HAVE_LIBDL
#undef	HAVE_DYLD
#undef	HAVE_LIBFFI
#undef	HAVE_ICONV

#undef	USE_AUDIO_NONE
#undef	USE_AUDIO_SUN
#undef	USE_AUDIO_NAS
#undef	USE_AUDIO_OSS
#undef	USE_AUDIO_MACOSX
#undef	OSS_DEVICE

/* header files */

#undef	HAVE_UNISTD_H
#undef	NEED_GETHOSTNAME_P

#undef	HAVE_DIRENT_H
#undef	HAVE_SYS_NDIR_H
#undef	HAVE_SYS_DIR_H
#undef	HAVE_NDIR_H
#undef	HAVE_DLFCN_H
#undef	HAVE_ICONV_H

#undef	HAVE_SYS_TIME_H
#undef	TIME_WITH_SYS_TIME

#undef	HAVE_SYS_FILIO_H

#undef	HAVE_SYS_AUDIOIO_H
#undef	HAVE_SUN_AUDIOIO_H

#undef	HAVE_PTY_H
#undef	HAVE_UTIL_H
#undef	HAVE_LIBUTIL_H
#undef	HAVE_STROPTS_H

#undef	HAVE_GL_GL_H
#undef	HAVE_OPENGL_GL_H

#undef	NEED_SUNOS_H

/* system calls/library functions */

#undef	AT_EXIT

#undef	HAVE_TZSET

#undef	HAVE_OPENPTY
#undef	HAVE_UNIX98_PTYS

#undef	HAVE_SNPRINTF
#undef	HAVE___SNPRINTF

#undef	HAVE_MMAP

#undef	HAVE_DYLD

#undef	HAVE_LANGINFO_CODESET

#undef	HAVE_ALLOCA
#undef	HAVE_ALLOCA_H

#undef	HAVE_UNSETENV

#undef	HAVE_NANOSLEEP

/* widths of primitive types */

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_VOID_P 4

/* structures */

#undef	HAVE_TM_GMTOFF
#undef	HAVE_TIMEZONE

/* typedefs */

#undef	socklen_t

#undef	squeakInt64

/* architecture */

#undef	OS_TYPE

#undef	VM_HOST
#undef	VM_HOST_CPU
#undef	VM_HOST_VENDOR
#undef	VM_HOST_OS
#undef	VM_BUILD_STRING


/* damage containment */

#undef	DARWIN

#ifdef NEED_SUNOS_H
# include "sunos.h"
#endif

/* other configured variables */

#undef SQ_VERSION
#undef VM_VERSION
#undef VM_LIBDIR
#undef VM_MODULE_PREFIX
#undef VM_DLSYM_PREFIX
#undef VM_X11DIR

/* avoid dependencies on glibc2.3 */

#undef HAVE_FEATURES_H

#if defined(HAVE_FEATURES_H)
# include "glibc.h"
#endif

#endif /* __sq_config_h */
