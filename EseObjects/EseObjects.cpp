///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.cpp - Unity build master file for EseObjects
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

//PCH header with system and common definitions
#include "stdafx.h"

#include "free_list.hpp"

void *JET_API jet_realloc_cpp(void *context, void *buff, ulong length)
{
	if(buff)
		delete(buff);
	
	if(length)
		return new char[length];
	
	return 0;
}

namespace EseObjects
{

//max size used for alloca
size_t const ESEOBJECTS_MAX_ALLOCA = 64;

#include "ForwardReferences.hpp"
#include "EseVersion.hpp"
#include "EseException.hpp"
#include "DemandLoadFunction.hpp"
#include "InternalBridgeFunctions.hpp"
#include "Instance.hpp"
#include "Session.hpp"
#include "Database.hpp"
#include "Transaction.hpp"
#include "TableID.hpp"
#include "Column.hpp"
#include "Bridge.hpp"
#include "Positioning.hpp"
#include "Key.hpp"
#include "Index.hpp"
#include "Bookmark.hpp"
#include "SecondaryBookmark.hpp"
#include "Cursor.hpp"
#include "Table.hpp"

}
