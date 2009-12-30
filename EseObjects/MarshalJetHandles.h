namespace msclr
{
	namespace interop
	{
		template<> inline IntPtr marshal_as<IntPtr, JET_API_PTR> (JET_API_PTR const &from)
		{
			return IntPtr(reinterpret_cast<void *>(from)); //pointer sized int
		}

		template<> inline IntPtr ^marshal_as<IntPtr ^, JET_API_PTR> (JET_API_PTR const &from)
		{
			return gcnew IntPtr(reinterpret_cast<void *>(from)); //pointer sized int
		}

#if defined(_WIN64)
		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr> (IntPtr const &from)
		{
			IntPtr copy = from;
			return copy.ToInt64();
		}

		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr ^> (IntPtr ^const &from)
		{
			return from->ToInt64();
		}
#else
		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr> (IntPtr const &from)
		{
			IntPtr copy = from;
			return copy.ToInt32();
		}

		template<> inline JET_API_PTR marshal_as<JET_API_PTR, IntPtr ^> (IntPtr ^const &from)
		{
			return from->ToInt32();
		}
#endif
		//
		//template<> inline IntPtr marshal_as<IntPtr, JET_HANDLE> (const JET_HANDLE &from)
		//{
		//	return marshal_as<IntPtr, JET_API_PTR>(from);
		//}

	}
}