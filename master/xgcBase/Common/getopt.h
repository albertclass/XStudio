/* Getopt for Microsoft C
This code is a modification of the Free Software Foundation, Inc.
Getopt library for parsing command line argument the purpose was
to provide a Microsoft Visual C friendly derivative. This code
provides functionality for both Unicode and Multibyte builds.

Date: 02/03/2011 - Ludvik Jerabek - Initial Release
Version: 1.0
Comment: Supports getopt, getopt_long, and getopt_long_only
and POSIXLY_CORRECT environment flag
License: LGPL

**DISCLAIMER**
THIS MATERIAL IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING, BUT Not LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE, OR NON-INFRINGEMENT. SOME JURISDICTIONS DO NOT ALLOW THE
EXCLUSION OF IMPLIED WARRANTIES, SO THE ABOVE EXCLUSION MAY NOT
APPLY TO YOU. IN NO EVENT WILL I BE LIABLE TO ANY PARTY FOR ANY
DIRECT, INDIRECT, SPECIAL OR OTHER CONSEQUENTIAL DAMAGES FOR ANY
USE OF THIS MATERIAL INCLUDING, WITHOUT LIMITATION, ANY LOST
PROFITS, BUSINESS INTERRUPTION, LOSS OF PROGRAMS OR OTHER DATA ON
YOUR INFORMATION HANDLING SYSTEM OR OTHERWISE, EVEN If WE ARE
EXPRESSLY ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*/
#ifndef __GETOPT_H_
#define __GETOPT_H_

#include "exports.h"

// Standard GNU options
#define null_argument       0   /*Argument Null*/
#define no_argument         0   /*Argument Switch Only*/
#define required_argument   1   /*Argument Required*/
#define optional_argument   2   /*Argument Optional*/   

// Shorter Options
#define ARG_NULL    0   /*Argument Null*/
#define ARG_NONE    0   /*Argument Switch Only*/
#define ARG_REQ     1   /*Argument Required*/
#define ARG_OPT     2   /*Argument Optional*/

#include <string.h>
#include <wchar.h>

namespace XGC
{
	namespace common
	{
		extern "C"
		{
			extern COMMON_API int optind;
			extern COMMON_API int opterr;
			extern COMMON_API int optopt;

			// Ansi
			struct COMMON_API option_a
			{
				const char* name;
				int has_arg;
				int *flag;
				char val;
			};
			extern COMMON_API char *optarg_a;
			extern COMMON_API int getopt_a( int argc, char *const *argv, const char *optstring );
			extern COMMON_API int getopt_long_a( int argc, char *const *argv, const char *options, const struct option_a *long_options, int *opt_index );
			extern COMMON_API int getopt_long_only_a( int argc, char *const *argv, const char *options, const struct option_a *long_options, int *opt_index );

			extern COMMON_API void getopt_clear();

			// Unicode
			struct option_w
			{
				const wchar_t* name;
				int has_arg;
				int *flag;
				wchar_t val;
			};
			extern COMMON_API wchar_t *optarg_w;
			extern COMMON_API int getopt_w( int argc, wchar_t *const *argv, const wchar_t *optstring );
			extern COMMON_API int getopt_long_w( int argc, wchar_t *const *argv, const wchar_t *options, const struct option_w *long_options, int *opt_index );
			extern COMMON_API int getopt_long_only_w( int argc, wchar_t *const *argv, const wchar_t *options, const struct option_w *long_options, int *opt_index );

		}
	}
}

// #ifdef _UNICODE
// #define getopt getopt_w
// #define getopt_long getopt_long_w
// #define getopt_long_only getopt_long_only_w
// #define option option_w
// #define optarg optarg_w
// #else
#define getopt getopt_a
#define getopt_long getopt_long_a
#define getopt_long_only getopt_long_only_a
#define option option_a
#define optarg optarg_a
/*#endif*/
#endif  // __GETOPT_H_
