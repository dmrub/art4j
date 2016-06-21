/* DTrackParse: C++ source file, A.R.T. GmbH
 *
 * Functions for processing data
 *
 * Copyright (c) 2007-2015, Advanced Realtime Tracking GmbH
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Version v2.4.1
 *
 */

#include "DTrackParse.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace DTrackSDK_Parse {

/**
 *	\brief	Search next line in buffer
 *	@param[in] 	str		buffer (total)
 *	@param[in] 	start	start position within buffer
 *	@param[in] 	len		buffer length in bytes
 *	@return		begin of line, NULL if no new line in buffer
 */
char* string_nextline(char* str, char* start, int len)
{
	char* s = start;
	char* se = str + len;
	int crlffound = 0;
	while (s < se)
	{
		if (*s == '\r' || *s == '\n')
		{	// crlf
			crlffound = 1;
		} else {
			if(crlffound)
			{	// begin of new line found
				return (*s) ? s : NULL;	// first character is '\0': end of buffer
			}
		}
		s++;
	}
	return NULL;	// no new line found in buffer
}


/**
 * 	\brief	Read next 'int' value from string
 *
 *	@param[in] 	str		string
 *	@param[out] i		read value
 *	@return		pointer behind read value in str; NULL in case of error
 */
char* string_get_i(char* str, int* i)
{
	char* s;
	*i = (int )strtol(str, &s, 0);
	return (s == str) ? NULL : s;
}


/**
 * 	\brief	Read next 'unsigned int' value from string
 *
 *	@param[in] 	str		string
 *	@param[out]	ui		read value
 *	@return		pointer behind read value in str; NULL in case of error
 */
char* string_get_ui(char* str, unsigned int* ui)
{
	char* s;
	*ui = (unsigned int )strtoul(str, &s, 0);
	return (s == str) ? NULL : s;
}


/**
 * 	\brief	Read next 'double' value from string
 *
 *	@param[in] 	str		string
 *	@param[out] d		read value
 *	@return 	pointer behind read value in str; NULL in case of error
 */
char* string_get_d(char* str, double* d)
{
    setlocale(LC_NUMERIC, "C");
	char* s;
	*d = (double )strtod(str, &s);
	return (s == str) ? NULL : s;
}


/**
 * 	\brief	Read next 'float' value from string
 *
 *	@param[in] 	str		string
 *	@param[out] f 		read value
 *	@return 	pointer behind read value in str; NULL in case of error
 */
char* string_get_f(char* str, float* f)
{
    setlocale(LC_NUMERIC, "C");
	char* s;
	*f = (float )strtod(str, &s);	// strtof() only available in GNU-C
	return (s == str) ? NULL : s;
}


/**
 * 	\brief Process next block '[...]' in string
 *
 *	@param[in] 	str		string
 *	@param[in] 	fmt		format string ('i' for 'int', 'f' for 'float')
 *	@param[out] idat	array for 'int' values (long enough due to fmt)
 *	@param[out] fdat	array for 'float' values (long enough due to fmt)
 *	@param[out] ddat	array for 'double' values (long enough due to fmt)
 *	@return 	pointer behind read value in str; NULL in case of error
 */
char* string_get_block(char* str, const char* fmt, int* idat, float* fdat, double *ddat)
{
	char* strend;
    int index_i, index_f;
	if ((str = strchr(str, '[')) == NULL)
	{       // search begin of block
		return NULL;
	}
	if ((strend = strchr(str, ']')) == NULL)
	{    // search end of block
		return NULL;
	}
	str++;                               // remove delimiters
	*strend = '\0';
	index_i = index_f = 0;
	while(*fmt)
	{
		switch(*fmt++)
		{
			case 'i':
				if((str = string_get_i(str, &idat[index_i++])) == NULL)
				{
					*strend = ']';
					return NULL;
				}
				break;
			case 'f':
				if((str = string_get_f(str, &fdat[index_f++])) == NULL)
				{
					*strend = ']';
					return NULL;
				}
				break;
			case 'd':
				if((str = string_get_d(str, &ddat[index_f++])) == NULL)
				{
					*strend = ']';
					return NULL;
				}
				break;
			default:	// unknown format character
				*strend = ']';
				return NULL;
		}
	}
	// ignore additional data inside the block
	*strend = ']';
	return strend + 1;
}


/**
 * 	\brief	Read next 'word' value from string
 *
 *	@param[in] 	str		string
 *	@param[out] w		read value
 *	@return		pointer behind read value in str; NULL in case of error
 */
char* string_get_word(char* str, std::string& w)
{
	char* strend;
	while (*str == ' ')
	{	// search begin of 'word'
		str++;
	}
	if (!(strend = strchr(str, ' ')))
	{	// search end of 'word'
		w.assign(str);
		strend = str;
		while (*strend != '\0')
		{	// search end of 'word'
			strend++;
		}
		return (strend == str) ? NULL : strend;
	}
	w.assign(str, (int )(strend - str));
	return strend;
}


/**
 * 	\brief	Read next 'quoted text' value from string
 *
 *	@param[in] 	str		string
 *	@param[out] qt		read value (without quotes)
 *	@return		pointer behind read value in str; NULL in case of error
 */
char* string_get_quoted_text(char* str, std::string& qt)
{
	char* strend;
	if (!(str = strchr(str, '\"')))
	{	// search begin of 'quoted text'
		return NULL;
	}
	str++;
	if (!(strend = strchr(str, '\"')))
	{	// search end of 'quoted text'
		return NULL;
	}
	qt.assign(str, (int )(strend - str));
	return strend + 1;
}


/**
 * 	\brief	Compare strings regarding DTrack2 parameter rules
 *
 *	@param 	str		string
 *	@param 	p		parameter string
 *	@return pointer behind parameter in str; NULL in case of error
 */
char* string_cmp_parameter(char* str, const char* p)
{
	bool lastwasdigit = false;
	while(*p)
	{
		if (!lastwasdigit)
		{	// skip leading zeros
			while(*p == '0')
			{
				p++;
			}
			while(*str == '0')
			{
				str++;
			}
		}
		if (*str != *p)
		{	// compare next character
			return NULL;
		}
		if (*p == ' ')
		{	// skip white space
			do {
				p++;
			} while (*p == ' ');
			
			do {
				str++;
			} while (*str == ' ');
			
			lastwasdigit = false;
			continue;
		}
		lastwasdigit = (*p >= '0' && *p <= '9');
		str++;
		p++;
	}
	do {  // skip white space
		str++;
	} while (*str == ' ');
	return str;
}

} // end namespace
