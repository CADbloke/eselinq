///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  stdafx.h - Precompiled headers
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

// stdafx.h : include file for standard system include files

#pragma once

//setup for ESE header
#define JET_VERSION 0x0601
//#define JET_VERSION 0x0502
//#define JET_UNICODE

#include "JetImportFixup.h"

#include <esent.h>
#include <malloc.h>

#include "JetHeaderSupplement.h"

#define alloca_array(_tyname, _count) ( reinterpret_cast<_tyname *>(alloca(sizeof (_tyname) * (_count) )) )

//template <class T> __forceinline void zero_struct(T &s) {memset(&s, 0, sizeof s);}

//extra symbols used in EseObjects
#define null 0

typedef __int64 int64;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

//CLR support
#include <msclr\marshal.h>
using namespace System;
using namespace msclr::interop;
using namespace System::Collections::Generic;
using System::Runtime::InteropServices::OutAttribute;

#include "MarshalJetHandles.h"


#pragma warning (disable: 4800) //unnecessary int -> bool conversion warning (as if an explicit conversion would be any more efficient)
#pragma warning (disable: 4461) //finalizer without destructor (used in cases where only trivial heap memory is held)