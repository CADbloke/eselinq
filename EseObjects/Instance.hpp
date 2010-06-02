///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Index - Definition and properties of ESE instances
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

DemandLoadFunction<JET_ERR (JET_API *)(JET_INSTANCE* pinstance, const char* szInstanceName, const char* szDisplayName, JET_GRBIT grbit)> JetCreateInstance2_demand(L"esent.dll", "JetCreateInstance2");
DemandLoadFunction<JET_ERR (JET_API *)(JET_INSTANCE* pinstance, JET_GRBIT grbit)> JetInit2_demand(L"esent.dll", "JetInit2");

public ref class Instance
{
public:
	enum struct InstanceState
	{
		/// <summary>
		/// Use for setting/getting global options or for options that need to be set for initialization in a single instance environment.
		/// </summary>
		Global,

		///<summary>Instance has been created but not initialized. Use to set per instance options that must be set before initialization.</summary>
		Created,

		///<summary>Instance is ready for use.</summary>
		Initialized,

		///<summary>Instance has been terminated.</summary>
		Disposed
	};

internal:
	JET_INSTANCE _JetInstance;
	InstanceState _State;
	Bridge ^_Bridge;

public:
	initonly ushort VersionMajor;
	initonly ushort VersionMinor;
	initonly ushort VersionBuild1;
	initonly ushort VersionBuild2;

	/// <summary>
	/// Allocates a new instance object and retrieves version information. An ESE instance is not created with this function.
	/// Set parameters and use Create* and Init* to prepare the instance for use.
	/// </summary>
	Instance() :
		_JetInstance(null),
		_State(InstanceState::Global),
		VersionMajor(GetEseVersionMajor()),
		VersionMinor(GetEseVersionMinor()),
		VersionBuild1(GetEseVersionBuild1()),
		VersionBuild2(GetEseVersionBuild2()),
		_Bridge(GetDefaultBridge())
	{}

	~Instance()
	{
		this->!Instance();
	}

	!Instance()
	{
		switch(_State)
		{
		case InstanceState::Global:
		case InstanceState::Disposed:
			break;

		default:
			JetTerm(_JetInstance);
		}

		_JetInstance = null;
		_State = InstanceState::Disposed;
	}

	property IntPtr JetInstance
	{
		IntPtr get()
		{
			JET_INSTANCE JetInstanceCopy = _JetInstance;

			return marshal_as<IntPtr>(JetInstanceCopy);
		}
	}

	property InstanceState State
	{
		InstanceState get() {return _State;}
	}

	/// <summary>
	/// Similar to standard disposal, but allowing error handling. (Dispose methods shouldn't raise exceptions).
	/// </summary>
	void TerminateComplete()
	{
		EseException::RaiseOnError(JetTerm2(_JetInstance, JET_bitTermComplete));

		_JetInstance = null;
		_State = InstanceState::Disposed;
	}

	/// <summary>
	/// Shut down as quickly as possible. May result in a memory leak in the database.
	/// </summary>
	void TerminateAbrubt()
	{
		EseException::RaiseOnError(JetTerm2(_JetInstance, JET_bitTermAbrupt));

		_JetInstance = null;
		_State = InstanceState::Disposed;
	}

	/// <summary>
	/// Shut down as quickly as possible, even if a backup is in progress.
	/// </summary>
	void TerminateAbruptStopBackup()
	{
		EseException::RaiseOnError(JetTerm2(_JetInstance, JET_bitTermAbrupt | JET_bitTermStopBackup));

		_JetInstance = null;
		_State = InstanceState::Disposed;
	}

	/// <summary>
	/// Prepares to shut down all ESE activities in the process.
	/// </summary>
	void StopService()
	{
		EseException::RaiseOnError(JetStopService());
	}

	/// <summary>
	/// Initializes an instance global to the entire process. Compatible with 5.0, since that version only supports one instance per process.
	/// Use if you only need one instance for the process or are running 5.0.
	/// </summary>
	void InitGlobal()
	{
		if(_State > InstanceState::Global)
			throw gcnew InvalidOperationException(L"Object must be in Global state to call InitGlobal");

		JET_INSTANCE NewInstance = null;

		EseException::RaiseOnError(JetInit(&NewInstance));

		_JetInstance = NewInstance;
		_State = InstanceState::Initialized;
	}

	/// <summary>
	/// Creates an instance. Requires 5.1 or later. InstanceName must be unique within the process. DisplayName is optional.
	/// </summary>
	void Create(String ^InstanceName, String ^DisplayName)
	{
		if(_State > InstanceState::Global)
			throw gcnew InvalidOperationException(L"Object must be in Global state to call Create");

		JET_INSTANCE JetInstanceCopy = null;
		marshal_context mc;

		EseException::RaiseOnError(JetCreateInstance2_demand(&JetInstanceCopy, mc.marshal_as<const char*>(InstanceName), mc.marshal_as<const char*>(DisplayName), 0));

		_JetInstance = JetInstanceCopy;

		_State = InstanceState::Created;
	}

	/// <summary>
	/// Initializes an instance. Requires 5.1 or later.
	/// </summary>
	void Init()
	{
		JET_INSTANCE JetInstanceCopy = null;

		switch(_State)
		{
		case InstanceState::Global:
			throw gcnew InvalidOperationException(L"Object must be in Created state to call Init");

		case InstanceState::Initialized:
			throw gcnew InvalidOperationException(L"Instance already initialized.");

		case InstanceState::Created:
			JetInstanceCopy = _JetInstance;
			EseException::RaiseOnError(JetInit(&JetInstanceCopy));
			_JetInstance = JetInstanceCopy;
			break;
		}
	}

	/// <summary>
	/// Initializes (and creates if necessary) an instance, with options. Requires 5.1 or later.
	/// </summary>
	void Init(bool ReplayIgnoreMissingDB, bool RecoveryWithoutUndo, bool TruncateLogsAfterRecovery, bool ReplayMissingMapEntryDB)
	{
		JET_INSTANCE JetInstanceCopy = null;

		switch(_State)
		{
		case InstanceState::Global:
			throw gcnew InvalidOperationException(L"Object must be in Created state to call Init");

		case InstanceState::Initialized:
			throw gcnew InvalidOperationException(L"Instance already initialized.");

		case InstanceState::Created:
			JetInstanceCopy = _JetInstance;
			JET_GRBIT flags = 0;

			flags |= ReplayIgnoreMissingDB * JET_bitReplayIgnoreMissingDB;
			flags |= RecoveryWithoutUndo * JET_bitRecoveryWithoutUndo;
			flags |= TruncateLogsAfterRecovery * JET_bitTruncateLogsAfterRecovery;
			flags |= ReplayMissingMapEntryDB * JET_bitReplayMissingMapEntryDB;

			EseException::RaiseOnError(JetInit2_demand(&JetInstanceCopy, flags));
			_JetInstance = JetInstanceCopy;
			break;
		}
	}

	//NEXT:support JetInit3 with its extra recovery options. Needs callbacks

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _Bridge;};
		void set(EseObjects::Bridge ^bridge) {_Bridge = bridge;};
	};

//
// System Parameters
//

private:
	int64 GetLongSysParm(ulong ParmID)
	{
		JET_API_PTR r = 0;

		EseException::RaiseOnError(JetGetSystemParameter(_JetInstance, null, ParmID, &r, null, 0));

		return r;
	}

	void SetLongSysParm(ulong ParmID, int64 NewVal)
	{
		JET_INSTANCE JetInstanceCopy = _JetInstance;

		EseException::RaiseOnError(JetSetSystemParameter(&JetInstanceCopy, null, ParmID, static_cast<JET_API_PTR>(NewVal), null));
	}

	bool GetBoolSysParm(ulong ParmID)
	{
		return GetLongSysParm(ParmID) != 0 ? true : false;
	}

	void SetBoolSysParm(ulong ParmID, bool NewVal)
	{
		SetLongSysParm(ParmID, NewVal ? 1 : 0);
	}

	String ^GetStringSysParm(ulong ParmID)
	{
		//hope it's enough space
		//no reliable way to detect if the buffer was truncated from JetGetSytemParamter. See ESE docs.
		char buffer[0x1000]; 

		EseException::RaiseOnError(JetGetSystemParameter(_JetInstance, null, ParmID, null, buffer, 0x800));

		return marshal_as<String ^>(buffer);
	}

	void SetStringSysParm(ulong ParmID, String ^NewVal)
	{
		JET_INSTANCE JetInstanceCopy = _JetInstance;
		marshal_context mc;

		EseException::RaiseOnError(JetSetSystemParameter(&JetInstanceCopy, null, ParmID, null, mc.marshal_as<char const *>(NewVal)));
	}

public:
	//System options:

	//Database options
	property bool CheckFormatWhenOpenFail
	{
		bool get() {return GetBoolSysParm(JET_paramDbExtensionSize);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramDbExtensionSize, NewVal);}
	}

	property int64 DatabasePageSize
	{
		int64 get() {return GetLongSysParm(JET_paramDatabasePageSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramDatabasePageSize, NewVal);}
	}

	property int64 DbExtensionSize
	{
		int64 get() {return GetLongSysParm(JET_paramDatabasePageSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramDatabasePageSize, NewVal);}
	}

	property bool EnableIndexChecking
	{
		bool get() {return GetBoolSysParm(JET_paramEnableIndexChecking);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableIndexChecking, NewVal);}
	}

	property bool EnableIndexCleanup
	{
		bool get() {return GetBoolSysParm(JET_paramEnableIndexCleanup);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableIndexCleanup, NewVal);}
	}

	property bool OneDatabasePerSession
	{
		bool get() {return GetBoolSysParm(JET_paramOneDatabasePerSession);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramOneDatabasePerSession, NewVal);}
	}

	enum struct DefragOpts
	{
		Disable = JET_OnlineDefragDisable,
		Ver50All = JET_OnlineDefragAllOBSOLETE,
		Databases = JET_OnlineDefragDatabases,
		SpaceTrees = JET_OnlineDefragSpaceTrees,
		All = JET_OnlineDefragAll
	};

	property DefragOpts EnableOnlineDefrag
	{
		DefragOpts get() {return safe_cast<DefragOpts>(GetLongSysParm(JET_paramEnableOnlineDefrag));}
		void set(DefragOpts NewVal) {SetLongSysParm(JET_paramEnableOnlineDefrag, safe_cast<int64>(NewVal));}
	}

	property int64 PageFragment
	{
		int64 get() {return GetLongSysParm(JET_paramPageFragment);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramPageFragment, NewVal);}
	}

	property int64 RecordUpgradeDirtyLevel
	{
		int64 get() {return GetLongSysParm(JET_paramRecordUpgradeDirtyLevel);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramRecordUpgradeDirtyLevel, NewVal);}
	}

	//backup/restore options
	property String ^AlternateDatabaseRecoveryPath
	{
		String ^get() {return GetStringSysParm(JET_paramAlternateDatabaseRecoveryPath);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramAlternateDatabaseRecoveryPath, NewVal);}
	}

	property bool CleanupMismatchedLogFiles
	{
		bool get() {return GetBoolSysParm(JET_paramCleanupMismatchedLogFiles);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramCleanupMismatchedLogFiles, NewVal);}
	}

	property bool DeleteOutOfRangeLogs
	{
		bool get() {return GetBoolSysParm(JET_paramDeleteOutOfRangeLogs);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramDeleteOutOfRangeLogs, NewVal);}
	}

	property int64 OSSnapshotTimeout
	{
		int64 get() {return GetLongSysParm(JET_paramOSSnapshotTimeout);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramOSSnapshotTimeout, NewVal);}
	}

	property bool ZeroDatabaseDuringBackup
	{
		bool get() {return GetBoolSysParm(JET_paramZeroDatabaseDuringBackup);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramZeroDatabaseDuringBackup, NewVal);}
	}

	//database cache options
	property int64 BatchIOBufferMax
	{
		int64 get() {return GetLongSysParm(JET_paramBatchIOBufferMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramBatchIOBufferMax, NewVal);}
	}

	property int64 CacheSize
	{
		int64 get() {return GetLongSysParm(JET_paramCacheSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCacheSize, NewVal);}
	}

	property int64 CacheSizeMin
	{
		int64 get() {return GetLongSysParm(JET_paramCacheSizeMin);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCacheSizeMin, NewVal);}
	}

	property int64 CacheSizeMax
	{
		int64 get() {return GetLongSysParm(JET_paramCacheSizeMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCacheSizeMax, NewVal);}
	}

	property int64 CheckpointDepthMax
	{
		int64 get() {return GetLongSysParm(JET_paramCheckpointDepthMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCheckpointDepthMax, NewVal);}
	}

	property int64 CheckpointIOMax
	{
		int64 get() {return GetLongSysParm(JET_paramCheckpointIOMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCheckpointIOMax, NewVal);}
	}

	property bool EnableViewCache
	{
		bool get() {return GetBoolSysParm(JET_paramEnableViewCache);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableViewCache, NewVal);}
	}

	property int64 LRUKCorrInterval
	{
		int64 get() {return GetLongSysParm(JET_paramLRUKCorrInterval);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLRUKCorrInterval, NewVal);}
	}

	property int64 LRUKHistoryMax
	{
		int64 get() {return GetLongSysParm(JET_paramLRUKHistoryMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLRUKHistoryMax, NewVal);}
	}

	property int64 LRUKPolicy
	{
		int64 get() {return GetLongSysParm(JET_paramLRUKPolicy);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLRUKPolicy, NewVal);}
	}

	property int64 LRUKTimeout
	{
		int64 get() {return GetLongSysParm(JET_paramLRUKTimeout);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLRUKTimeout, NewVal);}
	}

	property int64 StartFlushThreshold
	{
		int64 get() {return GetLongSysParm(JET_paramStartFlushThreshold);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramStartFlushThreshold, NewVal);}
	}

	property int64 StopFlushThreshold
	{
		int64 get() {return GetLongSysParm(JET_paramStopFlushThreshold);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramStopFlushThreshold, NewVal);}
	}

	//error handling option
	property int64 ExceptionAction
	{
		int64 get() {return GetLongSysParm(JET_paramExceptionAction);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramExceptionAction, NewVal);}
	}

	//event log options
	property bool EventLogCache
	{
		bool get() {return GetBoolSysParm(JET_paramEventLogCache);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEventLogCache, NewVal);}
	}

	property int64 EventLoggingLevel
	{
		int64 get() {return GetLongSysParm(JET_paramEventLoggingLevel);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramEventLoggingLevel, NewVal);}
	}

	property String ^EventSource
	{
		String ^get() {return GetStringSysParm(JET_paramEventSource);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramEventSource, NewVal);}
	}

	property String ^EventSourceKey
	{
		String ^get() {return GetStringSysParm(JET_paramEventSourceKey);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramEventSourceKey, NewVal);}
	}

	property bool NoInformationEvent
	{
		bool get() {return GetBoolSysParm(JET_paramNoInformationEvent);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramNoInformationEvent, NewVal);}
	}

	///<summary>Period in ms that ESE will retry accessing a locked file before failing.</summary>
	property int64 AccessDeniedRetryPeriod
	{
		int64 get() {return GetLongSysParm(JET_paramAccessDeniedRetryPeriod);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramAccessDeniedRetryPeriod, NewVal);}
	}

	///<summary>If true, creates any missing directories in the path of a new file.</summary>
	property bool CreatePathIfNotExist
	{
		bool get() {return GetBoolSysParm(JET_paramCreatePathIfNotExist);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramCreatePathIfNotExist, NewVal);}
	}

	///<summary>Enables the use of the system cache as a secondary file cache. This may enable you to use a smaller ESE cache.</summary>
	property bool EnableFileCache
	{
		bool get() {return GetBoolSysParm(JET_paramEnableFileCache);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableFileCache, NewVal);}
	}

	///<summary>Sets IO priority of instance. 0 is normal, 1 is low. Requires Vista and 6.0+.</summary>
	property int64 IOPriority
	{
		int64 get() {return GetLongSysParm(JET_paramIOPriority);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIOPriority, NewVal);}
	}

	///<summary>Controls how many IO requests can be pending at one time.</summary>
	property int64 OutstandingIOMax
	{
		int64 get() {return GetLongSysParm(JET_paramOutstandingIOMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramOutstandingIOMax, NewVal);}
	}

	///<summary>Default tuple index increment. Requires 6.0+</summary>
	property int64 IndexTupleIncrement
	{
		int64 get() {return GetLongSysParm(JET_paramIndexTupleIncrement);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIndexTupleIncrement, NewVal);}
	}

	///<summary>Default tuple index start position. Requires 6.0+</summary>
	property int64 IndexTupleStart
	{
		int64 get() {return GetLongSysParm(JET_paramIndexTupleStart);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIndexTupleStart, NewVal);}
	}

	///<summary>Default tuple index maximum length. Requires 5.1+</summary>
	property int64 IndexTupleLengthMax
	{
		int64 get() {return GetLongSysParm(JET_paramIndexTuplesLengthMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIndexTuplesLengthMax, NewVal);}
	}

	///<summary>Default tuple index minimum length. Requires 5.1+</summary>
	property int64 IndexTupleLengthMin
	{
		int64 get() {return GetLongSysParm(JET_paramIndexTuplesLengthMin);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIndexTuplesLengthMin, NewVal);}
	}

	///<summary>Default tuple index max string lenght. Requres 5.1+</summary>
	property int64 IndexTuplesToIndexMax
	{
		int64 get() {return GetLongSysParm(JET_paramIndexTuplesToIndexMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramIndexTuplesToIndexMax, NewVal);}
	}

	void GetUnicodeIndexDefault([Out] ulong %lcid, [Out] UnicodeMapFlags %mapflags)
	{
		JET_UNICODEINDEX jui = {0}, *pjui = &jui;

		EseException::RaiseOnError(JetGetSystemParameter(_JetInstance, null, JET_paramUnicodeIndexDefault, reinterpret_cast<JET_API_PTR *>(&pjui), null, sizeof jui));

		lcid = jui.lcid;
		mapflags = UnicodeMapFlagsFromBits(jui.dwMapFlags);
	}

	void SetUnicodeIndexDefault(ulong lcid, UnicodeMapFlags mapflags)
	{
		JET_INSTANCE local_instance = _JetInstance;
		JET_UNICODEINDEX jui = {lcid, UnicodeMapFlagsToBits(mapflags)};

		EseException::RaiseOnError(JetSetSystemParameter(&local_instance, null, JET_paramUnicodeIndexDefault, reinterpret_cast<JET_API_PTR>(&jui), null));
	}

	///<summary>Read only parameter indicating maximum index key length for current page size. Requires 6.0+; value is always 255 in prior versions.</summary>
	property int64 KeyMost
	{
		int64 get() {return GetLongSysParm(JET_paramKeyMost);}
	}

	///<summary>Read only parameter indicating greatest column type supported</summary>
	property int64 MaxColtyp
	{
		int64 get() {return GetLongSysParm(JET_paramMaxColtyp);}
	}

	///<summary>Sets multiple system parameters as a package. 0-memory optimized, 1-legacy default. See JET_paramConfiguration. Requires 6.0+.</summary>
	property int64 Configuration
	{
		int64 get() {return GetLongSysParm(JET_paramConfiguration);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramConfiguration, NewVal);}
	}

	///<summary>Enables or disables the modification of various options. Designed for use with Configuration. See JET_paramEnableAdvanced. Requires 6.0+.</summary>
	property bool EnableAdvanced
	{
		bool get() {return GetBoolSysParm(JET_paramEnableAdvanced);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableAdvanced, NewVal);}
	}

	///<summary>The number of B+ tree resources to cache after they've been closed. Useful to increase with a large number of tables. Requires 6.0+.</summary>
	property int64 CacheClosedTables
	{
		int64 get() {return GetLongSysParm(JET_paramCachedClosedTables);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCachedClosedTables, NewVal);}
	}

	///<summary>Prevents the engine from publishing perfmon entries. Can reduce overhead. Requires 6.0+.</summary>
	property int64 DisablePerfmon
	{
		int64 get() {return GetLongSysParm(JET_paramDisablePerfmon);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramDisablePerfmon, NewVal);}
	}

	///<summary>Pre-allocate version store pages in a global pool in single instance mode. Requires 5.1+..</summary>
	property int64 GlobalMinVerPages
	{
		int64 get() {return GetLongSysParm(JET_paramGlobalMinVerPages);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramGlobalMinVerPages, NewVal);}
	}

	///<summary>Maximum cursors that can be opened. Requires EseObjects.Table and Cursor each use a single cursor resource.</summary>
	property int64 MaxCursors
	{
		int64 get() {return GetLongSysParm(JET_paramMaxCursors);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxCursors, NewVal);}
	}

	///<summary>Maximum instances that can be opened. Requires 5.1+.</summary>
	property int64 MaxInstances
	{
		int64 get() {return GetLongSysParm(JET_paramMaxInstances);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxInstances, NewVal);}
	}

	///<summary>Maximum number of discrete B+ tree resources that can be opened at once. In general, two resources for each table plus one for each secondary index is required to open all objects in a database.</summary>
	property int64 MaxOpenTables
	{
		int64 get() {return GetLongSysParm(JET_paramMaxOpenTables);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxOpenTables, NewVal);}
	}

	///<summary>Maximum number of sessions that can be opened at once.</summary>
	property int64 MaxSessions
	{
		int64 get() {return GetLongSysParm(JET_paramMaxSessions);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxSessions, NewVal);}
	}

	///<summary>Maximum number of temp tables that can be created. Some EseObjects operations require temp tables to query information. Zero in 5.1+ disables the temp database.</summary>
	property int64 MaxTemporaryTables
	{
		int64 get() {return GetLongSysParm(JET_paramMaxTemporaryTables);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxTemporaryTables, NewVal);}
	}

	///<summary>
	///Version store pages used during updates in a transaction, affecting maximum size of a transactional update.
	///The most commonly exhausted resource. Increasing this value can avoid JET_errVersionStoreOutOfMemory exceptions.
	///In 6.0+ the version store page size can be changed via VerPageSize. Prior versions always use 16k.
	///</summary>
	property int64 MaxVerPages
	{
		int64 get() {return GetLongSysParm(JET_paramMaxVerPages);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramMaxVerPages, NewVal);}
	}

	///<summary>Size in bytes of a special B+ tree lookup cache.</summary>
	property int64 PageHintCacheSize
	{
		int64 get() {return GetLongSysParm(JET_paramPageHintCacheSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramPageHintCacheSize, NewVal);}
	}

	///<summary>Number of open B+ trees the engine tries to stay below. Obsolete in 6.0+.</summary>
	property int64 PreferredMaxOpenTables
	{
		int64 get() {return GetLongSysParm(JET_paramPreferredMaxOpenTables);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramPreferredMaxOpenTables, NewVal);}
	}

	///<summary>Preferred number of version store pages to use, reserving the remainder for internal operations. Zero sets the value to 90% of the limit. Obsolete in 6.0+.</summary>
	property int64 PreferredVerPages
	{
		int64 get() {return GetLongSysParm(JET_paramPreferredVerPages);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramPreferredVerPages, NewVal);}
	}

	///<summary>Size of version store pages, a power of 2 between 1024 and 65536. Requires 6.0+.</summary>
	property int64 VerPageSize
	{
		int64 get() {return GetLongSysParm(JET_paramVerPageSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramVerPageSize, NewVal);}
	}

	///<summary>Maximum background cleanup tasks that can be queued. Requires 5.1+.</summary>
	property int64 VersionStoreTaskQueueMax
	{
		int64 get() {return GetLongSysParm(JET_paramVersionStoreTaskQueueMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramVersionStoreTaskQueueMax, NewVal);}
	}

	//temp table options

	///<summary>Increases performance of temp tables at the cost of not being able to roll back changes to temp tables.</summary>
	property bool EnableTempTableVersioning
	{
		bool get() {return GetBoolSysParm(JET_paramEnableTempTableVersioning);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramEnableTempTableVersioning, NewVal);}
	}

	///<summary>Initial size of temp database in pages. 14 is the smallest possible size.</summary>
	property int64 PageTempDBMin
	{
		int64 get() {return GetLongSysParm(JET_paramPageTempDBMin);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramPageTempDBMin, NewVal);}
	}

	///<summary>Relative or absolute path to temp folder or file. If using a path, it must end in backslash.</summary>
	property String ^TempPath
	{
		String ^get() {return GetStringSysParm(JET_paramTempPath);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramTempPath, NewVal);}
	}

	//transaction log options

	///<summary>Three letter prefix used for engine files. Default is edb.</summary>
	property String ^BaseName
	{
		String ^get() {return GetStringSysParm(JET_paramBaseName);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramBaseName, NewVal);}
	}

	///<summary>Enables circular logging.</summary>
	property bool CircularLog
	{
		bool get() {return GetBoolSysParm(JET_paramCircularLog);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramCircularLog, NewVal);}
	}

	///<summary>Default commit mode. 0 is normal, 1 is lazy flush, 2 is wait for last level 0 commit, 8 is wait for all level 0 commits.</summary>
	property int64 CommitDefault
	{
		int64 get() {return GetLongSysParm(JET_paramCommitDefault);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramCommitDefault, NewVal);}
	}

	///<summary>Deletes older versions of log files. Should not be used if the older database is in an inconsitent state in 5.0.</summary>
	property bool DeleteOldLogs
	{
		bool get() {return GetBoolSysParm(JET_paramDeleteOldLogs);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramDeleteOldLogs, NewVal);}
	}

	///<summary>Ignores log version. Obsolete in 5.1+</summary>
	property bool IgnoreLogVersion
	{
		bool get() {return GetBoolSysParm(JET_paramIgnoreLogVersion);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramIgnoreLogVersion, NewVal);}
	}

	///<summary>Uses old filenames for database objects. Requires 6.0+.</summary>
	property bool LegacyFileNames
	{
		bool get() {return GetBoolSysParm(JET_paramLegacyFileNames);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramLegacyFileNames, NewVal);}
	}

	///<summary>The amount of memory (in disk sectors, usually 512 bytes) used as a write cache for the transaction log. May need to be increased for high load situations.</summary>
	property int64 LogBuffers
	{
		int64 get() {return GetLongSysParm(JET_paramLogBuffers);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLogBuffers, NewVal);}
	}

	///<summary>Creates a check point when the speficied number of log secotrs have been generated. Obsolete in 5.1+</summary>
	property int64 LogCheckpointPeriod
	{
		int64 get() {return GetLongSysParm(JET_paramLogCheckpointPeriod);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLogCheckpointPeriod, NewVal);}
	}

	///<summary>Creates additional log files asynchronously to minimize switching time. Requires 5.1+.</summary>
	property bool LogFileCreateAsynch
	{
		bool get() {return GetBoolSysParm(JET_paramLogFileCreateAsynch);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramLogFileCreateAsynch, NewVal);}
	}

	///<summary>Relative or absolute path to log file directory. Path must end in backslash.</summary>
	property String ^LogFilePath
	{
		String ^get() {return GetStringSysParm(JET_paramLogFilePath);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramLogFilePath, NewVal);}
	}

	///<summary>Fixed size of transaction log files in 1024 byte units.</summary>
	property int64 LogFileSize
	{
		int64 get() {return GetLongSysParm(JET_paramLogFileSize);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLogFileSize, NewVal);}
	}

	///<summary>Wait for this many sessions to be waiting to flush to disk prior to forcing the flush. Obsolete in 5.1+.</summary>
	property int64 LogWaitingUserMax
	{
		int64 get() {return GetLongSysParm(JET_paramLogWaitingUserMax);}
		void set(int64 NewVal) {SetLongSysParm(JET_paramLogWaitingUserMax, NewVal);}
	}

	///<summary>Controls log based ARIES style crash recovery for attached databases.
	///Disabling the option will increase performance, but make recovery from a crash unlikely.
	///Not recommended unless database contents are not useful after a crash.
	///Must be set to "On" for enabled or "Off" for disabled.
	///</summary>
	property String ^Recovery
	{
		String ^get() {return GetStringSysParm(JET_paramRecovery);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramRecovery, NewVal);}
	}

	///<summary>Relative or absolute path to directory containing checkpoint file (normally edb.chk). Path must end in backslash.</summary>
	property String ^SystemPath
	{
		String ^get() {return GetStringSysParm(JET_paramSystemPath);}
		void set(String ^NewVal) {SetStringSysParm(JET_paramSystemPath, NewVal);}
	}

	///<summary>Tries to coalesce disk flushes by waiting this amount of time prior to forcing a flush. Obsolete in 5.1+.</summary>
	property bool WaitLogFlush
	{
		bool get() {return GetBoolSysParm(JET_paramWaitLogFlush);}
		void set(bool NewVal) {SetBoolSysParm(JET_paramWaitLogFlush, NewVal);}
	}

	///<summary>Enables multi instance mode. Requires 5.1+. Calls JetEnableMultiInstance with no parameters (use individual properties).</summary>
	///<returns>True iff the database was already in multi instance mode</returns>
	bool EnableMultInstanceMode()
	{
		JET_ERR status = JetEnableMultiInstance(null, 0, null);

		return status == JET_errSystemParamsAlreadySet;
			return true;

		EseException::RaiseOnError(status);

		return false;
	}
};