///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Index - Definition and properties of ESE indexes
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
///Represents an ESE Index, used for fast location and sorting of records in a table. Stores a JET_INDEXID and associated properties.
///<pr/>Does not hold any ESE resources: objects from this class do not need to be disposed.
///</summary>
public ref class Index
{
public:
	///<summary>Column information as queried from an index.</summary>
	value struct KeyColumn
	{
		String ^Name;
		Column::Type Type;
		ushort Country;
		ushort Langid;
		Column::CodePage CP;
		ushort Collate;
		///<remarks>If true, sort is descending. If false, sort is ascending</remarks>
		bool SortDescending;
		UnicodeMapFlags MapFlags;
	};

internal:
	JET_INDEXID *_JetIndexID;

private:
	JET_GRBIT _JetFlags;
	ulong _LCID;
	ulong _VarSegMax;
	ushort _KeyMaxBytes;
	String ^_IndexName;
	array<KeyColumn> ^_KeyColumns;

	//nearly empty to populate via RetrieveInfo on a list
	Index() :
		_JetIndexID(new JET_INDEXID)
	{}

	!Index()
	{
		if(_JetIndexID)
			delete _JetIndexID;
	}

internal:
	//the JET_INDEXID can become stale if all the internal ESE references are closed
	//callers will recieve JET_errInvalidIndexId and should call this function to get an updated index ID 
	void FixIndexID(JET_SESID sesid, JET_TABLEID tableid)
	{
		String ^IndexName = _IndexName;
		marshal_context mc;
		char const *indexname = mc.marshal_as<char const *>(IndexName);

		EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, indexname, _JetIndexID, sizeof(JET_INDEXID), JET_IdxInfoIndexId));
	}

	static bool CollectInformation(JET_SESID sesid, JET_TABLEID tableid, char const *indexname_filter, array<Index ^> ^indexes)
	{
		JET_ERR status;
		JET_INDEXLIST jil = {sizeof jil};

		EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, indexname_filter, &jil, sizeof jil, JET_IdxInfo));

		int index_count = indexes->Length;

		try
		{
			status = JetMove(sesid, jil.tableid, JET_MoveFirst, 0);
			switch(status)
			{
			case JET_errSuccess:
				break;

			case JET_errNoCurrentRecord:
				return false; //unexpeted end of dataset

			default:
				EseException::RaiseOnError(status);
			}

			for(int i = 0; i < index_count; i++)
			{
				//temp holding variables, used to hold retrieved values on the stack (as opposed to pinning the member fields)
				ulong local_ulong;
				ushort local_ushort;

				if(indexes[i] == nullptr)
					indexes[i] = gcnew Index();

				EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidgrbitIndex, &local_ulong, sizeof local_ulong, null, 0, null));
				indexes[i]->_JetFlags = local_ulong;

				int key_count = 1;
				EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidcColumn, &key_count, sizeof key_count, null, 0, null));

				array<KeyColumn> ^KeyColumns = gcnew array<KeyColumn>(key_count);
				indexes[i]->_KeyColumns = KeyColumns;

				char retrieved_index_name[JET_cbNameMost+1];
				ulong retrieved_index_name_len = 0;

				EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidindexname, retrieved_index_name, sizeof retrieved_index_name, &retrieved_index_name_len, 0, null));
				retrieved_index_name[retrieved_index_name_len] = null;
				indexes[i]->_IndexName = marshal_as<String ^>(retrieved_index_name);

				EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, retrieved_index_name, &local_ulong, sizeof local_ulong, JET_IdxInfoLCID));
				indexes[i]->_LCID = local_ulong;

				EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, retrieved_index_name, &local_ushort, sizeof local_ushort, JET_IdxInfoVarSegMac));
				indexes[i]->_VarSegMax = local_ushort;

				if(GetEseVersionMajor() >= 6)
				{
					EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, retrieved_index_name, &local_ushort, sizeof local_ushort, JET_IdxInfoKeyMost));
					indexes[i]->_KeyMaxBytes = local_ushort;
				}
				else
					indexes[i]->_KeyMaxBytes = 0;

				EseException::RaiseOnError(JetGetTableIndexInfo(sesid, tableid, retrieved_index_name, indexes[i]->_JetIndexID, sizeof(JET_INDEXID), JET_IdxInfoIndexId));

				for(int j = 0; j < key_count; j++)
				{
					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidcoltyp, &local_ulong, sizeof local_ulong, null, 0, null));
					KeyColumns[j].Type = safe_cast<Column::Type>(local_ulong);

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidCountry, &local_ushort, sizeof local_ushort, null, 0, null));
					KeyColumns[j].Country = local_ushort;

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidLangid, &local_ushort, sizeof local_ushort, null, 0, null));
					KeyColumns[j].Langid = local_ushort;

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidCp, &local_ushort, sizeof local_ushort, null, 0, null));
					KeyColumns[j].CP = safe_cast<Column::CodePage>(local_ushort);

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidCollate, &local_ushort, sizeof local_ushort, null, 0, null));
					KeyColumns[j].Collate = local_ushort;

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidgrbitColumn, &local_ulong, sizeof local_ulong, null, 0, null));
					KeyColumns[j].SortDescending = local_ulong == JET_bitKeyDescending ? true : false; //JET_bitKeyDescending is defined as 1 and JET_bitKeyAscending as 0 anyway

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidLCMapFlags, &local_ulong, sizeof local_ulong, null, 0, null));
					KeyColumns[j].MapFlags = UnicodeMapFlagsFromBits(local_ulong);

					{
						char colname[JET_cbNameMost + 1] = {0};
						ulong name_len = 0;

						EseException::RaiseOnError(JetRetrieveColumn(sesid, jil.tableid, jil.columnidcolumnname, colname, sizeof colname, &name_len, 0, null));

						KeyColumns[j].Name = astring_from_memblock(colname, name_len);
					}

					status = JetMove(sesid, jil.tableid, JET_MoveNext, 0);

					switch(status)
					{
					case JET_errSuccess:
						break;

					case JET_errNoCurrentRecord:
						if(j+1 == key_count)
							return true;
						else
							return false; //unexpected end of data

					default:
						EseException::RaiseOnError(status);
					}
				}
			}
		}
		finally
		{
			JetCloseTable(sesid, jil.tableid);
		}

		return true;
	}

public:
	///<summary>
	///Extra conditions applied to allow this record to appear in the index based on this field.
	///Exactly one of MustBeNull and MustBeNonNull must be set;
	///</summary>
	value struct ConditionalColumn
	{
		///<summary>Name of contingent column.</summary>
		String ^Name;
		bool MustBeNull;
		bool MustBeNonNull;

		ConditionalColumn NewMustBeNull(String ^Name)
		{
			ConditionalColumn cc = {Name, true, false};
			return cc;
		}

		ConditionalColumn NewMustBeNotNull(String ^Name)
		{
			ConditionalColumn cc = {Name, false, true};
			return cc;
		}
	};

internal:
	static JET_GRBIT ConditionalColumnFlagsToBits(ConditionalColumn %o)
	{
		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		JET_GRBIT b = 0;
		b |= o.MustBeNull * JET_bitIndexColumnMustBeNull;
		b |= o.MustBeNonNull * JET_bitIndexColumnMustBeNonNull;
		return b;
	}

public:
	///<summary>Parameters for a tuple index. A zero value represents default or not present.</summary>
	value struct TupleLimits
	{
		///<summary>Minimum length of a tuple. Default 3.</summary>
		ulong LengthMin;
		///<summary>Maximum length of a tuple. Default 10.</summary>
		ulong LengthMax;
		///<summary>Index at most this many characters from the beginning of the string. Default 32767.</summary>
		ulong CharsToIndexMax;
		///<summary>Stride length. Requires 6.0+ to adjust from default of 1.</summary>
		ulong CharsToIncrement;
		///<summary>The offset into the value to begin retreiving tuples. Requires 6.0+.</summary>
		ulong CharStart;
	};

	///<summary>Parameters for creating a new index.</summary>
	value struct CreateOptions
	{
		///<summary>Name of the index. See Jet internal naming restrictions.</summary>
		String ^Name;
		///<summary>Selects and orders columns to include as key fields. Separate column names with dots(.). Each key name must be prefixed with + for ascending or - for descending sort.</summary>
		String ^KeyColumns;
		///<summary>Initial storage density 20-100.</summary>
		ulong Density;

		///<summary>Locale ID</summary>
		ulong LCID;
		///<summary>Additional Unicode mapping flags (optional) (Sets JET_bitIndexUnicode)</summary>
		UnicodeMapFlags UnicodeMapFlags;
		
		//exactly one of the follwing two options:
		///<summary>Maximum length in bytes of each column to store for non-tuple indexes. Not compatible with specifying TupleLimits.</summary>
		ulong VarSegMac;
		///<summary>Specifies limits for a tuple index. Not compatible with VarSegMac. Requires 5.2+.</summary>
		TupleLimits ^TupleLimits;//Sets JET_bitIndexTupleLimits.
		
		///<summary>Additional conditional field options.</summary>
		ICollection<ConditionalColumn> ^ConditionalColumns;
		
		///<summary>Maximum allowable size in bytes for all keys in the index. Requires 6.0+. Sets JET_bitIndexKeyMost if nonzero.</summary>
		ulong KeyMost;

		///<summary>Duplicate entries are not allowed.</summary>
		bool Unique;
		///<summary>Primary (clustered) index. Every table must have exactly one primary index. ESE will create its own if none are defined.</summary>
		bool Primary;
		///<summary>None of the columns which are indexed over can contain null.</summary>
		bool DisallowNull;
		///<summary>Ignore rows with all null key values.</summary>
		bool IgnoreAllNull;
		///<summary>Ignore rows with any null key values.</summary>
		bool IgnoreAnyNull;
		///<summary>Ignore rows with null as the first key value.</summary>
		bool IgnoreFirstNull;
		///<summary>Hint to flush indexing operations lazily. Does not affect data updates.</summary>
		bool LazyFlush;
		///<summary>Hint that no rows would qualify to be included in the index, so a scan can be avoided and an empty index created.</summary>
		bool IndexEmpty;
		///<summary>Creatinon of the index visible to other transactions; attempting to create an index with the same name unversioned in multiple concurent transactions will fail.</summary>
		bool Unversioned;
		///<summary>Null is sorted last (as opposed to first, which is the default behavior).</summary>
		bool SortNullHigh;
		///<summary>Tuple index. Requires 5.1+</summary>
		bool Tuples;
		///<summary>Cross product multivalued index. Requires 6.0+</summary>
		bool CrossProduct;
		///<summary>Raise an error when an index key would be truncated. Requires 6.0+</summary>
		bool DisallowTruncation;

		//convenience constructors
		///<summary>New index struct for secondary index with specified values></summary>
		static CreateOptions NewPrimary(String ^Name, String ^KeyColumns, bool Unique)
		{
			CreateOptions o;
			o.Name = Name;
			o.KeyColumns = KeyColumns;
			o.Unique = Unique;
			o.Primary = true;
			return o;
		}

		///<summary>New index struct for secondary index with specified values></summary>
		static CreateOptions NewSecondary(String ^Name, String ^KeyColumns, bool Unique)
		{
			CreateOptions o;
			o.Name = Name;
			o.KeyColumns = KeyColumns;
			o.Unique = Unique;
			o.Primary = false;
			return o;
		}
	};

internal:
	static JET_GRBIT CreateOptionsFlagsToBits(CreateOptions %o)
	{
		//bool converts to either 1 for true or 0 for false
		//multiplying to either identity of the bitmask or 0
		JET_GRBIT b = 0;
		b |= o.Unique * JET_bitIndexUnique;
		b |= o.Primary * JET_bitIndexPrimary;
		b |= o.DisallowNull * JET_bitIndexDisallowNull;
		b |= o.IgnoreAllNull * JET_bitIndexIgnoreNull;
		b |= o.IgnoreAnyNull * JET_bitIndexIgnoreAnyNull;
		b |= o.IgnoreFirstNull * JET_bitIndexIgnoreFirstNull;
		b |= o.LazyFlush * JET_bitIndexLazyFlush;
		b |= o.IndexEmpty * JET_bitIndexEmpty;
		b |= o.Unversioned * JET_bitIndexUnversioned;
		b |= o.SortNullHigh * JET_bitIndexSortNullsHigh;
		b |= o.Tuples * JET_bitIndexTuples;
		b |= o.CrossProduct * JET_bitIndexCrossProduct;
		b |= o.DisallowTruncation * JET_bitIndexDisallowTruncation;
		return b;
	}

	template <class T> static void PopulateJetIndexCreateCommon(T &jic, CreateOptions %NewIx, marshal_context %mc, free_list &fl)
	{
		String ^Name = NewIx.Name;

		jic.cbStruct = sizeof jic;
		jic.szIndexName = const_cast<char *>(mc.marshal_as<char const *>(Name));
		jic.grbit = CreateOptionsFlagsToBits(NewIx);

		{
			String ^KeyColumns = NewIx.KeyColumns;

			ulong buffct = KeyColumns->Length + 2; //plus double null terminator

			char const *src = mc.marshal_as<char const *>(KeyColumns);

			char *buff = fl.alloc_array_zero<char>(buffct);

			memcpy(buff, src, KeyColumns->Length);

			for(ulong i = 0; i < buffct; i++)
				if(buff[i] == '.')
					buff[i] = '\0'; //ESE needs null delimited, double null ending already done

			jic.szKey = buff;
			jic.cbKey = buffct;
		}
		
		//union between lcid and dwMapFlags
		if(NewIx.UnicodeMapFlags.SortKey)
		{
			jic.grbit |= JET_bitIndexUnicode;

			jic.pidxunicode = fl.alloc_zero<JET_UNICODEINDEX>();
			jic.pidxunicode->lcid = NewIx.LCID;
			jic.pidxunicode->dwMapFlags = UnicodeMapFlagsToBits(NewIx.UnicodeMapFlags);
		}
		else
			jic.lcid = NewIx.LCID;

		//union between cbVarSegMac and ptuplelimits
		if(NewIx.TupleLimits != nullptr)
		{
			if(NewIx.VarSegMac != 0)
				throw gcnew ArgumentException("Can't specify both TupleLimtis and VarSegMac");

			jic.grbit |= JET_bitIndexTupleLimits;

			jic.ptuplelimits = fl.alloc_zero<JET_TUPLELIMITS>();
			jic.ptuplelimits->chLengthMin = NewIx.TupleLimits->LengthMin;
			jic.ptuplelimits->chLengthMax = NewIx.TupleLimits->LengthMax;
			jic.ptuplelimits->chToIndexMax = NewIx.TupleLimits->CharsToIndexMax;
			jic.ptuplelimits->cchIncrement = NewIx.TupleLimits->CharsToIncrement;
			jic.ptuplelimits->ichStart = NewIx.TupleLimits->CharStart;
		}
		else
			jic.cbVarSegMac = NewIx.VarSegMac;

		if(NewIx.ConditionalColumns && NewIx.ConditionalColumns->Count > 0)
		{
			ulong k = 0;
			jic.cConditionalColumn = NewIx.ConditionalColumns->Count;
			jic.rgconditionalcolumn = fl.alloc_array_zero<JET_CONDITIONALCOLUMN>(jic.cConditionalColumn);

			for each (ConditionalColumn %CC in NewIx.ConditionalColumns)
			{
				String ^Name = CC.Name;

				jic.rgconditionalcolumn[k].cbStruct = sizeof jic.rgconditionalcolumn[k];
				jic.rgconditionalcolumn[k].szColumnName = const_cast<char *>(mc.marshal_as<char const *>(Name));
				jic.rgconditionalcolumn[k].grbit = ConditionalColumnFlagsToBits(CC);
			}
		}
	}

	static JET_INDEXCREATE MakeJetIndexCreate(CreateOptions %NewIx, marshal_context %mc, free_list &fl)
	{
		JET_INDEXCREATE jic = {sizeof jic};
		PopulateJetIndexCreateCommon(jic, NewIx, mc, fl);

		if(NewIx.KeyMost)
		{
			jic.cbKeyMost = NewIx.KeyMost;
			jic.grbit |= JET_bitIndexKeyMost;
		}

		return jic;
	}

	static JET_INDEXCREATE_SHORT MakeJetIndexCreateShort(CreateOptions %NewIx, marshal_context %mc, free_list &fl)
	{
		JET_INDEXCREATE_SHORT jic = {sizeof jic};
		PopulateJetIndexCreateCommon(jic, NewIx, mc, fl);
		return jic;
	}

	Index(CreateOptions %co, JET_SESID sesid, JET_TABLEID tableid) :
		_IndexName(co.Name),
		_JetFlags(CreateOptionsFlagsToBits(co)),
		_LCID(co.LCID),
		_VarSegMax(co.VarSegMac),
		_JetIndexID(new JET_INDEXID)
	{
		array<wchar_t> ^delimiters = gcnew array<wchar_t>(1);
		delimiters[0] = L'.';
		
		array<String ^> ^ColNames = co.KeyColumns->Split(delimiters);
		
		_KeyColumns = gcnew array<KeyColumn>(ColNames->Length);

		ulong i = 0;

		for each(String ^s in ColNames)
		{
			_KeyColumns[i].Name = s;
			i++;
		}

		FixIndexID(sesid, tableid);
	}

public:
	///<summary>Opens an existing index on a table.</summary>
	Index(Table ^Table, String ^Name) :
		_JetIndexID(new JET_INDEXID)
	{
		marshal_context mc;
		char const *index_namestr = mc.marshal_as<char const *>(Name);
		JET_TABLEID tableid = GetTableTableID(Table);
		JET_SESID sesid = GetTableSesid(Table);

		array<Index ^> ^ix_arr = gcnew array<Index ^>(1);
		ix_arr[0] = this;

		if(!CollectInformation(sesid, tableid, index_namestr, ix_arr))
			throw gcnew ArgumentException("Unexpected end of index results");
	}

	///<summary>Creates a new index on the specified table. Calls JetCreateIndex2.</summary>
	static Index ^Create(Table ^Table, CreateOptions %co)
	{
		JET_TABLEID tableid = GetTableTableID(Table);
		JET_SESID sesid = GetTableSesid(Table);

		marshal_context mc;
		free_list fl;

		if(co.KeyMost) //need full 6.0+ only struct
		{
			JET_INDEXCREATE jic = MakeJetIndexCreate(co, mc, fl);
			
			EseException::RaiseOnError(JetCreateIndex2(sesid, tableid, &jic, 1));
		}
		else //can use short <6.0 struct
		{
			JET_INDEXCREATE_SHORT jic = MakeJetIndexCreateShort(co, mc, fl);
			
			EseException::RaiseOnError(JetCreateIndex2(sesid, tableid, reinterpret_cast<JET_INDEXCREATE *>(&jic), 1));
		}

		return gcnew Index(co, sesid, tableid);
	}

	static void Delete(Table ^Table, String ^Name)
	{
		JET_TABLEID tableid = GetTableTableID(Table);
		JET_SESID sesid = GetTableSesid(Table);
		marshal_context mc;
		char const *namestr = mc.marshal_as<char const *>(Name);

		EseException::RaiseOnError(JetDeleteIndex(sesid, tableid, namestr));
	}

public:
	property ulong ColumnCount
	{
		ulong get() {return _KeyColumns->Length;}
	}

	property ulong LCID
	{
		ulong get() {return _LCID;}
	}

	property ulong VarSegMac
	{
		ulong get() {return _VarSegMax;}
	}

	property ushort KeyMaxBytes
	{
		ushort get() {return _KeyMaxBytes;}
	}

	property String ^IndexName
	{
		String ^get() {return _IndexName;}
	}

	property IEnumerable<KeyColumn> ^KeyColumns
	{
		IEnumerable<KeyColumn> ^get() {return _KeyColumns;}
	}
};