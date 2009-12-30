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