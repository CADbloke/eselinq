private ref class TableID
{
internal:
	JET_TABLEID _JetTableID;

private:
	Transaction ^_Trans;
	Database ^_Db;

internal:
	TableID(JET_TABLEID JetTableID, Transaction ^Trans, Database ^Db) :
		_JetTableID(JetTableID),
		_Trans(Trans),
		_Db(Db)
	{}

	void CheckValidity()
	{ 
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

		EseException::RaiseOnError(JetDupCursor(_Trans->Session->_JetSesid, _JetTableID, &newtab, 0));

		return gcnew TableID(newtab, _Trans, _Db);
	}

	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _Trans->Session;}
	}

	property EseObjects::Transaction ^Trans
	{
		EseObjects::Transaction ^get() {return _Trans;}
	}

	property EseObjects::Database ^Database
	{
		EseObjects::Database ^get() {return _Db;}
	}
};