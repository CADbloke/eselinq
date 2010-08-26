///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.ColumnTest
///////////////////////////////////////////////////////////////////////////////
//
//This software is licenced under the terms of the MIT License:
//
//Copyright (c) 2010 Christopher Smith
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

using System;
using System.Collections.Generic;
using NUnit.Framework;
using EseObjects;

namespace Test.DatabaseTests
{
	[TestFixture]
	class TransactionTest
	{
		[Test]
		public void SimpleBeginRollback()
		{
			var tr = new Transaction(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.Rollback();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
		}

		[Test]
		public void SimpleBeginCommit()
		{
			var tr = new Transaction(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.Commit();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		public void SimpleBeginMethodCommit()
		{
			var tr = Transaction.Begin(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.Commit();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		public void SimpleBeginReadonlyMethodCommit()
		{
			var tr = Transaction.BeginReadonly(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.Commit();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		public void SimpleBeginCommitLasyFlush()
		{
			var tr = new Transaction(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.CommitLazyFlush();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		public void SimpleBeginCommitWaitLastLevel0Commit()
		{
			var tr = new Transaction(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.CommitWaitLastLevel0Commit();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		public void SimpleBeginCommitWaitAllLevel0Commit()
		{
			var tr = new Transaction(E.S);
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
			tr.CommitWaitAllLevel0Commit();
			Assert.That(tr.CurrentStatus, Is.EqualTo(Transaction.Status.Committed));
		}

		[Test]
		[ExpectedException(typeof(CreateTableException), ExpectedMessage = "JET_errTransReadOnly", MatchType = MessageMatch.Contains)]
		public void ModificationInReadonlyTrans()
		{
			using(var tr = Transaction.BeginReadonly(E.S))
				Table.Create(E.D, Table.CreateOptions.NewWithLists("FailTable"));
		}

		[Test]
		public void ImplicitTransaction()
		{
			Assert.That(E.S.CurrentTransaction, Is.Null);
		}

		[Test]
		public void NestedTransactions()
		{
			using(var tr1 = new Transaction(E.S))
			using(var tr2 = new Transaction(E.S))
			using(var tr3 = new Transaction(E.S))
			using(var tr4 = new Transaction(E.S))
			using(var tr5 = new Transaction(E.S))
			using(var tr6 = new Transaction(E.S))
			using(var tr7 = new Transaction(E.S))
				Assert.That(tr7.CurrentStatus, Is.EqualTo(Transaction.Status.Active));
		}

		[Test]
		public void NestedTransactionsRollbackAll()
		{
			var tr1 = new Transaction(E.S);
			var tr2 = new Transaction(E.S);
			var tr3 = new Transaction(E.S);
			var tr4 = new Transaction(E.S);
			var tr5 = new Transaction(E.S);
			var tr6 = new Transaction(E.S);
			var tr7 = new Transaction(E.S);

			tr7.RollbackAll();

			Assert.That(tr1.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr2.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr3.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr4.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr5.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr6.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
			Assert.That(tr7.CurrentStatus, Is.EqualTo(Transaction.Status.Rollbacked));
		}

		[Test]
		public void RollbackCreateTable()
		{
			using(var tr = Transaction.Begin(E.S))
			{
				Table.Create(E.D, Table.CreateOptions.NewWithLists("EtherealTable"));
				tr.Rollback();
			}

			try
			{
				new Table(E.D, "EtherealTable").Dispose();
				Assert.Fail("Exception expected");
			}
			catch(EseException e)
			{
				Assert.That(e.Symbol, Is.EqualTo("JET_errObjectNotFound"));
			}
		}

		[Test]
		public void CommitCreateTable()
		{
			using(var tr = Transaction.Begin(E.S))
			{
				Table.Create(E.D, Table.CreateOptions.NewWithLists("PhysicalTable")).Dispose();
				tr.Commit();
			}

			new Table(E.D, "PhysicalTable").Dispose();
			Table.Delete(E.D, "PhysicalTable");
		}
	}
}
