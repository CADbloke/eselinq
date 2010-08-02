///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Cursor - Cursor positioning and data access
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

DemandLoadFunction<JET_ERR (JET_API *)(JET_SESID sesid, JET_TABLEID tableid, JET_RECSIZE *precsize, JET_GRBIT const grbit)> JetGetRecordSize_demand(L"esent.dll", "JetGetRecordSize");

///<summary>Represents an open cursor in the database.
///The object is invalid if the associated database object's session is disposed.
///</summary>
public ref class Cursor sealed : IReadRecord
{
	TableID ^_TableID;
	Index ^_CurrentIndex;

internal:
	Cursor(TableID ^TableID) :
		_TableID(TableID),
		_CurrentIndex(nullptr)
	{}

public:
	///<summary>Opens a new cursor for the corresponding table. Inherits OpenOptions flags used when opening that table originally. Calls JetDupCursor.</summary>
	Cursor(Table ^SrcTable) :
		_TableID(GetTableIDObj(SrcTable)->Duplicate()),
		_CurrentIndex(nullptr)
	{}

	///<summary>
	///Opens a new cursor for the same table as the specified cursor.
	///Inherits OpenOptions flags used when opening that cursor/table originally, but not index selection or cursor position.
	///Primary index selected, positioned to first record.
	///Calls JetDupCursor.
	///</summary>
	Cursor(Cursor ^SrcCursor) :
		_TableID(SrcCursor->_TableID->Duplicate()),
		_CurrentIndex(nullptr)
	{}

	///<summary>Opens an existing table with default options. Lifetime is tied to associated database's session.</summary>
	Cursor(Database ^Db, String ^TableName) :
		_TableID(nullptr),
		_CurrentIndex(nullptr)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(TableName);

		JET_TABLEID NewTableID = null;

		EseException::RaiseOnError(JetOpenTable(Db->Session->_JetSesid, Db->_JetDbid, NameChar, null, 0, 0, &NewTableID));

		_TableID = gcnew EseObjects::TableID(NewTableID, Db->Session->_CurrentTrans, Db);
	}

	~Cursor()
	{
		_TableID->~TableID();
	}

	///<summary>Duplicates the current JET_TABLEID and returns a new Table object.</summary>
	virtual property EseObjects::Table ^Table
	{
		EseObjects::Table ^get()
		{
			return MakeTableFromTableID(_TableID->Duplicate());
		}
	}

	///<summary>Reuses the current JET_TABLEID as a new Table object. By sharing the JET_TABLEID, disposing from either will affect both.</summary>
	virtual property EseObjects::Table ^AsTable
	{
		EseObjects::Table ^get()
		{
			return MakeTableFromTableID(_TableID);
		}
	}

	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _TableID->Session;}
	}

	property EseObjects::Database ^Database
	{
		EseObjects::Database ^get() {return _TableID->Database;}
	}

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _TableID->Bridge;};
		void set(EseObjects::Bridge ^bridge) {_TableID->Bridge = bridge;};
	};

	//move options
	///<summary>When moving the cursor, moves the specified number of equal index elements instead of records. Controls use of JET_bitMoveKeyNE with Move</summary>
	property bool MoveKeyNE;


	//----------------------------Move methods---------------------------------

	///<summary>
	///Moves the cursor to the relative number of records (or key values if MoveKeyNE is set) from the current position.
	///Positive moves forward, negative backward.
	///To move to the next record, use Move(1). Previous record, Move(-1).
	///Calls JetMove.
	///</summary>
	bool Move(long RelativePosition)
	{
		ulong flags = 0;
		flags |= MoveKeyNE * JET_bitMoveKeyNE;

		JET_ERR status = JetMove(Session->_JetSesid, TableID->_JetTableID, RelativePosition, 0);

		if(status == JET_errNoCurrentRecord)
			return false;
		
		EseException::RaiseOnError(status);

		return true;
	}

	///<summary>Moves to the first record in the index. Calls JetMove with JET_MoveFirst.</summary>
	bool MoveFirst()
	{
		JET_ERR status = JetMove(Session->_JetSesid,_TableID->_JetTableID, JET_MoveFirst, 0);

		if(status == JET_errNoCurrentRecord)
			return false;

		EseException::RaiseOnError(status);

		return true;
	}

	///<summary>Moves to the last record in the index. Calls JetMove with JET_MoveLast.</summary>
	bool MoveLast()
	{
		JET_ERR status = JetMove(Session->_JetSesid,_TableID->_JetTableID, JET_MoveLast, 0);

		if(status == JET_errNoCurrentRecord)
			return false;

		EseException::RaiseOnError(status);

		return true;
	}

	///<summary>Deletes the current row selected by the cursor. Calls JetDelete.</summary>
	void Delete()
	{
		EseException::RaiseOnError(JetDelete(Session->_JetSesid, TableID->_JetTableID));
	}

//---------------------------Field retrieval support methods-------------------
internal:
	Object ^Retrieve(Type ^type, JET_COLUMNID colid, JET_COLTYP coltyp, ushort cp, JET_GRBIT flags, ulong size_hint, ulong size_limit, ulong RetrieveOffsetLV, ulong RetrieveTagSequence)
	{
		free_list fl;
		void *buff;

		if(size_hint <= ESEOBJECTS_MAX_ALLOCA) //small enough to allocate on stack for speed
			buff = alloca_array(char, size_hint);
		else
			fl.alloc_array<char>(size_hint);

		ulong buffsz;
		ulong req_buffsz = 0;

		if(size_limit)
			req_buffsz = min(size_hint, size_limit);
		else
			buffsz = size_hint;

		JET_RETINFO ret_info = {sizeof ret_info};

		ret_info.ibLongValue = RetrieveOffsetLV;
		ret_info.itagSequence = RetrieveTagSequence;

		if(ret_info.itagSequence == 0)
			ret_info.itagSequence = 1;

		JET_ERR status = JetRetrieveColumn(Session->_JetSesid, _TableID->_JetTableID, colid, buff, buffsz, &req_buffsz, flags, &ret_info);

		switch(status)
		{
		case JET_errSuccess:
			break;

		case JET_errInvalidBufferSize:
		case JET_wrnBufferTruncated:
		case JET_errBufferTooSmall:
			//can only request up to size limit
			if(size_limit)
			{
				//already at limit?
				if(buffsz == size_limit) 
					break; //already retreived full request
				//still more space to limit, get as much as possible
				req_buffsz = min(req_buffsz, size_limit);
			}
			//buffer needs to be bigger
			buff = fl.alloc_array<char>(req_buffsz);
			buffsz = req_buffsz;

			//this call shouldn't fail in a way we could have fixed here (i.e. buffer too small)
			EseException::RaiseOnError(JetRetrieveColumn(Session->_JetSesid, _TableID->_JetTableID, colid, buff, buffsz, &req_buffsz, flags, &ret_info));
			break;

		case JET_wrnColumnNull:
			return Bridge->ValueBytesToObject(type, true, IntPtr(0), 0, safe_cast<Column::Type>(coltyp), cp);
			
		default:
			//if it was some other error, raise it
			EseException::RaiseOnError(status);
			break;
		}

		return Bridge->ValueBytesToObject(type, false, IntPtr(buff), req_buffsz, safe_cast<Column::Type>(coltyp), cp);
	}

public:
	///<summary>Retrieves the data from the specificd column at the current cursor position. Calls JetRetrieveColumn.
	///Uses the specified retrieval options.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	generic <class T> virtual T Retrieve(Column ^Col, IReadRecord::RetrieveOptions ro)
	{
		JET_GRBIT flags = RetrieveOptionsFlagsToBits(ro);

		return safe_cast<T>(Retrieve(T::typeid, Col->_JetColID, Col->_JetColTyp, Col->_CP, flags, ro.SizeHint, ro.SizeLimit, ro.RetrieveOffsetLV, ro.RetrieveTagSequence));
	}

	///<summary>Retrieves the data from the specificd column at the current cursor position. Calls JetRetrieveColumn.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	generic <class T> virtual T Retrieve(Column ^Col)
	{
		return safe_cast<T>(Retrieve(T::typeid, Col->_JetColID, Col->_JetColTyp, Col->_CP, 0, ESEOBJECTS_MAX_ALLOCA, 0, 0, 1));
	}

	///<summary>Retrieves the data from the specificd column at the current cursor position. Calls JetRetrieveColumn.
	///Uses the specified retrieval options.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	virtual Object ^Retrieve(Column ^Col, Type ^Type, IReadRecord::RetrieveOptions ro)
	{
		JET_GRBIT flags = RetrieveOptionsFlagsToBits(ro);

		return Retrieve(Type, Col->_JetColID, Col->_JetColTyp, Col->_CP, flags, ro.SizeHint, ro.SizeLimit, ro.RetrieveOffsetLV, ro.RetrieveTagSequence);
	}

	///<summary>Retrieves the data from the specificd column at the current cursor position. Calls JetRetrieveColumn.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	virtual Object ^Retrieve(Column ^Col, Type ^Type)
	{
		return Retrieve(Type, Col->_JetColID, Col->_JetColTyp, Col->_CP, 0, ESEOBJECTS_MAX_ALLOCA, 0, 0, 1);
	}

internal:
	JET_COLUMNDEF LookupColumnDef(String ^Name)
	{
		marshal_context mc;
		char const *NameStr = mc.marshal_as<char const *>(Name);
		JET_COLUMNDEF jcd = {sizeof jcd};

		EseException::RaiseOnError(JetGetTableColumnInfo(_TableID->Session->_JetSesid, _TableID->_JetTableID, NameStr, &jcd, sizeof jcd, JET_ColInfo));

		return jcd;
	}

public:
	///<summary>Retrieves the data from the specificd column by name at the current cursor position. Calls JetGetTableColumnInfo and JetRetrieveColumn.
	///Less efficient than using a Column object to identify the column.
	///Uses the specified retrieval options.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	generic <class T> virtual T Retrieve(String ^Col, IReadRecord::RetrieveOptions ro)
	{
		JET_GRBIT flags = RetrieveOptionsFlagsToBits(ro);
		JET_COLUMNDEF jcd = LookupColumnDef(Col);

		return safe_cast<T>(Retrieve(T::typeid, jcd.columnid, jcd.coltyp, jcd.cp, flags, ro.SizeHint, ro.SizeLimit, ro.RetrieveOffsetLV, ro.RetrieveTagSequence));
	}

	///<summary>Retrieves the data from the specificd column by name at the current cursor position. Calls JetGetTableColumnInfo and JetRetrieveColumn.
	///Less efficient than using a Column object to identify the column.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	generic <class T> virtual T Retrieve(String ^Col)
	{
		JET_COLUMNDEF jcd = LookupColumnDef(Col);

		return safe_cast<T>(Retrieve(T::typeid, jcd.columnid, jcd.coltyp, jcd.cp, 0, ESEOBJECTS_MAX_ALLOCA, 0, 0, 1));
	}

	///<summary>Retrieves the data from the specificd column by name at the current cursor position. Calls JetGetTableColumnInfo and JetRetrieveColumn.
	///Less efficient than using a Column object to identify the column.
	///Uses the specified retrieval options.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	virtual Object ^Retrieve(String ^Col, Type ^Type, IReadRecord::RetrieveOptions ro)
	{
		JET_GRBIT flags = RetrieveOptionsFlagsToBits(ro);
		JET_COLUMNDEF jcd = LookupColumnDef(Col);

		return Retrieve(Type, jcd.columnid, jcd.coltyp, jcd.cp, flags, ro.SizeHint, ro.SizeLimit, ro.RetrieveOffsetLV, ro.RetrieveTagSequence);
	}

	///<summary>Retrieves the data from the specificd column by name at the current cursor position. Calls JetGetTableColumnInfo and JetRetrieveColumn.
	///Less efficient than using a Column object to identify the column.
	///The type parameter determines the type of the return value. Type must be supported by current Bridge. Use Object to retrieve the default type based on the column type.
	///</summary>
	virtual Object ^Retrieve(String ^Col, Type ^Type)
	{
		JET_COLUMNDEF jcd = LookupColumnDef(Col);	

		return Retrieve(Type, jcd.columnid, jcd.coltyp, jcd.cp, 0, ESEOBJECTS_MAX_ALLOCA, 0, 0, 1);
	}

	///<summary>Retrieves a value by Column with Retrieve&lt;Object ^&gt;(Col)</summary>
    property Object ^default[Column ^]
    {
        Object ^get(Column ^Col) { return Retrieve<Object ^>(Col); }
    }

	///<summary>Retrieves a value by name with Retrieve&lt;Object ^&gt;(Col)</summary>
    property Object ^default[String ^]
    {
        Object ^get(String ^Col) { return Retrieve<Object ^>(Col); }
    }

	///<summary>
	///Retrieves the sequence number of a multi-valued tagged column from an index. This is an expensive operation.
	///Calls JetRetrieveColum with JET_bitRetrieveTag, returning the result in pretinfo->itagSequence.
	///</summary>
	//TEST: not sure how this is supposed to work
	virtual ulong RetrieveIndexTagSequence(Column ^Col)
	{
		JET_RETINFO ret_info = {sizeof ret_info};

		EseException::RaiseOnError(JetRetrieveColumn(Session->_JetSesid, _TableID->_JetTableID, Col->_JetColID, null, 0, null, JET_bitRetrieveTag, &ret_info));

		return ret_info.itagSequence;
	}

internal:
	//disassemble and free jec returned structure
	static void FreeEnumColumn(JET_ENUMCOLUMN *jec, ulong jec_ct)
	{		
		if(jec)
		{
			for(ulong i = 0; i < jec_ct; i++)
			{
				if(jec[i].err == JET_wrnColumnSingleValue)
				{
					if(jec[i].pvData)
						delete jec[i].pvData;
				}
				else
				{
					for(ulong j = 0; j < jec[i].cEnumColumnValue; j++)
						if(jec[i].rgEnumColumnValue[j].pvData)
							delete jec[i].rgEnumColumnValue[j].pvData;							

					delete jec[i].rgEnumColumnValue;
				}
			}

			delete jec;
		}
	}

public:
	///<summary>Retrieves all values of a multivalued field. Calls JetEnumerateColumns. Requires 5.1+.</summary>
	///<remarks>Also works on single valued columns and fields.</remarks>
	///<param name="SizeLimit">Maximum size in bytes to return for any one value</param>
	generic <class T> virtual array<T> ^RetrieveAllValues(Column ^Col, ulong SizeLimit)
	{
		JET_SESID JetSesid = Session->_JetSesid;
		JET_TABLEID JetTableID = TableID->_JetTableID;

		JET_ENUMCOLUMNID jeci = {0};
		jeci.columnid = Col->_JetColID;

		ulong jec_ct = 0;
		JET_ENUMCOLUMN *jec = null;

		EseException::RaiseOnError(JetEnumerateColumns(JetSesid, JetTableID, 1, &jeci, &jec_ct, &jec, jet_realloc_cpp, null, SizeLimit, 0));

		try
		{
			if(jec_ct < 1)
				throw gcnew ApplicationException("No fields returned");

			array<T> ^Values = gcnew array<T>(jec->cEnumColumnValue);

			for(ulong i = 0; i < jec->cEnumColumnValue; i++)
				Values[i] = safe_cast<T>(Bridge->ValueBytesToObject(
					T::typeid,
					jec->rgEnumColumnValue[i].err == JET_wrnColumnNull,
					IntPtr(jec->rgEnumColumnValue[i].pvData),
					jec->rgEnumColumnValue[i].cbData,
					Col->ColumnType,
					Col->_CP));

			return Values;
		}
		finally
		{
			FreeEnumColumn(jec, jec_ct);
		}
	}

	///<summary>Retrieves all values of a multivalued field. Calls JetEnumerateColumns. Requires 5.1+.</summary>
	///<remarks>Also works on single valued columns and fields.</remarks>
	///<param name="SizeLimit">Maximum size in bytes to return for any one value</param>
	virtual array<Object ^> ^RetrieveAllValues(Column ^Col, Type ^Type, ulong SizeLimit)
	{
		JET_SESID JetSesid = Session->_JetSesid;
		JET_TABLEID JetTableID = TableID->_JetTableID;

		JET_ENUMCOLUMNID jeci = {0};
		jeci.columnid = Col->_JetColID;

		ulong jec_ct = 0;
		JET_ENUMCOLUMN *jec = null;

		EseException::RaiseOnError(JetEnumerateColumns(JetSesid, JetTableID, 1, &jeci, &jec_ct, &jec, jet_realloc_cpp, null, SizeLimit, 0));

		try
		{
			if(jec_ct < 1)
				throw gcnew ApplicationException("No fields returned");

			array<Object ^> ^Values = gcnew array<Object ^>(jec->cEnumColumnValue);

			for(ulong i = 0; i < jec->cEnumColumnValue; i++)
				Values[i] = Bridge->ValueBytesToObject(
					Type,
					jec->rgEnumColumnValue[i].err == JET_wrnColumnNull,
					IntPtr(jec->rgEnumColumnValue[i].pvData),
					jec->rgEnumColumnValue[i].cbData,
					Col->ColumnType,
					Col->_CP);

			return Values;
		}
		finally
		{
			FreeEnumColumn(jec, jec_ct);
		}
	}

	///<summary>Retrieves all values of a multivalued field. Calls JetEnumerateColumns. Requires 5.1+.</summary>
	///<remarks>Also works on single valued columns and fields.</remarks>
	generic <class T> virtual array<T> ^RetrieveAllValues(Column ^Col)
	{
		return RetrieveAllValues<T>(Col, 0);
	}

	///<summary>Retrieves all values of a multivalued field. Calls JetEnumerateColumns. Requires 5.1+.</summary>
	///<remarks>Also works on single valued columns and fields.</remarks>
	virtual array<Object ^> ^RetrieveAllValues(Column ^Col, Type ^Type)
	{
		return RetrieveAllValues(Col, Type, 0);
	}

	///<summary>Builds an array containing all values in all fields in the row, including all values of mutli valued fields. Calls JetEnumerateColumns. Requires 5.1+.</summary>
	///<param name="SizeLimit">Maximum size in bytes to return for any one value</param>
	///<remarks>Multivalued fields are returned as arrays of values</remarks>
	virtual array<Field> ^RetrieveAllFields(ulong SizeLimit)
	{
		JET_SESID JetSesid = Session->_JetSesid;
		JET_TABLEID JetTableID = TableID->_JetTableID;

		IDictionary<JET_COLUMNID, Column ^> ^Cols = QueryTableColumns(JetSesid, JetTableID);

		ulong jec_ct = 0;
		JET_ENUMCOLUMN *jec = null;

		EseException::RaiseOnError(JetEnumerateColumns(JetSesid, JetTableID, 0, null, &jec_ct, &jec, jet_realloc_cpp, null, SizeLimit, JET_bitEnumerateCompressOutput));

		try
		{
			array<Field> ^Fields = gcnew array<Field>(jec_ct);

			for(ulong i = 0; i < jec_ct; i++)
			{
				Column ^Col = Cols[jec[i].columnid];

				Fields[i].Col = Col;

				//different structure layout depending if the column had more than one value
				if(jec[i].err == JET_wrnColumnSingleValue)
					Fields[i].Val = Bridge->ValueBytesToObject(
						Object::typeid,
						false,
						IntPtr(jec[i].pvData),
						jec[i].cbData,
						Col->ColumnType,
						Col->_CP);
				else
				{
					array<Object ^> ^Values = gcnew array<Object ^>(jec[i].cEnumColumnValue);

					for(ulong j = 0; j < jec[i].cEnumColumnValue; j++)
						Values[j] = Bridge->ValueBytesToObject(
							Object::typeid,
							jec[i].err == JET_wrnColumnNull,
							IntPtr(jec[i].rgEnumColumnValue[j].pvData),
							jec[i].rgEnumColumnValue[j].cbData,
							Col->ColumnType,
							Col->_CP);

					Fields[i].Val = Bridge->MultivalueToObject<array<Object ^> ^>(Values);
				}
			}

			return Fields;
		}
		finally
		{
			FreeEnumColumn(jec, jec_ct);
		}
	}

	virtual array<Field> ^RetrieveAllFields()
	{
		return RetrieveAllFields(0);
	}

	///<summary>Represents a JET_RECSIZE, reporting record size and count measurements. Requires 6.0+.</summary>
	value struct RecordSize
	{
		///<summary>Size of non-key data.</summary>
		int64 Data;
		///<summary>Size of long data stored out of row.</summary>
		int64 LongData;
		///<summary>Size of record overhead, including key data.</summary>
		int64 Overhead;
		///<summary>Count of non-tagged (fixed and variable) fields.</summary>
		int64 NonTagged;
		///<summary>Count of tagged fields.</summary>
		int64 Tagged;
		///<summary>Count of long values stored out of row associated with row.</summary>
		int64 LongValues;
		///<summary>Count of values after the first in a field for all columns.</summary>
		int64 MultiValues;

		static RecordSize operator+(RecordSize x, RecordSize y)
		{
			x.Data += y.Data;
			x.LongData += y.LongData;
			x.Overhead += y.LongData;
			x.NonTagged += y.NonTagged;
			x.Tagged += y.Tagged;
			x.LongValues += y.LongValues;
			x.MultiValues += y.MultiValues;
			return x;
		}
	};

internal:
	static RecordSize JetRecSizeToRecordSize(JET_RECSIZE jrs)
	{
		RecordSize rs;
		rs.Data = jrs.cbData;
		rs.LongData = jrs.cbLongValueData;
		rs.Overhead = jrs.cbOverhead;
		rs.NonTagged = jrs.cNonTaggedColumns;
		rs.Tagged = jrs.cTaggedColumns;
		rs.LongValues = jrs.cLongValues;
		rs.MultiValues = jrs.cMultiValues;
		return rs;
	}

public:
	///<summary>Record size measurements of current record.</summary>
	property RecordSize CurrentRecordSize
	{
		RecordSize get()
		{
			JET_RECSIZE jrs;

			EseException::RaiseOnError(JetGetRecordSize_demand(Session->_JetSesid, _TableID->_JetTableID, &jrs, 0));

			return JetRecSizeToRecordSize(jrs);
		}
	};

	///<summary>Record size measurements of current record, inclding only data stored in-row.</summary>
	property RecordSize CurrentLocalRecordSize
	{
		RecordSize get()
		{
			JET_RECSIZE jrs;

			EseException::RaiseOnError(JetGetRecordSize_demand(Session->_JetSesid, _TableID->_JetTableID, &jrs, JET_bitRecordSizeLocal));

			return JetRecSizeToRecordSize(jrs);
		}
	};

	//---------------------------Update support------------------------------------

	//NEXT: Add support for JetRetriveColumns to get multiple cols at once
	//5.0: retrieve all tagged fields from JetRetrieveColumns, retrieve all fields with JetGetTableInfo JET_COLUMNLIST/JetRetrieveColumns for equivalency with JetEnumerateColumns
	//NEXT: JetEnumerateColumns: unexploited modes and options
	//NEXT: use of columnidNextTagged

	///<summary>Represents one update session of a record, bracketed by JetPrepareUpdate and either JetUpdate on Update to save or JetPrepareUpdate with JET_prepCancel on Cancel or Dispose without Update.</summary>
	ref class Update : IWriteRecord
	{
		Cursor ^_Cursor;
		bool Active;

	internal:
		Update(Cursor ^Cursor, ulong flags) :
			_Cursor(Cursor),
			Active(true)
		{
			EseException::RaiseOnError(JetPrepareUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, flags));
		}

	public:
		///<summary>
		///Completes the update, inserting or updating the row data.
		///No further updates can be made with this object after calling this function. Calls JetUpdate.
		///</summary>
		void Complete()
		{
			if(!Active)
				throw gcnew InvalidOperationException("Update is no longer active. It has already been completed or canceled");

			ulong buffszrq = 0;

			JET_ERR status = JetUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, null, 0, &buffszrq);

			switch(status)
			{
			case JET_errSuccess:
				break;

			default:
				EseException::RaiseOnError(status);
			}

			Active = false;
		}

		///<summary>
		///Completes the update, inserting or updating the row data.
		///No futher updates can be made with this object after calling this function. Calls JetUpdate.
		///Retrieves a bookmark for the updated row.
		///</summary>
		Bookmark ^CompleteWithBookmark()
		{
			if(!Active)
				throw gcnew InvalidOperationException("Update is no longer active. It has already been completed or canceled");

			uchar *buff = null;

			try
			{
				ulong buffsz = JET_cbBookmarkMost;
				buff = new uchar[buffsz];
				ulong buffszrq = 0;

				JET_ERR status = JetUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, buff, buffsz, &buffszrq);

				switch(status)
				{
				case JET_errSuccess:
					break;

				case JET_errBufferTooSmall:
					delete[] buff;
					//buffsz = buffszrq; //apparently, buffszrq doesn't actaully return the required size
					buff = new uchar[JET_cbBookmarkMost * 8];
					
					//let's try it again with a bigger buffer
					status = JetUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, buff, buffsz, &buffszrq);
				}
				
				EseException::RaiseOnError(status);

				Active = false;

				Bookmark ^Bkmk = gcnew Bookmark(buffszrq, buff);
				buff = null; //now owned by Bookmark, don't want to free here

				return Bkmk;
			}
			finally
			{
				if(buff)
					delete[] buff;
			}
		}

		///<summary>
		///Completes the update, inserting or updating the row data.
		///No futher updates can be made with this object after calling this function. Calls JetUpdate.
		///Seeks to updated record. Use to go to a newly inserted record.
		///</summary>
		void CompleteSeek()
		{
			if(!Active)
				throw gcnew InvalidOperationException("Update is no longer active. It has already been completed or canceled");

			ulong buffsz = 2048;
			uchar *buff = static_cast<uchar *>(alloca(buffsz));
			ulong buffszrq = 0;

			EseException::RaiseOnError(JetUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, buff, buffsz, &buffszrq));

			Active = false;

			EseException::RaiseOnError(JetGotoBookmark(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, buff, buffszrq));
		}

		///<summary>
		///Cancels the update, making no change to the database.
		///No updates can be made with this object after calling this function.
		///Calls JetPrepareUpdate with JET_prepCancel
		///</summary>
		void Cancel()
		{
			if(!Active)
				throw gcnew InvalidOperationException("Update is no longer active. It has already been completed or canceled");

			EseException::RaiseOnError(JetPrepareUpdate(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, JET_prepCancel));
			Active = false;
		}

		///<summary>Dispose cancels the update iff it is still active.</summary>
		~Update()
		{
			if(Active)
				Cancel();
		}

	private:
		void Set(JET_COLUMNID colid, JET_COLTYP coltyp, ushort cp, Object ^Value)
		{
			free_list fl;
			marshal_context mc;
			void *buff = null;
			ulong buffsz = 0;
			bool empty;

			to_memblock_bridge(_Cursor->Bridge, Value, buff, buffsz, empty, coltyp, cp, mc, fl);

			EseException::RaiseOnError(JetSetColumn(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, colid, buff, buffsz, empty ? JET_bitSetZeroLength : 0, null));
		}

		void Set(JET_COLUMNID colid, JET_COLTYP coltyp, ushort cp, Object ^Value, IWriteRecord::SetOptions so)
		{
			free_list fl;
			marshal_context mc;
			void *buff = null;
			ulong buffsz = 0;
			bool empty;

			to_memblock_bridge(_Cursor->Bridge, Value, buff, buffsz, empty, coltyp, cp, mc, fl);

			JET_GRBIT flags = SetOptionsFlagsToBits(so);
			JET_SETINFO si = {sizeof si};
			si.ibLongValue = so.OffsetLV;
			si.itagSequence = so.TagSequence;

			EseException::RaiseOnError(JetSetColumn(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, colid, buff, buffsz, flags | (empty ? JET_bitSetZeroLength : 0), &si));
		}

	public:
		///<summary>Modifies the value of a particular column.</summary>
		///<remarks>Updates do not actually affect the database unless the update is completed. See Complete.
		///<pr/>Retrieval functions will return the original value before the update (prior to calling Complete which saves the changes) unless RetrieveCopy specified as a retrieve option.
		///</remarks>
		virtual void Set(Column ^Col, Object ^Value)
		{
			Set(Col->_JetColID, Col->_JetColTyp, Col->_CP, Value);
		}

		///<summary>Modifies the value of a particular column.</summary>
		///<remarks>Updates do not actually affect the database unless the update is completed. See Complete.
		///<pr/>Retrieval functions will return the original value before the update (prior to calling Complete which saves the changes) unless RetrieveCopy specified as a retrieve option.
		///</remarks>
		virtual void Set(Column ^Col, Object ^Value, IWriteRecord::SetOptions so)
		{
			Set(Col->_JetColID, Col->_JetColTyp, Col->_CP, Value, so);
		}

		///<summary>Modifies the value of a particular column.</summary>
		///<remarks>Updates do not actually affect the database unless the update is completed. See Complete.
		///<pr/>Retrieval functions will return the original value before the update (prior to calling Complete which saves the changes) unless RetrieveCopy specified as a retrieve option.
		///</remarks>
		virtual void Set(String ^Col, Object ^Value)
		{
			JET_COLUMNDEF jcd = _Cursor->LookupColumnDef(Col);

			Set(jcd.columnid, jcd.coltyp, jcd.cp, Value);
		}

		///<summary>Modifies the value of a particular column.</summary>
		///<remarks>Updates do not actually affect the database unless the update is completed. See Complete.
		///<pr/>Retrieval functions will return the original value before the update (prior to calling Complete which saves the changes) unless RetrieveCopy specified as a retrieve option.
		///</remarks>
		virtual void Set(String ^Col, Object ^Value, IWriteRecord::SetOptions so)
		{
			JET_COLUMNDEF jcd = _Cursor->LookupColumnDef(Col);

			Set(jcd.columnid, jcd.coltyp, jcd.cp, Value, so);
		}

		//NEXT: JetSetColumns to set multiple columns?

		///<summary>Copies a value from another cursor without bridging the data. Calls JetRetrieveColumn and JetSetColumn.</summary>
		virtual void Set(Column ^DestCol, Cursor ^SrcCsr, Column ^SrcCol)
		{
			free_list fl;
			void *buff;

			buff = alloca_array(char, ESEOBJECTS_MAX_ALLOCA);

			ulong buffsz = ESEOBJECTS_MAX_ALLOCA;
			ulong req_buffsz = 0;

			JET_RETINFO ret_info = {sizeof ret_info};

			ret_info.ibLongValue = 0;
			ret_info.itagSequence = 1;

			JET_ERR status = JetRetrieveColumn(SrcCsr->Session->_JetSesid, SrcCsr->_TableID->_JetTableID, SrcCol->_JetColID, buff, buffsz, &req_buffsz, 0, &ret_info);

			switch(status)
			{
			case JET_errSuccess:
				break;

			case JET_errInvalidBufferSize:
			case JET_wrnBufferTruncated:
			case JET_errBufferTooSmall:
				//buffer needs to be bigger
				buff = fl.alloc_array<char>(req_buffsz);
				buffsz = req_buffsz;

				//this call shouldn't fail in a way we could have fixed here (i.e. buffer too small)
				EseException::RaiseOnError(JetRetrieveColumn(SrcCsr->Session->_JetSesid,SrcCsr-> _TableID->_JetTableID, SrcCol->_JetColID, buff, buffsz, &req_buffsz, 0, &ret_info));
				break;

			case JET_wrnColumnNull:
				EseException::RaiseOnError(JetSetColumn(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, DestCol->_JetColID, null, 0, 0, null));
				
			default:
				//if it was some other error, raise it
				EseException::RaiseOnError(status);
				break;
			}

			EseException::RaiseOnError(JetSetColumn(_Cursor->Session->_JetSesid, _Cursor->TableID->_JetTableID, DestCol->_JetColID, buff, req_buffsz, req_buffsz == 0 ? JET_bitSetZeroLength : 0, null));
		}

		///<summary>Provides access to read the record being updated.</summary>
		virtual property IReadRecord ^Read
		{
			IReadRecord ^get() {return _Cursor;}
		}

		///<summary>Shortcut to read fields from the current record.</summary>
		generic <class T> T Retrieve(Column ^Col, IReadRecord::RetrieveOptions ro)
		{
			return _Cursor->Retrieve<T>(Col, ro);
		}

		///<summary>Shortcut to read fields from the current record.</summary>
		generic <class T> T Retrieve(Column ^Col)
		{
			return _Cursor->Retrieve<T>(Col);
		}

		///<summary>Shortcut to read fields from the current record.</summary>
		generic <class T> T Retrieve(String ^Col, IReadRecord::RetrieveOptions ro)
		{
			return _Cursor->Retrieve<T>(Col, ro);
		}

		///<summary>Shortcut to read fields from the current record.</summary>
		generic <class T> T Retrieve(String ^Col)
		{
			return _Cursor->Retrieve<T>(Col);
		}
		
		///<summary>Retrieves a value by Column with Retrieve&lt;Object ^&gt;(Col)</summary>
		property Object ^default[Column ^]
		{
			Object ^get(Column ^Col) { return _Cursor->Retrieve<Object ^>(Col); }
			void set(Column ^Col, Object ^Value) { Set(Col, Value); }
		}

		///<summary>Retrieves a value by name with Retrieve&lt;Object ^&gt;(Col)</summary>
		property Object ^default[String ^]
		{
			Object ^get(String ^Col) { return _Cursor->Retrieve<Object ^>(Col); }
			void set(String ^Col, Object ^Value) { Set(Col, Value); }
		}

		///<summary>Record size measurements of current record being updated or inserted.</summary>
		property RecordSize CurrentRecordSize
		{
			RecordSize get()
			{
				JET_RECSIZE jrs;

				EseException::RaiseOnError(JetGetRecordSize_demand(_Cursor->Session->_JetSesid, _Cursor->_TableID->_JetTableID, &jrs, JET_bitRecordSizeInCopyBuffer));

				return JetRecSizeToRecordSize(jrs);
			}
		}

		///<summary>Record size measurements of current record being updated or inserted, inclding only data stored in-row.</summary>
		property RecordSize CurrentLocalRecordSize
		{
			RecordSize get()
			{
				JET_RECSIZE jrs;

				EseException::RaiseOnError(JetGetRecordSize_demand(_Cursor->Session->_JetSesid, _Cursor->_TableID->_JetTableID, &jrs, JET_bitRecordSizeInCopyBuffer | JET_bitRecordSizeLocal));

				return JetRecSizeToRecordSize(jrs);
			}
		}
	};

	///<summary>Prepares to insert a new record. Default column values are used for inital state. Calls JetPrepareUpdate JET_prepInsert.</summary>
	///<returns>An update object representing the lifetime of the update.</returns>
	Update ^BeginInsert()
	{
		return gcnew Update(this, JET_prepInsert);
	}

	///<summary>Prepares to insert a new record. Initial state copied from current record. Calls JetPrepareUpdate JET_prepInsertCopy.</summary>
	///<returns>An update object representing the lifetime of the update.</returns>
	Update ^BeginInsertCopy()
	{
		return gcnew Update(this, JET_prepInsertCopy);
	}

	///<summary>Prepares to insert a new record, copying the current record and deleting it if completed. Used to modify primary key fields. Calls JetPrepareUpdate JET_prepInsertCopyDeleteOriginal.</summary>
	///<returns>An update object representing the lifetime of the update.</returns>
	Update ^BeginInsertCopyDeleteOriginal()
	{
		return gcnew Update(this, JET_prepInsertCopyDeleteOriginal);
	}

	///<summary>Prepares to replace current record's value. Calls JetPrepareUpdate JET_prepReplace.</summary>
	///<remarks>If present, increments version column upon completion.
	///<pr/>Cannot modify primary key values (see BeginInsertCopyDeleteOriginal).
	///</remarks>
	///<returns>An update object representing the lifetime of the update.</returns>
	Update ^BeginReplace()
	{
		return gcnew Update(this, JET_prepReplace);
	}

	///<summary>Prepares to replace current record's value. Calls JetPrepareUpdate JET_prepReplaceNoLock.</summary>
	///<remarks>Does not acquire a write lock on the row first. More efficient than BeginReplace but can throw EseException with JET_errWriteConflict in Complete() if another session modifies the same row concurrently.
	///<pr/>Cannot modify primary key values (see BeginInsertCopyDeleteOriginal).
	///</remarks>
	///<returns>An update object representing the lifetime of the update.</returns>
	Update ^BeginReplaceNoLock()
	{
		return gcnew Update(this, JET_prepReplaceNoLock);
	}

	///<summary>Performs an atomic, concurrent addition to an escrow column. Calls JetEscrowUpdate.</summary>
	///<returns>The volaitle previous value of the column. This value is returned without versioning and is not transactional.</returns>
	int EscrowUpdate(Column ^Col, int addend)
	{
		int old_value = 0;

		EseException::RaiseOnError(JetEscrowUpdate(Session->_JetSesid, _TableID->_JetTableID, Col->_JetColID, &addend, sizeof addend, &old_value, sizeof old_value, null, 0));

		return old_value;
	}

	///<summary>Performs an atomic, concurrent addition to an escrow column. Operation will be committed even if the transaction is rolled back. Calls JetEscrowUpdate with JET_bitEscrowNoRollback.</summary>
	///<returns>The volaitle previous value of the column. This value is returned without versioning and is not transactional.</returns>
	int EscrowUpdateNoRollback(Column ^Col, int addend)
	{
		int old_value = 0;

		EseException::RaiseOnError(JetEscrowUpdate(Session->_JetSesid, _TableID->_JetTableID, Col->_JetColID, &addend, sizeof addend, &old_value, sizeof old_value, null, JET_bitEscrowNoRollback));

		return old_value;
	}

//---------------------------Seek support--------------------------------------
internal:
	//seek implicitly with current key built
	void Seek(JET_GRBIT grbit, bool %has_currency, bool %not_equal)
	{
		JET_ERR status = JetSeek(Session->_JetSesid, _TableID->_JetTableID, grbit);

		switch(status)
		{
		case JET_errRecordNotFound:
			not_equal = true;
			has_currency = false;
			return;

		case JET_wrnSeekNotEqual:
			not_equal = true;
			has_currency = true;
			return;

		default:
			not_equal = false;
			has_currency = true;
			break;
		}

		EseException::RaiseOnError(status);
	}

	//uses current key
	bool CheckUniqueness()
	{
		JET_ERR status = JetSeek(Session->_JetSesid, _TableID->_JetTableID, JET_bitSeekEQ | JET_bitCheckUniqueness);

		if(status == JET_wrnUniqueKey)
			return true;
		
		return false;
	}

	void LoadKey(Key ^key)
	{
		EseException::RaiseOnError(JetMakeKey(Session->_JetSesid, _TableID->_JetTableID, key->_JetKey, key->_KeyLength, JET_bitNormalizedKey));
	}

	void LoadKey(IEnumerable<Field> ^KeyFields, JET_GRBIT grbit)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldsIntoTableID(sesid, tabid, Bridge, KeyFields, grbit);
	}

	bool SetIxRange(JET_GRBIT grbit)
	{
		JET_ERR status = JetSetIndexRange(Session->_JetSesid, _TableID->_JetTableID, grbit);

		if(status == JET_errNoCurrentRecord)
			return false;

		EseException::RaiseOnError(status);

		return false;
	}

public:
	///<summary>Seeks to the specified position.</summary>
	///<returns>True iff there is a current record after the seek.</returns>
	bool Seek(Seekable ^s)
	{
		bool has_currency, not_exact;
		s->SeekTo(has_currency, not_exact, this);
		return has_currency;
	}

	///<summary>Seeks to the specified position.</summary>
	///<param name="NotEqual">Returns true iff the exact record couldn't be found.</param>
	///<returns>True iff there is a current record after the seek.</returns>
	bool Seek(Seekable ^s, [Out] bool %NotEqual)
	{
		bool has_currency = false;
		s->SeekTo(has_currency, NotEqual, this);
		return has_currency;
	}

	
	//----------------------------Direct Seek methods--------------------------

	///<summary>Seeks to the specified 1 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		
		bool has_currency, not_exact;		
		Seek(JET_bitSeekEQ, has_currency, not_exact);
		return has_currency;
	}
	///<summary>Seeks to the specified 1 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Match MatchMode, SeekRel Rel)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey | MatchToGrbit(MatchMode));

		bool has_currency, not_exact;
		Seek(SeekRelToGrbit(Rel), has_currency, not_exact);
		return has_currency;
	}

	///<summary>Seeks to the specified 2 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, 0);
		
		bool has_currency, not_exact;		
		Seek(JET_bitSeekEQ, has_currency, not_exact);
		return has_currency;
	}
	///<summary>Seeks to the specified 2 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2, Match MatchMode, SeekRel Rel)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, MatchToGrbit(MatchMode));

		bool has_currency, not_exact;
		Seek(SeekRelToGrbit(Rel), has_currency, not_exact);
		return has_currency;
	}

	///<summary>Seeks to the specified 3 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2, Column ^C3, Object ^K3)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C3, K3, 0);
		
		bool has_currency, not_exact;		
		Seek(JET_bitSeekEQ, has_currency, not_exact);
		return has_currency;
	}
	///<summary>Seeks to the specified 3 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2, Column ^C3, Object ^K3, Match MatchMode, SeekRel Rel)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C3, K3, MatchToGrbit(MatchMode));

		bool has_currency, not_exact;
		Seek(SeekRelToGrbit(Rel), has_currency, not_exact);
		return has_currency;
	}

	///<summary>Seeks to the specified 4 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2, Column ^C3, Object ^K3, Column ^C4, Object ^K4)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C3, K3, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C4, K4, 0);
		
		bool has_currency, not_exact;		
		Seek(JET_bitSeekEQ, has_currency, not_exact);
		return has_currency;
	}
	///<summary>Seeks to the specified 4 field key position.</summary>
	bool Seek(Column ^C1, Object ^K1, Column ^C2, Object ^K2, Column ^C3, Object ^K3, Column ^C4, Object ^K4, Match MatchMode, SeekRel Rel)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C1, K1, JET_bitNewKey);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C2, K2, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C3, K3, 0);
		Key::LoadFieldIntoTableID(sesid, tabid, Bridge, C4, K4, MatchToGrbit(MatchMode));

		bool has_currency, not_exact;
		Seek(SeekRelToGrbit(Rel), has_currency, not_exact);
		return has_currency;
	}

	///<summary>Checks if key appears exactly once in the index. Calls JetMakeKey and JetSeek JET_bitCheckUniqueness.</summary>
	bool CheckUniqueness(Key ^key)
	{
		LoadKey(key);
		return CheckUniqueness();
	}

	///<summary>Checks if key appears exactly once in the index. Calls JetMakeKey and JetSeek JET_bitCheckUniqueness.</summary>
	bool CheckUniqueness(IEnumerable<Field> ^KeyFields)
	{
		LoadKey(KeyFields, 0);
		return CheckUniqueness();
	}

	//---------------------------Limit/range support-------------------------------

	///<summary>Sets a limit that will be encountered while scrolling forward (positive Move values) at the specified key.</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	///<returns>True iff there are any values between the current position and the specified key.</returns>
	bool SetUpperLimit(Limitable ^key)
	{
		return key->LimitTo(this, true);
	}

	///<summary>Sets a limit that will be encountered while scrolling backwards (negative Move values) at the specified key.</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	///<returns>True iff there are any values between the current position and the specified key.</returns>
	bool SetLowerLimit(Limitable ^key)
	{
		return key->LimitTo(this, false);
	}

	///<summary>Positions the cursor to k1 and sets a upper limit to stop at k2, so that by scrolling forward the values within the range will be read.</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool ForwardRange(Seekable ^k1, Limitable ^k2)
	{
		bool has_currency, not_equal;
		k1->SeekTo(has_currency, not_equal, this);
		k2->LimitTo(this, true);
		return has_currency;
	}

	///<summary>Positions the cursor to k1 and sets a lower limit to stop at k2, so that by scrolling backward the values within the range will be read.</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool BackwardRange(Seekable ^k1, Limitable ^k2)
	{
		bool has_currency, not_equal;
		k1->SeekTo(has_currency, not_equal, this);
		k2->LimitTo(this, false);
		return has_currency;
	}

	///<summary>Positions the cursor to k1 and sets a upper limit to stop at k2, so that by scrolling forward the values within the range will be read.
	///<pr/>Both keys will have wildcards and relations appended that make the range as inclusive as possible.
	///<pr/>There must be at least one key field unspecified to leave room for the wildcard.
	///</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool ForwardRangeInclusive(array<Field> ^k1, array<Field> ^k2)
	{
		bool has_currency, not_equal;
		
		LoadKey(k1, JET_bitFullColumnStartLimit);
		Seek(JET_bitSeekGE, has_currency, not_equal);
		
		LoadKey(k2, JET_bitFullColumnEndLimit);
		SetIxRange(JET_bitRangeUpperLimit | JET_bitRangeInclusive);
		
		return HasCurrent;
	}

	///<summary>Positions the cursor to k1 and sets a lower limit to stop at k2, so that by scrolling backward the values within the range will be read.
	///<pr/>Both keys will have wildcards and relations appended that make the range as inclusive as possible.
	///<pr/>There must be at least one key field unspecified to leave room for the wildcard.
	///</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool BackwardRangeInclusive(array<Field> ^k1, array<Field> ^k2)
	{
		bool has_currency, not_equal;

		LoadKey(k1, JET_bitFullColumnEndLimit);
		Seek(JET_bitSeekLE, has_currency, not_equal);
		
		LoadKey(k2, JET_bitFullColumnStartLimit);
		SetIxRange(JET_bitRangeInclusive);
		
		return HasCurrent;
	}

	///<summary>Positions the cursor to beginning of k1 and sets a upper limit to stop at the end of k1, so that by scrolling forward the values within the range will be read.
	///<pr/>The key will have wildcards and relations appended that make the range as inclusive as possible.
	///<pr/>The wildcard will be used in opposite directions to include the maximum number of values starting with the specified k1 as the first key field.
	///<pr/>There must be at least one key field unspecified to leave room for the wildcard.
	///</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool ForwardRangeInclusive(Field k1)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		bool has_currency, not_equal;
		
		Key::LoadSingleFieldIntoTableID(sesid, tabid, Bridge, k1, JET_bitFullColumnStartLimit);
		Seek(JET_bitSeekGE, has_currency, not_equal);
		
		Key::LoadSingleFieldIntoTableID(sesid, tabid, Bridge, k1, JET_bitFullColumnEndLimit);
		SetIxRange(JET_bitRangeUpperLimit | JET_bitRangeInclusive);
		
		return HasCurrent;
	}

	///<summary>Positions the cursor to beginning of k1 and sets a upper limit to stop at the end of k1, so that by scrolling backward the values within the range will be read.
	///<pr/>The key will have wildcards and relations appended that make the range as inclusive as possible.
	///<pr/>The wildcard will be used in opposite directions to include the maximum number of values starting with the specified k1 as the first key field.
	///<pr/>There must be at least one key field unspecified to leave room for the wildcard.
	///</summary>
	///<remarks>The limit will be canceled by CancelRange, any method of moving the cursor other than Move, or setting a new limit.</remarks>
	bool BackwardRangeInclusive(Field k1)
	{
		JET_SESID sesid = Session->_JetSesid;
		JET_TABLEID tabid = _TableID->_JetTableID;

		bool has_currency, not_equal;

		Key::LoadSingleFieldIntoTableID(sesid, tabid, Bridge, k1, JET_bitFullColumnEndLimit);
		Seek(JET_bitSeekLE, has_currency, not_equal);
		
		Key::LoadSingleFieldIntoTableID(sesid, tabid, Bridge, k1, JET_bitFullColumnStartLimit);
		SetIxRange(JET_bitRangeInclusive);
		
		return HasCurrent;
	}

	///<summary>Cancels any range limit currently in effect.</summary>
	///<returns>True iif there was a range in effect previously that has been cancels.</returns>
	bool CancelRange()
	{
		JET_ERR status = JetSetIndexRange(Session->_JetSesid, _TableID->_JetTableID, JET_bitRangeRemove);

		if(status == JET_errInvalidOperation) //i.e. no range in effect
			return false;

		EseException::RaiseOnError(status);

		return true;
	}

	//---------------------------Index support-------------------------------------

	///<summary>Controls the currently selected index.</summary>
	///<remarks>When setting the property, the corresponding entry in the index is used if possible, otherwise the first item is selected.</remarks>
	property Index ^CurrentIndex
	{
		Index ^get() {return _CurrentIndex;}
		void set(Index ^newIndex)
		{
			JET_GRBIT mode = JET_bitNoMove;

			//there are a couple of conditions that could make the call work when it fails, so the call is done in a loop
			ulong loop_limit = 3; //in case the fixes don't work, don't retry excessively

			while(loop_limit)
			{
				loop_limit--;

				JET_ERR status = JetSetCurrentIndex4(Session->_JetSesid, TableID->_JetTableID, null, newIndex->_JetIndexID, mode, 0);

				switch(status)
				{
				case JET_errInvalidIndexId:
					//apparently the indexid expired
					newIndex->FixIndexID(Session->_JetSesid, TableID->_JetTableID);
					continue;

				case JET_errNoCurrentRecord:
					//if there's no corresponding entry, just start at the top.
					mode = JET_bitMoveFirst;
					continue;

				default:
					EseException::RaiseOnError(status);
					_CurrentIndex = newIndex;
					return;
				}
			}
		}
	}

	///<summary>Selects the specified index, moving to the corresponding record in the index. Calls JetSetCurrentIndex4.</summary>
	///<remarks>If the index doesn't have a corresponding record, the call fails.</remarks>
	void SetIndexCorresponding(Index ^newIndex, ulong TagSequence)
	{
		//the index entry could have expired, which requires retrying the call
		ulong loop_limit = 2; //but don't keep retrying if it doesn't work

		while(loop_limit)
		{
			loop_limit--;

			JET_ERR status = JetSetCurrentIndex4(Session->_JetSesid, TableID->_JetTableID, null, newIndex->_JetIndexID, JET_bitNoMove, TagSequence);

			switch(status)
			{
			case JET_errInvalidIndexId:
				//apparently the indexid expired
				newIndex->FixIndexID(Session->_JetSesid, TableID->_JetTableID);
				continue;

			default:
				EseException::RaiseOnError(status);
				_CurrentIndex = newIndex;
				return;
			}
		}
	}

	///<summary>Selects the specified index, moving to the corresponding record in the index. Calls JetSetCurrentIndex4.</summary>
	///<remarks>If the index doesn't have a corresponding record, the call fails.</remarks>
	void SetIndexCorresponding(Index ^newIndex)
	{
		SetIndexCorresponding(newIndex, 0);
	}

	///<summary>Selects the specified index, moving to the first record in the index. Calls JetSetCurrentIndex4.</summary>
	///<remarks>If the index doesn't have a corresponding record, the call fails.
	///<pr/>Tag sequence to seek within a multivalued key is taken from RetrieveTagSequence.
	///</remarks>
	void SetIndexMoveFirst(Index ^newIndex)
	{
		if(_CurrentIndex != nullptr && newIndex->_JetIndexID == _CurrentIndex->_JetIndexID) //setting to the same index won't move the cursor
		{
			EseException::RaiseOnError(JetMove(Session->_JetSesid, TableID->_JetTableID, JET_MoveFirst, 0));
			return;
		}

		//the index entry could have expired, which requires retrying the call
		ulong loop_limit = 2; //but don't keep retrying if it doesn't work

		while(loop_limit)
		{
			loop_limit--;

			JET_ERR status = JetSetCurrentIndex4(Session->_JetSesid, TableID->_JetTableID, null, newIndex->_JetIndexID, JET_bitMoveFirst, 0);

			switch(status)
			{
			case JET_errInvalidIndexId:
				//apparently the indexid expired
				newIndex->FixIndexID(Session->_JetSesid, TableID->_JetTableID);
				continue;

			default:
				EseException::RaiseOnError(status);
				_CurrentIndex = newIndex;
				return;
			}
		}
	}

	///<summary>Intersects the index ranges selected in the specified cursors. Calls JetIntersectIndexes.</summary>
	///<remarks>Each Cursor must have a range set and the desired secondary index selected. All indexes must be on the same table. Any particular index can only be used once.
	///<pr/>The current position of the specified indexes is undefined after the call.
	///</remarks>
	///<param name="Results">Returns a temp table cursor with the result rows. The cursor can only be scanned forward.</param>
	///<param name="BookmarkCol">The returned temp table has a single column containing bookmarks. This value returns that column reference.</param>
	///<param name="Indexes">A collection of cursors with ranges set and indexes selected to intersect. At most 64 indexes can be specified.</param>
	///<returns>The number of records in the returned cursor</returns>
	static ulong IntersectIndexes([Out] Cursor ^%Results, [Out] Column ^%BookmarkCol, ICollection<Cursor ^> ^Indexes)
	{
		free_list fl;
		JET_SESID sesid;
		JET_INDEXRANGE *jixrs;
		JET_RECORDLIST jrl = {sizeof jrl};

		jixrs = fl.alloc_array_zero<JET_INDEXRANGE>(Indexes->Count);
		ulong i = 0;

		for each(Cursor ^Csr in Indexes)
		{
			sesid = Csr->Session->_JetSesid;
			jixrs[i].cbStruct = sizeof jixrs[i];
			jixrs[i].tableid = Csr->_TableID->_JetTableID;
			jixrs[i].grbit = JET_bitRecordInIndex;
			i++;
		}

		EseException::RaiseOnError(JetIntersectIndexes(sesid, jixrs, Indexes->Count, &jrl, 0));

		return jrl.cRecord;
	}

	//---------------------------Misc----------------------------------------------

	///<summary>Counts the number of index entries going forward in from the current position. Calls JetIndexRecordCount.</summary>
	///<remarks>Any limit or range will constrain the count, which can be used to count records in a particular range.</remarks>
	///<param name="limit">Maximum count to return. Used to limit the IO activity walking entries if a particular threshold is reached.</param>
	ulong ForwardRecordCount(ulong limit)
	{
		ulong ct = 0;

		EseException::RaiseOnError(JetIndexRecordCount(Session->_JetSesid, TableID->_JetTableID, &ct, limit));

		return ct;
	}

	///<summary>Counts the number of index entries going forward in from the current position. Calls JetIndexRecordCount.</summary>
	///<remarks>Any limit or range will limit the count, which can be used to count records in a particular range.
	///<pr/>All eligible records will be counted. This can cause a large amount of IO.
	///</remarks>
	ulong ForwardRecordCount()
	{
		return ForwardRecordCount(MAXUINT);
	}

	///<summary>Approximate position within an index. Use with Cursor.ApproximatePosition. Represents a JET_RECPOS.</summary>
	value struct RecordPosition
	{
		///<summary>Approximate number of entries previous to the current key.</summary>
		ulong EntriesLessThan;
		///<summary>Approximate number of entries equal to current key. Always 1 in current versions.</summary>
		ulong EntriesInRange;
		///<summary>Approximate number of entries in the index total.</summary>
		ulong EntriesTotal;
	};

	///<summary>Approximate positioning of cursor within current index. Get to estimate current postion, set to move to approximate position. Calls JetGetRecordPosition for get and JetGotoPosition for set.</summary>
	///<remarks>The position is not exact and not stable. Use bookmarks instead when exact positioning is needed.
	///<pr/>Position is not transactionally isolated. Corresponding position in index (from set or get of property) can change between contiguous calls of the function.
	///<pr/>A suggested use is in a UI operated scroll bar.
	///</remarks>
	property RecordPosition ApproximatePosition
	{
		RecordPosition get()
		{
			JET_RECPOS jrp = {sizeof jrp};

			EseException::RaiseOnError(JetGetRecordPosition(Session->_JetSesid, TableID->_JetTableID, &jrp, sizeof jrp));

			RecordPosition rp;
			rp.EntriesLessThan = jrp.centriesLT;
			rp.EntriesInRange = jrp.centriesInRange;
			rp.EntriesTotal = jrp.centriesTotal;
			return rp;
		}
		void set(RecordPosition rp)
		{
			JET_RECPOS jrp = {sizeof jrp};
			jrp.centriesLT = rp.EntriesLessThan;
			jrp.centriesInRange = rp.EntriesInRange;
			jrp.centriesTotal= rp.EntriesTotal;

			EseException::RaiseOnError(JetGotoPosition(Session->_JetSesid, TableID->_JetTableID, &jrp));
		}
	}

	property bool HasCurrent
	{
		bool get()
		{
			return JetMove(Session->_JetSesid, TableID->_JetTableID, 0, 0) != JET_errNoCurrentRecord;
		}
	}

	///<summary>Notifies ESE that the entire index is to be scanned to optimize data access. Calls JetSetTableSequential. Requires 5.1+.</summary>
	///<remarks>Call ResetSequentialScanHint to end this mode.</remarks>
	void SetSequentialScanHint()
	{
		EseException::RaiseOnError(JetSetTableSequential(Session->_JetSesid, TableID->_JetTableID, 0));
	}

	///<summary>Notifies ESE that full index scanning has ended. See SetSequentialScanHint. Calls JetResetTableSequential. Requires 5.1+.</summary>
	void ResetSequentialScanHint()
	{
		EseException::RaiseOnError(JetResetTableSequential(Session->_JetSesid, TableID->_JetTableID, 0));
	}

	///<summary>Explicitly prevent other sessions from writing to the current row by taking a pessimistic read lock on the record. Calls JetGetLock.</summary>
	///<remarks>Normally, versioning is used to provide a consistent picture of the database.
	///<pr/>Read locks are compatible with other read locks, but not write locks.
	///<pr/>Locks are released at the end of the transaction.
	///</remarks>
	void AcquireReadLock()
	{
		EseException::RaiseOnError(JetGetLock(Session->_JetSesid, TableID->_JetTableID, JET_bitReadLock));
	}

	///<summary>Explicitly reservses the ability to write to the row. Calls JetGetLock.</summary>
	///<remarks>A write lock is implicitly taken when a row is modified. This function allows early aquisition.
	///<pr/>Write locks are not compatible with other read or read locks held by other sessions, but are compatible with read locks in the same session.
	///<pr/>Locks are released at the end of the transaction.
	///</remarks>
	void AcquireWriteLock()
	{
		EseException::RaiseOnError(JetGetLock(Session->_JetSesid, TableID->_JetTableID, JET_bitWriteLock));
	}

	///<summary>Acquires both a read and write lock. See AcquireReadLock and AcquireWriteLock. Calls JetGetLock.</summary>
	///<remarks>Locks are released at the end of the transaction.</remarks>
	void AcquireReadWriteLock()
	{
		EseException::RaiseOnError(JetGetLock(Session->_JetSesid, TableID->_JetTableID, JET_bitReadLock | JET_bitWriteLock));
	}

	///<summary>Best effort detection if an update to the current row would result in a write conflict. Calls JetGetCursorInfo.</summary>
	///<returns>True if a write conflict will occur. False if a write conflict is unknown. Note that a write conflict could still occur even if false is returned.</returns>
	bool IsWriteConflictExpected()
	{
		return JetGetCursorInfo(Session->_JetSesid, TableID->_JetTableID, null, 0, 0) == JET_errWriteConflict;
	}

	property IntPtr JetTableID
	{
		IntPtr get()
		{
			JET_TABLEID JetTableidCopy = _TableID->_JetTableID;

			return marshal_as<IntPtr>(JetTableidCopy);
		}
	}

internal:
	property EseObjects::TableID ^TableID
	{
		EseObjects::TableID ^get() {return _TableID;}
	}
};


///<summary>Position constructed from a colleciton of fields.</summary>
public ref struct FieldPosition : public Limitable
{
	///<summary>Specifies the key fields that make up the position. Not all the fields need to be specified when using a wildcard option.</summary>
	ICollection<Field> ^Fields;
	///<summary>Specifies the match mode to use when putting together the key. Controls how unspecified fields are interpreted.</summary>
	EseObjects::Match Match;
	///<summary>Relative position to key.</summary>
	EseObjects::SeekRel Rel;

	FieldPosition() :
		Fields(nullptr),
		Match(Match::Full),
		Rel(SeekRel::EQ)
	{}

	FieldPosition(ICollection<Field> ^Fields) :
		Fields(Fields),
		Match(Match::Full),
		Rel(SeekRel::EQ)
	{}

	FieldPosition(ICollection<Field> ^Fields, EseObjects::Match Match, SeekRel Rel) :
		Fields(Fields),
		Match(Match),
		Rel(Rel)
	{}

internal:
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) override
	{
		c->LoadKey(Fields, MatchToGrbit(Match));
		c->Seek(SeekRelToGrbit(Rel), HasCurrency, NotEqual);
	}

	virtual bool LimitTo(Cursor ^c, bool upper) override
	{
		c->LoadKey(Fields, MatchToGrbit(Match));
		return c->SetIxRange((SeekRelIsInclusive(Rel) ? JET_bitRangeInclusive : 0) | (upper ? JET_bitRangeUpperLimit : 0));
	}
};

///<summary>Position constructed from a Key object.</summary>
public ref struct KeyPosition : public Limitable
{
	///<summary>Specifies the key value to use for the position. The match mode was specified when the key was built.</summary>
	EseObjects::Key ^Key;
	///<summary>Relative position to key.</summary>
	EseObjects::SeekRel Rel;

	KeyPosition() :
		Key(nullptr),
		Rel(SeekRel::EQ)
	{}

	KeyPosition(EseObjects::Key ^Key) :
		Key(Key),
		Rel(SeekRel::EQ)
	{}

	KeyPosition(EseObjects::Key ^Key, SeekRel Rel) :
		Key(Key),
		Rel(Rel)
	{}

internal:
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) override
	{
		c->LoadKey(Key);
		c->Seek(SeekRelToGrbit(Rel), HasCurrency, NotEqual);
	}

	virtual bool LimitTo(Cursor ^c, bool upper) override
	{
		c->LoadKey(Key);
		return c->SetIxRange((SeekRelIsInclusive(Rel) ? JET_bitRangeInclusive : 0) | (upper ? JET_bitRangeUpperLimit : 0));
	}
};

public ref struct CombinedBookmarkPosition : public Seekable
{
	EseObjects::Bookmark ^Primary;
	EseObjects::SecondaryBookmark ^Secondary;

	CombinedBookmarkPosition() :
		Primary(nullptr),
		Secondary(nullptr)
	{}

	CombinedBookmarkPosition(EseObjects::Bookmark ^Primary, EseObjects::SecondaryBookmark ^Secondary) :
		Primary(Primary),
		Secondary(Secondary)
	{}

internal:
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) override
	{		
		JET_ERR status = JetGotoSecondaryIndexBookmark(GetCursorSesid(c), GetCursorTableID(c), Secondary->_JetBookmark, Secondary->_BookmarkLength, Primary->_JetBookmark, Primary->_BookmarkLength, 0);

		NotEqual = false;
		switch(status)
		{
		case JET_errRecordDeleted:
		case JET_errNoCurrentRecord:
			HasCurrency = false;
			return;
		}

		EseException::RaiseOnError(status);
		HasCurrency = true;
	}
};


JET_TABLEID GetCursorTableID(Cursor ^Csr)
{
	return Csr->TableID->_JetTableID;
}

JET_SESID GetCursorSesid(Cursor ^Csr)
{
	return Csr->Session->_JetSesid;
}

Bridge ^GetCursorBridge(Cursor ^Csr)
{
	return Csr->Bridge;
}