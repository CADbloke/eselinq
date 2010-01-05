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

///<summary>Running context to save and read members of a partiuclar type to a particular table using member name to column name correleation.
///<pr/>An instance must only be used with the type and table it was constructed with.</summary>
generic <class T> public ref class RecordLevelMemberiwse
{
	StreamingContext ^context;
	array<MemberInfo ^> ^meta;
	array<Column ^> ^cols;

public:
	///<summary>Builds a new object, using table information from the specified IWriteRecord.</summary>
	RecordLevelMemberiwse(IWriteRecord ^wr) :
		context(gcnew StreamingContext(StreamingContextStates::Persistence)),
		meta(FormatterServices::GetSerializableMembers(T::typeid)),
		cols(gcnew array<Column ^>(meta->Length))
	{
		Table ^table = wr->Read->AsTable;

		for(long i = 0; i < meta->Length; i++)
			cols[i] = gcnew Column(table, meta[i]->Name);
	}

	///<summary>Builds a new object, using table information from the specified table.</summary>
	RecordLevelMemberiwse(Table ^table) :
		context(gcnew StreamingContext(StreamingContextStates::Persistence)),
		meta(FormatterServices::GetSerializableMembers(T::typeid)),
		cols(gcnew array<Column ^>(meta->Length))
	{
		for(long i = 0; i < meta->Length; i++)
			cols[i] = gcnew Column(table, meta[i]->Name);
	}

	///<summary>Writes a single record using metadata associated with the object.</summary>
	void Write(IWriteRecord ^wr, T o)
	{
		array<Object ^> ^data = FormatterServices::GetObjectData(o, meta);

		for(long i = 0; i < meta->Length; i++)
			wr->Set(cols[i], data[i]);
	}

	///<summary>Reads a single record using metadata associated with the object.</summary>
	T Read(IReadRecord ^rr)
	{
		array<Object ^> ^data = gcnew array<Object ^>(meta->Length);

		for(long i = 0 ; i < meta->Length; i++)
			data[i] = rr->Retrieve(cols[i], meta[i]->ReflectedType);

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
///Implementors are expected to also provide a constructor with a single IReadRecord parameter for deserialization.
///</summary>
public interface struct IRecordSerializable
{
	void WriteToRecord(IWriteRecord ^wr);
};

}