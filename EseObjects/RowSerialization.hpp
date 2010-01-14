///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  RowSerialization - Serialization support with rows
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

namespace Serialization
{
using namespace System::Runtime::Serialization;
using namespace System::Reflection;

generic <class T> public interface struct IRecordLevelBridge
{
	void Write(IWriteRecord ^wr, T o);
	T Read(IReadRecord ^rr);
};

///<summary>Running context to save and read members of a partiuclar type to a particular table using member name to column name correleation.
///<pr/>An instance must only be used with the type and table it was constructed with.</summary>
generic <class T> public ref class RecordLevelMemberiwse sealed : IRecordLevelBridge<T>
{
	StreamingContext ^context;
	array<MemberInfo ^> ^meta;
	array<Type ^> ^field_types;
	array<Column ^> ^cols;

public:
	///<summary>Builds a new object, using table information from the specified IWriteRecord.</summary>
	RecordLevelMemberiwse(IWriteRecord ^wr) :
		context(gcnew StreamingContext(StreamingContextStates::Persistence)),
		meta(FormatterServices::GetSerializableMembers(T::typeid)),
		field_types(gcnew array<Type ^>(meta->Length)),
		cols(gcnew array<Column ^>(meta->Length))
	{
		Table ^table = wr->Read->AsTable;

		for(long i = 0; i < meta->Length; i++)
			cols[i] = gcnew Column(table, meta[i]->Name);

		for(long i = 0; i < meta->Length; i++)
			field_types[i] = safe_cast<FieldInfo ^>(meta[i])->FieldType;
	}

	///<summary>Builds a new object, using table information from the specified table.</summary>
	RecordLevelMemberiwse(Table ^table) :
		context(gcnew StreamingContext(StreamingContextStates::Persistence)),
		meta(FormatterServices::GetSerializableMembers(T::typeid)),
		field_types(gcnew array<Type ^>(meta->Length)),
		cols(gcnew array<Column ^>(meta->Length))
	{
		for(long i = 0; i < meta->Length; i++)
			cols[i] = gcnew Column(table, meta[i]->Name);

		for(long i = 0; i < meta->Length; i++)
			field_types[i] = safe_cast<FieldInfo ^>(meta[i])->FieldType;
	}

	///<summary>Writes a single record using metadata associated with the object.</summary>
	virtual void Write(IWriteRecord ^wr, T o)
	{
		array<Object ^> ^data = FormatterServices::GetObjectData(o, meta);

		for(long i = 0; i < meta->Length; i++)
			wr->Set(cols[i], data[i]);
	}

	///<summary>Reads a single record using metadata associated with the object.</summary>
	virtual T Read(IReadRecord ^rr)
	{
		array<Object ^> ^data = gcnew array<Object ^>(meta->Length);

		for(long i = 0 ; i < meta->Length; i++)
			data[i] = rr->Retrieve(cols[i], field_types[i]);

		Object ^obj = FormatterServices::GetUninitializedObject(T::typeid);
		return safe_cast<T>(FormatterServices::PopulateObjectMembers(obj, meta, data));
	}

	///<summary>Writes a single record after querying metadata.
	///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
	///</summary>
	static void WriteSingle(IWriteRecord ^wr, T o)
	{
		RecordLevelMemberiwse rlm(wr);
		rlm.Write(wr, o);
	}

	///<summary>Reads a single record after querying metadata.
	///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
	///</summary>
	static T ReadSingle(IReadRecord ^rr)
	{
		RecordLevelMemberiwse rlm(rr->AsTable);
		return rlm.Read(rr);
	}
};

///<summary>Provides an explicit way to control serialization to a record.
///<pr/>Implementors are expected to also provide a public constructor with a single IReadRecord parameter for deserialization.
///</summary>
public interface struct IRecordSerializable
{
	void WriteToRecord(IWriteRecord ^wr);
};

generic <class T>
	where T: IRecordSerializable
public ref class RecordLevelExplicit sealed : IRecordLevelBridge<T>
{
	ConstructorInfo ^con;

public:
	RecordLevelExplicit()
	{
		array<Type ^> ^con_arg_tys = gcnew array<Type ^>(1);
		con_arg_tys[0] = IReadRecord::typeid;

		con = T::typeid->GetConstructor(con_arg_tys);

		if(con == nullptr)
			throw gcnew ArgumentException("Classes implementing IRecordSerializable must provide a public constructor with a single IReadRecord argument. This constructor was not found.");
	}

	///<summary>Writes a single record after querying metadata.
	///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
	///</summary>
	virtual void Write(IWriteRecord ^wr, T o)
	{
		o->WriteToRecord(wr);
	}

	///<summary>Reads a single record after querying metadata.
	///<pr/>For multiple records, it is more efficient to create a single object and use the Write method, as it avoids re-querying metadata and finding columns each time.
	///</summary>
	virtual T Read(IReadRecord ^rr)
	{
		array<Object ^> ^con_args = gcnew array<Object ^>(1);
		con_args[0] = rr;

		return safe_cast<T>(con->Invoke(con_args));
	}
};

///<summary>Enables use of automatic memberwise field level serilization to rows. Individual field serilization will use defaults unless overridden by other attributes.</summary>
[AttributeUsageAttribute(AttributeTargets::Class | AttributeTargets::Struct)]
public ref struct EseSerialMemberwiseAttribute : Attribute
{};

///<summary>By default, only fields are saved or restored. Attaching this attribute to a property will cause it to be saved and restored like a field.</summary>
[AttributeUsageAttribute(AttributeTargets::Property)]
public ref struct EsePersistentPropertyAttribute : Attribute
{};

///<summary>By default, all public fields are saved or restored. Attaching this attribute to a field will prevent it from being saved or restored.</summary>
[AttributeUsageAttribute(AttributeTargets::Field)]
public ref struct EseNonpersistentFieldAttribute : Attribute
{};

///<summary>Overrides name of the column. Defaults to the name of the member.</summary>
[AttributeUsageAttribute(AttributeTargets::Property | AttributeTargets::Field)]
public ref struct EseColumnNameAttribute : Attribute
{
	///<summary>Overrides name of the column. Defaults to the name of the member.</summary>
	property String ^Name;

	EseColumnNameAttribute(String ^Name)
	{
		this->Name = Name;
	}
};

///<summary>Can override various column properties when creating a new table to support storage of this class. Overriden by a EseTableCreateOptionsAttribute.</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref class EseColumnOptionsAttribute : Attribute
{
	bool _NullabilitySpecified;
	bool _NotNull;

public:
	property bool NotNull
	{
		bool get() {return _NotNull;}
		void set(bool nv) {_NullabilitySpecified = true, _NotNull = nv;}
	}

	///<summary>Int form of Column::Type enum, overriding automatic detection of the best type to represent the column with.</summary>
	property int ColumnType;
	///<summary>Code page for string columns, either 1252 for English ASCII or 1200 for Unicode.</summary>
	property int CodePage;
	///<summary>Length limit for values. Defaults to no limit.</summary>
	property int MaxLength;
	///<summary>Column has a fixed width, always using the same storage in a given row. Not compatible with long data types or tagged columns.</summary>
	property bool Fixed;
	///<summary>Column is tagged. Tagged columns do not take up space if null.</summary>
	property bool Tagged;
	///<summary>This column specifies the version of the row. Incremented with each update on the row. Must be a Long column. Not compatible with Tagged, AtoIncrement or EscrowUpdate. Can not be used with a temp table.</summary>
	property bool Version;
	///<summary>Column is automatically incremented each time a row could be added to the table. Not guaranteed to be contiguous. Must be type Long or Currency. Can not be used with a temp table.</summary>
	property bool Autoincrement;
	///<summary>For use only with temp tables. Make this field part of the temp table's key. Key order is the same as the column order.</summary>
	property bool TTKey;
	///<summary>For use only with temp tables. Make key sort order for this column descending.</summary>
	property bool TTDescending;
	///<summary>Allows a tagged column to contain multiple instances of the same field in a singlw row.</summary>
	property bool MultiValued;
	///<summary>Allows concurrent update with EscrowUpdate. Must be type Long. Not compatible with Tagged, Version or Autoincrement. Can not be used with a temp table.</summary>
	property bool EscrowUpdate;
	///<summary>Column created without a version. Other transactions attempting to add a column with the same concurrently name will fail. Can not be used with a temp table.</summary>
	property bool Unversioned;
	///<summary>Deletes the row when the value in this escrow field reaches zero. Can not be used with a temp table.</summary>
	property bool DeleteOnZero;
};

///<summary>Directs EseObjects to expand the constituient fields of the field or property's type into the parent type. Subfields will be named Field,Member by default (override with EseColumnNameAttribute).</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref struct EseExpandField : Attribute
{};

//TODO: should be expanded to support bridgable types, bridging of the collection itself
//TODO: multivalue should be compatible with binary and xml serialization
///<summary>Directs EseObjects to make the field multivalued and tagged. At present, field must be an array of objects.</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref struct EseFieldMultivalued : Attribute
{};

///<summary>Directs EseObjects to always use binary field serialization for this field instead of another strategy.</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref struct EseBinaryFieldSerialization : Attribute
{};

///<summary>Directs EseObjects to always use XML field serialization for this field instead of another strategy.</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref struct EseXmlFieldSerialization : Attribute
{};

///<summary>By default, ref classes use reference semantics when storing references. This overrides that behavior to use value semantics (with primary key definition) instead.</summary>
[AttributeUsageAttribute(AttributeTargets::Field | AttributeTargets::Property)]
public ref struct EseUseValueSemantics : Attribute
{};

///<summary>Creates a specific secondary index for the class. Overriden by a EseTableCreateOptionsAttribute.</summary>
[AttributeUsageAttribute(AttributeTargets::Class | AttributeTargets::Struct, AllowMultiple = true)]
public ref struct EseIndexAttribute : Attribute
{
	///<summary>Name of the index. See Jet naming restrictions.</summary>
	String ^Name;
	///<summary>Selects and orders columns to include as key fields. Separate column names by dots (.). Each key name must be prefixed with + for ascending or - for descending sort.</summary>
	property String ^Columns;
	///<summary>Initial storage density 20-100.</summary>
	property ulong Density;

	///<summary>Locale ID</summary>
	property ulong LCID;
	
	///<summary>Maximum length in bytes of each column to store for non-tuple indexes..</summary>
	property ulong VarSegMac;
		
	///<summary>Maximum allowable size in bytes for all keys in the index. Requires 6.0+. Sets JET_bitIndexKeyMost if nonzero.</summary>
	property ulong KeyMost;

	///<summary>Duplicate entries are not allowed.</summary>
	property bool Unique;
	///<summary>None of the columns which are indexed over can contain null.</summary>
	property bool DisallowNull;
	///<summary>Ignore rows with all null key values.</summary>
	property bool IgnoreAllNull;
	///<summary>Ignore rows with any null key values.</summary>
	property bool IgnoreAnyNull;
	///<summary>Ignore rows with null as the first key value.</summary>
	property bool IgnoreFirstNull;
	///<summary>Hint to flush indexing operations lazily. Does not affect data updates.</summary>
	property bool LazyFlush;
	///<summary>Hint that no rows would qualify to be included in the index, so a scan can be avoided and an empty index created.</summary>
	property bool IndexEmpty;
	///<summary>Creatinon of the index visible to other transactions; attempting to create an index with the same name unversioned in multiple concurent transactions will fail.</summary>
	property bool Unversioned;
	///<summary>Null is sorted last (as opposed to first, which is the default behavior).</summary>
	property bool SortNullHigh;
	///<summary>Tuple index. Requires 5.1+</summary>
	property bool Tuples;
	///<summary>Cross product multivalued index. Requires 6.0+</summary>
	property bool CrossProduct;
	///<summary>Raise an error when an index key would be truncated. Requires 6.0+</summary>
	property bool DisallowTruncation;

	EseIndexAttribute(String ^Name, String ^Columns)
	{
		this->Name = Name;
		this->Columns = Columns;
	}
};

///<summary>Creates a specific primary index for the class. Primary indexes are special because they identify identity columns for value identity storage. Overriden by a EseTableCreateOptionsAttribute.</summary>
[AttributeUsageAttribute(AttributeTargets::Class | AttributeTargets::Struct, AllowMultiple = false)]
public ref struct EsePrimaryIndexAttribute : EseIndexAttribute
{
	EsePrimaryIndexAttribute(String ^Name, String ^Columns) :
		EseIndexAttribute(Name, Columns)
	{}
};

///<summary>Must be a public static property or field of type EseObjects.Table.CreateOptions, used explicitly when creating a table for storing objects of the attached class.</summary>
///<remarks>Presence of this attribute overrides any implicit table definition. Most of the functionality can be reached from other attributes.</remarks>
[AttributeUsageAttribute(AttributeTargets::Property | AttributeTargets::Field)]
public ref struct EseTableCreateOptionsAttribute : Attribute
{};


//base linkage to .NET member
ref struct MemberLink abstract
{
	Type ^MemberType;

	MemberLink(Type ^MemberType) :
		MemberType(MemberType)
	{}

	virtual Object ^Get(Object ^obj) = 0;
	virtual void Set(Object ^obj, Object ^value) = 0;
};

//linkage to a field member
ref struct FieldLink sealed : MemberLink
{
	FieldInfo ^Fi;

	FieldLink(FieldInfo ^Fi) :
		MemberLink(Fi->FieldType),
		Fi(Fi)
	{}

	virtual Object ^Get(Object ^obj) override
	{
		return Fi->GetValue(obj);
	}
	virtual void Set(Object ^obj, Object ^value) override
	{
		Fi->SetValue(obj, value);
	}
};

//linkage to a property member
ref struct PropertyLink sealed : MemberLink
{
	MethodInfo ^GetMi;
	MethodInfo ^SetMi;

	PropertyLink(PropertyInfo ^Pi) :
		MemberLink(Pi->PropertyType),
		GetMi(Pi->GetGetMethod()),
		SetMi(Pi->GetSetMethod())
	{}

	virtual Object ^Get(Object ^obj) override
	{
		return GetMi->Invoke(obj, nullptr);
	}
	virtual void Set(Object ^obj, Object ^value) override
	{
		array<Object ^> ^args = gcnew array<Object ^>(1);
		args[0] = value;
		SetMi->Invoke(obj, args);
	}		
};

//linkage to a ESE column. Defaults to a direct (bridged) linkage
ref struct ColumnLink
{
	MemberLink ^Ml;
	Column ^Col;

	ColumnLink(MemberLink ^Ml, Column ^Col) :
		Ml(Ml),
		Col(Col)
	{}

	virtual void SaveField(Object ^obj, IWriteRecord ^wr)
	{
		wr->Set(Col, Ml->Get(obj));
	}
	virtual void LoadField(Object ^obj, IReadRecord ^rr)
	{
		Ml->Set(obj, rr->Retrieve(Col, Ml->MemberType));
	}
};

ref struct BinaryColumnLink sealed : ColumnLink
{
	Formatters::Binary::BinaryFormatter ^formatter;

	BinaryColumnLink(MemberLink ^Ml, Column ^Col) :
		ColumnLink(Ml, Col),
		formatter(gcnew Formatters::Binary::BinaryFormatter())
	{}

	virtual void SaveField(Object ^obj, IWriteRecord ^wr) override
	{
		System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream();
		formatter->Serialize(stream, Ml->Get(obj));
		
		wr->Set(Col, stream->ToArray());
	}
	virtual void LoadField(Object ^obj, IReadRecord ^rr) override
	{
		System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream(rr->Retrieve<array<uchar> ^>(Col));

		Ml->Set(obj, formatter->Deserialize(stream));
	}
};

ref struct XmlColumnLink sealed : ColumnLink
{
	System::Xml::Serialization::XmlSerializer ^xmls;

	XmlColumnLink(MemberLink ^Ml, Column ^Col) :
		ColumnLink(Ml, Col),
		xmls((gcnew System::Xml::Serialization::XmlSerializerFactory())->CreateSerializer(Ml->MemberType))
	{}

	virtual void SaveField(Object ^obj, IWriteRecord ^wr) override
	{
		System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream();
		xmls->Serialize(stream, Ml->Get(obj));

		wr->Set(Col, Text::Encoding::UTF8->GetString(stream->ToArray(), 0, static_cast<long>(stream->Position)));
	}
	virtual void LoadField(Object ^obj, IReadRecord ^rr) override
	{
		String ^string = rr->Retrieve<String ^>(Col);
		array<uchar> ^arr = Text::Encoding::UTF8->GetBytes(string);
		System::IO::MemoryStream ^stream = gcnew System::IO::MemoryStream(arr);

		Ml->Set(obj, xmls->Deserialize(stream));
	}
};

array<ColumnLink ^> ^LoadLinks(Table ^table, Type ^ty, String ^Prefix);

ref struct ExpandedColumnLink sealed : ColumnLink
{
	array<ColumnLink ^> ^Links;

	ExpandedColumnLink(MemberLink ^Ml, String ^ColName, Table ^table) :
		ColumnLink(Ml, Col),
		Links(LoadLinks(table, Ml->MemberType, ColName + ","))
	{}

	virtual void SaveField(Object ^obj, IWriteRecord ^wr) override
	{
		Object ^member_obj = Ml->Get(obj);

		for each(ColumnLink ^l in Links)
			l->SaveField(member_obj, wr);
	}

	virtual void LoadField(Object ^obj, IReadRecord ^rr) override
	{
		Object ^member_obj = Ml->Get(obj);

		for each(ColumnLink ^l in Links)
			l->LoadField(member_obj, rr);

		Ml->Set(obj, member_obj); //set back in case the member is a value type
	}
};

ref struct MultivaluedColumnLink sealed : ColumnLink
{
	IWriteRecord::SetOptions so;
	
	MultivaluedColumnLink(MemberLink ^Ml, Column ^Col, Table ^table) :
		ColumnLink(Ml, Col)
	{
		so.TagSequence = 0; //0 for append. Sets will always append values
	}

	virtual void SaveField(Object ^obj, IWriteRecord ^wr) override
	{
		array<Object ^> ^arr = safe_cast<array<Object ^> ^>(obj);

		//append all values
		for each(Object ^o in arr)
			wr->Set(Col, o, so);
	}

	virtual void LoadField(Object ^obj, IReadRecord ^rr) override
	{
		Ml->Set(obj, rr->RetrieveAllValues<Object ^>(Col)); //set back in case the member is a value type
	}
};

ColumnLink ^MakeColumnLink(Table ^table, MemberInfo ^mi, MemberLink ^ml, String ^Prefix)
{
	Attribute ^ColNameAtt = Attribute::GetCustomAttribute(mi, EseColumnNameAttribute::typeid);
	String ^ColName;

	ColName = ColNameAtt ?
		safe_cast<EseColumnNameAttribute ^>(ColNameAtt)->Name :
		String::Concat(Prefix, mi->Name);

	if(Attribute::GetCustomAttribute(mi, EseExpandField::typeid))
		return gcnew ExpandedColumnLink(ml, ColName, table);

	Column ^Col = gcnew Column(table, ColName);

	if(Attribute::GetCustomAttribute(mi, EseFieldMultivalued::typeid))
		return gcnew MultivaluedColumnLink(ml, Col, table);

	if(Attribute::GetCustomAttribute(mi, EseBinaryFieldSerialization::typeid))
		return gcnew BinaryColumnLink(ml, Col);

	if(Attribute::GetCustomAttribute(mi, EseXmlFieldSerialization::typeid))
		return gcnew XmlColumnLink(ml, Col);

	return gcnew ColumnLink(ml, Col);
}

//creates linkages from metadata
array<ColumnLink ^> ^LoadLinks(Table ^table, Type ^ty, String ^Prefix)
{
	List<ColumnLink ^> ^links = gcnew List<ColumnLink ^>();

	for each(FieldInfo ^fi in ty->GetFields(BindingFlags::Public | BindingFlags::Instance))
		if(!Attribute::GetCustomAttribute(fi, EseNonpersistentFieldAttribute::typeid)) //don't save if nonpersistent
			links->Add(MakeColumnLink(table, fi, gcnew FieldLink(fi), Prefix));

	for each(PropertyInfo ^pi in ty->GetProperties(BindingFlags::Public | BindingFlags::Instance))
		if(Attribute::GetCustomAttribute(pi, EsePersistentPropertyAttribute::typeid)) //only save if a persistent property
			links->Add(MakeColumnLink(table, pi, gcnew PropertyLink(pi), Prefix));

	return links->ToArray();
}

generic <class T> public ref class StandardRowSerializer : IRecordLevelBridge<T>
{
	array<ColumnLink ^> ^Links;

public:
	StandardRowSerializer(Table ^table) :
		Links(LoadLinks(table, T::typeid, String::Empty))
	{}

	///<summary>Writes a single record using metadata associated with the object.</summary>
	virtual void Write(IWriteRecord ^wr, T obj)
	{
		for each(ColumnLink ^l in Links)
			l->SaveField(obj, wr);
	}

	///<summary>Reads a single record using metadata associated with the object.</summary>
	virtual T Read(IReadRecord ^rr)
	{
		Object ^obj = FormatterServices::GetUninitializedObject(T::typeid);
		
		for each(ColumnLink ^l in Links)
			l->LoadField(obj, rr);

		return safe_cast<T>(obj);
	}

	static Table ^CreateTableForStandardRowSerializer(Type ^type)
	{
		return nullptr;
	}

};


}

//Type genericType = typeof(Collection<>).MakeGenericType(typeof(DateTime));
//object instance = Activator.CreateInstance(genericType);