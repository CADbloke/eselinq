///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.TableID - Internal common support for JET_TABLEIDs
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

private ref class TableID
{
internal:
	JET_TABLEID _JetTableID;

private:
	Transaction ^_Trans;
	Database ^_Db;
	Bridge ^_Bridge;

internal:
	TableID(JET_TABLEID JetTableID, Transaction ^Trans, Database ^Db) :
		_JetTableID(JetTableID),
		_Trans(Trans),
		_Db(Db),
		_Bridge(Db->_Bridge)
	{}

	void CheckValidity()
	{ 
		if(!_Trans)
			return; //ok

		switch(_Trans->_Status)
		{
		case Transaction::Status::Active:
		case Transaction::Status::Zero:
		case Transaction::Status::Committed:
			//ok
			break;

		case Transaction::Status::Rollbacked:
			_JetTableID = null; //it's invalid anyway
			throw gcnew InvalidOperationException("Transaction supporting table has been closed. Table handle invalid.");
		}
	}

	~TableID()
	{
		this->!TableID();
	}

	!TableID()
	{
		if(_JetTableID)
			if(_Trans)
				switch(_Trans->_Status)
				{
				case Transaction::Status::Active:
				case Transaction::Status::Zero:
				case Transaction::Status::Committed:
					JetCloseTable(Session->_JetSesid, _JetTableID);
					_JetTableID = null;
					break;

				case Transaction::Status::Rollbacked:
					_JetTableID = null; //it's invalid anyway
				}
	}

	TableID ^Duplicate()
	{
		CheckValidity();

		JET_TABLEID newtab = null;

		EseException::RaiseOnError(JetDupCursor(_Db->Session->_JetSesid, _JetTableID, &newtab, 0));

		return gcnew TableID(newtab, _Trans, _Db);
	}

	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _Db->Session;}
	}

	property EseObjects::Transaction ^Trans
	{
		EseObjects::Transaction ^get() {return _Trans;}
	}

	property EseObjects::Database ^Database
	{
		EseObjects::Database ^get() {return _Db;}
	}

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _Bridge;};
		void set(EseObjects::Bridge ^bridge) {_Bridge = bridge;};
	};
};