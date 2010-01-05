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
		throw gcnew InvalidOperationException("Data type conversion not defined from" + coltyp.ToString() + " to " + type->ToString());
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
		ThrowConversionError(type, coltyp);
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

	///<summary>Retrieves the bytes representing an object.</summary>
	///<remarks>Note that certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This is the first object->value conversion method attempted.
	///<pr/>Set isnull to True to use a null value in the database. This overrides returning null to use the GetValueSize path.
	///<pr/>Return null to use the ObjectValueSize unmanaged conversion (or the built in conversions, which will occur after that).
	///<pr/>The default implementation always returns null.
	///</remarks>
	virtual array<byte> ^ValueBytesFromObject(Object ^o, bool %isnull, Column::Type coltyp, ushort cp)
	{
		return nullptr;
	}

	///<summary>Retrieves the size in bytes needed to store the specified object. The specified amount of memory will be allocated for ValueObjectToBytes.</summary>
	///<remarks>Note that certain column types have a preset size and it is an error to return a different size in those cases.
	///<pr/>This method is not used to size builtin conversions; these must use a different method.
	///<pr/>This method must be implemented to convert values from objects.
	///<pr/>Set isnull to True if the database value should be set as null (isnull defaults to False).
	///<pr/>If 0 is returned, ValueBytesFromObject will not be called.
	///<pr/>Return 0 with isnull = false to set an empty value in the database.
	///<pr/>Return 0 with isnull = true to set a null value in the database.
	///<pr/>Return UInt32.MaxValue to attempt a built in conversion. The default implementation always returns UInt32.MaxValue.
	///</remarks>
	virtual ulong ObjectValueSize(Object ^o, bool %isnull, Column::Type coltyp, ushort cp)
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

	///<summary>Creates an object from the data in the current row, represented by IReadRecord.</summary>
	generic <class T> virtual T RowToObject(IReadRecord ^rr)
	{
		throw gcnew NotImplementedException();
	}

	///<summary>Set up to insert a new row from the specified object. Update will be implicitly completed unless an exception is thrown.</summary>
	virtual void RowInsertFromObject(IWriteRecord ^wr, Object ^o)
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
	ulong user_size = b->ObjectValueSize(o, user_null, Coltyp, cp);

	if(user_null)
		goto exit_return_null;

	switch(user_size)
	{
	case 0:
		empty = true;
		buff = null;
		max = 0;
		return;

	case UInt32::MaxValue:
		break; //continue to builtin conversion

	default:
		//use user conversion for this type
		buff = fl.alloc_array_zero<uchar>(user_size);
		max = user_size;
		b->ValueBytesFromObject(o, buff, max, Coltyp, cp);
		return;
	}

	if(o == nullptr)
		goto exit_return_null;

	//use built in conversion
	if(!to_memblock(o, buff, max, empty, coltyp, cp, mc, fl))
		Bridge::ThrowConversionError(Coltyp, o->GetType());

	return;

	//return null value
exit_return_null:
		empty = false;
		buff = null;
		max = 0;
}