public ref class Transaction
{
public:
	enum struct Status
	{
		Active,
		Committed,
		Rollbacked,
		Zero
	};

internal:
	Status _Status;

private:
	initonly EseObjects::Session ^_Session;
	initonly Transaction ^_PreviousTransaction;

	Transaction(EseObjects::Session ^Session, bool Begin) :
		_Status(Status::Active),
		_Session(Session),
		_PreviousTransaction(Session->_CurrentTrans)
	{
		if(Begin)
			Session->BeginTransaction();
		Session->_CurrentTrans = this;
	}

	Transaction(EseObjects::Session ^Session, Transaction::Status Status, Transaction ^Previous) :
		_Status(Status),
		_Session(Session),
		_PreviousTransaction(Previous)
	{}

public:
	Transaction(EseObjects::Session ^Session) :
		_Status(Status::Active),
		_Session(Session),
		_PreviousTransaction(Session->_CurrentTrans)
	{
		Session->BeginTransaction();
		Session->_CurrentTrans = this;
	}

	static Transaction ^BeginReadonly(EseObjects::Session ^Session)
	{
		Session->BeginReadonlyTransaction();
		return gcnew Transaction(Session, false);
	}

	//Creates a transaction object for use with cursors, etc. without the actual transaction. Not recommended.
	static Transaction ^CreateZero(EseObjects::Session ^Session)
	{
		return gcnew Transaction(Session, Status::Zero, nullptr);
	}

	//Dispose rolls back the transaction if it's active (otherwise it does nothing)
	~Transaction()
	{
		if(_Status == Status::Active)
			Rollback();
	}

	property Status CurrentStatus
	{
		Status get() {return _Status;}
	}

	property EseObjects::Session ^Session
	{
		EseObjects::Session ^get() {return _Session;}
	}

	property Transaction ^PreviousTransaction
	{
		Transaction ^get() {return _PreviousTransaction;}
	}

//cleanup helper functions
private:
	//check if this tran can be closed right now:
	//-is current (and can will be the one closed by Commit/Rollback)
	//-is active
	//throws if that's not the case
	void CheckCloseState()
	{
		if(_Status != Status::Active)
			throw gcnew InvalidOperationException("Transaction must be active to close. Current status: " + _Status.ToString());

		if(!Object::ReferenceEquals(Session->CurrentTransaction, this))
			throw gcnew InvalidOperationException("Can't close transaction, as another transaction has been opened since. You must close that transaction first by committing, rolling back or disposing it.");
	}

	void Close()
	{
		//should already be checked that this is current by calling CheckCloseOrdering
		Session->_CurrentTrans = PreviousTransaction;
	}

public:
	void Rollback()
	{
		CheckCloseState();
		Session->RollbackTransaction();
		_Status = Status::Rollbacked;
		Close();
	}

	//called on any transaction in the same chain, rolls back (and marks as such) all trans currently open on the session
	void RollbackAll()
	{
		Session->RollbackAllTransactions();

		EseObjects::Transaction ^Current;

		Current = Session->_CurrentTrans;

		//mark all active as rollbacked
		while(Current != nullptr)
		{
			Current->_Status = Status::Rollbacked;
			Current = Current->PreviousTransaction;
		}

		//no longer a current trans
		Session->_CurrentTrans = nullptr;
	}

	void Commit()
	{
		CheckCloseState();
		Session->CommitTransaction();
		_Status = Status::Committed;
		Close();
	}

	void CommitLazyFlush()
	{
		CheckCloseState();
		Session->CommitTransactionLazyFlush();
		_Status = Status::Committed;
		Close();
	}

	void CommitWaitLastLevel0Commit()
	{
		CheckCloseState();
		Session->CommitTransactionWaitLastLevel0Commit();
		_Status = Status::Committed;
		Close();
	}

	void CommitWaitAllLevel0Commit()
	{
		CheckCloseState();
		Session->CommitTransactionWaitAllLevel0Commit();
		_Status = Status::Committed;
		Close();
	}
};