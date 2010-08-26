///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Transaction - Representation of ESE transactions
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

///<summary>Object oriented transaction management.</summary>
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
	///<summary>Begins a new transaction that lasts for the lifetime of the object. Calls JetBeginTransaction. </summary>
	Transaction(EseObjects::Session ^Session) :
		_Status(Status::Active),
		_Session(Session),
		_PreviousTransaction(Session->_CurrentTrans)
	{
		Session->BeginTransaction();
		Session->_CurrentTrans = this;
	}

	///<summary>Begins a new transaction that lasts for the lifetime of the object. Calls JetBeginTransaction. </summary>
	static Transaction ^Begin(EseObjects::Session ^Session)
	{
		Session->BeginTransaction();
		return gcnew Transaction(Session, false);
	}

	///<summary>Begins a new readonly transaction that lasts for the lifetime of the object. Calls JetBeginTransaction2 with JET_bitTransactionReadOnly.</summary>
	static Transaction ^BeginReadonly(EseObjects::Session ^Session)
	{
		Session->BeginReadonlyTransaction();
		return gcnew Transaction(Session, false);
	}

	///<summary>Creates a transaction object for use with cursors, etc. without the actual transaction. Not recommended.</summary>
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
	///<summary>Cancels a transaction. Calls JetRollback.</summary>
	void Rollback()
	{
		CheckCloseState();
		Session->RollbackTransaction();
		_Status = Status::Rollbacked;
		Close();
	}

	///<summary>Cancels all current transactions. Calls JetRollback with JET_bitRollbackAll.</summary>
	///<remarks>Called on any transaction in the same chain, rolls back (and marks as such) all trans currently open on the session</remarks>
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

	///<summary>Commits a transaction. Calls JetCommitTransaction.</summary>
	void Commit()
	{
		CheckCloseState();
		Session->CommitTransaction();
		_Status = Status::Committed;
		Close();
	}

	///<summary>Commits a transaction with lazy flush semantics. Calls JetCommitTransaction with JET_bitCommitLazyFlush.</summary>
	void CommitLazyFlush()
	{
		CheckCloseState();
		Session->CommitTransactionLazyFlush();
		_Status = Status::Committed;
		Close();
	}

	///<summary>Calls JetCommitTransaction with JET_bitWaitLastLevel0Commit</summary>
	void CommitWaitLastLevel0Commit()
	{
		CheckCloseState();
		Session->CommitTransactionWaitLastLevel0Commit();
		_Status = Status::Committed;
		Close();
	}

	///<summary>Calls JetCommitTransaction with JET_bitWaitAllLevel0Commit</summary>
	void CommitWaitAllLevel0Commit()
	{
		CheckCloseState();
		Session->CommitTransactionWaitAllLevel0Commit();
		_Status = Status::Committed;
		Close();
	}
};