///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.BasicTestData.Linq.BasicLinq
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
using System.Linq;

using NUnit.Framework;

using EseObjects;
using EseLinq;

namespace Test.DatabaseTests.Linq
{
	public struct ABC2
	{
		public int a;
		public float b;
		public string c;
	}

	class BasicLinq
	{
		public static void RunTests()
		{
			using(var tr = new Transaction(E.S))
			{
				Column[] col;
				Index[] ix;

				var tab = Table.Create(E.D, new Table.CreateOptions
				{
					Name = "ABC",
					Columns = new Column.CreateOptions[]
					{
						new Column.CreateOptions("a", Column.Type.Long),
						new Column.CreateOptions("b", Column.Type.SingleFloat),
						new Column.CreateOptions("c", Column.Type.LongText, Column.CodePage.English) 
					},
					Indexes = new Index.CreateOptions[]
					{
						Index.CreateOptions.NewPrimary("PK", "+a")
					}
				}, out col, out ix);

				var csr = new Cursor(tab);

				using(var u = csr.BeginInsert())
				{
					u.Set(col[0], 3);
					u.Set(col[1], 5.55);
					u.Set(col[2], "foo");
					u.Complete();
				}
				using(var u = csr.BeginInsert())
				{
					u.Set(col[0], 4);
					u.Set(col[1], 4.444);
					u.Set(col[2], "bar");
					u.Complete();
				}
				using(var u = csr.BeginInsert())
				{
					u.Set(col[0], 5);
					u.Set(col[1], 5.55);
					u.Set(col[2], "foo");
					u.Complete();
				}

				Provider pro = new Provider(E.S);
				//Query<ABC2> src = new Query<ABC2>(pro, tab);
				//var src = tab.AsEnumerable<ABC2>().AsQueryable();
				var src = tab.AsQueryable<ABC2>(pro);

				IEnumerable<ABC2> q1 = src.Where(abc => abc.a == 3);

				Console.WriteLine("A: 3");
				foreach(ABC2 abc in q1)
					Console.WriteLine(abc.a);

				var q2 = src.Where<ABC2>(abc => abc.a == 3).Select<ABC2, int>(abc => abc.a);

				Console.WriteLine("B: 3");
				foreach(int x in q2)
					Console.WriteLine(x);

				var q3 = from abc in src
						 where abc.a == 3
						 select abc.b;

				Console.WriteLine("C 5.55");
				foreach(var x in q3)
					Console.WriteLine(x);

				var q4 = (from abc in src
						  where abc.c == "foo"
						  select abc.b).Distinct<float>();

				Console.WriteLine("D 5.55");
				foreach(var x in q4)
					Console.WriteLine(x);


				var q5 = from n in src
						 from m in src
						 select n.a + m.a;
				Console.WriteLine("E");
				foreach(var x in q5)
					Console.WriteLine(x);

				var q6 = from abc in src
						 select new
						 {
							 abc.a,
							 abc.b
						 };

				Console.WriteLine("F");
				foreach(var x in q6)
					Console.WriteLine("{0} {1}", x.a, x.b);

				var q7 = (from abc in src
						  join abc2 in src on abc.a equals abc2.a
						  select new
						  {
							  abc.a,
							  abc.b,
							  abc.c,
							  a2 = abc2.a,
							  b2 = abc2.b,
							  c2 = abc2.c
						  });

				//Console.WriteLine("G");
				//foreach(var x in q7)
				//    Console.WriteLine("{0} {1} {2} {3} {4} {5}", x.a, x.b, x.c, x.a2, x.b2, x.c2);

				//var q8a = (from abc in src.AsQueryable()
				//          orderby abc.a
				//          select abc);

				//Console.WriteLine("H");
				//foreach(var x in q8a)
				//    Console.WriteLine(x.a);

				//var q8b = (from abc in src
				//          orderby abc.a, abc.b
				//          select abc);

				//Console.WriteLine("H");
				//foreach(var x in q8b)
				//    Console.WriteLine("{0} {1} {2}", x.a, x.b, x.c);
			}
		}
	}
}
