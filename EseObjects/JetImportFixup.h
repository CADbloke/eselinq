//Prior to 6.0 (Vista), all ESE functions were without charset suffix as exported in esent.dll.
//When JET_VERSION is defined to something less than 0600, this is the way they're presented in the headers.
//When JET_VERSION is at least 0600, *A (or *W if JET_UNICODE is defined) versions are used (similar to other Win32 functions).
//Unfortunately, earlier verisons of esent.dll do not export *A and *W versions but have base name functions that all operate in ASCII.
//Due to naming limitations, I suspect that most ESE internals continue to operate in ASCII despite new wide character versions of interface elements.
//This does not affect unicode table columns, as these are always referenced via void pointer.
//To create a single binary that can work with multiple versions:
//-I need to import the suffixless versions of the functions whenever possible
//-and enable the new symobls defined in 6.0.
//To this end, JET_VERSION is defined to 0600 to enable all symbols but the legacy entry points are redefined to the suffixless names in this header.
//These suffixless symbols are exported by esent.dll to the exact same binary locations as the *A verions, so this change is safe.

#define JetInit3A JetInit3
#define JetCreateInstanceA JetCreateInstance
#define JetCreateInstance2A JetCreateInstance2
#define JetSetSystemParameterA JetSetSystemParameter
#define JetGetSystemParameterA JetGetSystemParameter
#define JetEnableMultiInstanceA JetEnableMultiInstance
#define JetBeginSessionA JetBeginSession
#define JetCreateDatabaseA JetCreateDatabase
#define JetCreateDatabase2A JetCreateDatabase2
#define JetAttachDatabaseA JetAttachDatabase
#define JetAttachDatabase2A JetAttachDatabase2
#define JetDetachDatabaseA JetDetachDatabase
#define JetDetachDatabase2A JetDetachDatabase2
#define JetGetObjectInfoA JetGetObjectInfo
#define JetGetTableInfoA JetGetTableInfo
#define JetCreateTableA JetCreateTable
#define JetCreateTableColumnIndexA JetCreateTableColumnIndex
#define JetCreateTableColumnIndex2A JetCreateTableColumnIndex2
#define JetDeleteTableA JetDeleteTable
#define JetRenameTableA JetRenameTable
#define JetGetTableColumnInfoA JetGetTableColumnInfo
#define JetGetColumnInfoA JetGetColumnInfo
#define JetAddColumnA JetAddColumn
#define JetDeleteColumnA JetDeleteColumn
#define JetDeleteColumn2A JetDeleteColumn2
#define JetRenameColumnA JetRenameColumn
#define JetSetColumnDefaultValueA JetSetColumnDefaultValue
#define JetGetTableIndexInfoA JetGetTableIndexInfo
#define JetGetIndexInfoA JetGetIndexInfo
#define JetCreateIndexA JetCreateIndex
#define JetCreateIndex2A JetCreateIndex2
#define JetDeleteIndexA JetDeleteIndex
#define JetGetDatabaseInfoA JetGetDatabaseInfo
#define JetGetDatabaseFileInfoA JetGetDatabaseFileInfo
#define JetOpenDatabaseA JetOpenDatabase
#define JetOpenTableA JetOpenTable
#define JetGetCurrentIndexA JetGetCurrentIndex
#define JetSetCurrentIndexA JetSetCurrentIndex
#define JetSetCurrentIndex2A JetSetCurrentIndex2
#define JetSetCurrentIndex3A JetSetCurrentIndex3
#define JetSetCurrentIndex4A JetSetCurrentIndex4
#define JetCompactA JetCompact
#define JetDefragmentA JetDefragment
#define JetDefragment2A JetDefragment2
#define JetDefragment3A JetDefragment3
#define JetSetDatabaseSizeA JetSetDatabaseSize
#define JetBackupA JetBackup
#define JetBackupInstanceA JetBackupInstance
#define JetRestoreA JetRestore
#define JetRestore2A JetRestore2
#define JetRestoreInstanceA JetRestoreInstance
#define JetGetAttachInfoA JetGetAttachInfo
#define JetGetAttachInfoInstanceA JetGetAttachInfoInstance
#define JetOpenFileA JetOpenFile
#define JetOpenFileInstanceA JetOpenFileInstance
#define JetGetLogInfoA JetGetLogInfo
#define JetGetLogInfoInstanceA JetGetLogInfoInstance
#define JetGetLogInfoInstance2A JetGetLogInfoInstance2
#define JetGetTruncateLogInfoInstanceA JetGetTruncateLogInfoInstance
#define JetExternalRestoreA JetExternalRestore
#define JetExternalRestore2A JetExternalRestore2
#define JetGetInstanceInfoA JetGetInstanceInfo
#define JetOSSnapshotFreezeA JetOSSnapshotFreeze
#define JetOSSnapshotGetFreezeInfoA JetOSSnapshotGetFreezeInfo
