///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Column - Column definition and properties
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

///<summary>
///Represents a single column in a table. Stores the name of the column (many ESE functions operate on only the column name) a JET_COLUMNID for fast selection and the properties of the column.
///Renaming the column will cause the name to go out of sync with other ESEObjects.Column objects. See RenameColumn for details.
///There are no disposable resources associated with an instance of this class.
///</summary>
public ref class Column
{
internal:
	JET_COLUMNID _JetColID;
	JET_COLTYP _JetColTyp;
	ushort _CP;

private:
	String ^_ColumnName;
	ushort _Country;
	ushort _Langid;
	ushort _Collate;
	ulong _MaxLength;
	ulong _Flags;
	String ^_BaseTableName;
	String ^_BaseColumnName;

	Column(JET_COLUMNID ColID, String ^Name) :
		_JetColID(ColID),
		_ColumnName(Name)
	{}

	JET_COLUMNDEF GetColDef(JET_SESID sesid, JET_TABLEID tableid)
	{
		JET_COLUMNID local_colid = _JetColID;
		JET_COLUMNDEF def = {sizeof def};

		EseException::RaiseOnError(JetGetTableColumnInfo(sesid, tableid, reinterpret_cast<char const *>(&local_colid), &def, sizeof def, JET_ColInfoByColid));

		return def;
	}

	JET_COLUMNBASE GetColBase(JET_SESID sesid, JET_TABLEID tableid)
	{
		marshal_context mc;
		String ^LocalNameHandle = _ColumnName;
		char const *NameStr = mc.marshal_as<char const *>(LocalNameHandle);
		JET_COLUMNBASE def = {sizeof def};

		EseException::RaiseOnError(JetGetTableColumnInfo(sesid, tableid, NameStr, &def, sizeof def, JET_ColInfoBase));

		return def;
	}

	void SetFieldsFromColumnDef(JET_COLUMNDEF &jcd)
	{
		_JetColID = jcd.columnid;
		_JetColTyp = jcd.coltyp;
		_Country = jcd.wCountry;
		_Langid = jcd.langid;
		_CP = jcd.cp;
		_Collate = jcd.wCollate;
		_MaxLength = jcd.cbMax;
		_Flags = jcd.grbit;
	}

internal:
	Column(JET_COLUMNDEF &jcd, String ^Name, String ^BaseTableName, String ^BaseColumnName) :
		_ColumnName(Name),
		_BaseTableName(BaseTableName),
		_BaseColumnName(BaseColumnName)
	{
		SetFieldsFromColumnDef(jcd);
	}

public:
	///<summary>Data types supported for data fields in ESE</summary>
	enum struct Type
	{
		///<summary>Invalid type, used as a default and placeholder.</summary>
		Nil = 0,
		///<summary>A single bit, false or true. Cannot be null. One byte of storage.</summary>
		Bit = JET_coltypBit,
		///<summary>Ranging 0 to 255. Equivalent to System.Byte.</summary>
		UnsignedByte = JET_coltypUnsignedByte,
		///<summary>Ranging -32768 to 32767. Equivalent to System.Short.</summary>
		Short = JET_coltypShort,
		///<summary>Ranging -2147483648 to 2147483647. Equivalent to System.Integer.</summary>
		Long = JET_coltypLong,
		///<summary>Ranging -9223372036854775808 to 9223372036854775807. Equivalent to System.Long.</summary>
		Currency = JET_coltypCurrency,
		///<summary>4 byte floating point number. Equivalent to System.Single.</summary>
		SingleFloat = JET_coltypIEEESingle,
		///<summary>8 byte floating point number. Equivalent to System.Double.</summary>
		DoubleFloat = JET_coltypIEEEDouble,
		///<summary>8 byte floating point number representing the fractional days since the year 1900.</summary>
		DateTime = JET_coltypDateTime,
		///<summary>Binary field up to 255 bytes in length.</summary>
		Binary = JET_coltypBinary,
		///<summary>
		///Text field up to 255 bytes in length.
		///Ascii strings are always case insensitive and stop at the first null for sorting and searching.
		///Unicode strings sort according to LCMapString. See LCID and Index.UnicodeMapFlags.
		///</summary>
		Text = JET_coltypText,
		///<summary>Long binary field up to 2147483647 bytes in length. Can be stored out of row and accessed as a stream.</summary>
		LongBinary = JET_coltypLongBinary,
		///<summary>Long text field up to 2147483647 bytes in length. Can be stored out of row and accessed as a stream. Same sorting properties as Text.</summary>
		LongText = JET_coltypLongText,
		///<summary>Ranging 0 to 4294967295. Requires 6.0+. Equivalent to System.UInt32</summary>
		UnsignedLong = JET_coltypUnsignedLong,
		///<summary>Ranging -9223372036854775808 to 9223372036854775807. Requires 6.0+. Also equivalent to System.Long.</summary>
		LongLong = JET_coltypLongLong,
		///<summary>Fixed length 16 byte binary column that sorts the same as a GUID string would in standard form.</summary>
		GUID = JET_coltypGUID,
		///<summary>Ranging 0 to 65535. Requires 6.0+. Equivalent to System.UInt16.</summary>
		UnsignedShort = JET_coltypUnsignedShort
	};

	///<summary>Jet only supports two code pages: English 1 byte chars (1252) or Unicode (1200). Zero is used for non-text fields.</summary>
	enum struct CodePage : UInt32
	{
		Zero = 0,
		English = 1252,
		Unicode = 1200
	};

	///<summary>Parameters for creating a new column</summary>
	value struct CreateOptions
	{
		///<summary>Name of the column. See Jet internal name restrictions. Can not be used with a temp table.</summary>
		String ^Name;
		///<summary>Data type of column. Note that the character set used by text fields is determined by the Unicode flag.</summary>
		Type Type;
		///<summary>Maximum length of a variable field, set length of a fixed size field.</summary>
		ulong MaxLength;
		///<summary>Default value. Must be a type that can be converted to the ColumnType specified. Can not be used with a temp table.</summary>
		Object ^DefaultValue;
		///<summary>Determines if the field is English (cp 1252) or Unicode (cp 1200).</summary>
		CodePage CP;
		///<summary>Column has a fixed width, always using the same storage in a given row. Not compatible with long data types or tagged columns.</summary>
		bool Fixed;
		///<summary>Column is tagged. Tagged columns do not take up space if null.</summary>
		bool Tagged;
		///<summary>Column is not allowed to be null.</summary>
		bool NotNull;
		///<summary>This column specifies the version of the row. Incremented with each update on the row. Must be a Long column. Not compatible with Tagged, AtoIncrement or EscrowUpdate. Can not be used with a temp table.</summary>
		bool Version;
		///<summary>Column is automatically incremented each time a row could be added to the table. Not guaranteed to be contiguous. Must be type Long or Currency. Can not be used with a temp table.</summary>
		bool Autoincrement;
		///<summary>For use only with temp tables. Make this field part of the temp table's key. Key order is the same as the column order.</summary>
		bool TTKey;
		///<summary>For use only with temp tables. Make key sort order for this column descending.</summary>
		bool TTDescending;
		///<summary>Allows a tagged column to contain multiple instances of the same field in a singlw row.</summary>
		bool MultiValued;
		///<summary>Allows concurrent update with EscrowUpdate. Must be type Long. Not compatible with Tagged, Version or Autoincrement. Can not be used with a temp table.</summary>
		bool EscrowUpdate;
		///<summary>Column created without a version. Other transactions attempting to add a column with the same concurrently name will fail. Can not be used with a temp table.</summary>
		bool Unversioned;
		///<summary>Deletes the row when the value in this escrow field reaches zero. Can not be used with a temp table.</summary>
		bool DeleteOnZero;

		//convenience constructors
		CreateOptions(String ^Name, Column::Type Type) :
			Name(Name),
			Type(Type)
		{}

		CreateOptions(String ^Name, Column::Type Type, ulong MaxLength, bool Fixed) :
			Name(Name),
			Type(Type),
			MaxLength(MaxLength),
			Fixed(Fixed)
		{}

		CreateOptions(String ^Name, Column::Type Type, CodePage CP) :
			Name(Name),
			Type(Type),
			CP(CP)
		{}

		CreateOptions(String ^Name, Column::Type Type, CodePage CP, ulong MaxLength, bool Fixed) :
			Name(Name),
			Type(Type),
			CP(CP),
			MaxLength(MaxLength),
			Fixed(Fixed)
		{}

		//NEXT: callbacks:
		//bool ColumnFinalize; //requires callbacks
		//bool UserDefinedDefault; //requries callbacks
	};

internal:
	static JET_GRBIT CreateOptionsFlagsToBits(CreateOptions %o)
	{
		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		JET_GRBIT b = 0;
		b |= o.Fixed * JET_bitColumnFixed;
		b |= o.Tagged * JET_bitColumnTagged;
		b |= o.NotNull * JET_bitColumnNotNULL;
		b |= o.Version * JET_bitColumnVersion;
		b |= o.Autoincrement * JET_bitColumnAutoincrement;
		b |= o.TTKey * JET_bitColumnTTKey;
		b |= o.TTDescending * JET_bitColumnTTDescending;
		b |= o.MultiValued * JET_bitColumnMultiValued;
		b |= o.EscrowUpdate * JET_bitColumnEscrowUpdate;
		b |= o.Unversioned * JET_bitColumnUnversioned;
		b |= o.DeleteOnZero * JET_bitColumnDeleteOnZero;
		return b;
	}

public:
	///<summary>Opens the specified column. Calls JetGetTableColumnInfo.</summary>
	Column(Table ^Table, String ^Name) :
		_JetColID(null),
		_ColumnName(Name)
	{
		marshal_context mc;
		char const *NameStr = mc.marshal_as<char const *>(Name);
		JET_COLUMNDEF jcd = {sizeof jcd};
		JET_TABLEID tableid = GetTableTableID(Table);
		JET_SESID sesid = GetTableSesid(Table);

		EseException::RaiseOnError(JetGetTableColumnInfo(sesid, tableid, NameStr, &jcd, sizeof jcd, JET_ColInfo));

		SetFieldsFromColumnDef(jcd);

		JET_COLUMNBASE jcb(GetColBase(sesid, tableid));

		_BaseTableName = marshal_as<String ^>(jcb.szBaseTableName);
		_BaseColumnName = marshal_as<String ^>(jcb.szBaseColumnName);
	}

	///<summary>Opens the specified column. Calls JetGetTableColumnInfo.</summary>
	Column(Cursor ^Csr, String ^Name) :
		_JetColID(null),
		_ColumnName(Name)
	{
		marshal_context mc;
		char const *NameStr = mc.marshal_as<char const *>(Name);
		JET_COLUMNDEF jcd = {sizeof jcd};
		JET_TABLEID tableid = GetCursorTableID(Csr);
		JET_SESID sesid = GetCursorSesid(Csr);

		EseException::RaiseOnError(JetGetTableColumnInfo(sesid, tableid, NameStr, &jcd, sizeof jcd, JET_ColInfo));

		SetFieldsFromColumnDef(jcd);

		JET_COLUMNBASE jcb(GetColBase(sesid, tableid));

		_BaseTableName = marshal_as<String ^>(jcb.szBaseTableName);
		_BaseColumnName = marshal_as<String ^>(jcb.szBaseColumnName);
	}

	///<summary>Creates a new column in the specified table. Calls JetAddColumn.</summary>
	static Column ^Create(Table ^Table, CreateOptions Parameters)
	{
		marshal_context mc;
		JET_COLUMNDEF jcd = {sizeof jcd};
		JET_TABLEID tableid = GetTableTableID(Table);
		JET_SESID sesid = GetTableSesid(Table);

		jcd.coltyp = System::Convert::ToUInt32(Parameters.Type);
		jcd.cp = System::Convert::ToUInt32(Parameters.CP);
		jcd.cbMax = Parameters.MaxLength;
		jcd.grbit = CreateOptionsFlagsToBits(Parameters);

		char const *NameStr = mc.marshal_as<char const *>(Parameters.Name);

		JET_COLUMNID newcolid = 0;
		
		EseException::RaiseOnError(JetAddColumn(sesid, tableid, NameStr, &jcd, null, 0, &newcolid));

		Column ^ret = gcnew Column(newcolid, Parameters.Name);

		ret->_JetColTyp = jcd.coltyp;
		ret->_Country = jcd.wCountry;
		ret->_Langid = jcd.langid;
		ret->_CP = jcd.cp;
		ret->_Collate = jcd.wCollate;
		ret->_MaxLength = jcd.cbMax;
		ret->_Flags = jcd.grbit;
		
		return ret;
	}

	///<summary>Deletes this column. Must be associated with the specified table. Calls JetDeleteColumn.</summary>
	static void Delete(Table ^SrcTable, String ^ColumnName)
	{
		marshal_context mc;
		String ^ColNameHandleCopy = ColumnName;
		JET_TABLEID tableid = GetTableTableID(SrcTable);
		JET_SESID sesid = GetTableSesid(SrcTable);

		char const *ColNameStr = mc.marshal_as<char const *>(ColNameHandleCopy);

		EseException::RaiseOnError(JetDeleteColumn(sesid, tableid, ColNameStr));
	}

	///<summary>
	///Changes the name of a column. Requires 5.1+. Calls JetRenameColumn.
	///Note: This function is not transactional and will be visible to other transactions immediately upon success.
	///Note: Each ESEObjects.Column object contains the JET_COLUMNID and name of the column it represents.
	///Although the name stored in this Column object will be updated in this function, any other Column objects will still have the old name. The JET_COLUMNID is not affected by this operation.
	///ESE has no way to report column name changes and ESEObjects does not intern Column objects.
	///Trying to use another column object that still refers to the old name with an ESE operation that requires the name will fail.
	///Note that Table objects do not have the same restriction since the name is always queried from a JET_TABLEID as needed. (no comparable function exists for columns).
	///</summary>
	void RenameColumn(Table ^SrcTable, String ^NewName)
	{
		marshal_context mc;
		String ^ColNameHandleCopy = _ColumnName;
		JET_TABLEID tableid = GetTableTableID(SrcTable);
		JET_SESID sesid = GetTableSesid(SrcTable);

		char const *OldColNameStr = mc.marshal_as<char const *>(ColNameHandleCopy);
		char const *NewColNameStr = mc.marshal_as<char const *>(NewName);

		EseException::RaiseOnError(JetRenameColumn(sesid, tableid, OldColNameStr, NewColNameStr, 0));

		_ColumnName = NewName;
	}

	///<summary>Internal identity of column used by ESE.</summary>
	property ulong JetColumnID {ulong get() {return _JetColID;}}

	///<summary>Name of the column at the time the Column object was created.</summary>
	property String ^Name {String ^get() {return _ColumnName;}}

	///<summary>Data type of column.</summary>
	property Type ColumnType
	{
		Type get() {return safe_cast<Type>(_JetColTyp);}
	}

	property ushort Country {ushort get() {return _Country;}}

	property ushort Langid {ushort get() {return _Langid;}}

	property CodePage CP{CodePage get() {return safe_cast<CodePage>(_CP);}}

	property ulong MaxLength {ulong get() {return _MaxLength;}}

	property bool Fixed {bool get() {return _Flags & JET_bitColumnFixed;}}
	property bool Tagged {bool get() {return _Flags & JET_bitColumnTagged;}}
	property bool NotNull {bool get() {return _Flags & JET_bitColumnNotNULL;}}
	property bool Version {bool get() {return _Flags & JET_bitColumnVersion;}}
	property bool Autoincrement {bool get() {return _Flags & JET_bitColumnAutoincrement;}}
	property bool Updatable {bool get() {return _Flags & JET_bitColumnUpdatable;}}	
	property bool TTKey {bool get() {return _Flags & JET_bitColumnTTKey;}}
	property bool TTDescending {bool get() {return _Flags & JET_bitColumnTTDescending;}}
	property bool MultiValued {bool get() {return _Flags & JET_bitColumnMultiValued;}}
	property bool EscrowUpdate {bool get() {return _Flags & JET_bitColumnEscrowUpdate;}}
	property bool Unversioned {bool get() {return _Flags & JET_bitColumnUnversioned;}}
	property bool DeleteOnZero {bool get() {return _Flags & JET_bitColumnDeleteOnZero;}}

	///<summary>For a derived table, the base table it is derived from.</summary>
	property String ^BaseTableName {String ^get() {return _BaseTableName;}}

	///<summary>For a derived table, the base column this column is derived from.</summary>
	property String ^BaseColumnName {String ^get() {return _BaseColumnName;}}
};

SortedList<JET_COLUMNID, Column ^> ^QueryTableColumns(JET_SESID JetSesid, JET_TABLEID SrcJetTableID)
{
	//going to fill this SortedList with the columns in the table
	//by opening a special temp table cursor with JetGetTableColumnInfo
	//this info is needed to correctly marshal and return results
	SortedList<JET_COLUMNID, Column ^> ^Cols;

	JET_COLUMNLIST jcl = {sizeof jcl};

	EseException::RaiseOnError(JetGetTableColumnInfo(JetSesid, SrcJetTableID, null, &jcl, sizeof jcl, JET_ColInfoListSortColumnid));

	try
	{
		Cols = gcnew SortedList<JET_COLUMNID, Column ^>(jcl.cRecord);
	
		JET_ERR status = JetMove(JetSesid, jcl.tableid, JET_MoveFirst, 0);

		if(status == JET_errNoCurrentRecord)
			return Cols; //apparently, there are no columns

		EseException::RaiseOnError(status);

		while(1)
		{
			char colname[JET_cbNameMost + 1] = {0};
			JET_COLUMNDEF jcd = {sizeof jcd};
			JET_COLUMNBASE jcb = {sizeof jcb};

			JET_RETRIEVECOLUMN jrc[] = 
			{
			/* 0*/	{jcl.columnidcolumnname, &colname, JET_cbNameMost + 1, 0, 0, 0, 1},
			/* 1*/	{jcl.columnidcolumnid, &jcd.columnid, sizeof jcd.columnid, 0, 0, 0, 1},
			/* 2*/	{jcl.columnidcoltyp, &jcd.coltyp, sizeof jcd.coltyp, 0, 0, 0, 1},
			/* 3*/	{jcl.columnidCountry, &jcd.wCountry, sizeof jcd.wCountry, 0, 0, 0, 1},
			/* 4*/	{jcl.columnidLangid, &jcd.langid, sizeof jcd.langid, 0, 0, 0, 1},
			/* 5*/	{jcl.columnidCp, &jcd.cp, sizeof jcd.cp, 0, 0, 0, 1},
			/* 6*/	{jcl.columnidCollate, &jcd.wCollate, sizeof jcd.wCollate, 0, 0, 0, 1},
			/* 7*/	{jcl.columnidcbMax, &jcd.cbMax, sizeof jcd.cbMax, 0, 0, 0, 1},
			/* 8*/	{jcl.columnidgrbit, &jcd.grbit, sizeof jcd.grbit, 0, 0, 0, 1},
			/* 9*/	{jcl.columnidBaseTableName, &jcb.szBaseTableName, sizeof jcb.szBaseTableName, 0, 0, 0, 1},
			/*10*/	{jcl.columnidBaseColumnName, &jcb.szBaseColumnName, sizeof jcb.szBaseColumnName, 0, 0, 0, 1}
			};

			EseException::RaiseOnError(JetRetrieveColumns(JetSesid, jcl.tableid, jrc, _countof(jrc)));

			//marshal text columns to build Column object
			Column ^Col = gcnew Column
			(
				jcd,
				astring_from_memblock(colname, jrc[0].cbActual),
				astring_from_memblock(jcb.szBaseTableName, jrc[9].cbActual),
				astring_from_memblock(jcb.szBaseColumnName, jrc[10].cbActual)
			);
			
			Cols->Add(jcd.columnid, Col);
			
			//proceed to next column record
			status = JetMove(JetSesid, jcl.tableid, JET_MoveNext, 0);
			if(status == JET_errNoCurrentRecord)
				break; //done enumerating
			else
				EseException::RaiseOnError(status); //if there was an error, raise it, otherwise continue
		}
	}
	finally
	{
		JetCloseTable(JetSesid, jcl.tableid);
	}

	return Cols;
}