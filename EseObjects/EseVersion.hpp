void GetEseVersion(ushort &Major, ushort &Minor, ushort &Build1, ushort &Build2)
{
	ulong handle = null;
	ulong size = GetFileVersionInfoSize(L"esent.dll", &handle);

	if (size > 0)
	{
		char *buff = new char[size];

		try
		{
			if (GetFileVersionInfo(L"esent.dll", handle, size, buff))
			{
				VS_FIXEDFILEINFO *fileinfo;
				UINT fileinfosize = 0;

				if(VerQueryValue(buff, L"\\", (void **)&fileinfo, &fileinfosize) && fileinfosize && fileinfo->dwSignature == 0xfeef04bd)
				{
					Major = HIWORD(fileinfo->dwFileVersionMS);
					Minor = LOWORD(fileinfo->dwFileVersionMS);
					Build1 = LOWORD(fileinfo->dwFileVersionLS);
					Build2 = HIWORD(fileinfo->dwFileVersionLS);
				}
			}
		}
		finally
		{
			delete buff;
		}
	}
}

ushort EseVersionMajor = 0, EseVersionMinor = 0, EseVersionBuild1 = 0, EseVersionBuild2 = 0;
bool EseVersionLoaded = false;

void EnsureEseVersionLoaded()
{
	if(!EseVersionLoaded)
	{
		EseVersionLoaded = true;
		GetEseVersion(EseVersionMajor, EseVersionMinor, EseVersionBuild1, EseVersionBuild2);
	}
}

ushort GetEseVersionMajor()
{
	EnsureEseVersionLoaded();
	return EseVersionMajor;
}

ushort GetEseVersionMinor()
{
	EnsureEseVersionLoaded();
	return EseVersionMinor;
}

ushort GetEseVersionBuild1()
{
	EnsureEseVersionLoaded();
	return EseVersionBuild1;
}

ushort GetEseVersionBuild2()
{
	EnsureEseVersionLoaded();
	return EseVersionBuild2;
}