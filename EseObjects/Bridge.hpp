///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Bridge - Data marshaling and conversion between Jet and .NET representations
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

///<summary>Methods of conversion between database and .NET representations of data. Interface likely to change.</summary>
public ref struct Bridge
{
	static void ThrowConversionError()
	{
		throw gcnew InvalidOperationException("Data type conversion not defined");
	}

	static void ThrowConversionError(System::Type ^type, Column::Type coltyp)
	{
		throw gcnew InvalidOperationException("Data type conversion not defined from " + type->ToString() + " to " + coltyp.ToString());
	}

	static void ThrowConversionError(Column::Type coltyp, System::Type ^type)
	{
		throw gcnew InvalidOperationException("Data type conversion not defined from " + coltyp.ToString() + " to " + type->ToString());
	}

	static void ThrowConversionError(System::Type ^type1, System::Type ^type2)
	{
		throw gcnew InvalidOperationException("Data type conversion not defined from " + type1->ToString() + " to " + type2->ToString());
	}

	///<summary>Invokes EseObjects default conversions.</summary>
	///<param name="success">Returns True iff the conversion was successful. The return value is undefined if this value is false.</param>
	///<param name="type">Provides the System::Type that the data should be converted to. If successful, the returned object is guaranteed to be of the specified type. Use Object's type to retrieve the defaul type based on the column type.</param>
	///<param name="isnull">True iff the database value is null. If False, bytes may still be 0, in which case the value is empty. The default conversion returns null. Retrieval functions convert null to zeros for value types.</param>
	///<param name="bytes">Supplies an IntPtr wrapped pointer to the raw data block to be converted.</param>
	///<param name="size">Size in bytes of the data area at bytes.</param>
	///<param name="coltyp">ESE column type of the source data.</param>
	///<param name="cp">Code page, needed for text types. Must be 1252 for ASCII or 1200 for Unicode when specifying a string object type.</param>
	static Object ^BuiltinValueBytesToObject(bool %success, Type ^type, bool isnull, IntPtr bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		if(isnull)
		{
			success = true;
			return nullptr;
		}

		bool inner_success = false; //need reference, have handle reference

		Object ^o = from_memblock(inner_success, type, bytes.ToPointer(), size, System::Convert::ToInt32(coltyp), cp);

		success = inner_success;

		return o;
	}

	///<summary>Converts a byte array for the specified type to a managed object.</summary>
	///<remarks>The default implementation raises an invalid type conversion error.</remarks>
	///<param name="type">Provides the System::Type that the data should be converted to. If successful, the returned object is guaranteed to be of the specified type. Use Object's type to retrieve the defaul type based on the column type.</param>
	///<param name="isnull">True iff the database value is null. If False, bytes may still be 0, in which case the value is empty. The default conversion returns null. Retrieval functions convert null to zeros for value types.</param>
	///<param name="bytes">Supplies a byte array containing the raw source data.</param>
	///<param name="coltyp">ESE column type of the source data.</param>
	///<param name="cp">Code page, needed for text types. Must be 1252 for ASCII or 1200 for Unicode when specifying a string object type.</param>
	virtual Object ^ValueBytesToObject(Type ^type, bool isnull, array<uchar> ^bytes, Column::Type coltyp, ushort cp)
	{
		ThrowConversionError(coltyp, type);
		return nullptr;
	}

	///<summary>Converts an IntPtr to an unmanaged block of raw data from the database to a managed object.</summary>
	///<remarks>The memory block is not persited after the function returns.
	///<pr/>Recommended for use with System.Runtime.InteropServices.Marshall.
	///<pr/>Default implementation attempts a default conversion, and if it fails calls the byte array version of this function.
	///</remarks>
	///<param name="type">Provides the System::Type that the data should be converted to. If successful, the returned object is guaranteed to be of the specified type. Use Object's type to retrieve the defaul type based on the column type.</param>
	///<param name="isnull">True iff the database value is null. If False, bytes may still be 0, in which case the value is empty. The default conversion returns null. Retrieval functions convert null to zeros for value types.</param>
	///<param name="bytes">Supplies an IntPtr wrapped pointer to the raw data block to be converted.</param>
	///<param name="size">Size in bytes of the data area at bytes.</param>
	///<param name="coltyp">ESE column type of the source data.</param>
	///<param name="cp">Code page, needed for text types. Must be 1252 for ASCII or 1200 for Unicode when specifying a string object type.</param>
	virtual Object ^ValueBytesToObject(Type ^type, bool isnull, IntPtr bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		bool success;
		Object ^o = BuiltinValueBytesToObject(success, type, isnull, bytes, size, coltyp, cp);

		if(success)
			return o;
		else
		{
			array<uchar> ^arr = gcnew array<uchar>(size);
			System::Runtime::InteropServices::Marshal::Copy(bytes, arr, 0, size);
			return ValueBytesToObject(type, isnull, arr, coltyp, cp);
		}
	}

	///<summary>Retrieves the bytes representing an object to be stored in ESE.</summary>
	///<remarks>Certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This is the first object->value conversion method attempted.
	///<pr/>Return null to use the ObjectValueSize unmanaged conversion (or the built in conversions, which will occur after that).
	///<pr/>The default implementation always returns null.
	///</remarks>
	///<param name="coltyp">ESE column type of the destination column.</param>
	///<param name="cp">Code page, needed for text types. Will be 1252 for ASCII or 1200 for Unicode for a text column type.</param>
	///<param name="isnull">Set isnull to True to use a null value in the database. The return value will be ignored in this case.</param>
	virtual array<byte> ^ValueBytesFromObject(Object ^o, bool %isnull, Column::Type coltyp, ushort cp)
	{
		return nullptr;
	}

	///<summary>See ValueBytesFromObject's allocator parameter.</summary>
	delegate IntPtr Allocator(ulong size);

	///<summary>Retrieves the bytes representing an object to be stored in ESE.</summary>
	///<param name="allocator">Allocates a block of unmanaged memory for use as a return value. Allocator can be called multiple times. The memory is allocated in the C++ heap and EseObjects will automatically take ownership of any allocations when this function returns.</param>
	///<param name="size">Size in bytes of the data area returned. This value must be set unless isnull was set to true or returning IntPtr.Zero or storing a zero-length value.</param>
	///<param name="coltyp">ESE column type of the destination column.</param>
	///<param name="cp">Code page, needed for text types. Will be 1252 for ASCII or 1200 for Unicode for a text column type.</param>
	///<param name="isnull">Set isnull to True to use a null value in the database. The return value will be ignored in this case.</param>
	///<returns>A pointer to a pinned block of memory that will be availaible until the original operation completes or allocated from Allocator representing the bytes to store in ESE.</returns>
	///<remarks>Certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This function is only called if the byte array version returns null.
	///<pr/>Return IntPtr.Zero to attempt a built in conversion. The default implementation always returns IntPtr.Zero.
	///<pr/>If data is allocated by some other means, it must be pinned and availaible at least until the requesting set operation completes.
	///</remarks>
	virtual IntPtr ValueBytesFromObject(Object ^o, bool %isnull, Column::Type coltyp, ushort cp, Allocator ^allocator, ulong %size)
	{
		return IntPtr::Zero;
	}

	///<summary>Creates a single object T from an array of multiple values. Each value is retreived using ValueBytesToObject.</summary>
	///<remarks>Default implementation returns the array of values</remarks>
	generic <class T> virtual Object ^MultivalueToObject(array<Object ^> ^values)
	{
		return values;
	}

	///<summary>Creates a single object T from an array of multiple values. Each value is retreived using ValueBytesToObject.</summary>
	///<remarks>Default implementation casts to an array of object, if an array or builds an array if o is an ICollection.</remarks>
	virtual array<Object ^> ^MultivalueFromObject(Object ^o)
	{
		{
			array<Object ^> ^arr = dynamic_cast<array<Object ^> ^>(o);

			if(arr)
				return arr;
		}

		{
			System::Collections::ICollection ^coll = dynamic_cast<System::Collections::ICollection ^>(o);

			if(coll)
			{
				int ecount = coll->Count;

				array<Object ^> ^arr = gcnew array<Object ^>(ecount);
				
				System::Collections::IEnumerator ^e = coll->GetEnumerator();
						
				for(int i = 0; i < ecount; i++)
				{
					arr[i] = e->Current;
					e->MoveNext();
				}

				return arr;
			}
		}

		ThrowConversionError(o->GetType(), array<Object ^>::typeid);
		return nullptr;
	}
};

Bridge ^GetDefaultBridge()
{
	return gcnew Bridge();
}

ref class FreeListAllocatorObj
{
	free_list *fl;

internal:
	FreeListAllocatorObj(free_list *fl) :
		fl(fl)
	{}

	IntPtr Allocate(ulong size)
	{
		return IntPtr(fl->alloc_array_zero<uchar>(size));
	}
};

void to_memblock_bridge(Bridge ^b, Object ^o, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	Column::Type Coltyp = safe_cast<Column::Type>(coltyp);

	bool user_null = false;
	array<uchar> ^user_array = b->ValueBytesFromObject(o, user_null, safe_cast<Column::Type>(coltyp), cp);

	if(user_null)
		goto exit_return_null;

	//user byte array availaible
	if(user_array != nullptr)
	{
		to_memblock(user_array, buff, max, empty, coltyp, cp, mc, fl);
		return;
	}

	//try user conversion for type
	{
		ulong size = 0;
		FreeListAllocatorObj ^flao = gcnew FreeListAllocatorObj(&fl);
		Bridge::Allocator ^allocator = gcnew Bridge::Allocator(flao, &FreeListAllocatorObj::Allocate);

		buff = b->ValueBytesFromObject(o, user_null, Coltyp, cp, allocator, size).ToPointer();
		max = size;

		if(user_null)
			goto exit_return_null;

		if(size == 0 && buff)
		{
			empty = true;
			buff = null;
			max = 0;
			return;
		}

		if(buff) //user conversion performed
			return;
	}

	//use built in conversion
	if(o == nullptr)
		goto exit_return_null;

	if(!to_memblock(o, buff, max, empty, coltyp, cp, mc, fl))
		Bridge::ThrowConversionError(Coltyp, o->GetType());

	return;

	//return null value
exit_return_null:
		empty = false;
		buff = null;
		max = 0;
}