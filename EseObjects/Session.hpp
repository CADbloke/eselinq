ref class Transaction;

public ref class Session : ICloneable
{
internal:
	Instance ^_Instance;
	JET_SESID _JetSesid;
	Transaction ^_CurrentTrans;

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
	Session(Instance ^Instance) :
		_Instance(Instance),
		_JetSesid(BeginSession(Instance->_JetInstance))
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

	virtual Object ^Clone()
	{
		JET_SESID NewJetSesid;

		EseException::RaiseOnError(JetDupSession(_JetSesid, &NewJetSesid));

		return gcnew Session(_Instance, NewJetSesid);
	}

	property EseObjects::Instance ^Instance
	{
		EseObjects::Instance ^get() {return _Instance;}
	}

	property IntPtr JetSesid
	{
		IntPtr get()
		{
			JET_INSTANCE JetSesidCopy = _JetSesid;

			return marshal_as<IntPtr>(JetSesidCopy);
		}
	}

	property Transaction ^CurrentTransaction
	{
		Transaction ^get() {return _CurrentTrans;}
	}

internal:
	void BeginTransaction()
	{
		EseException::RaiseOnError(JetBeginTransaction(_JetSesid));
	}

	void BeginReadonlyTransaction()
	{
		EseException::RaiseOnError(JetBeginTransaction2(_JetSesid, JET_bitTransactionReadOnly));
	}

	void CommitTransaction()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, 0));
	}

	void CommitTransactionLazyFlush()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitCommitLazyFlush));
	}

	void CommitTransactionWaitLastLevel0Commit()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitWaitLastLevel0Commit));
	}

	void CommitTransactionWaitAllLevel0Commit()
	{
		EseException::RaiseOnError(JetCommitTransaction(_JetSesid, JET_bitWaitAllLevel0Commit));
	}

	void RollbackTransaction()
	{
		EseException::RaiseOnError(JetRollback(_JetSesid, 0));
	}

	void RollbackAllTransactions()
	{
		EseException::RaiseOnError(JetRollback(_JetSesid, JET_bitRollbackAll));
	}

public:
	property ulong Version
	{
		ulong get()
		{
			ulong ver;

			EseException::RaiseOnError(JetGetVersion(_JetSesid, &ver));

			return ver;
		}
	}

	void CompactVersionStore()
	{
		EseException::RaiseOnError(JetIdle(_JetSesid, JET_bitIdleCompact));
	}

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