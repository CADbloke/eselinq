///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.SecondaryBookmarks - ESE support for secondary bookmarks
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

///<summary>SecondaryBokmarks allow an efficient return to a particular location in a secondary index. Requires 5.1+</summary>
///<remarks>SecondaryBookmarks can only seek alone if the key is unique. Otherwise, you must also use a primary bookmark.
///<pr/>A secondary bookmark is only valid with the same index it was created from.
///<pr/>The secondary bookmark's value is stable for the lifetime of the corresponding row, as long as the indexed fields don't change. You can convert the bookmark to/from a byte array to save or restore it.
///<pr/>A secondary bookmark can also be natively bridged to or from a Binary or LongBinary type column.
///<pr/>CompareTo will sort in the same order as the database index.
///</remarks>

//NEXT:better code sharing between this class and Bookmark would be nice
public ref class SecondaryBookmark : public Seekable, IComparable<SecondaryBookmark ^>, IComparable
{
internal:
	ulong _BookmarkLength;
	uchar *_JetBookmark;

	!SecondaryBookmark()
	{
		if(_JetBookmark)
			delete[] _JetBookmark;
	}

	SecondaryBookmark(ulong BookmarkLength, uchar *JetBookmark) :
		_BookmarkLength(BookmarkLength),
		_JetBookmark(JetBookmark)
	{}

public:
	SecondaryBookmark(array<uchar> ^Bytes) :
		_BookmarkLength(Bytes->Length),
		_JetBookmark(new uchar[_BookmarkLength])
	{
		for(ulong i = 0; i < _BookmarkLength; i++)
			_JetBookmark[i] = Bytes[i];
	}

	SecondaryBookmark(Cursor ^Csr) :
		_BookmarkLength(0),
		_JetBookmark(null)
	{
		JET_SESID sesid = GetCursorSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		ulong len_req = 0;
		JET_ERR status;

		status = JetGetSecondaryIndexBookmark(sesid, tabid, _JetBookmark, _BookmarkLength, &len_req, null, 0, null, 0);

		_JetBookmark = new uchar[len_req];
		_BookmarkLength = len_req;

		status = JetGetSecondaryIndexBookmark(sesid, tabid, _JetBookmark, _BookmarkLength, &len_req, null, 0, null, 0);

		if(status != JET_errBufferTooSmall) //this status is expected since we aren't retrieving the primary bookmark
			EseException::RaiseOnError(status);
	}

	array<uchar> ^ToByteArray()
	{
		array<uchar> ^Arr = gcnew array<uchar>(_BookmarkLength);

		for(ulong i = 0; i < _BookmarkLength; i++)
			Arr[i] = _JetBookmark[i];

		return Arr;
	}

	virtual int CompareTo(SecondaryBookmark ^other)
	{
		if(other == nullptr)
			return 1;

		if(_BookmarkLength == other->_BookmarkLength)
			return memcmp(_JetBookmark, other->_JetBookmark, _BookmarkLength);
		else if(_BookmarkLength < other->_BookmarkLength)
			return -1;
		else //length is greater
			return 1;
	}

	virtual int CompareTo(Object ^other)
	{
		return CompareTo(safe_cast<SecondaryBookmark ^>(other));
	}

	static bool operator ==(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) == 0;
	}

	static bool operator !=(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength != b2._BookmarkLength || memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) != 0;
	}

	static bool operator >(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength > b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) > 0);
	}

	static bool operator <(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength < b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) < 0);
	}

	static bool operator >=(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength > b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) >= 0);
	}

	static bool operator <=(SecondaryBookmark %b1, SecondaryBookmark %b2)
	{
		return b1._BookmarkLength < b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) <= 0);
	}
	
internal:
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) override
	{
		JET_ERR status = JetGotoSecondaryIndexBookmark(GetCursorSesid(c), GetCursorTableID(c), _JetBookmark, _BookmarkLength, NULL, 0, 0);

		NotEqual = false;
		HasCurrency = false;
		switch(status)
		{
		case JET_errRecordDeleted:
		case JET_errNoCurrentRecord:
			return;
		}

		EseException::RaiseOnError(status);
		HasCurrency = true;
	}
};

SecondaryBookmark ^SecondaryBookmarkFromMemblock(uchar *buff, ulong max)
{
	return gcnew SecondaryBookmark(max, buff);
}

void SecondaryBookmarkGetBuffer(SecondaryBookmark ^t, uchar *&buff, ulong &max)
{
	buff = t->_JetBookmark;
	max = t->_BookmarkLength;
}