///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Key - Exposes support for saving ESE keys
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

///<summary>Keys allow an efficient positioning to a particular location in an index.</summary>
///<remarks>
///<pr/>A key is only valid in the index it was created from.
///<pr/>The key's value is stable.  You can convert the key to/from a byte array to save or restore it.
///<pr/>A key can also be natively bridged to or from a Binary or LongBinary type column.
///<pr/>CompareTo will sort in the same order as the database index.
///</remarks>
public ref class Key : public Seekable, IComparable<Key ^>, IComparable
{
internal:
	ulong _KeyLength;
	uchar *_JetKey;

	!Key()
	{
		if(_JetKey)
			delete[] _JetKey;
	}

	Key(ulong KeyLength, uchar *JetKey) :
		_KeyLength(KeyLength),
		_JetKey(JetKey)
	{}

	static void LoadFieldIntoTableID(JET_SESID sesid, JET_TABLEID tabid, Bridge ^Bridge, Column ^Col, Object ^Val, JET_GRBIT flags)
	{
		free_list fl;
		marshal_context mc;
		void *data;
		ulong data_len;
		bool empty;

		to_memblock_bridge(Bridge, Val, data, data_len, empty, Col->_JetColTyp, Col->_CP, mc, fl);

		EseException::RaiseOnError(JetMakeKey(sesid, tabid, data, data_len, flags | (empty ? JET_bitKeyDataZeroLength : 0)));
	}

	static void LoadFieldsIntoTableID(JET_SESID sesid, JET_TABLEID tabid, Bridge ^Bridge, IEnumerable<Field> ^KeyFields, JET_GRBIT final_grbit)
	{
		bool last = false, first = true;

		IEnumerator<Field> ^e = KeyFields->GetEnumerator();
		last = !e->MoveNext();

		while(!last)
		{
			Field current = e->Current;
			last = !e->MoveNext();

			LoadFieldIntoTableID(sesid, tabid, Bridge, current.Col, current.Val, (first ? JET_bitNewKey : 0) | (last ? final_grbit : 0));
		
			first = false;
		}
	}

	static void LoadSingleFieldIntoTableID(JET_SESID sesid, JET_TABLEID tabid, Bridge ^Bridge, Field field, JET_GRBIT grbit)
	{
		LoadFieldIntoTableID(sesid, tabid, Bridge, field.Col, field.Val, JET_bitNewKey | grbit);
	}

private:
	Key() :
	   _KeyLength(0),
	   _JetKey(null)
	{}

	void GetBytesFromTableID(JET_SESID sesid, JET_TABLEID tabid, JET_GRBIT grbit)
	{
		if(_JetKey)
			delete _JetKey;

		_KeyLength = 0;
		_JetKey = null;

		ulong len_req = 0;
		JET_ERR status;

		status = JetRetrieveKey(sesid, tabid, _JetKey, _KeyLength, &len_req, grbit);

		_JetKey = new uchar[len_req];
		_KeyLength = len_req;

		status = JetRetrieveKey(sesid, tabid, _JetKey, _KeyLength, &len_req, grbit);

		EseException::RaiseOnError(status);
	}

public:
	///<summary>Makes a key object from binary data. Use to save or transmit key data.</summary>
	Key(array<uchar> ^Bytes) :
		_KeyLength(Bytes->Length),
		_JetKey(new uchar[_KeyLength])
	{
		for(ulong i = 0; i < _KeyLength; i++)
			_JetKey[i] = Bytes[i];
	}

	///<summary>Makes a Key object corresponding to the cursor's current record.</summary>
	Key(Cursor ^Csr) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetCursorSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		GetBytesFromTableID(sesid, tabid, 0); //0, the default grbit is retrieve key for current record
	}

	///<summary>Makes an exact match key for the specified table (via a cursor) and fields</summary>
	Key(Cursor ^Csr, IEnumerable<Field> ^KeyFields) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetCursorSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);
		Bridge ^bridge = GetCursorBridge(Csr);

		LoadFieldsIntoTableID(sesid, tabid, bridge, KeyFields, 0);
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy);
	}

	///<summary>Makes an exact match key for the specified table and fields.</summary>
	Key(Table ^Tab, IEnumerable<Field> ^KeyFields) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetTableSesid(Tab);
		JET_TABLEID tabid = GetTableTableID(Tab);
		Bridge ^bridge = GetTableBridge(Tab);

		LoadFieldsIntoTableID(sesid, tabid, bridge, KeyFields, 0);
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy); //JET_bitRetrieveCopy retrieves copy from current constructed key
	}


	///<summary>Makes a key for the specified table (via a cursor) and fields with the specified match mode.</summary>
	Key(Cursor ^Csr, IEnumerable<Field> ^KeyFields, Match MatchMode) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetCursorSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);
		Bridge ^bridge = GetCursorBridge(Csr);

		LoadFieldsIntoTableID(sesid, tabid, bridge, KeyFields, MatchToGrbit(MatchMode));
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy); 
	}

	///<summary>Makes a key for the specified table and fields and fields with the specified match mode.</summary>
	Key(Table ^Tab, IEnumerable<Field> ^KeyFields, Match MatchMode) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetTableSesid(Tab);
		JET_TABLEID tabid = GetTableTableID(Tab);
		Bridge ^bridge = GetTableBridge(Tab);

		LoadFieldsIntoTableID(sesid, tabid, bridge, KeyFields, MatchToGrbit(MatchMode));
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy); 
	}

	///<summary>Retrieves a binary representation of the key.</summary>
	array<uchar> ^ToByteArray()
	{
		array<uchar> ^Arr = gcnew array<uchar>(_KeyLength);

		for(ulong i = 0; i < _KeyLength; i++)
			Arr[i] = _JetKey[i];

		return Arr;
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	virtual int CompareTo(Key ^other)
	{
		if(other == nullptr)
			return 1;

		if(_KeyLength == other->_KeyLength)
			return memcmp(_JetKey, other->_JetKey, _KeyLength);
		else if(_KeyLength < other->_KeyLength)
			return -1;
		else //length is greater
			return 1;
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	virtual int CompareTo(Object ^other)
	{
		return CompareTo(safe_cast<Key ^>(other));
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator ==(Key %b1, Key %b2)
	{
		return b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) == 0;
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator !=(Key %b1, Key %b2)
	{
		return b1._KeyLength != b2._KeyLength || memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) != 0;
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator >(Key %b1, Key %b2)
	{
		return b1._KeyLength > b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) > 0);
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator <(Key %b1, Key %b2)
	{
		return b1._KeyLength < b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) < 0);
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator >=(Key %b1, Key %b2)
	{
		return b1._KeyLength > b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) >= 0);
	}

	///<summary>Compares one key to another. Sorts in the same order that keys sort in the source index.</summary>
	static bool operator <=(Key %b1, Key %b2)
	{
		return b1._KeyLength < b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) <= 0);
	}

internal:
	virtual void SeekTo(bool %has_currency, bool %not_equal, Cursor ^c) override
	{
		JET_SESID sesid = GetCursorSesid(c);
		JET_TABLEID tabid = GetCursorTableID(c);

		EseException::RaiseOnError(JetMakeKey(sesid, tabid, _JetKey, _KeyLength, JET_bitNormalizedKey));
		JET_ERR status = JetSeek(sesid, tabid, JET_bitSeekEQ);

		switch(status)
		{
		case JET_errRecordNotFound:
			not_equal = true;
			has_currency = false;
			return;

		case JET_wrnSeekNotEqual:
			not_equal = true;
			has_currency = true;
			return;

		default:
			not_equal = false;
			has_currency = true;
			break;
		}

		EseException::RaiseOnError(status);
	}
};

Key ^KeyFromMemblock(uchar *buff, ulong max)
{
	return gcnew Key(max, buff);
}

void KeyGetBuffer(Key ^t, uchar *&buff, ulong &max)
{
	buff = t->_JetKey;
	max = t->_KeyLength;
}