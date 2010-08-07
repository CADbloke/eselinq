///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Bookmark - Exposes ESE boomark support
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

///<summary>Bookmarks allow an efficient return to a particular location in a primary index.</summary>
///<remarks>
///A bookmark can be created from the current record of a cusor on a primary or secondary index.
///When on a secondary index, the bookmark still represents the corresponding location on the primary index.
///See SecondaryBookmark for positioning within a secondary index.
///<pr/>A bookmark is only valid in the table it was created from.
///<pr/>The bookmark's value is stable for the lifetime of the corresponding row. You can convert the bookmark to/from a byte array to save or restore it.
///<pr/>A bookmark can also be natively bridged to or from a Binary or LongBinary type column.
///<pr/>CompareTo will sort in the same order as the database index.
///</remarks>
public ref class Bookmark : public Seekable, IComparable<Bookmark ^>, IComparable
{
internal:
	ulong _BookmarkLength;
	uchar *_JetBookmark;

	!Bookmark()
	{
		if(_JetBookmark)
		{
			delete[] _JetBookmark;
			_JetBookmark = null;
		}
	}

	Bookmark(ulong BookmarkLength, uchar *JetBookmark) :
		_BookmarkLength(BookmarkLength),
		_JetBookmark(JetBookmark)
	{}

private:


public:
	///<summary>Create a bookmark from a previously stored byte array from ToByteArray.</summary>
	Bookmark(array<uchar> ^Bytes) :
		_BookmarkLength(Bytes->Length),
		_JetBookmark(new uchar[_BookmarkLength])
	{
		for(ulong i = 0; i < _BookmarkLength; i++)
			_JetBookmark[i] = Bytes[i];
	}

	///<summary>Create a bookmark from the current position of a cursor.</summary>
	Bookmark(Cursor ^Csr) :
		_BookmarkLength(0),
		_JetBookmark(null)
	{
		JET_SESID sesid = GetCursorSesid(Csr);
		JET_TABLEID tabid = GetCursorTableID(Csr);

		ulong len_req = 0;
		JET_ERR status;

		status = JetGetBookmark(sesid, tabid, _JetBookmark, _BookmarkLength, &len_req);

		if(status == JET_errBufferTooSmall)
		{
			_JetBookmark = new uchar[len_req];
			_BookmarkLength = len_req;

			status = JetGetBookmark(sesid, tabid, _JetBookmark, _BookmarkLength, &len_req);
		}

		EseException::RaiseOnError(status);
	}

	///<summary>Get the byte representation of the bookmark for future restoration.</summary>
	array<uchar> ^ToByteArray()
	{
		array<uchar> ^Arr = gcnew array<uchar>(_BookmarkLength);

		for(ulong i = 0; i < _BookmarkLength; i++)
			Arr[i] = _JetBookmark[i];

		return Arr;
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	virtual int CompareTo(Bookmark ^other)
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

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	virtual int CompareTo(Object ^other)
	{
		return CompareTo(safe_cast<Bookmark ^>(other));
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator ==(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) == 0;
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator !=(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength != b2._BookmarkLength || memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) != 0;
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator >(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength > b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) > 0);
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator <(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength < b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) < 0);
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator >=(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength > b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) >= 0);
	}

	///<summary>Compare one bookmark to another. Sorts in the same order as the index it came from.</summary>
	static bool operator <=(Bookmark %b1, Bookmark %b2)
	{
		return b1._BookmarkLength < b2._BookmarkLength || (b1._BookmarkLength == b2._BookmarkLength && memcmp(b1._JetBookmark, b2._JetBookmark, b1._BookmarkLength) <= 0);
	}

internal:
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) override
	{
		JET_ERR status = JetGotoBookmark(GetCursorSesid(c), GetCursorTableID(c), _JetBookmark, _BookmarkLength);

		NotEqual = false;
		switch(status)
		{
		case JET_errRecordDeleted:
		case JET_errNoCurrentRecord:
			HasCurrency = false;
			return;
		}

		EseException::RaiseOnError(status);
	}
};

Bookmark ^BookmarkFromMemblock(uchar *buff, ulong max)
{
	return gcnew Bookmark(max, buff);
}

void BookmarkGetBuffer(Bookmark ^t, uchar *&buff, ulong &max)
{
	buff = t->_JetBookmark;
	max = t->_BookmarkLength;
}