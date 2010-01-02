///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  JetHeaderSupplement - Additional ESE header definitions
///////////////////////////////////////////////////////////////////////////////
// 
//This software is licenced under the terms of the MIT License:
//
//Copyright (c) 2009 Christopher Smith
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

//This header exists to provide retro versions of items that can't work in older versions of ESE with a newer version specified.

#if defined(_M_AMD64) || defined(_M_IA64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif

//This struct has an extra member in 6.0+ which screws up array alignment when calling into a <6.0 library.
//The version defined below is JET_INDEXCREATE as defined for 5.2
typedef struct tagJET_INDEXCREATE_SHORT
	{
	unsigned long			cbStruct;				// size of this structure (for future expansion)
	char					*szIndexName;			// index name
	char					*szKey;					// index key definition
	unsigned long			cbKey;					// size of key definition in szKey
	JET_GRBIT				grbit;					// index options
	unsigned long			ulDensity;				// index density

	union
		{
		unsigned long		lcid;					// lcid for the index (if JET_bitIndexUnicode NOT specified)
		JET_UNICODEINDEX	*pidxunicode;			// pointer to JET_UNICODEINDEX struct (if JET_bitIndexUnicode specified)
		};

	union
		{
		unsigned long		cbVarSegMac;			// maximum length of variable length columns in index key (if JET_bitIndexTupleLimits specified)
		JET_TUPLELIMITS		*ptuplelimits;			// pointer to JET_TUPLELIMITS struct (if JET_bitIndexTupleLimits specified)
		};

	JET_CONDITIONALCOLUMN_A	*rgconditionalcolumn;	// pointer to conditional column structure
	unsigned long			cConditionalColumn;		// number of conditional columns
	JET_ERR					err;					// returned error code
	} JET_INDEXCREATE_SHORT;

#include <poppack.h>
