///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Storage.Attributes
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

using System;
using System.Runtime.Serialization;
using System.Reflection;
using EseObjects;

namespace EseLinq.Storage
{

	public interface IRecordBridge<T>
	{
		void Write(IWriteRecord wr, T o);
		T Read(IReadRecord rr);
	}

	///<summary>Provides an explicit way to control serialization to a record.
	///<pr/>Implementors are expected to also provide a public constructor with a single IReadRecord parameter for deserialization.
	///</summary>
	public interface IRecordSerializable
	{
		void WriteToRecord(IWriteRecord wr);
	}

	public sealed class RecordLevelExplicit<T> : IRecordBridge<T>
		where T : IRecordSerializable
	{
		ConstructorInfo con;

		public RecordLevelExplicit()
		{
			Type[] con_arg_tys = new Type[1];
			con_arg_tys[0] = typeof(IReadRecord);

			con = typeof(T).GetConstructor(con_arg_tys);

			if(con == null)
				throw new ArgumentException("Classes implementing IRecordSerializable must provide a public constructor with a single IReadRecord argument. This constructor was not found.");
		}

		///<summary>Writes a single record after querying metadata.
		///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
		///</summary>
		public void Write(IWriteRecord wr, T o)
		{
			o.WriteToRecord(wr);
		}

		///<summary>Reads a single record after querying metadata.
		///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
		///</summary>
		public T Read(IReadRecord rr)
		{
			object[] con_args = new object[1];
			con_args[0] = rr;

			return (T)(con.Invoke(con_args));
		}
	}


	///<summary>Enables use of automatic memberwise field level serilization to rows. Individual field serilization will use defaults unless overridden by other attributes.</summary>
	[AttributeUsageAttribute(AttributeTargets.Class | AttributeTargets.Struct)]
	public class MemberwiseStorageAttribute : Attribute
	{}

	///<summary>By default, only fields are saved or restored. Attaching this attribute to a property will cause it to be saved and restored like a field.</summary>
	[AttributeUsageAttribute(AttributeTargets.Property)]
	public class PersistentPropertyAttribute : Attribute
	{}

	///<summary>By default, all public fields are saved or restored. Attaching this attribute to a field will prevent it from being saved or restored.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field)]
	public class NonpersistentFieldAttribute : Attribute
	{}

	///<summary>Overrides name of the table. Defaults to the name of the member.</summary>
	[AttributeUsageAttribute(AttributeTargets.Class | AttributeTargets.Struct)]
	public class TableNameAttribute : Attribute
	{
		///<summary>Overrides name of the table. Defaults to the name of the member.</summary>
		public string Name;

		public TableNameAttribute(string Name)
		{
			this.Name = Name;
		}
	}

	///<summary>Overrides name of the column. Defaults to the name of the member.</summary>
	[AttributeUsageAttribute(AttributeTargets.Property | AttributeTargets.Field)]
	public class ColumnNameAttribute : Attribute
	{
		///<summary>Overrides name of the column. Defaults to the name of the member.</summary>
		public string Name;

		public ColumnNameAttribute(string Name)
		{
			this.Name = Name;
		}
	}

	///<summary>Can override various column properties when creating a new table to support storage of this class. Overriden by a EseTableCreateOptionsAttribute.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class ColumnCreateOptionsAttribute : Attribute
	{
		internal bool _NullabilitySpecified;
		internal bool _NotNull;

		public bool NotNull
		{
			get
			{
				return _NotNull;
			}
			set
			{
				_NullabilitySpecified = true;
				_NotNull = value;
			}
		}

		///<summary>Int form of Column.Type enum, overriding automatic detection of the best type to represent the column with.</summary>
		public int ColumnType;
		///<summary>Code page for string columns, either 1252 for English ASCII or 1200 for Unicode.</summary>
		public int CodePage;
		///<summary>Length limit for values. Defaults to no limit.</summary>
		public uint MaxLength;
		///<summary>Column has a fixed width, always using the same storage in a given row. Not compatible with long data types or tagged columns.</summary>
		public bool Fixed;
		///<summary>Column is tagged. Tagged columns do not take up space if null.</summary>
		public bool Tagged;
		///<summary>This column specifies the version of the row. Incremented with each update on the row. Must be a Long column. Not compatible with Tagged, AtoIncrement or EscrowUpdate. Can not be used with a temp table.</summary>
		public bool Version;
		///<summary>Column is automatically incremented each time a row could be added to the table. Not guaranteed to be contiguous. Must be type Long or Currency. Can not be used with a temp table.</summary>
		public bool Autoincrement;
		///<summary>For use only with temp tables. Make this field part of the temp table's key. Key order is the same as the column order.</summary>
		public bool MultiValued;
		///<summary>Allows concurrent update with EscrowUpdate. Must be type Long. Not compatible with Tagged, Version or Autoincrement. Can not be used with a temp table.</summary>
		public bool EscrowUpdate;
		///<summary>Column created without a version. Other transactions attempting to add a column with the same concurrently name will fail. Can not be used with a temp table.</summary>
		public bool Unversioned;
		///<summary>Deletes the row when the value in this escrow field reaches zero. Can not be used with a temp table.</summary>
		public bool DeleteOnZero;
	}

	///<summary>Directs EseObjects to expand the constituient fields of the field or property's type into the parent type. Subfields will be named Field,Member by default (override with EseColumnNameAttribute).</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class ExpandFieldAttribute : Attribute
	{}

	//TODO: should be expanded to support bridgable types, bridging of the collection itself
	//TODO: multivalue should be compatible with binary and xml serialization
	///<summary>Directs EseObjects to make the field multivalued and tagged. At present, field must be an array of objects.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class FieldMultivaluedAttribute : Attribute
	{}

	///<summary>Directs EseObjects to always use binary field serialization for this field instead of another strategy.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class BinaryFieldSerializationAttribute : Attribute
	{}

	///<summary>Directs EseObjects to always use XML field serialization for this field instead of another strategy.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class XmlFieldSerializationAttribute : Attribute
	{}

	///<summary>By default, classes use reference semantics when storing references. This overrides that behavior to use value semantics (with primary key definition) instead.</summary>
	[AttributeUsageAttribute(AttributeTargets.Field | AttributeTargets.Property)]
	public class UseValueSemanticsAttribute : Attribute
	{}

	///<summary>Creates a specific secondary index for the class. Overriden by a EseTableCreateOptionsAttribute.</summary>
	[AttributeUsageAttribute(AttributeTargets.Class | AttributeTargets.Struct, AllowMultiple = true)]
	public class IndexAttribute : Attribute
	{
		///<summary>Name of the index. See Jet naming restrictions.</summary>
		public string Name;
		///<summary>Selects and orders columns to include as key fields. Separate column names by dots (.). Each key name must be prefixed with + for ascending or - for descending sort.</summary>
		public string KeyColumns;
		///<summary>Initial storage density 20-100.</summary>
		public uint Density;

		///<summary>Locale ID</summary>
		public uint LCID;

		///<summary>Maximum length in bytes of each column to store for non-tuple indexes..</summary>
		public uint VarSegMac;

		///<summary>Maximum allowable size in bytes for all keys in the index. Requires 6.0+. Sets JET_bitIndexKeyMost if nonzero.</summary>
		public uint KeyMost;

		///<summary>Duplicate entries are not allowed.</summary>
		public bool Unique;
		///<summary>None of the columns which are indexed over can contain null.</summary>
		public bool DisallowNull;
		///<summary>Ignore rows with all null key values.</summary>
		public bool IgnoreAllNull;
		///<summary>Ignore rows with any null key values.</summary>
		public bool IgnoreAnyNull;
		///<summary>Ignore rows with null as the first key value.</summary>
		public bool IgnoreFirstNull;
		///<summary>Hint to flush indexing operations lazily. Does not affect data updates.</summary>
		public bool LazyFlush;
		///<summary>Hint that no rows would qualify to be included in the index, so a scan can be avoided and an empty index created.</summary>
		public bool Unversioned;
		///<summary>Null is sorted last (as opposed to first, which is the default behavior).</summary>
		public bool SortNullHigh;
		///<summary>Tuple index. Requires 5.1+</summary>
		public bool Tuples;
		///<summary>Cross product multivalued index. Requires 6.0+</summary>
		public bool CrossProduct;
		///<summary>Raise an error when an index key would be truncated. Requires 6.0+</summary>
		public bool DisallowTruncation;

		public IndexAttribute(string Name, string KeyColumns)
		{
			this.Name = Name;
			this.KeyColumns = KeyColumns;
		}
	}

	///<summary>Creates a specific primary index for the class. Primary indexes are special because they identify identity columns for value identity storage. Overriden by a EseTableCreateOptionsAttribute.</summary>
	[AttributeUsageAttribute(AttributeTargets.Class | AttributeTargets.Struct, AllowMultiple = false)]
	public class PrimaryIndexAttribute : IndexAttribute
	{
		public PrimaryIndexAttribute(string Name, string Columns) :
			base(Name, Columns)
		{
		}
	};

	///<summary>Must be a public static property or field of type EseObjects.Table.CreateOptions, used explicitly when creating a table for storing objects of the attached class.</summary>
	///<remarks>Presence of this attribute overrides any implicit table definition. Most of the functionality can be reached from other attributes.</remarks>
	[AttributeUsageAttribute(AttributeTargets.Property | AttributeTargets.Field)]
	public class TableCreateOptionsAttribute : Attribute
	{}
}