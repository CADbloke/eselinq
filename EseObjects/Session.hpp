///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  EseObjects.Session - Representation of ESE sessions
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

ref class Transaction;

///<summary>A session represents a single sequence of communication with ESE. There should be one session for each logical work thread.</summary>
///<remarks>
///Normally, a session with a transaction open is tied to the OS thread that opened the transaction.
///Transfer of ownership can only take place with {Set/Reset}SessionContext or by ending the transaction.
///</remarks>
public ref class Session
{
internal:
	Instance ^_Instance;
	JET_SESID _JetSesid;
	Transaction ^_CurrentTrans;
	Bridge ^_Bridge;

private:
	static JET_SESID BeginSession(JET_INSTANCE JetInstance)
	{
		JET_SESID NewJetSesid = null;

		EseException::RaiseOnError(JetBeginSession(JetInstance, &NewJetSesid, null, null));

		return NewJetSesid;
	}

	Session(Instance ^Instance, JET_SESID JetSesid) :
		_Instance(Instance),
		_JetSesid(JetSesid)
	{}

public:
	///<summary>Creates a new session.</summary>
	Session(Instance ^Instance) :
		_Instance(Instance),
		_JetSesid(BeginSession(Instance->_JetInstance)),
		_Bridge(Instance->_Bridge)
	{}

	~Session()
	{
		this->!Session();
	}

	!Session()
	{
		if(!_JetSesid)
			return;

		JetEndSession(_JetSesid, 0);
		_JetSesid = null;
	}

	///<summary>Copies certain aspects of the session into a new session. Calls JetDupSession.</summary>
	virtual Session ^Clone()
	{
		JET_SESID NewJetSesid;

		EseException::RaiseOnError(JetDupSession(_JetSesid, &NewJetSesid));

		return gcnew Session(_Instance, NewJetSesid);
	}

	property EseObjects::Instance ^Instance
	{
		EseObjects::Instance ^get() {return _Instance;}
	}

	///<summary>Provides the internal JET_SESID handle that represents the session to ESE.</summary>
	property IntPtr JetSesID
	{
		IntPtr get()
		{
			JET_INSTANCE JetSesidCopy = _JetSesid;

			return marshal_as<IntPtr>(JetSesidCopy);
		}
	}

	///<summary>Retrieves the current Transaction object.</summary>
	property Transaction ^CurrentTransaction
	{
		Transaction ^get() {return _CurrentTrans;}
	}

	property EseObjects::Bridge ^Bridge
	{
		EseObjects::Bridge ^get() {return _Bridge;};
		void set(EseObjects::Bridge ^bridge) {_Bridge = bridge;};
	};

internal:
	///<summary>Begins a transaction without using a EseObjects.Transaction object. Calls JetBeginTransaction.</summary>
	void BeginTransaction()
	{
		EseException::RaiseOnError(JetBeginTransaction(_JetSesid));
	}

	///<summary>Begins a readonly transaction without using a EseObjects.Transaction. Calls JetBeginTransaction2 with JET_bitTransactionReadOnly.</summary>
	void BeginReadonlyTransaction()
	{
		EseException::RaiseOnError(JetBeginTransaction2(_JetSesid, JET_bitTransactionReadOnly));
	}

	///<summary>Commits a transaction without using a EseObjects.Transaction. Calls JetCommitTransaction.</summary>
	void CommitTransaction()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, 0));
	}

	///<summary>Commits a transaction with lazy flush semantics without using a EseObjects.Transaction. Calls JetCommitTransaction with JET_bitCommitLazyFlush.</summary>
	void CommitTransactionLazyFlush()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitCommitLazyFlush));
	}

	///<summary>Calls JetCommitTransaction with JET_bitWaitLastLevel0Commit</summary>
	void CommitTransactionWaitLastLevel0Commit()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitWaitLastLevel0Commit));
	}

	///<summary>Calls JetCommitTransaction with JET_bitWaitAllLevel0Commit</summary>
	void CommitTransactionWaitAllLevel0Commit()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitWaitAllLevel0Commit));
	}

	///<summary>Cancels a transaction without using a EseObjects.Transaction. Calls JetRollback.</summary>
	void RollbackTransaction()
	{
		EseException::RaiseOnError(JetRollback(_JetSesid, 0));
	}

	///<summary>Cancels all current transactions without using EseObjects.Transaction objects. Calls JetRollback with JET_bitRollbackAll.</summary>
	void RollbackAllTransactions()
	{
		EseException::RaiseOnError(JetRollback(_JetSesid, JET_bitRollbackAll));
	}

public:
	///<summary>Calls JetGetVersion to retrieve the version of the database engine.</summary>
	property ulong Version
	{
		ulong get()
		{
			ulong ver;

			EseException::RaiseOnError(JetGetVersion(_JetSesid, &ver));

			return ver;
		}
	}

	///<summary>Triggers cleanup of the version store. Calls JetIdle with JET_bitIdleCompact.</summary>
	void CompactVersionStore()
	{
		EseException::RaiseOnError(JetIdle(_JetSesid, JET_bitIdleCompact));
	}

	///<summary>Indicates if the version store is at least half full. Calls JetIdle with JET_bitIdleStatus.</summary>
	property bool IsVersionStoreHalfFull
	{
		bool get()
		{
			return JetIdle(_JetSesid, JET_bitIdleStatus) == JET_wrnIdleFull;
		}
	}

	///<summary>Associates a unique context parameter with the current thread and specified session. Calls JetSetSessionContext.</summary>
	///<param name="Context">A user specified context value. It must be unique to the instance.</param>
	///<remarks>Normally, sessions cannot be shared between threads, but by using this method each time a thread uses a session and ResetSessionContext
	///when it is done, serialized shared access can be implemented.
	///<pr/>Must be called prior to opening a transaction.
	///<pr/>Thread safe. Multiple threads can request access to the same session concurrently and only one will be successful.
	///</remarks>
	void SetSessionContext(IntPtr Context)
	{
		EseException::RaiseOnError(JetSetSessionContext(_JetSesid, marshal_as<JET_API_PTR>(Context)));
	}

	///<summary>Releases context previously allocated with SetSessionContext. Calls JetResetSessionContext.</summary>
	void ResetSessionContext()
	{
		EseException::RaiseOnError(JetResetSessionContext(_JetSesid));
	}
};