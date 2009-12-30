///<summary>Keys allow an efficient positioning to a particular location in an index.</summary>
///<remarks>
///<pr/>A key is only valid in the index it was created from.
///<pr/>The key's value is stable.  You can convert the key to/from a byte array to save or restore it.
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

	static void LoadFieldIntoTableID(JET_SESID sesid, JET_TABLEID tabid, Column ^Col, Object ^Val, JET_GRBIT flags)
	{
		free_list fl;
		marshal_context mc;
		void *data;
		ulong data_len;
		bool empty;

		to_memblock(Val, data, data_len, empty, Col->_JetColTyp, Col->_CP, mc, fl);

		EseException::RaiseOnError(JetMakeKey(sesid, tabid, data, data_len, flags | (empty ? JET_bitKeyDataZeroLength : 0)));
	}

	static void LoadFieldsIntoTableID(JET_SESID sesid, JET_TABLEID tabid, IEnumerable<Field> ^KeyFields, JET_GRBIT final_grbit)
	{
		bool last = false, first = true;

		IEnumerator<Field> ^e = KeyFields->GetEnumerator();
		last = !e->MoveNext();

		while(!last)
		{
			Field current = e->Current;
			last = !e->MoveNext();

			LoadFieldIntoTableID(sesid, tabid, current.Col, current.Val, (first ? JET_bitNewKey : 0) | (last ? final_grbit : 0));
		
			first = false;
		}
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
		JET_SESID sesid = GetCurosrSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		GetBytesFromTableID(sesid, tabid, 0); //0, the default grbit is retrieve key for current record
	}

	///<summary>Makes an exact match key for the specified table (via a cursor) and fields</summary>
	Key(Cursor ^Csr, IEnumerable<Field> ^KeyFields) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetCurosrSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		LoadFieldsIntoTableID(sesid, tabid, KeyFields, 0);
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy);
	}

	///<summary>Makes an exact match key for the specified table and fields.</summary>
	Key(Table ^Tab, IEnumerable<Field> ^KeyFields) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetTableSesid(Tab);
		JET_TABLEID tabid = GetTableTableID(Tab);

		LoadFieldsIntoTableID(sesid, tabid, KeyFields, 0);
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy); //JET_bitRetrieveCopy retrieves copy from current constructed key
	}


	///<summary>Makes a key for the specified table (via a cursor) and fields with the specified match mode.</summary>
	Key(Cursor ^Csr, IEnumerable<Field> ^KeyFields, Match MatchMode) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetCurosrSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		LoadFieldsIntoTableID(sesid, tabid, KeyFields, MatchToGrbit(MatchMode));
		GetBytesFromTableID(sesid, tabid, JET_bitRetrieveCopy); 
	}

	///<summary>Makes a key for the specified table and fields and fields with the specified match mode.</summary>
	Key(Table ^Tab, IEnumerable<Field> ^KeyFields, Match MatchMode) :
		_KeyLength(0),
		_JetKey(null)
	{
		JET_SESID sesid = GetTableSesid(Tab);
		JET_TABLEID tabid = GetTableTableID(Tab);

		LoadFieldsIntoTableID(sesid, tabid, KeyFields, MatchToGrbit(MatchMode));
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

	virtual int CompareTo(Object ^other)
	{
		return CompareTo(safe_cast<Key ^>(other));
	}

	static bool operator ==(Key %b1, Key %b2)
	{
		return b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) == 0;
	}

	static bool operator !=(Key %b1, Key %b2)
	{
		return b1._KeyLength != b2._KeyLength || memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) != 0;
	}

	static bool operator >(Key %b1, Key %b2)
	{
		return b1._KeyLength > b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) > 0);
	}

	static bool operator <(Key %b1, Key %b2)
	{
		return b1._KeyLength < b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) < 0);
	}

	static bool operator >=(Key %b1, Key %b2)
	{
		return b1._KeyLength > b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) >= 0);
	}

	static bool operator <=(Key %b1, Key %b2)
	{
		return b1._KeyLength < b2._KeyLength || (b1._KeyLength == b2._KeyLength && memcmp(b1._JetKey, b2._JetKey, b1._KeyLength) <= 0);
	}

internal:
	virtual void SeekTo(bool %has_currency, bool %not_equal, Cursor ^c) override
	{
		JET_SESID sesid = GetCurosrSesid(c);
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