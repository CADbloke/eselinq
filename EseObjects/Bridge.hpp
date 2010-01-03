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
		throw gcnew InvalidOperationException("Data type conversion not defined from" + coltyp.ToString() + " to " + type->ToString());
	}

	///<summary>Converts a byte array for the specified type to a managed object. The default implementation raises an invalid type conversion error.</summary>
	generic <class T> virtual T ValueBytesToObject(array<uchar> ^bytes, Column::Type coltyp, ushort cp)
	{
		ThrowConversionError(T::typeid, coltyp);
		return T();
	}

	///<summary>Converts an IntPtr to an unmanaged block of raw data from the database to a managed object.</summary>
	///<remarks>The memory block is not persited after the function returns.
	///<pr/>Recommended for use with System.Runtime.InteropServices.Marshall.
	///<pr/>Default implementation calls the byte array version of this method.
	///</remarks>
	generic <class T> virtual T ValueBytesToObject(IntPtr bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		array<uchar> ^arr = gcnew array<uchar>(size);
		System::Runtime::InteropServices::Marshal::Copy(bytes, arr, 0, size);
		return ValueBytesToObject<T>(arr, coltyp, cp);
	}

	///<summary>Converts a pointer to an unmanaged block of raw data from the database to a managed object.</summary>
	///<remarks>The memory block is not persited after the function returns.
	///<pr/>All of the built in conversions are done through this method.
	///<pr/>Default implementation passes any unknown types to the IntPtr version of this method.
	///<pr/>Requested conversions to Object should produce a default type based on the column type. All column types are covered by the default implementation.
	///</remarks>
	generic <class T> virtual T ValueBytesToObject(void *bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		JET_COLTYP jet_coltyp = System::Convert::ToUInt32(coltyp);

		if(T::typeid == Boolean::typeid)
			return safe_cast<T>(from_memblock<Boolean>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Byte::typeid)
			return safe_cast<T>(from_memblock<Byte>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == SByte::typeid)
			return safe_cast<T>(from_memblock<SByte>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Char::typeid)
			return safe_cast<T>(from_memblock<Char>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Single::typeid)
			return safe_cast<T>(from_memblock<Single>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Double::typeid)
			return safe_cast<T>(from_memblock<Double>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Int16::typeid)
			return safe_cast<T>(from_memblock<Int16>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Int32::typeid)
			return safe_cast<T>(from_memblock<Int32>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Int64::typeid)
			return safe_cast<T>(from_memblock<Int64>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == UInt16::typeid)
			return safe_cast<T>(from_memblock<UInt16>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == UInt32::typeid)
			return safe_cast<T>(from_memblock<UInt32>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == UInt64::typeid)
			return safe_cast<T>(from_memblock<UInt64>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Guid::typeid)
			return safe_cast<T>(from_memblock<Guid>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == String::typeid)
			return safe_cast<T>(from_memblock<String ^>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == array<uchar>::typeid)
			return safe_cast<T>(from_memblock<array<uchar> ^>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == DateTime::typeid)
			return safe_cast<T>(from_memblock<DateTime>(bytes, size, jet_coltyp, cp));
		else if(T::typeid == Object::typeid)
			return safe_cast<T>(from_memblock(bytes, size, jet_coltyp, cp));
		else if(T::typeid->IsSerializable)
			return safe_cast<T>(from_memblock_binserialize(bytes, size, jet_coltyp, cp));
		else
			return ValueBytesToObject<T>(IntPtr(bytes), size, coltyp, cp);
	}

	///<summary>Retrieves the bytes representing an object.</summary>
	///<remarks>Note that certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This is the first object->value conversion method attempted.
	///<pr/>Return null to use the ObjectValueSize unmanaged conversion (or the built in conversions, which will occur after that).
	///<pr/>The default implementation always returns null.
	///</remarks>
	virtual array<byte> ^ValueBytesFromObject(Object ^o, Column::Type coltyp, ushort cp)
	{
		return nullptr;
	}

	///<summary>Retrieves the size in bytes needed to store the specified object. The specified amount of memory will be allocated for ValueObjectToBytes.</summary>
	///<remarks>Note that certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This method is not used for the built in conversions.
	///<pr/>This method must be implemented to convert values from objects.
	///<pr/>Return UInt32.MaxValue to attempt a built in conversion. The default implementation always returns UInt32.MaxValue.
	///</remarks>
	virtual ulong ObjectValueSize(Object ^o, Column::Type coltyp, ushort cp)
	{
		return UInt32::MaxValue;
	}

	///<summary>Sets bytes to the bytes representing the specified object. Size allocated according to ObjectValueSize.</summary>
	///<remarks>Default implementation raises an error.</remarks>
	virtual void ValueBytesFromObject(Object ^o, IntPtr bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		ThrowConversionError(o->GetType(), coltyp);
	}

	///<summary>Sets bytes to the bytes representing the specified object. Size allocated according to ObjectValueSize.</summary>
	///<remarks>Default implementation calls the IntPtr version of this method.</remarks>
	virtual void ValueBytesFromObject(Object ^o, void *bytes, ulong size, Column::Type coltyp, ushort cp)
	{
		ValueBytesFromObject(o, IntPtr(bytes), size, coltyp, cp);
	}

	///<summary>Creates a single object T from an array of multiple values. Each value is retreived using ValueBytesToObject.</summary>
	///<remarks>Default implementation returns the array of values</remarks>
	virtual Object ^MultivalueToObject(array<Object ^> ^values)
	{
		return values;
	}

	///<summary>Creates an object from the data in the current row, represented by ReadRecord.</summary>
	generic <class T> virtual T RowToObject(ReadRecord ^rr)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Set up to insert a new row from the specified object. Update will be implicitly completed unless an exception is thrown.</summary>
	virtual void RowInsertFromObject(WriteRecord ^wr, Object ^o)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Creates an object from the data obtained from scrolling the cursor forward until there is no current record. The process should not modify the table.</summary>
	///<remarks>Limits and positioning should not be modified. The cursor may be associated with a temporary table.</remarks>
	generic <class T> virtual T CursorToObject(Cursor ^csr)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Creates an object from the data in the specified table. The process should not modify the table.</summary>
	generic <class T> virtual T TableToObject(Table ^tab)
	{
		throw gcnew NotImplementedException();		
	}

	///<summary>Appends data from the specified object to the specified table.</summary>
	virtual void TableAppendFromObject(Object ^o, Table ^tab)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Creates a new table from the specified object.</summary>
	virtual Table ^TableCreateFromObject(Object ^o)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Creates an object from the data in the specified database.</summary>
	generic <class T> virtual T DatabaseToObject(Database ^db)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Appends data from the specified object to the specified database.</summary>
	virtual void DatabaseAppendFromObject(Object ^o, Database ^db)
	{
		throw gcnew NotImplementedException();
	}
};

Bridge ^GetDefaultBridge()
{
	return gcnew Bridge();
}

void to_memblock_bridge(Bridge ^b, Object ^o, void *&buff, ulong &max, bool &empty, JET_COLTYP coltyp, ushort cp, marshal_context %mc, free_list &fl)
{
	if(o == nullptr)
	{
		empty = false;
		buff = null;
		max = 0;
		return;
	}

	array<uchar> ^user_array = b->ValueBytesFromObject(o, safe_cast<Column::Type>(coltyp), cp);

	//user byte array availaible
	if(user_array != nullptr)
	{
		to_memblock(user_array, buff, max, empty, coltyp, cp, mc, fl);
		return;
	}

	ulong user_size = b->ObjectValueSize(o, safe_cast<Column::Type>(coltyp), cp);

	//use user conversion for this type
	if(user_size != UInt32::MaxValue)
	{
		if(user_size == 0)
		{
			empty = true;
			buff = null;
			max = 0;
			return;
		}

		buff = fl.alloc_array_zero<uchar>(user_size);
		max = user_size;
		b->ValueBytesFromObject(o, buff, max, safe_cast<Column::Type>(coltyp), cp);
		return;
	}

	//use built in conversion
	to_memblock(o, buff, max, empty, coltyp, cp, mc, fl);
}