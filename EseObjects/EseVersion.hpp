///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseVersion - Retrieves module version information from the ESE DLL
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