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

template <class T> T from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	unsupported_type_conversion();
	return safe_cast<T>(nullptr);
}

template <class T> void to_memblock(T o, void *&buff, ulong &bytes, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	unsupported_type_conversion();
}

//convert to scalar type (integers especially)
template <class T> T from_memblock_scalar(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
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

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

//scalar converions
template <> Boolean from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Boolean>(buff, max, coltyp, cp);}
template <> Byte from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Byte>(buff, max, coltyp, cp);}
template <> SByte from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<SByte>(buff, max, coltyp, cp);}
template <> Char from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Char>(buff, max, coltyp, cp);}
template <> Single from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Single>(buff, max, coltyp, cp);}
template <> Double from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Double>(buff, max, coltyp, cp);}
template <> Int16 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int16>(buff, max, coltyp, cp);}
template <> Int32 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int32>(buff, max, coltyp, cp);}
template <> Int64 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<Int64>(buff, max, coltyp, cp);}
template <> UInt16 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt16>(buff, max, coltyp, cp);}
template <> UInt32 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt32>(buff, max, coltyp, cp);}
template <> UInt64 from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp) {return from_memblock_scalar<UInt64>(buff, max, coltyp, cp);}

//Decimal is pickier about it's implicit conversions and doesn't have a builitin conversion from ulong (uses an upcast to unsigned int64 here, which works)
template <> Decimal from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
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

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

template <> Guid from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
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

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

template <> String ^from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
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
		else
			throw gcnew InvalidOperationException("Unknown code page.");
	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

//always provides binary representation of data
template <> array<uchar> ^from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	array<uchar> ^arr = gcnew array<uchar>(max);

	for(ulong i = 0; i < max; i++)
		arr[i] = (reinterpret_cast<uchar *>(buff))[i];

	return arr;
}

template <> DateTime from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	switch(coltyp)
	{
	case JET_coltypDateTime:
	case JET_coltypIEEEDouble:
		return DateTime::FromOADate(*reinterpret_cast<double *>(buff));

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

//convert to default
Object ^from_memblock(void *buff, ulong max, JET_COLTYP coltyp, ushort cp)
{
	switch(coltyp)
	{
	case JET_coltypBit:
		return from_memblock<Boolean>(buff, max, coltyp, cp);

	case JET_coltypUnsignedByte:
		return from_memblock<Byte>(buff, max, coltyp, cp);

	case JET_coltypShort:
		return from_memblock<Int16>(buff, max, coltyp, cp);

	case JET_coltypUnsignedShort:
		return from_memblock<UInt16>(buff, max, coltyp, cp);

	case JET_coltypLong:
		return from_memblock<Int32>(buff, max, coltyp, cp);

	case JET_coltypUnsignedLong:
		return from_memblock<UInt32>(buff, max, coltyp, cp);

	case JET_coltypLongLong:
	case JET_coltypCurrency:
		return from_memblock<Int64>(buff, max, coltyp, cp);

	case JET_coltypIEEESingle:
		return from_memblock<Single>(buff, max, coltyp, cp);

	case JET_coltypIEEEDouble:
		return from_memblock<Double>(buff, max, coltyp, cp);

	case JET_coltypDateTime:
		return from_memblock<DateTime>(buff, max, coltyp, cp);

	case JET_coltypText:
	case JET_coltypLongText:
		if(cp == 1252 || cp == 1200) //a known code page in other words
			return from_memblock<String ^>(buff, max, coltyp, cp);
		else
			return from_memblock<array<uchar> ^>(buff, max, coltyp, cp);

	case JET_coltypGUID:
		return from_memblock<Guid>(buff, max, coltyp, cp);

	case JET_coltypNil:
	case JET_coltypBinary:
	case JET_coltypLongBinary:
	default:
		return from_memblock<array<uchar> ^>(buff, max, coltyp, cp);
	}
}


template <class T, class U> void alloc_and_assign(T t, void *&buff, ulong &max, free_list &fl)
{
	buff = fl.alloc_zero<U>();
	max = sizeof(U);
	*reinterpret_cast<U *>(buff) = t;
}

template <class T> void to_memblock_scalar(T t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false; //these fixed size types are never empty

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		alloc_and_assign<T, uchar>(t, buff, max, fl);
		break;

	case JET_coltypShort:
		alloc_and_assign<T, short>(t, buff, max, fl);
		break;

	case JET_coltypUnsignedShort:
		alloc_and_assign<T, ushort>(t, buff, max, fl);
		break;
	
	case JET_coltypLong:
		alloc_and_assign<T, long>(t, buff, max, fl);
		break;

	case JET_coltypUnsignedLong:
		alloc_and_assign<T, ulong>(t, buff, max, fl);
		break;

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		alloc_and_assign<T, int64>(t, buff, max, fl);
		break;

	case JET_coltypIEEESingle:
		alloc_and_assign<T, float>(t, buff, max, fl);
		break;
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		alloc_and_assign<T, double>(t, buff, max, fl);
		break;

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

//scalar conversions:
template <> void to_memblock(Boolean t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Boolean>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Byte t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Byte>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(SByte t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<SByte>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Char t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Char>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Single t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Single>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Double t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Double>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Int16 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int16>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Int32 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int32>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(Int64 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<Int64>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(UInt16 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt16>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(UInt32 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt32>(t, buff, max, empty, coltyp, cp, mc, fl);}
template <> void to_memblock(UInt64 t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl) {return to_memblock_scalar<UInt64>(t, buff, max, empty, coltyp, cp, mc, fl);}

//always uses binary representation of data
template <> void to_memblock(array<uchar> ^arr, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	if(arr == nullptr)
	{
		max = 0;
		buff = null;
		empty = false;
		return;
	}

	max = arr->Length;
	uchar *buffc = fl.alloc_array<uchar>(max);

	if(max == 0)
		empty = true;

	for(ulong i = 0; i < max; i++)
		buffc[i] = arr[i];

	buff = buffc;
}

template <> void to_memblock(Guid g, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	to_memblock<array<uchar> ^>(g.ToByteArray(), buff, max, empty, coltyp, cp, mc, fl);
}

template <> void to_memblock(Decimal t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false; //these fixed size types can't be empty

	switch(coltyp)
	{
	case JET_coltypBit:
	case JET_coltypUnsignedByte:
		alloc_and_assign<uchar, uchar>((uchar)t, buff, max, fl);
		break;

	case JET_coltypShort:
		alloc_and_assign<short, short>((short)t, buff, max, fl);
		break;

	case JET_coltypUnsignedShort:
		alloc_and_assign<ushort, ushort>((ushort)t, buff, max, fl);
		break;
	
	case JET_coltypLong:
		alloc_and_assign<long, long>((int64)t, buff, max, fl);
		break;

	case JET_coltypUnsignedLong:
		alloc_and_assign<ulong, ulong>((int64)t, buff, max, fl);
		break;

	case JET_coltypCurrency:
	case JET_coltypLongLong:
		alloc_and_assign<int64, int64>((int64)t, buff, max, fl);
		break;

	case JET_coltypIEEESingle:
		alloc_and_assign<float, float>((float)t, buff, max, fl);
		break;
	
	case JET_coltypIEEEDouble:
	case JET_coltypDateTime:
		alloc_and_assign<double, double>((double)t, buff, max, fl);
		break;

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

template <> void to_memblock(String ^s, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	if(s == nullptr)
	{
		empty = false;
		buff = null;
		max = 0;
		return;
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
		}
		else if(cp == 1200)
		{
			buff = const_cast<wchar_t *>(mc.marshal_as<wchar_t const *>(s));
			max = s->Length * 2;
		}
		else
			throw gcnew InvalidOperationException("Unknown code page.");
		break;

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}

template <> void to_memblock(DateTime t, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	empty = false;

	switch(coltyp)
	{
	case JET_coltypDateTime:
	case JET_coltypIEEEDouble:
		alloc_and_assign<double, double>(t.ToOADate(), buff, max, fl);
		break;

	default:
		throw gcnew InvalidOperationException("Invalid data type conversion.");
	}
}


//convert from object
template <> void to_memblock(Object ^o, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	System::Type ^ty = o->GetType();

	if(ty == Boolean::typeid)
		to_memblock<Boolean>(safe_cast<Boolean>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Byte::typeid)
		to_memblock<Byte>(safe_cast<Byte>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == SByte::typeid)
		to_memblock<SByte>(safe_cast<SByte>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Char::typeid)
		to_memblock<Char>(safe_cast<Char>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Single::typeid)
		to_memblock<Single>(safe_cast<Single>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Double::typeid)
		to_memblock<Double>(safe_cast<Double>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Int16::typeid)
		to_memblock<Int16>(safe_cast<Int16>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Int32::typeid)
		to_memblock<Int32>(safe_cast<Int32>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Int64::typeid)
		to_memblock<Int64>(safe_cast<Int64>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == UInt16::typeid)
		to_memblock<UInt16>(safe_cast<UInt16>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == UInt32::typeid)
		to_memblock<UInt32>(safe_cast<UInt32>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == UInt64::typeid)
		to_memblock<UInt64>(safe_cast<UInt64>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == Guid::typeid)
		to_memblock<Guid>(safe_cast<Guid>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == String::typeid)
		to_memblock<String ^>(safe_cast<String ^>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == array<uchar>::typeid)
		to_memblock<array<uchar> ^>(safe_cast<array<uchar> ^>(o), buff, max, empty, coltyp, cp, mc, fl);
	else if(ty == DateTime::typeid)
		to_memblock<DateTime>(safe_cast<DateTime>(o), buff, max, empty, coltyp, cp, mc, fl);
	else
		throw gcnew InvalidOperationException("Cannot convert column data from type: " + ty->ToString());
}

#pragma warning(pop)