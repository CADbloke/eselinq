# Introduction #

EseLinq aims to provide a high level adapter for .NET to the Microsoft <a href='http://msdn.microsoft.com/en-us/library/ms684493%28EXCHG.10%29.aspx'>Extensible Storage Engine</a> a journaled, concurrent ISAM storage engine with indexing, sparse columns, native data types, long data and many other features. ESENT has been a documented native component of Windows since Windows 2000.

# Related Projects #

  * <a href='http://www.codeplex.com/ManagedEsent'>ManagedEsent</a>, a very direct managed wrapper for ESE within .NET with "Minimal editorialization". EseObjects provides a more abstract, higher level interface to ESE.

  * <a href='http://www.iiobo.com/ExtensibleStorageEngine.aspx'>iiobo ESE C#/C++ toolkit</a>, a collection of code samples.

# Status #

EseObjects is in a beta status. More testing is needed but the core feature set has been implemented. EseLinq, is inefficient (it always scans all source tables) but is fully functional by leveraging LINQ to Objects.

  * Any operations involving callbacks are not implemented. There does not seem to be a good way to do this in a multi instance environment.

  * Features introduced in Windows 7/Windows Server 2008 [R2](https://code.google.com/p/eselinq/source/detail?r=2) have not been implemented.