///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.EseException - Exception subclass with ESE specific fields
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

///<summary>Specialized exception class for EseObjects which includes error code, symbol and description.</summary>
public ref class EseException : System::Runtime::InteropServices::ExternalException
{
public:
	//NEXT: better handling of warnings
	static bool IgnoreWarnings = true;

	///<summary>Converts Ese error codes to HRESULTs. Based on HRESULT_FROM_JET_ERR from Ese documentation</summary>
	HRESULT JetErrToHRESULT(JET_ERR code)
	{
		ulong const FACILITY_JET_ERR = 0xE5E;

		if(code == JET_errSuccess)
			return S_OK;
		else if(code == JET_errOutOfMemory)
			return E_OUTOFMEMORY;
		else if(code < 0)
			return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_JET_ERR, -code & 0xFFFF);
		else
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_JET_ERR, code & 0xFFFF);
	}

private:
	int _Code;
	String ^_Symbol;
	String ^_Description;

public:
	EseException(JET_ERR Code, String ^Symbol, String ^Description) :
		ExternalException(_Description, JetErrToHRESULT(Code)), //convert to HRESULT, since that's what ExternalException expects
		_Code(Code),
		_Symbol(Symbol),
		_Description(Description)
	{}

	EseException(EseException %ee) :
		_Code(ee._Code),
		_Symbol(ee._Symbol),
		_Description(ee.Description)
		{}

	static EseException ^CreateFromCode(JET_ERR Code)
	{
		char buffer[0x1000];

		JetGetSystemParameter(null, null, JET_paramErrorToString, reinterpret_cast<JET_API_PTR *>(&Code), buffer, sizeof buffer);

		String ^str = marshal_as<String ^>(buffer);

		array<wchar_t> ^separators = gcnew array<wchar_t>(2);
		separators[0] = L',';
		array<String ^> ^fields = str->Split(separators, 2);

		String ^Symbol, ^Description;

		switch(fields->Length)
		{
		case 0:
			Symbol = "";
			Description = "Unknown error";
			break;

		case 1:
			Symbol = fields[0];
			Description = fields[0];
			break;

		default: //length is at most 2 due to 2nd argument of Split
			Symbol = fields[0];
			Description = fields[1];
		}

		return gcnew EseException(Code, Symbol, Description);
	}


	static void RaiseOnError(JET_ERR Code)
	{
		if(Code == 0) // 0 = Success
			return;
		else if(Code > 0) // > 0 = Warning
		{
			if(IgnoreWarnings)
				return;
		}
		// < 0 = Error

		EseException ^e = CreateFromCode(Code);

		throw e;
	}

	///<summary>Integral Code number. Negative values are errors, positive warnings, zero is success.</summary>
	property JET_ERR Code
	{
		JET_ERR get() {return _Code;}
	}

	///<summary>Short (symbolic) name of error.</summary>
	property String ^Symbol
	{
		String ^get() {return _Symbol;}
	}

	///<summary>Long name for error.</summary>
	property String ^Description
	{
		String ^get() {return _Description;}
	}
};
