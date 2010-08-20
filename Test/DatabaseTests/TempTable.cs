///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.TempTable
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
using NUnit.Framework;
using EseObjects;

namespace Test.DatabaseTests
{
	[TestFixture]
	class TempTable
	{
		[Test]
		public static void Test()
		{
			using(var tr = new Transaction(E.S))
			{
				var cto = Table.CreateTempOptions.NewWithLists();

				cto.Columns.Add(new Column.CreateOptions{Type = Column.Type.Long, TTKey = true});

				Column[] cols;

				using(var csr = Table.CreateTemp(E.S, cto, out cols))
				{
					using(var upd = csr.BeginInsert())
					{
						upd.Set(cols[0], 3);
						upd.Complete();
					}
					using(var upd = csr.BeginInsert())
					{
						upd.Set(cols[0], 4);
						upd.Complete();
					}

					csr.MoveFirst();
					Assert.That(csr.Retrieve<int>(cols[0]), Is.EqualTo(3));

					csr.Move(1);
					Assert.That(csr.Retrieve<int>(cols[0]), Is.EqualTo(4));

					tr.Commit();
				}
			}
		}
	}
}
