template <class T> struct DemandLoadFunction
{
	PCWSTR Lib;
	PCSTR Name;
	T Func;
	DemandLoadFunction(PCWSTR Lib, PCSTR Name) :
		Lib(Lib),
		Name(Name),
		Func(NULL)
	{}

	void DemandQuiet()
	{
		if(!Func)
		{
			HMODULE Module = GetModuleHandle(Lib);

			if(Module)
				Func = reinterpret_cast<T>(GetProcAddress(Module, Name));
		}
	}

	void Demand()
	{
		HMODULE Module = GetModuleHandle(Lib);

		if(!Module)
			throw gcnew NotImplementedException(marshal_as<String ^>(Lib));

		Func = reinterpret_cast<T>(GetProcAddress(Module, Name));
		
		if(!Func)
			throw gcnew NotImplementedException(marshal_as<String ^>(Name));
	}

	operator T()
	{
		Demand();
		return Func;
	}
};