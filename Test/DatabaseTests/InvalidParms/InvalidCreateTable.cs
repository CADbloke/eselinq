///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.InvalidCreateTable
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
	class InvalidCreateTable
	{
		[Test]
		[ExpectedException(typeof(CreateTableException), ExpectedMessage="JET_errInvalidColumnType", MatchType=MessageMatch.Contains)]
		public static void BadColumnType()
		{
			using(var tr = new Transaction(E.S))
			{
				var Columns = new List<Column.CreateOptions>();
				var Indexes = new List<Index.CreateOptions>();

				var nt = new Table.CreateOptions
				{
					Name = "BadTable",
					Columns = Columns,
					Indexes = Indexes
				};

				Columns.Add(new Column.CreateOptions
				{
					Name = "InvalidTable",
					Type = (Column.Type)300
				});

				using(var tab = Table.Create(E.D, nt))
				{} //nothing to do since this should fail

				tr.Commit();
			}
		}

		[Test]
		[ExpectedException(typeof(CreateTableException), ExpectedMessage = "JET_errInvalidName", MatchType = MessageMatch.Contains)]
		public void ColumNameTooLong()
		{
			using(var tr = new Transaction(E.S))
			{
				var Columns = new List<Column.CreateOptions>();
				var Indexes = new List<Index.CreateOptions>();

				var nt = new Table.CreateOptions
				{
					Name = "BadTable",
					Columns = Columns,
					Indexes = Indexes
				};

				Columns.Add(new Column.CreateOptions
				{
					Name = new String('q', 100),
					Type = Column.Type.Long
				});

				using(var tab = Table.Create(E.D, nt))
				{} //nothing to do since this should fail
				tr.Commit();
			}
		}
	}
}
