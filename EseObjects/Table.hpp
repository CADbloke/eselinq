///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Table - Representation of ESE tables
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

DemandLoadFunction<JET_ERR (JET_API *)(JET_SESID sesid, JET_OPENTEMPORARYTABLE *popentemporarytable)> JetOpenTemporaryTable_demand(L"esent.dll", "JetOpenTemporaryTableA");

///<summary>Special version of exception with extra fields explaining element specific errors, if any.</summary>
public ref struct CreateTableException : EseException
{
	initonly array<EseException ^const> ^SpecificColumnErrors;
	initonly array<EseException ^const> ^SpecificIndexErrors;

	CreateTableException(JET_ERR Code, array<EseException ^const> ^SpecificColumnErrors, array<EseException ^const> ^SpecificIndexErrors) :
		EseException(*EseException::CreateFromCode(Code)),
		SpecificColumnErrors(SpecificColumnErrors),
		SpecificIndexErrors(SpecificIndexErrors)
	{}
};

///<summary>Represents a single ESE table in a database.
///Counts as an open table and uses a cursor resource.
///No information properties are availaible on a temporary table.
///</summary>
public ref class Table
{
	TableID ^_TableID;

	~Table()
	{
		_TableID->~TableID();
	}

internal:
	Table(TableID ^TableID) :
		_TableID(TableID)
	{}

public:
	///<summary>Extra options that can be specified when opening a table or cursor.</summary>
	value struct OpenOptions
	{
		///<summary>Table not availaible for read access by another session while open.</summary>
		bool DenyRead;
		///<summary>Table not avaiaible for write access by another session while open.</summary>
		bool DenyWrite;
		///<summary>Do not cache pages for the table.</summary>
		bool NoCache;
		///<summary>Premit DDL modifications despite FixedDDL setting. Requires DenyRead.</summary>
		bool PermitDDL;
		///<summary>Hint to pre-read table.</summary>
		bool Preread;
		///<summary>Requests read only access to table.</summary>
		bool ReadOnly;
		///<summary>Hint for agressive prefetching to optimize sequential scanning.</summary>
		bool Sequential;
		///<summary>Requests write access to table.</summary>
		bool Updatable;
	};

private:
	static JET_GRBIT TableOpenOptsToBits(OpenOptions %o)
	{
		JET_GRBIT b = 0;

		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		b |= o.DenyRead * JET_bitTableDenyRead;
		b |= o.DenyWrite * JET_bitTableDenyWrite;
		b |= o.NoCache * JET_bitTableNoCache;
		b |= o.PermitDDL * JET_bitTablePermitDDL;
		b |= o.Preread * JET_bitTablePreread;
		b |= o.ReadOnly * JET_bitTableReadOnly;
		b |= o.Sequential * JET_bitTableSequential;
		b |= o.Updatable * JET_bitTableUpdatable;

		return b;
	}

public:
	///<summary>Parameters for creaing a new table. 0 represents default or not present.</summary>
	value struct CreateOptions
	{
		///<summary>Name of table. See Jet internal naming restrictions. Required.</summary>
		String ^Name;
		///<summary>Uses the specified table as a template to inherit column and index definitions.</summary>
		String ^TemplateName;
		///<summary>Initial number of pages to allocate. Pre-allocation can reduce fragmentation.</summary>
		ulong Pages;
		///<summary>Table density, range 20-100 percent. Default 80.</summary>
		ulong Density;
		///<summary>Structure of table is fixed.</summary>
		bool FixedDDL;
		///<summary>Can be used as TemplateTableName when creating another table. Implies FixedDDL.</summary>
		bool IsTemplateTable;
		///<summary>Sequenced list of columns.</summary>
		ICollection<Column::CreateOptions> ^Columns;
		///<summary>List of indexes.</summary>
		ICollection<Index::CreateOptions> ^Indexes;

		//convenience constructors
		CreateOptions(String ^Name, ICollection<Column::CreateOptions> ^Columns, ICollection<Index::CreateOptions> ^Indexes) :
			Name(Name),
			Columns(Columns),
			Indexes(Indexes)
		{}

		///<summary>New table struct initialized with name and new System.Collections.Generic.Lists backing the Columns and Indexes members.</summary>
		static CreateOptions NewWithLists(String ^Name)
		{
			CreateOptions c;
			c.Name = Name;
			c.Columns = gcnew List<Column::CreateOptions>();
			c.Indexes = gcnew List<Index::CreateOptions>();
			return c;
		}

		//NEXT: callbacks
		//delegate Callback;
	};


	///<summary>Parameters for creaing a new temp table. 0 represents default or not present.</summary>
	value struct CreateTempOptions
	{
		///<summary>Additional Unicode mapping flags for keys (optional)</summary>
		UnicodeMapFlags UnicodeMapFlags;
		///<summary>Specifies the LCID for keys in the temp table. Not used unless UnicodeMapFlags.SortKey is specified.</summary>(*CreatedColumns)[i] = gcnew Column(jcd, NewCol.Name, Parameters.Name, NewCol.Name);
		ulong LCID;
		///<summary>Enables support to seek to a particular key. Can reduce efficency.</summary>
		bool Indexed;
		///<summary>
		///Removes duplicate records from the final dataset.
		///Does not imply a warning if a duplicate is entered. (See ErrorOnDuplicateInsertion).
		///Always implied unless ForwardOnly is specified instead.
		///</summary>
		bool Unique;
		///<summary>Enables changes to temp table data after insert. Can reduce efficency.</summary>
		bool Updatable;
		///<summary>Enables aritrary repositioning of the cursor. If false, only allows starting over and moving forward.</summary>
		bool Scrollable;
		///<summary>Sorts null values high (instead of default low) in primary key.</summary>
		bool SortNullsHigh;
		///<summary>Forces all data to be stored with a B+ tree instead of attempting a more efficient strategy. Enables the most functionaility.</summary>
		bool ForceMaterialization;
		///<summary>Raises an error if an insert would result in a key collision. Can reduce efficency.</summary>
		bool ErrorOnDuplicateInsertion;
		///<summary>Attempts to optimize for forward-only operation. Depending on options, may fail with JET_errCannotMaterializeForwardOnlySort. Can enable duplicate key values. A forward-only cursor cannot be duplicated properly. Requires 5.2+.</summary>
		bool ForwardOnly;
		///<summary>Instructs EseObjects to attempt to open with ForwardOnly and retry without the flag if the create fails.</summary>
		bool ForwardOnlyIfPossible;
		///<summary>Maximum size for a key representing a row. Larger sizes will be truncated. Requires 6.0+.</summary>
		ulong KeyMost;
		///<summary>Maximum data from any variable column to include in key column. Requires 6.0+.</summary>
		ulong VarSegMac;
		///<summary>Sequenced list of columns.</summary>
		ICollection<Column::CreateOptions> ^Columns;

		CreateTempOptions(ICollection<Column::CreateOptions> ^Columns) :
			Columns(Columns)
		{}

		///<summary>New temp table struct initialized with a new System.Collections.Generic.List backing the Columns member.</summary>
		static CreateTempOptions NewWithLists()
		{
			CreateTempOptions c;
			c.Columns = gcnew List<Column::CreateOptions>();
			return c;
		}
	};

internal:
	static JET_GRBIT CreateOptionsFlagsToBits(CreateOptions %o)
	{
		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		JET_GRBIT b = 0;
		b |= o.FixedDDL * JET_bitTableCreateFixedDDL;
		b |= o.IsTemplateTable * JET_bitTableCreateTemplateTable;
		return b;
	}

	static JET_GRBIT CreateTempOptionsFlagsToBits(CreateTempOptions %o)
	{
		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		JET_GRBIT b = 0;
		b |= o.Indexed * JET_bitTTIndexed;
		b |= o.Unique * JET_bitTTUnique;
		b |= o.Updatable * JET_bitTTUpdatable;
		b |= o.Scrollable * JET_bitTTScrollable;
		b |= o.SortNullsHigh * JET_bitTTSortNullsHigh;
		b |= o.ForceMaterialization * JET_bitTTForceMaterialization;
		b |= o.ErrorOnDuplicateInsertion * JET_bitTTErrorOnDuplicateInsertion;
		b |= o.ForwardOnly * JET_bitTTForwardOnly;
		return b;
	}

public:
	///<summary>Opens an existing table with default options.</summary>
	Table(Database ^Db, String ^TableName) :
		_TableID(nullptr)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(TableName);

		JET_TABLEID NewTableID = null;

		EseException::RaiseOnError(JetOpenTable(Db->Session->_JetSesid, Db->_JetDbid, NameChar, null, 0, 0, &NewTableID));

		_TableID = gcnew EseObjects::TableID(NewTableID, Db->Session->_CurrentTrans, Db);
	}

	///<summary>Opens an existing table with specified options.</summary>
	Table(Database ^Db, String ^TableName, OpenOptions ^OpenOptions) :
		_TableID(nullptr)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(TableName);
		JET_GRBIT flags = TableOpenOptsToBits(*OpenOptions);

		JET_TABLEID NewTableID = null;

		EseException::RaiseOnError(JetOpenTable(Db->Session->_JetSesid, Db->_JetDbid, NameChar, null, 0, flags, &NewTableID));

		_TableID = gcnew EseObjects::TableID(NewTableID, Db->Session->_CurrentTrans, Db);
	}

private:
	static Table ^InternalCreate(Database ^Db, CreateOptions Parameters, [Out] array<Column ^> ^*CreatedColumns, [Out] array<Index ^> ^*CreatedIndexes)
	{
		marshal_context mc;
		free_list fl;

		JET_TABLECREATE jtc = {sizeof jtc};
		JET_INDEXCREATE_SHORT **jic_ptrs;

		jtc.szTableName = const_cast<char *>(mc.marshal_as<char const *>(Parameters.Name));
		jtc.szTemplateTableName = const_cast<char *>(mc.marshal_as<char const *>(Parameters.TemplateName));
		jtc.ulPages = Parameters.Pages;
		jtc.ulDensity = Parameters.Density;
		jtc.grbit = CreateOptionsFlagsToBits(Parameters); //explicit options

		if(Parameters.Columns && Parameters.Columns->Count > 0)
		{
			jtc.cColumns = Parameters.Columns->Count;
			jtc.rgcolumncreate = fl.alloc_array_zero<JET_COLUMNCREATE>(jtc.cColumns);

			JET_COLUMNCREATE *jccs = jtc.rgcolumncreate; //array of column creates
			ulong i = 0; //index into jccs;

			for each(Column::CreateOptions %NewCol in Parameters.Columns)
			{
				String ^Name = NewCol.Name;

				jccs[i].cbStruct = sizeof jccs[i];
				jccs[i].szColumnName = const_cast<char *>(mc.marshal_as<char const *>(Name));
				jccs[i].coltyp = System::Convert::ToUInt32(NewCol.Type);
				jccs[i].cbMax = NewCol.MaxLength;
				jccs[i].grbit = Column::CreateOptionsFlagsToBits(NewCol);
				jccs[i].cp = System::Convert::ToUInt32(NewCol.CP);

				bool empty;

				if(NewCol.DefaultValue)
					to_memblock_bridge(Db->Bridge, NewCol.DefaultValue, jccs[i].pvDefault, jccs[i].cbDefault, empty, jccs[i].coltyp, static_cast<ushort>(jccs[i].cp), mc, fl);
				else
				{
					jccs[i].pvDefault = null;
					jccs[i].cbDefault = 0;
				}

				i++;
			}
		}


		if(Parameters.Indexes && Parameters.Indexes->Count > 0)
		{
			bool needs_longstruct = false;

			for each(Index::CreateOptions %NewIx in Parameters.Indexes)
				if(NewIx.KeyMost)
					needs_longstruct = true;

			jtc.cIndexes = Parameters.Indexes->Count;
			jic_ptrs = fl.alloc_array_zero<JET_INDEXCREATE_SHORT *>(jtc.cIndexes);

			if(needs_longstruct)
			{
				JET_INDEXCREATE *jics = fl.alloc_array_zero<JET_INDEXCREATE>(jtc.cIndexes);

				jtc.rgindexcreate = jics;
				ulong i = 0;

				for each(Index::CreateOptions %NewIx in Parameters.Indexes)
				{
					jics[i] = Index::MakeJetIndexCreate(NewIx, mc, fl);
					jic_ptrs[i] = reinterpret_cast<JET_INDEXCREATE_SHORT *>(jics + i); //JET_INDEXCREATE is JET_INDEXCREATE_SHORT with an extra field

					i++;
				}
			}
			else
			{
				JET_INDEXCREATE_SHORT *jics = fl.alloc_array_zero<JET_INDEXCREATE_SHORT>(jtc.cIndexes);
				jtc.rgindexcreate = reinterpret_cast<JET_INDEXCREATE *>(jics); //short is the < 6.0 version, compatible with either version
				ulong i = 0;

				for each(Index::CreateOptions %NewIx in Parameters.Indexes)
				{
					jics[i] = Index::MakeJetIndexCreateShort(NewIx, mc, fl);
					jic_ptrs[i] = jics + i; //save pointer for return info

					i++;
				}
			}
		}

		//NEXT: support JetCreateTableColumnIndex2. Requires callbacks
		int Status = JetCreateTableColumnIndex(Db->Session->_JetSesid, Db->_JetDbid, &jtc);

		if(Status < 0) //if an error
		{
			array<EseException ^> ^ColErrs;
			array<EseException ^> ^IxErrs;

			if(Parameters.Columns && Parameters.Columns->Count > 0)
			{
				ColErrs = gcnew array<EseException ^>(Parameters.Columns->Count);

				int i = 0;

				for each(Column::CreateOptions %NewCol in Parameters.Columns)
				{
					ColErrs[i] = EseException::CreateFromCode(jtc.rgcolumncreate[i].err);
					i++;
				}
			}

			if(Parameters.Indexes && Parameters.Indexes->Count > 0)
			{
				IxErrs = gcnew array<EseException ^>(Parameters.Indexes->Count);

				int i = 0;

				for each(Index::CreateOptions %NewIx in Parameters.Indexes)
				{
					IxErrs[i] = EseException::CreateFromCode((*jic_ptrs)[i].err);
					i++;
				}
			}

			CreateTableException ^OverallError = gcnew CreateTableException(Status, ColErrs, IxErrs);

			throw OverallError;
		}

		//create return objects

		EseObjects::TableID ^NTableID = gcnew EseObjects::TableID(jtc.tableid, Db->Session->CurrentTransaction, Db);

		Table ^NTable = gcnew Table(NTableID);

		if(CreatedColumns)
		{
			*CreatedColumns = gcnew array<Column ^>(jtc.cColumns);

			int i = 0;

			for each(Column::CreateOptions %NewCol in Parameters.Columns)
			{
				JET_COLUMNDEF jcd = {sizeof jcd};
				jcd.columnid = jtc.rgcolumncreate[i].columnid;
				jcd.coltyp = jtc.rgcolumncreate[i].coltyp;
				jcd.cp = static_cast<ushort>(jtc.rgcolumncreate[i].cp); //the fields are different sizes for some reason, only known values are small enough anyway
				jcd.cbMax = jtc.rgcolumncreate[i].cbMax;
				jcd.grbit = jtc.rgcolumncreate[i].grbit;

				(*CreatedColumns)[i] = gcnew Column(jcd, NewCol.Name, Parameters.Name, NewCol.Name);

				i++;
			}
		}

		if(CreatedIndexes)
		{
			*CreatedIndexes = gcnew array<Index ^>(jtc.cIndexes);

			int i = 0;

			for each(Index::CreateOptions %NewIx in Parameters.Indexes)
			{
				(*CreatedIndexes)[i] = gcnew Index(NewIx, Db->Session->_JetSesid, jtc.tableid);
				i++;
			}
		}
		
		return NTable;
	}

	static Cursor ^InternalCreateTemp(Session ^Session, CreateTempOptions Parameters, [Out] array<Column ^> ^*CreatedColumns)
	{
		marshal_context mc;
		free_list fl;

		if(!Parameters.Columns || Parameters.Columns->Count < 1)
			throw gcnew ArgumentException("Must specify at least one column when creating a temp table");

		ulong col_ct = Parameters.Columns->Count;
		JET_COLUMNDEF *jcds = fl.alloc_array_zero<JET_COLUMNDEF>(col_ct);
		JET_COLUMNID *jcids = fl.alloc_array_zero<JET_COLUMNID>(col_ct);
		JET_TABLEID jtid = null;
		JET_UNICODEINDEX *juix = null;
		JET_GRBIT grbit = CreateTempOptionsFlagsToBits(Parameters);
		
		if(Parameters.ForwardOnlyIfPossible)
			grbit |= JET_bitTTForwardOnly;

		{
			int i = 0;

			for each(Column::CreateOptions %NewCol in Parameters.Columns)
			{
				if(NewCol.Name != nullptr)
					throw gcnew ArgumentException("Temp table columns can not have names");

				jcds[i].cbStruct = sizeof jcds[i];
				jcds[i].coltyp = System::Convert::ToUInt32(NewCol.Type);
				jcds[i].cbMax = NewCol.MaxLength;
				jcds[i].grbit = Column::CreateOptionsFlagsToBits(NewCol);
				jcds[i].cp = System::Convert::ToUInt32(NewCol.CP);

				i++;
			}
		}

		if(Parameters.UnicodeMapFlags.SortKey)
		{
			juix = fl.alloc_zero<JET_UNICODEINDEX>();
			juix->lcid = Parameters.LCID;
			juix->dwMapFlags = UnicodeMapFlagsToBits(Parameters.UnicodeMapFlags);
		}

		//JetOpenTemporaryTable, only availaible in 6.0+ needs to be called if KeyMost or VarSegMac specified.
		//otherwise, call JetOpenTempTable3, which works on earlier platforms.
		
		if(Parameters.KeyMost || Parameters.VarSegMac) //need to use 6.0+ version
		{
			JET_OPENTEMPORARYTABLE jott = {sizeof jott};
			jott.prgcolumndef = jcds;
			jott.ccolumn = col_ct;
			jott.pidxunicode = juix;
			jott.grbit = grbit;
			jott.prgcolumnid = jcids;
			jott.cbKeyMost = Parameters.KeyMost;
			jott.cbVarSegMac = Parameters.VarSegMac;

			JET_ERR status = JetOpenTemporaryTable_demand(Session->_JetSesid, &jott);

			if(Parameters.ForwardOnlyIfPossible && status == JET_errCannotMaterializeForwardOnlySort)
			{
				jott.grbit &= ~JET_bitTTForwardOnly;

				status = JetOpenTemporaryTable_demand(Session->_JetSesid, &jott);
			}

			EseException::RaiseOnError(status);

			jtid = jott.tableid;
		}
		else
		{
			JET_ERR status = JetOpenTempTable3(Session->_JetSesid, jcds, col_ct, juix, grbit, &jtid, jcids);

			if(Parameters.ForwardOnlyIfPossible && status == JET_errCannotMaterializeForwardOnlySort)
			{
				grbit &= ~JET_bitTTForwardOnly;

				status = JetOpenTempTable3(Session->_JetSesid, jcds, col_ct, juix, grbit, &jtid, jcids);
			}
			
			EseException::RaiseOnError(status);
		}

		if(CreatedColumns)
		{
			*CreatedColumns = gcnew array<Column ^>(col_ct);

			int i = 0;

			for each(Column::CreateOptions %NewCol in Parameters.Columns)
			{
				jcds[i].columnid = jcids[i];
				(*CreatedColumns)[i] = gcnew Column(jcds[i], nullptr, nullptr, nullptr);
				i++;
			}
		}

		EseObjects::TableID ^NTableID = gcnew EseObjects::TableID(jtid, Session->CurrentTransaction, gcnew EseObjects::Database(Session));

		Cursor ^NCursor = gcnew Cursor(NTableID);

		return NCursor;
	}

public:
	///<summary>
	///Creates a new table with specified columns and indexes in the database, also returning the new columns and indexes as objects.
	///</summary>
	///<param name="CreatedColumns">Returns an array of created columns, in the same order as specified in Parameters.Columns. Does not include derived columns.</param>
	///<param name="CreatedIndexes">Returns an array of created indexes, in the same order as specified in Parameters.Indexes.</param>
	static Table ^Create(Database ^Db, CreateOptions Parameters, [Out] array<Column ^> ^%CreatedColumns, [Out] array<Index ^> ^%CreatedIndexes)
	{
		array<Column ^> ^CreatedColArr;
		array<Index ^> ^CreatedIxArr;

		Table ^NTable = InternalCreate(Db, Parameters, &CreatedColArr, &CreatedIxArr);

		CreatedColumns = CreatedColArr;
		CreatedIndexes = CreatedIxArr;

		return NTable;
	}

	///<summary>
	///Creates a new table with specified columns and indexes in the database.
	///</summary>
	static Table ^Create(Database ^Db, CreateOptions Parameters)
	{
		return InternalCreate(Db, Parameters, null, null);
	}

	///<summary>
	///Creates a new temporary table with specified columns and indexes in the database, also returning the new columns and indexes as objects.
	///Temp tables are more efficient, but have some restrictions. Calls JetOpenTempTable3 or JetOpenTemporaryTable depending on features requried.
	///No DTD or property information is availaible on a temp table, so this function returns the JET_TABLEID as a cursor.
	///</summary>
	///<param name="CreatedColumns">Returns an array of created columns, in the same order as specified in Parameters.Columns. Does not include derived columns.</param>
	static Cursor ^CreateTemp(Session ^Session, CreateTempOptions Parameters, [Out] array<Column ^> ^%CreatedColumns)
	{
		array<Column ^> ^CreatedColArr;

		Cursor ^NCursor = InternalCreateTemp(Session, Parameters, &CreatedColArr);

		CreatedColumns = CreatedColArr;

		return NCursor;
	}

	///<summary>Session associated with this table handle</summary>
	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _TableID->Session;}
	}

	///<summary>Database associated with this table</summary>
	property EseObjects::Database ^Database
	{
		EseObjects::Database ^get() {return _TableID->Database;}
	}

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _TableID->Bridge;};
		void set(EseObjects::Bridge ^bridge) {_TableID->Bridge = bridge;};
	};

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

	JET_OBJECTINFO GetObjInfo()
	{
		JET_OBJECTINFO ObjInfo = {sizeof ObjInfo};

		EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, &ObjInfo, sizeof ObjInfo, JET_TblInfo));

		return ObjInfo;
	}

public:
	///<summary>Name of table. Queried by calling JetGetTableInfo(JET_TblInfoName) Can be set, which renames the table, calling JetRenameTable.</summary>
	property String ^Name
	{
		String ^get()
		{
			char buff[JET_cbNameMost+1];

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, buff, JET_cbNameMost+1, JET_TblInfoName));

			return marshal_as<String ^>(buff);
		}
		void set(String ^NewName)
		{
			marshal_context mc;
			char oldname[JET_cbNameMost+1];
			char const *newname = mc.marshal_as<char const *>(NewName);

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, oldname, JET_cbNameMost+1, JET_TblInfoName));

			EseException::RaiseOnError(JetRenameTable(Session->_JetSesid, Database->_JetDbid, oldname, newname));
		}
	}

	///<summary>Table can have bookmarks. From JetGetTableInfo JET_OBJECTINFO JET_bitTableInfoBookmark.</summary>
	property bool Bookmarkable
	{
		bool get() {return GetObjInfo().grbit & JET_bitTableInfoBookmark;}
	}

	///<summary>Table can be rolled back. From JetGetTableInfo JET_OBJECTINFO JET_bitTableInfoRollback.</summary>
	property bool Rollbackable
	{
		bool get() {return GetObjInfo().grbit & JET_bitTableInfoRollback;}
	}

	///<summary>Table can be updated. From JetGetTableInfo JET_OBJECTINFO JET_bitTableInfoUpdatable.</summary>
	property bool Updatable
	{
		bool get() {return GetObjInfo().grbit & JET_bitTableInfoUpdatable;}
	}

	///<summary>Table can be is a system table for internal use. From JetGetTableInfo JET_OBJECTINFO JET_bitObjectSystem.</summary>
	property bool SystemTable
	{
		bool get() {return GetObjInfo().flags & JET_bitObjectSystem;}
	}

	///<summary>Table is derived from a template table. From JetGetTableInfo JET_OBJECTINFO JET_bitObjectTableDerived.</summary>
	property bool Derived
	{
		bool get() {return GetObjInfo().flags & JET_bitObjectTableDerived;}
	}

	///<summary>Table DDL cannot be modified. From JetGetTableInfo JET_OBJECTINFO JET_bitObjectTableFixedDDL.</summary>
	property bool FixedDDL
	{
		bool get() {return GetObjInfo().flags & JET_bitObjectTableFixedDDL;}
	}

	///<summary>From JetGetTableInfo JET_OBJECTINFO JET_bitObjectTableNoFixedVarColumnsInDerivedTables.</summary>
	property bool ObjectTableNoFixedVarColumnsInDerivedTables
	{
		bool get() {return GetObjInfo().flags & JET_bitObjectTableNoFixedVarColumnsInDerivedTables;}
	}

	///<summary>Table is a template table. From JetGetTableInfo JET_OBJECTINFO JET_bitObjectTableTemplate.</summary>
	property bool TemplateTable
	{
		bool get() {return GetObjInfo().flags & JET_bitObjectTableTemplate;}
	}

	///<summary>Table is a template table. From JetGetTableInfo JET_OBJECTINFO cRecord.</summary>
	property ulong RecordCount
	{
		ulong get() {return GetObjInfo().cRecord;}
	}

	///<summary>Calls JetGetTableInfo JET_TblInfoSpaceAlloc value 1.</summary>
	property ulong PageCount
	{
		ulong get()
		{
			ulong arr[2];

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, &arr, sizeof(ulong) * 2, JET_TblInfoSpaceAlloc));

			return arr[0]; //1st item is # of pages
		}
	}

	///<summary>Calls JetGetTableInfo JET_TblInfoSpaceAlloc value 2.</summary>
	property ulong TargetDensity
	{
		ulong get()
		{
			ulong arr[2];

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, &arr, sizeof(ulong) * 2, JET_TblInfoSpaceAlloc));

			return arr[1]; //2nd item is target page density
		}
	}

	///<summary>Total pages availaible in table, indexes, and long value tree. Calls JetGetTableInfo JET_TblInfoSpaceAvailable</summary>
	property ulong SpaceAvailaible
	{
		ulong get()
		{
			ulong val;

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, &val, sizeof val, JET_TblInfoSpaceAvailable));

			return val;
		}
	}

	///<summary>Total extents owned by table. Calls JetGetTableInfo JET_TblInfoSpaceOwned</summary>
	property ulong OwnedExtents
	{
		ulong get()
		{
			ulong val;

			EseException::RaiseOnError(JetGetTableInfo(Session->_JetSesid, _TableID->_JetTableID, &val, sizeof val, JET_TblInfoSpaceOwned));

			return val;
		}
	}

	///<summary>Updates table statistics (number of records and pages) in the table. Calls JetComputeStats.</summary>
	void RecomputeStatistics()
	{
		EseException::RaiseOnError(JetComputeStats(Session->_JetSesid, _TableID->_JetTableID));
	}

	property IList<Column ^> ^Columns
	{
		IList<Column ^> ^get()
		{
			return QueryTableColumns(Session->_JetSesid, _TableID->_JetTableID)->Values;
		}
	}

	property array<Index ^> ^Indexes
	{
		array<Index ^> ^get()
		{
			int index_ct = 0;

			EseException::RaiseOnError(JetGetTableIndexInfo(Session->_JetSesid, _TableID->_JetTableID, null, &index_ct, sizeof index_ct, JET_IdxInfoCount));

			array<Index ^> ^indexes = gcnew array<Index ^>(index_ct);

			Index::CollectInformation(Session->_JetSesid, _TableID->_JetTableID, "", indexes);

			return indexes;
		}
	}

	///<summary>Creates a new Column object by name.</summary>
    property Column ^default[String ^]
    {
        Column ^get(String ^Name) { return gcnew Column(this, Name); }
    }
};


JET_TABLEID GetTableTableID(Table ^Tab)
{
	return Tab->TableID->_JetTableID;
}

JET_SESID GetTableSesid(Table ^Tab)
{
	return Tab->Session->_JetSesid;
}

TableID ^GetTableIDObj(Table ^Tab)
{
	return Tab->TableID;
}

Bridge ^GetTableBridge(Table ^Tab)
{
	return Tab->Bridge;
}

Table ^MakeTableFromTableID(TableID ^Tabid)
{
	return gcnew Table(Tabid);
}