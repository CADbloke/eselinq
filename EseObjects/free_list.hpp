///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  free_list - Facility to free a list of heap allocated memory
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

class free_list
{
	struct Entry
	{
		Entry *Next;
		void *Item;
		bool IsArray;

		Entry(Entry *Next, void *Item, bool IsArray) :
			Next(Next),
			Item(Item),
			IsArray(IsArray)
		{}
	}*Head;

	void Push(void *Item, bool IsArray)
	{
		Entry *e = new Entry(Head, Item, IsArray);
		Head = e;
	}

public:
	free_list() :
		Head(null)
	{}

	template <class T> T *alloc_zero()
	{
		T *x = new T;

		memset(x, 0, sizeof(T));

		Push(x, false);

		return x;
	}

	template <class T> T *alloc_array(size_t ct)
	{
		T *x = new T[ct];

		Push(x, true);

		return x;
	}

	template <class T> T *alloc_array_zero(size_t ct)
	{
		T *x = new T[ct];

		memset(x, 0, sizeof(T) * ct);

		Push(x, true);

		return x;
	}

	~free_list()
	{
		while(Head)
		{
			Entry *e = Head;
			Head = e->Next;

			if(e->IsArray)
				delete[] e->Item;
			else
				delete e->Item;
			delete e;
		}
	}
};