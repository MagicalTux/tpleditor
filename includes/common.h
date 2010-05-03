/**
 * Copyright (c) 2010, Mark Karpeles
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Kabushiki Kaisha Tibanne nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *     * Sourcecode under ths license cannot be released under any license
 *       excessively restricting freedom to choose another license such as the
 *       GNU General Public License. This clause should be kept if another
 *       license is chosen.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __WIN32
#ifndef RC_INVOKED
//#include <w32api.h>

#define _WIN32_WINNT Windows2000
#define WINVER Windows2000
#undef _WIN32_WINNT
#define _WIN32_IE IE601
#define __MSVCRT_VERSION__ IE601

//#include <windows.h>
#endif /* !RC_INVOKED */
#endif /* __WIN32 */

#ifndef TPLV3_VERSION

#if (TPLV3_VERSION_TYPE==0)
#define TPLV3_VERSION_TYPE_STR alpha
#endif

#if (TPLV3_VERSION_TYPE==1)
#define TPLV3_VERSION_TYPE_STR beta
#endif

#if (TPLV3_VERSION_TYPE==2)
#define TPLV3_VERSION_TYPE_STR RC
#endif

#if (TPLV3_VERSION_TYPE==3)
#define TPLV3_VERSION_TYPE_STR .
#endif

#define TPLV3_VERSION_DOT .

#define XSTR(a) #a
#define STR(a) XSTR(a)

#define XCONCAT(a,b) a ## b
#define CONCAT(a,b) XCONCAT(a,b)

/* detect host platform */
#if defined(__APPLE__)
# define HOST_PLATFORM macos
#elif defined(__linux__) && defined(__x86_64)
# define HOST_PLATFORM linux64
#elif defined(__linux__)
# define HOST_PLATFORM linux
#elif defined(__unix__)
# define HOST_PLATFORM unix
#elif defined(__WIN32)
# define HOST_PLATFORM win32
#else
# define HOST_PLATFORM unknown
#endif

#define TPLV3_VERSION STR(CONCAT(CONCAT(CONCAT(TPLV3_VERSION_MAJOR, TPLV3_VERSION_DOT), CONCAT(TPLV3_VERSION_MINOR, TPLV3_VERSION_TYPE_STR)), TPLV3_VERSION_REVISION)) "-" STR(HOST_PLATFORM)

#endif /* !defined(TPLV3_VERSION) */

