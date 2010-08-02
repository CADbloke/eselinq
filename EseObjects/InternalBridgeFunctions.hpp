///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  FieldDataConversion - Built in conversions for field values
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

//Possible loss of data warning.
//In attempting to provide data conversions that could be possible, some can lose data.
//Default conversions preserve data, but the caller could demand something narrowing.
#pragma warning(push)
#pragma warning(disable:4244)

__declspec(deprecated("Type conversion unsupported. Correct the type or add another specialization.")) void unsupported_type_conversion(){}

template <class T> T from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	unsupported_type_conversion();
	success = false;
	return T();
}

template <class T> bool to_memblock(T o, void *&buff, ulong &bytes, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	unsupported_type_conversion();
	return false;
}

//convert to scalar type (integers especially)
template <class T> T from_memblock_scalar(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		return *reinterpret_cast<uchar *>(buff);
	
	case JET_coltypShort:
		return *reinterpret_cast<short *>(buff);

	case JET_coltypUnsignedShort:
		return *reinterpret_cast<ushort *>(buff);
	
	case JET_coltypLong:
		return *reinterpret_cast<long *>(buff);

	case JET_coltypUnsignedLong:
		return *reinterpret_cast<ulong *>(buff);

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		return *reinterpret_cast<int64 *>(buff);

	case JET_coltypIEEESingle:
		return *reinterpret_cast<float *>(buff);
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		return *reinterpret_cast<double *>(buff);
	}

	success = false;
	return T();
}

//scalar converions
template <> Boolean from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Boolean>(success, buff, max, coltyp, cp);}
template <> Byte from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Byte>(success, buff, max, coltyp, cp);}
template <> SByte from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<SByte>(success, buff, max, coltyp, cp);}
template <> Char from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Char>(success, buff, max, coltyp, cp);}
template <> Single from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Single>(success, buff, max, coltyp, cp);}
template <> Double from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Double>(success, buff, max, coltyp, cp);}
template <> Int16 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int16>(success, buff, max, coltyp, cp);}
template <> Int32 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int32>(success, buff, max, coltyp, cp);}
template <> Int64 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int64>(success, buff, max, coltyp, cp);}
template <> UInt16 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt16>(success, buff, max, coltyp, cp);}
template <> UInt32 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt32>(success, buff, max, coltyp, cp);}
template <> UInt64 from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt64>(success, buff, max, coltyp, cp);}


//Decimal is pickier about it's implicit conversions and doesn't have a builitin conversion from ulong (uses an upcast to unsigned int64 here, which works)
template <> Decimal from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		return safe_cast<Decimal>(*reinterpret_cast<uchar *>(buff));
	
	case JET_coltypShort:
		return safe_cast<Decimal>(*reinterpret_cast<short *>(buff));

	case JET_coltypUnsignedShort:
		return safe_cast<Decimal>(*reinterpret_cast<ushort *>(buff));
	
	case JET_coltypLong:
		return safe_cast<Decimal>(*reinterpret_cast<long *>(buff));

	case JET_coltypUnsignedLong:
		return safe_cast<Decimal>(static_cast<unsigned __int64>(*reinterpret_cast<ulong *>(buff)));

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		return safe_cast<Decimal>(*reinterpret_cast<int64 *>(buff));

	case JET_coltypIEEESingle:
		return safe_cast<Decimal>(*reinterpret_cast<float *>(buff));
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		return safe_cast<Decimal>(*reinterpret_cast<double *>(buff));
	}

	success = false;
	return Decimal();
}

template <> Guid from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
	case JET_coltypGUID:
		{
			if(max < 16)
				throw gcnew InvalidOperationException("Data must be at least 16 bytes to extract GUID.");

			array<uchar> ^arr = gcnew array<uchar>(16);

			for(ulong i = 0; i < 16; i++)
				arr[i] = (reinterpret_cast<uchar *>(buff))[i];

			return Guid(arr);
		}
	}

	success = false;
	return Guid();
}

String ^astring_from_memblock(void *buff, ulong max)
{
	return gcnew String(reinterpret_cast<char *>(buff), 0, max, System::Text::Encoding::ASCII);
}

template <> String ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypText:
	case JET_coltypLongText:
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		if(cp == 1252)
			return gcnew String(reinterpret_cast<char *>(buff), 0, max, System::Text::Encoding::ASCII);
		else if(cp == 1200)
			return gcnew String(reinterpret_cast<wchar_t *>(buff), 0, max / sizeof(wchar_t));
	}

	success = false;
	return nullptr;
}

//always provides binary representation of data
template <> array<uchar> ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	array<uchar> ^arr = gcnew array<uchar>(max);

	for(ulong i = 0; i < max; i++)
		arr[i] = (reinterpret_cast<uchar *>(buff))[i];

	success = true;
	return arr;
}

template <> DateTime from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypDateTime:
	case JET_coltypIEEEDouble:
		return DateTime::FromOADate(*reinterpret_cast<double *>(buff));
	}

	success = false;
	return DateTime();
}

template <> Key ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		return KeyFromMemblock(buff, max);
	}

	success = false;
	return nullptr;
}

template <> Bookmark ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		return BookmarkFromMemblock(buff, max);
	}

	success = false;
	return nullptr;
}

template <> SecondaryBookmark ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	success = true; //successful until determined otherwise

	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		return SecondaryBookmarkFromMemblock(buff, max);
	}

	success = false;
	return nullptr;
}

Object ^from_memblock_binserialize(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	bool to_array_success;
	System::Runtime::Serialization::Formatters::Binary::BinaryFormatter ^formatter = gcnew System::Runtime::Serialization::Formatters::Binary::BinaryFormatter();
	System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream(from_memblock<array<uchar> ^>(to_array_success, buff, max, coltyp, cp));
	
	success = true; //it'll only fail via exception
	return formatter->Deserialize(stream);
}

//convert to default
Object ^from_memblock(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	switch(coltyp)
	{
	case JET_coltypBit:
		return from_memblock<Boolean>(success, buff, max, coltyp, cp);

	case JET_coltypUnsignedByte:
		return from_memblock<Byte>(success, buff, max, coltyp, cp);

	case JET_coltypShort:
		return from_memblock<Int16>(success, buff, max, coltyp, cp);

	case JET_coltypUnsignedShort:
		return from_memblock<UInt16>(success, buff, max, coltyp, cp);

	case JET_coltypLong:
		return from_memblock<Int32>(success, buff, max, coltyp, cp);

	case JET_coltypUnsignedLong:
		return from_memblock<UInt32>(success, buff, max, coltyp, cp);

	case JET_coltypLongLong:
	case JET_coltypCurrency:
		return from_memblock<Int64>(success, buff, max, coltyp, cp);

	case JET_coltypIEEESingle:
		return from_memblock<Single>(success, buff, max, coltyp, cp);

	case JET_coltypIEEEDouble:
		return from_memblock<Double>(success, buff, max, coltyp, cp);

	case JET_coltypDateTime:
		return from_memblock<DateTime>(success, buff, max, coltyp, cp);

	case JET_coltypText:
	case JET_coltypLongText:
		if(cp == 1252 || cp == 1200) //a known code page in other words
			return from_memblock<String ^>(success, buff, max, coltyp, cp);
		else
			return from_memblock<array<uchar> ^>(success, buff, max, coltyp, cp);

	case JET_coltypGUID:
		return from_memblock<Guid>(success, buff, max, coltyp, cp);

	case JET_coltypNil:
	case JET_coltypBinary:
	case JET_coltypLongBinary:
	default:
		return from_memblock<array<uchar> ^>(success, buff, max, coltyp, cp);
	}

	success = false;
	return nullptr;
}

Object ^from_memblock(bool &success, Type ^type, void *bytes, ulong size, JET_COLTYP coltyp, ushort cp)
{
 	if(type == Boolean::typeid)
		return from_memblock<Boolean>(success, bytes, size, coltyp, cp);
	if(type == Byte::typeid)
		return from_memblock<Byte>(success, bytes, size, coltyp, cp);
	if(type == SByte::typeid)
		return from_memblock<SByte>(success, bytes, size, coltyp, cp);
	if(type == Char::typeid)
		return from_memblock<Char>(success, bytes, size, coltyp, cp);
	if(type == Single::typeid)
		return from_memblock<Single>(success, bytes, size, coltyp, cp);
	if(type == Double::typeid)
		return from_memblock<Double>(success, bytes, size, coltyp, cp);
	if(type == Int16::typeid)
		return from_memblock<Int16>(success, bytes, size, coltyp, cp);
	if(type == Int32::typeid)
		return from_memblock<Int32>(success, bytes, size, coltyp, cp);
	if(type == Int64::typeid)
		return from_memblock<Int64>(success, bytes, size, coltyp, cp);
	if(type == UInt16::typeid)
		return from_memblock<UInt16>(success, bytes, size, coltyp, cp);
	if(type == UInt32::typeid)
		return from_memblock<UInt32>(success, bytes, size, coltyp, cp);
	if(type == UInt64::typeid)
		return from_memblock<UInt64>(success, bytes, size, coltyp, cp);
	if(type == Guid::typeid)
		return from_memblock<Guid>(success, bytes, size, coltyp, cp);
	if(type == String::typeid)
		return from_memblock<String ^>(success, bytes, size, coltyp, cp);
	if(type == array<uchar>::typeid)
		return from_memblock<array<uchar> ^>(success, bytes, size, coltyp, cp);
	if(type == DateTime::typeid)
		return from_memblock<DateTime>(success, bytes, size, coltyp, cp);
	if(type == Key::typeid)
		return from_memblock<Key ^>(success, bytes, size, coltyp, cp);
	if(type == Bookmark::typeid)
		return from_memblock<Bookmark ^>(success, bytes, size, coltyp, cp);
	if(type == SecondaryBookmark::typeid)
		return from_memblock<SecondaryBookmark ^>(success, bytes, size, coltyp, cp);
	if(type == Object::typeid)
		return from_memblock(success, bytes, size, coltyp, cp);
	if(type->IsSerializable)
		return from_memblock_binserialize(success, bytes, size, coltyp, cp);

	success = false;
	return nullptr;
}

//generic <class T> T from_memblock_generic(bool &success, void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
//{
//	Object ^o = from_memblock(success, T::typeid, buff, max, coltyp, cp);
//
//	if(success)
//		return safe_cast<T>(o);
//
//	return T();
//}

template <class T, class U> void alloc_and_assign(T t, void *&buff, ulong &max, free_list &fl)
{
	buff = fl.alloc_zero<U>();
	max = sizeof(U);
	*reinterpret_cast<U *>(buff) = t;
}

template <class T> bool to_memblock_scalar(T t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false; //these fixed size types are never empty

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		alloc_and_assign<T, uchar>(t, buff, max, fl);
		return true;

	case JET_coltypShort:
		alloc_and_assign<T, short>(t, buff, max, fl);
		return true;

	case JET_coltypUnsignedShort:
		alloc_and_assign<T, ushort>(t, buff, max, fl);
		return true;
	
	case JET_coltypLong:
		alloc_and_assign<T, long>(t, buff, max, fl);
		return true;

	case JET_coltypUnsignedLong:
		alloc_and_assign<T, ulong>(t, buff, max, fl);
		return true;

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		alloc_and_assign<T, int64>(t, buff, max, fl);
		return true;

	case JET_coltypIEEESingle:
		alloc_and_assign<T, float>(t, buff, max, fl);
		return true;
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		alloc_and_assign<T, double>(t, buff, max, fl);
		return true;
	}

	return false;
}

//scalar conversions:
template <> bool to_memblock(Boolean t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Boolean>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Byte t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Byte>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(SByte t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<SByte>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Char t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Char>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Single t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Single>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Double t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Double>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Int16 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int16>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Int32 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int32>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(Int64 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int64>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(UInt16 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt16>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(UInt32 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt32>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> bool to_memblock(UInt64 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt64>(t, buff, max, empty, coltyp, cp, mc, fl);}

//always uses binary representation of data
template <> bool to_memblock(array<uchar> ^arr, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	if(arr == nullptr)
	{
		max = 0;
		buff = null;
		empty = false;
		return true;
	}

	max = arr->Length;
	uchar *buffc = fl.alloc_array<uchar>(max);

	if(max == 0)
		empty = true;

	for(ulong i = 0; i < max; i++)
		buffc[i] = arr[i];

	buff = buffc;
	
	return true;
}

template <> bool to_memblock(Guid g, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	return to_memblock<array<uchar> ^>(g.ToByteArray(), buff, max, empty, coltyp, cp, mc, fl);
}

template <> bool to_memblock(Decimal t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false; //these fixed size types can't be empty

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		alloc_and_assign<uchar, uchar>((uchar)t, buff, max, fl);
		return true;

	case JET_coltypShort:
		alloc_and_assign<short, short>((short)t, buff, max, fl);
		return true;

	case JET_coltypUnsignedShort:
		alloc_and_assign<ushort, ushort>((ushort)t, buff, max, fl);
		return true;
	
	case JET_coltypLong:
		alloc_and_assign<long, long>((int64)t, buff, max, fl);
		return true;

	case JET_coltypUnsignedLong:
		alloc_and_assign<ulong, ulong>((int64)t, buff, max, fl);
		return true;

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		alloc_and_assign<int64, int64>((int64)t, buff, max, fl);
		return true;

	case JET_coltypIEEESingle:
		alloc_and_assign<float, float>((float)t, buff, max, fl);
		return true;
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		alloc_and_assign<double, double>((double)t, buff, max, fl);
		return true;
	}

	return false;
}

template <> bool to_memblock(String ^s, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	if(s == nullptr)
	{
		empty = false;
		buff = null;
		max = 0;
		return true;
	}

	if(s->Length == 0)
		empty = true;

	switch(coltyp)
	{
	case JET_coltypText:
	case JET_coltypLongText:
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		if(cp == 1252)
		{
			buff = const_cast<char *>(mc.marshal_as<char const *>(s));
			max = s->Length;
			return true;
		}
		else if(cp == 1200)
		{
			buff = const_cast<wchar_t *>(mc.marshal_as<wchar_t const *>(s));
			max = s->Length * 2;
			return true;
		}
	}

	return false;
}

template <> bool to_memblock(DateTime t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false;

	switch(coltyp)
	{
	case JET_coltypDateTime:
	case JET_coltypIEEEDouble:
		alloc_and_assign<double, double>(t.ToOADate(), buff, max, fl);
		return true;
	}

	return false;
}

template <> bool to_memblock(Key ^t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		void *src_buff = 0;
		if(src_buff)
		{
			KeyGetBuffer(t, src_buff, max);
			buff = fl.alloc_array<uchar>(max);
			memcpy(buff, src_buff, max);
			empty = max == 0;
			return true;
		}
		else
			return false;
	}

	return false;
}

template <> bool to_memblock(Bookmark ^t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		void *src_buff = 0;
		BookmarkGetBuffer(t, src_buff, max);
		if(src_buff)
		{
			buff = fl.alloc_array<uchar>(max);
			memcpy(buff, src_buff, max);
			empty = max == 0;
			return true;
		}
		else
			return false;
	}

	return false;
}

template <> bool to_memblock(SecondaryBookmark ^t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	switch(coltyp)
	{
	case JET_coltypBinary:
	case JET_coltypLongBinary:
		void *src_buff = 0;
		SecondaryBookmarkGetBuffer(t, src_buff, max);
		if(src_buff)
		{
			buff = fl.alloc_array<uchar>(max);
			memcpy(buff, src_buff, max);
			empty = max == 0;
			return true;
		}
		else
			return false;
	}

	return false;
}

bool to_memblock_binserialize(Object ^o, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	System::Runtime::Serialization::Formatters::Binary::BinaryFormatter ^formatter = gcnew System::Runtime::Serialization::Formatters::Binary::BinaryFormatter();
	System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream();
	formatter->Serialize(stream, o);
	
	to_memblock(stream->ToArray(), buff, max, empty, coltyp, cp , mc, fl);

	return true;
}

//convert from object
template <> bool to_memblock(Object ^o, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	System::Type ^ty = o->GetType();

	if(ty == Boolean::typeid)
		return to_memblock<Boolean>(safe_cast<Boolean>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Byte::typeid)
		return to_memblock<Byte>(safe_cast<Byte>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == SByte::typeid)
		return to_memblock<SByte>(safe_cast<SByte>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Char::typeid)
		return to_memblock<Char>(safe_cast<Char>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Single::typeid)
		return to_memblock<Single>(safe_cast<Single>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Double::typeid)
		return to_memblock<Double>(safe_cast<Double>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Int16::typeid)
		return to_memblock<Int16>(safe_cast<Int16>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Int32::typeid)
		return to_memblock<Int32>(safe_cast<Int32>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Int64::typeid)
		return to_memblock<Int64>(safe_cast<Int64>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == UInt16::typeid)
		return to_memblock<UInt16>(safe_cast<UInt16>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == UInt32::typeid)
		return to_memblock<UInt32>(safe_cast<UInt32>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == UInt64::typeid)
		return to_memblock<UInt64>(safe_cast<UInt64>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Guid::typeid)
		return to_memblock<Guid>(safe_cast<Guid>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == String::typeid)
		return to_memblock<String ^>(safe_cast<String ^>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == array<uchar>::typeid)
		return to_memblock<array<uchar> ^>(safe_cast<array<uchar> ^>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == DateTime::typeid)
		return to_memblock<DateTime>(safe_cast<DateTime>(o), buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Key::typeid)
		return to_memblock<Key ^>((Key ^)o, buff, max, empty, coltyp, cp, mc, fl);
	if(ty == Bookmark::typeid)
		return to_memblock<Bookmark ^>((Bookmark ^)o, buff, max, empty, coltyp, cp, mc, fl);
	if(ty == SecondaryBookmark::typeid)
		return to_memblock<SecondaryBookmark ^>((SecondaryBookmark ^)o, buff, max, empty, coltyp, cp, mc, fl);
	
	if(o->GetType()->IsSerializable)
		return to_memblock_binserialize(o, buff, max, empty, coltyp, cp, mc, fl);

	return false;
}

#pragma warning(pop)
