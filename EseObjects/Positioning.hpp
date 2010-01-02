///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Positioning - Descriptions of seekable and limitable positions
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

///<summary>Internal interface representing values that can you can seek to.</summary>
public ref struct Seekable abstract
{
internal:
	Seekable() {}
	virtual void SeekTo(bool %HasCurrency, bool %NotEqual, Cursor ^c) = 0;
};

///<summary>Internal interface representing values that you can set up a limit to.</summary>
public ref struct Limitable abstract : public Seekable
{
internal:
	Limitable() {}
	virtual bool LimitTo(Cursor ^c, bool upper) = 0;
};

public enum struct SeekRel
{
	///<summary>First entry equal to key. JET_bitSeekEQ in JetSeek.</summary>
	///<remarks>Not valid with a key that is using any wildcard.</remarks>
	EQ,
	///<summary>First entry greater or equal to key. JET_bitSeekGE in JetSeek.</summary>
	///<remarks>Not valid with WildcardEnd or WildcardEndPrefix.</remarks>
	GE,
	///<summary>First entry greater than key. JET_bitSeekGT in JetSeek.</summary>
	///<remarks>Not valid with WildcardEnd or WildcardEndPrefix.</remarks>
	GT,
	///<summary>Last entry less than key. JET_bitSeekLE in JetSeek.</summary>
	///<remarks>Not valid with WildcardStart or WildcardStartPrefix.</remarks>
	LE,
	///<summary>Last entry less than or equal to key. JET_bitSeekLT in JetSeek.</summary>
	///<remarks>Not valid with WildcardStart or WildcardStartPrefix.</remarks>
	LT
};

JET_GRBIT SeekRelToGrbit(SeekRel rel)
{
	switch(rel)
	{
	case SeekRel::EQ:
		return JET_bitSeekEQ;
	case SeekRel::GE:
		return JET_bitSeekGE;
	case SeekRel::GT:
		return JET_bitSeekGT;
	case SeekRel::LE:
		return JET_bitSeekLE;
	case SeekRel::LT:
		return JET_bitSeekLT;
	default:
		throw gcnew ArgumentException("Invalid SeekRel");
	}
}

bool SeekRelIsInclusive(SeekRel rel)
{
	switch(rel)
	{
	case SeekRel::EQ:
	case SeekRel::GE:
	case SeekRel::LE:
		return true;

	case SeekRel::GT:
	case SeekRel::LT:
		return false;

	default:
		throw gcnew ArgumentException("Invalid SeekRel");
	}
}

//5.0: needs to support the old wildcard method with JET_bitStrLimit and JET_bitSubStrLimit instead of these	
public enum struct Match
{
	///<summary>No wildcard used; fields are matched as specified.</summary> 
	Full,
	///<summary>Specified field values will be used, and any unspecified on the end will be matched to the last possible value. (JET_bitFullColumnEndLimit in JetMakeKey)</summary>
	WildcardEnd,
	///<summary>Specified field values will be used, the last of which is only matched as a prefix. Any other fields unspecified on the end will be matched to the last possible value. (JET_bitPartialColumnEndLimit in JetMakeKey)</summary>
	WildcardEndPrefix,
	///<summary>Specified field values will be used, and any unspecified on the end will be matched to the first possible value. (JET_bitFullColumnStartLimit in JetMakeKey)</summary>
	WildcardStart,
	///<summary>Specified field values will be used, the last of which is only matched as a prefix. Any other fields unspecified on the end will be matched to the first possible value. (JET_bitPartialColumnStartLimit in JetMakeKey)</summary>
	WildcardStartPrefix
};

JET_GRBIT MatchToGrbit(Match match)
{
	switch(match)
	{
	case Match::Full:
		return 0;
	case Match::WildcardEnd:
		return JET_bitFullColumnEndLimit;
	case Match::WildcardStart:
		return JET_bitFullColumnStartLimit;
	case Match::WildcardEndPrefix:
		return JET_bitPartialColumnEndLimit;
	case Match::WildcardStartPrefix:
		return JET_bitPartialColumnStartLimit;
	default:
		throw gcnew ArgumentException("Invalid Match");
	}
}