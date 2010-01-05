///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  ForwardReferences - Various forward references needed by components in EseObjects
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

ref class Session;
ref class Database;
ref class TableID;
ref class Table;
ref class Cursor;
ref class Column;
ref class Key;
ref struct Bridge;
interface struct ReadRecord;
interface struct WriteRecord;

JET_TABLEID GetTableTableID(Table ^Tab);
JET_SESID GetTableSesid(Table ^Tab);
TableID ^GetTableIDObj(Table ^Tab);
Bridge ^GetTableBridge(Table ^Tab);
Table ^MakeTableFromTableID(TableID ^Tabid);

JET_TABLEID GetCursorTableID(Cursor ^Csr);
JET_SESID GetCurosrSesid(Cursor ^Csr);
Bridge ^GetCursorBridge(Cursor ^Csr);

Bridge ^GetDefaultBridge();

///<summary>Extra options for Unicode mapping. Represents JET_UNICODEINDEX.dwMapFlags. See Win32 LCMapMapString for option details. LCMAP_SORTKEY must always be included.</summary>
public value struct UnicodeMapFlags
{
	///<summary>Mandatory to be set to True</summary>
	bool SortKey;
	bool ByteReverse;
	bool IgnoreCase;
	bool IgnoreNonSpace;
	bool IgnoreSymbols;
	bool IgnoreKanaType;
	bool IgnoreWidth;
	bool StringSort;
};

JET_GRBIT UnicodeMapFlagsToBits(UnicodeMapFlags o)
{
	//bool converts to either 1 for true or 0 for false
	//multiplying to either identity of the bitmask or 0
	JET_GRBIT b = 0;
	b |= o.SortKey * LCMAP_SORTKEY;
	b |= o.ByteReverse * LCMAP_BYTEREV;
	b |= o.IgnoreCase * NORM_IGNORECASE;
	b |= o.IgnoreNonSpace * NORM_IGNORENONSPACE;
	b |= o.IgnoreSymbols * NORM_IGNORESYMBOLS;
	b |= o.IgnoreKanaType * NORM_IGNOREKANATYPE;
	b |= o.IgnoreWidth * NORM_IGNOREWIDTH;
	b |= o.StringSort * SORT_STRINGSORT;
	return b;
}

UnicodeMapFlags UnicodeMapFlagsFromBits(JET_GRBIT b)
{
	UnicodeMapFlags o;
	o.SortKey = b & LCMAP_SORTKEY;
	o.ByteReverse = b & LCMAP_BYTEREV;
	o.IgnoreCase = b & NORM_IGNORECASE;
	o.IgnoreNonSpace = b & NORM_IGNORENONSPACE;
	o.IgnoreSymbols = b & NORM_IGNORESYMBOLS;
	o.IgnoreKanaType = b & NORM_IGNOREKANATYPE;
	o.IgnoreWidth = b & NORM_IGNOREWIDTH;
	o.StringSort = b & SORT_STRINGSORT;
	return o;
}

///<summary>Single column value pair.</summary>
public value struct Field
{
	///<summary>Column the field belongs to.</summary>
	Column ^Col;
	///<summary>Value of the field.</summary>
	Object ^Val;

	Field(Column ^Col, Object ^Val) :
		Col(Col),
		Val(Val)
	{}
};

///<summary>Subset of Cursor methods and properties for safe readonly access to a single row. See Cursor for descriptions.</summary>
public interface struct IReadRecord
{
	value struct RetrieveOptions
	{
		///<summary>A SizeHint that accurately covers the amount of data in the field can increase efficency.
		///Too small a size will still succeed, but it will require two calls to JetRetrieveColumn.
		///Zero uses a default size suitable for small values.
		///Excessively large values will increase temporary memory usage.
		///</summary>
		ulong SizeHint;
		///<summary>The maximum number of bytes to be returned. Certain column types must be fully retrieved.
		///Suggested for use with long values.
		///0 imposes no limit.
		///</summary>
		ulong SizeLimit;
		///<summary>Retreive modified value instead of original value.</summary>
		bool RetrieveCopy;
		///<summary>Retrieve value from index without accessing main record if possible. Should not be used with the clustered index. Not compatible with RetrieveFromPrimaryBookmark.</summary>
		bool RetrieveFromIndex;
		///<summary>Retrieve value from main record instead of using the index. Should not be used with the clustered index. Not compatible with RetrieveFromIndex.</summary>
		bool RetrieveFromPrimaryBookmark;
		///<summary>Count null tagged entries in TagSequence. If false, they are skipped.</summary>
		bool RetrieveNull;
		///<summary>Retreive NULL value if the multivalued column has no set records instead of the default value.</summary>
		bool RetrieveIgnoreDefault;
		///<summary>Retrieves tuple segment of index. RetrieveFromIndex must be set.</summary>
		bool RetrieveTuple;
		///<summary>Offset into long value to begin reading.</summary>
		ulong RetrieveOffsetLV;
		///<summary>Sequence number of tagged field to read.</summary>
		ulong RetrieveTagSequence;
	};

	///<summary>Duplicates the current JET_TABLEID and returns a new Table object.</summary>
	property EseObjects::Table ^Table{EseObjects::Table ^get();}

	///<summary>Reuses the current JET_TABLEID as a new Table object. By sharing the JET_TABLEID, disposing from either will affect both.</summary>
	property EseObjects::Table ^AsTable{EseObjects::Table ^get();}

	generic <class T> T Retrieve(Column ^Col);
	generic <class T> T Retrieve(Column ^Col, RetrieveOptions ro);
	generic <class T> array<T> ^RetrieveAllValues(Column ^Col);
	generic <class T> array<T> ^RetrieveAllValues(Column ^Col, ulong SizeLimit);

	Object ^Retrieve(Column ^Col, Type ^Type);
	Object ^Retrieve(Column ^Col, Type ^Type, RetrieveOptions ro);
	array<Object ^> ^RetrieveAllValues(Column ^Col, Type ^Type);
	array<Object ^> ^RetrieveAllValues(Column ^Col, Type ^Type, ulong SizeLimit);

	ulong RetrieveIndexTagSequence(Column ^Col);
	array<Field> ^RetreiveAllFields(ulong SizeLimit);
	array<Field> ^RetreiveAllFields();
};

ulong RetrieveOptionsFlagsToBits(IReadRecord::RetrieveOptions ro)
{
	ulong flags = 0;
	flags |= ro.RetrieveCopy * JET_bitRetrieveCopy;
	flags |= ro.RetrieveFromIndex * JET_bitRetrieveFromIndex;
	flags |= ro.RetrieveFromPrimaryBookmark * JET_bitRetrieveFromPrimaryBookmark;
	//flags |= ro.RetrieveTag * JET_bitRetrieveTag; //see RetrieveIndexTagSequence;
	flags |= ro.RetrieveNull * JET_bitRetrieveNull;
	flags |= ro.RetrieveIgnoreDefault * JET_bitRetrieveIgnoreDefault;
	flags |= ro.RetrieveTuple * JET_bitRetrieveLongId;
	return flags;
}

///<summary>Subset of Cursor.Update methods and properties for filling in the fields of a single row. See Cursor.Update for descriptions.</summary>
public interface struct IWriteRecord
{
	///<summary>Extra options availabile when setting a value.</summary>
	value struct SetOptions
	{
		///<summary>Append to the end of a long value (instead of overwriting a section).</summary>
		bool AppendLV;
		///<summary>Overwrite entire long value (instead of overwriting a section).</summary>
		bool OverwriteLV;
		///<summary>Byte offset into long value to set data.</summary>
		ulong OffsetLV;
		///<summary>Tag sequence to modify in a tagged column. Zero appends a new value to the end.</summary>
		ulong TagSequence;
	};

	property IReadRecord ^Read {IReadRecord ^get();}
	void Set(Column ^Col, Object ^Value);
	void Set(Column ^Col, Object ^Value, SetOptions so);
};

JET_GRBIT SetOptionsFlagsToBits(IWriteRecord::SetOptions so)
{
	JET_GRBIT flags = 0;
	flags |= so.AppendLV * JET_bitSetAppendLV;
	flags |= so.OverwriteLV * JET_bitSetOverwriteLV;
	return flags;
}
