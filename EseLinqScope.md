## Basic Purpose ##

EseLinq is the top of a software stack that can connect .NET applications to the Microsoft Extensible Storage Engine (ESE). EseObjects provides the interface to the C API provided by the ESE library (esent.dll). All of the components run in a single process provided by the application.

| MyApp.exe | Application | User application |
|:----------|:------------|:-----------------|
| **EseLinq.dll** | **EseLinq** | **Adapters and LINQ support** |
| **EseObjects.dll** | **EseObjects** | **Managed interface to C API** |
| esent.dll | ESE         | Microsoft Extensible Storage Engine |
| kernel32.dll, etc | Win32       | Underlying OS    |
(bold items are part of this project)

EseLinq, using the underlying Extensible Storage Engine provided with Windows, can provide a table oriented database to .NET applications. Such a database could provide storage for a wide variety of applications.

ESE databases are durable (through journaling), supports multithreading (with isolated transactions), supports native storage of binary types, large values, indexing (including tuple indexes). See Microsoft's page on the [Extensible Storage Engine](http://msdn.microsoft.com/en-us/library/ms684493%28EXCHG.10%29.aspx) for more details.

## Data Structure ##

ESE can provide efficient storage to a schema that contains:
  * A small number of tables with many records each
  * A large number of tables
  * Tables with large numbers of optional columns (via tagged columns) that can also be indexed
  * Metadata operations are journaled; tables can be created and deleted on the fly as normal operations.

ESE is particularly useful for storing data that is not well suited to a traditional relational model: cases where lightly structured data would not benefit from the extra features of a SQL database. ESE can provide direct, lightweight in-process access to your data.

## Data Access ##

All data is access through cursors. The only query language provided is through LINQ (via EseLinq); there is no SQL support provided by any layer. An application that required a sophisticated runtime ad-hoc query system (i.e. something like SQL) would be ill-suited to EseLinq.

However, applications have a lot of control over how data is accessed by controlling how and when cursors scroll, seek and use indexes.

EseLinq also provides facilities to serialize .NET objects into rows.

## Scale ##

ESE can support databases in excess of 1 terabyte according to Microsoft's documentation. B+ tree indexes can provide efficient access to a variety of datasets. ESE can also be efficient on databases only a few megabytes in size. ESE has trouble creating a data environment without reserving a few MB though. The ESE documentation has more details on specific limits, but most are quite large.

The EseLinq stack does not provide any kind of replication or network access services; it is designed to work in a single process on a single system. An application would have to provide network or multisystem distribution semantics itself. For example, Microsoft Active Directory uses ESE for storing its data and provides the network and replication services itself.