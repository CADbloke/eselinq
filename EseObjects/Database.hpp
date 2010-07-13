///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Bookmark - Exposes ESE boomark support
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
///Represents an open handle to a database. The handle is associated with a particular session. Disposing this object does not destroy or detach the database; it only closes the handle to it.
///Class provides static methods to attach and detach databases by name.
///</summary>
public ref class Database
{
	initonly Session ^_Session;
	initonly String ^_DatabaseName;

internal:
	JET_DBID _JetDbid;
	Bridge ^_Bridge;

private:
	Database(Session ^Session, String ^DatabaseName, JET_DBID JetDbid) :
		_Session(Session),
		_DatabaseName(DatabaseName),
		_JetDbid(JetDbid),
		_Bridge(Session->_Bridge)
	{}

internal:
	//creates a temp database placeholder object
	Database(Session ^Session) :
		_Session(Session),
		_DatabaseName("<temp>"),
		_JetDbid(null),
		_Bridge(Session->_Bridge)
	{}

public:
	value struct CreateOptions
	{
		///<summary>Base name of the database to be created. Normally ends in .edb</summary>
		String ^FileName;
		///<summary>Maximum size of database enforced by the engine in pages. Zero represents no limit.</summary>
		ulong DatabaseSizeMax;
		///<summary>If the database already exists, an error will be raised unless this flag is True. Requires 5.1+</summary>
		bool OverwriteExisting;
		///<summary>Disables logging and full recovery. The database is likely to be in an unusable state after a dirty close with this flag enabled.</summary>
		bool RecoveryOff;
		///<summary>Disables redundancy of certain internal structures.</summary>
		bool ShadowingOff;

		//convenience constructors
		CreateOptions(String ^FileName) :
			FileName(FileName)
		{}
	};

	value struct OpenOptions
	{
		bool Exclusive;
		bool ReadOnly;
	};

	value struct AttachOptions
	{
		bool DeleteCorruptIndexes;
		bool DeleteUnicodeIndexes;
		bool ReadOnly;
		ulong DatabaseSizeMax;
	};

	value struct DetachOptions
	{
		bool ForceCloseAndDetach;
		bool ForceDetach;
	};

private:
	static JET_GRBIT CreateOptionsFlagsToBits(CreateOptions %x)
	{
		JET_GRBIT b = 0;
		b |= x.OverwriteExisting * JET_bitDbOverwriteExisting;
		b |= x.RecoveryOff * JET_bitDbRecoveryOff;
		b |= x.ShadowingOff * JET_bitDbShadowingOff;
		return b;
	}

	static JET_GRBIT OpenOptionsToBits(OpenOptions %x)
	{
		JET_GRBIT b = 0;
		b |= x.Exclusive * JET_bitDbExclusive;
		b |= x.ReadOnly * JET_bitDbReadOnly;
		return b;
	}

	static JET_GRBIT AttachOptionsToBits(AttachOptions %x)
	{
		JET_GRBIT b = 0;
		b |= x.DeleteCorruptIndexes * JET_bitDbDeleteCorruptIndexes;
		b |= x.DeleteUnicodeIndexes * JET_bitDbDeleteUnicodeIndexes;
		b |= x.ReadOnly * JET_bitDbReadOnly;
		return b;
	}

	static JET_GRBIT DetachOptionsToBits(DetachOptions %x)
	{
		JET_GRBIT b = 0;
		b |= x.ForceCloseAndDetach * JET_bitForceCloseAndDetach;
		b |= x.ForceDetach * JET_bitForceDetach;
		return b;
	}

public:

	///<summary>Open an already attached database.</summary>
	Database(Session ^Session, String ^DatabaseName) :
		_Session(Session),
		_DatabaseName(DatabaseName),
		_JetDbid(null),
		_Bridge(Session->_Bridge)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(DatabaseName);
		JET_DBID NewJetDbid = null;

		EseException::RaiseOnError(JetOpenDatabase(Session->_JetSesid, NameChar, null, &NewJetDbid, 0));

		_JetDbid = NewJetDbid;
	}

	///<summary>Open an already attached database wiht the specified options.</summary>
	Database(Session ^Session, String ^DatabaseName, OpenOptions Opts) :
		_Session(Session),
		_DatabaseName(DatabaseName),
		_JetDbid(null),
		_Bridge(Session->_Bridge)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(DatabaseName);
		JET_DBID NewJetDbid = null;
		JET_GRBIT flags = OpenOptionsToBits(Opts);

		EseException::RaiseOnError(JetOpenDatabase(Session->_JetSesid, NameChar, null, &NewJetDbid, flags));

		_JetDbid = NewJetDbid;
	}

	~Database()
	{
		this->!Database();
	}

	!Database()
	{
		if(!_JetDbid)
			return;

		if(_Session->_JetSesid) //only close if session is still open; otherwise ESE will crash
			JetCloseDatabase(_Session->_JetSesid, _JetDbid, 0);
		_JetDbid = null;
	}

	///<summary>Attach an existing database file to this instance. ESE databases cannot be shared between instances concurrently.</summary>
	static Database ^AttachDatabase(Session ^Session, String ^DatabaseName, AttachOptions Opts)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(DatabaseName);
		JET_GRBIT flags = AttachOptionsToBits(Opts);

		EseException::RaiseOnError(JetAttachDatabase2(Session->_JetSesid, NameChar, Opts.DatabaseSizeMax, flags));

		OpenOptions Opts2;
		Opts2.ReadOnly = Opts.ReadOnly;

		return gcnew Database(Session, DatabaseName, Opts2);
	}

	///<summary>Attach an existing database file to this instance. ESE databases cannot be shared between instances concurrently.</summary>
	static Database ^AttachDatabase(Session ^Session, String ^DatabaseName)
	{
		AttachOptions Opts;

		return AttachDatabase(Session, DatabaseName, Opts);
	}

	///<summary>Cleanly detaches databse. All EseObjects.Databse objects associated must be disposed first.</summary>
	static void DetachDatabase(Session ^Session, String ^DatabaseName)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(DatabaseName);

		EseException::RaiseOnError(JetDetachDatabase(Session->_JetSesid, NameChar));
	}

	///<summary>Detaches database with additional options. All EseObjects.Database objects associated must be disposed, unless forcing a close. Requires 5.1+.</summary>
	static void DetachDatabase(Session ^Session, String ^DatabaseName, DetachOptions ^Options)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(DatabaseName);
		ulong flags = DetachOptionsToBits(*Options);

		EseException::RaiseOnError(JetDetachDatabase2(Session->_JetSesid, NameChar, flags));
	}

	///<summary>Creates a new database file, attaches it to the instance and opens it.</summary>
	static Database ^Create(Session ^Session, CreateOptions CreateOptions)
	{
		marshal_context mc;
		char const *NameChar = mc.marshal_as<char const *>(CreateOptions.FileName);
		JET_GRBIT flags = CreateOptionsFlagsToBits(CreateOptions);
		JET_DBID NewJetDbid = null;
		
		EseException::RaiseOnError(JetCreateDatabase2(Session->_JetSesid, NameChar, CreateOptions.DatabaseSizeMax, &NewJetDbid, flags));

		return gcnew Database(Session, CreateOptions.FileName, NewJetDbid);
	}

	///<summary>Gets the session the Database object is associated with. Database objects are only valid for the session they are connected to.</summary>
	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _Session;}
	}

	property String ^DatabaseName
	{
		String ^get() {return _DatabaseName;}
	}
	
	/// <summary>
	/// Attempts to shrink and copy an unopened database file. This effect is more complete than defragmentation.
	/// Calls JetCompact. Requires 5.1+.
	/// </summary>
	/// <param name="SourceFile"></param>
	/// <param name="DestinationFile"></param>
	/// <param name="Repair">Also attempts repairs of a corrupted file.</param>
	/// <param name="DumpStats">Creates a dump file DFRGINFO.TXT of statistics during processing.</param>
	static void CompactOffline(EseObjects::Session ^Session, String ^SourceFile, String ^DestinationFile, bool Repair, bool DumpStats)
	{
		marshal_context mc;
		char const *Source = mc.marshal_as<char const *>(SourceFile);
		char const *Dest = mc.marshal_as<char const *>(DestinationFile);

		ulong flags = 0;
		flags |= Repair * JET_bitCompactRepair;
		flags |= DumpStats * JET_bitCompactStats;

		EseException::RaiseOnError(JetCompact(Session->_JetSesid, Source, Dest, null, null, flags));
		//NEXT: add version that also supports pfnStatus
	}

	/// <summary>
	/// Attempts to resize an unopened database file. Only expanding the size is supported currently with this function: use CompactDatabaseOffline to shrink the size.
	/// Calls JetSetDatabaseSize. Requires 5.1+.
	/// </summary>
	/// <param name="DatabaseFile"></param>
	/// <param name="DesiredSizeInPages"></param>
	/// <param name="ResultSizeInPages"></param>
	static void SetSizeOffline(EseObjects::Session ^Session, String ^DatabaseFile, ulong DesiredSizeInPages, ulong %ResultSizeInPages)
	{
		marshal_context mc;
		char const *Source = mc.marshal_as<char const *>(DatabaseFile);
		ulong ResultSizeCopy = 0;

		EseException::RaiseOnError(JetSetDatabaseSize(Session->_JetSesid, Source, DesiredSizeInPages, &ResultSizeCopy));

		ResultSizeInPages = ResultSizeCopy;
	}

	///<summary>A collection of all table names, system and user, in the database. Calls JetGetObjectInfo JET_ObjInfoNoStats.</summary>
	property array<String ^> ^TableNames
	{
		array<String ^> ^get()
		{
			JET_SESID sesid = _Session->_JetSesid;
			JET_OBJECTLIST jol = {sizeof jol};
			
			EseException::RaiseOnError(JetGetObjectInfo(sesid, _JetDbid, JET_objtypTable, NULL, NULL, &jol, sizeof jol, JET_ObjInfoListNoStats));

			try
			{
				array<String ^> ^Tables = gcnew array<String ^>(jol.cRecord);

				EseException::RaiseOnError(JetMove(sesid, jol.tableid, JET_MoveFirst, 0));

				for(ulong i = 0; i < jol.cRecord; i++)
				{
					char name[JET_cbNameMost+1];
					ulong buff_req;

					EseException::RaiseOnError(JetRetrieveColumn(sesid, jol.tableid, jol.columnidobjectname, name, sizeof name, &buff_req, 0, null));

					Tables[i] = astring_from_memblock(name, buff_req);

					JET_ERR status = JetMove(sesid, jol.tableid, JET_MoveNext, 0);

					if(status == JET_errNoCurrentRecord)
						break;

					EseException::RaiseOnError(status);
				}

				return Tables;
			}
			finally
			{
				JetCloseTable(sesid, jol.tableid);
			}
		}
	}

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _Bridge;};
		void set(EseObjects::Bridge ^bridge) {_Bridge = bridge;};
	};

	property IntPtr JetDbID
	{
		IntPtr get()
		{
			JET_DBID JetDbidCopy = _JetDbid;

			return marshal_as<IntPtr>(JetDbidCopy);
		}
	}
};