//facility to free a list of heap allocated memory

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