///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  MarshalJetHandles - Adds specializatios to marhsall jet handles
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

namespace msclr
{
	namespace interop
	{
		template<> inline IntPtr marshal_as<IntPtr, JET_API_PTR> (JET_API_PTR const &from)
		{
			return IntPtr(reinterpret_cast<void *>(from)); //pointer sized int
		}

		template<> inline IntPtr ^marshal_as<IntPtr ^, JET_API_PTR> (JET_API_PTR const &from)
		{
			return gcnew IntPtr(reinterpret_cast<void *>(from)); //pointer sized int
		}

#if defined(_WIN64)
		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr> (IntPtr const &from)
		{
			IntPtr copy = from;
			return copy.ToInt64();
		}

		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr ^> (IntPtr ^const &from)
		{
			return from->ToInt64();
		}
#else
		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr> (IntPtr const &from)
		{
			IntPtr copy = from;
			return copy.ToInt32();
		}

		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr ^> (IntPtr ^const &from)
		{
			return from->ToInt32();
		}
#endif
		//
		//template<> inline IntPtr marshal_as<IntPtr, JET_HANDLE> (const JET_HANDLE &from)
		//{
		//	return marshal_as<IntPtr, JET_API_PTR>(from);
		//}

	}
}