///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.KeyTest
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

using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;

namespace Test.Functionality
{
	class KeyTest
	{
		public static void Fail()
		{
			throw new ApplicationException("Key test failure.");
		}

		public static void ScrollOrderLines2025(Cursor c, TestData td)
		{
			if(c.Retrieve<string>(td.OrderLineDesc) != "Silver hammer with claw")
				Fail();
			if(!c.Move(1))
				Fail();
			if(c.Retrieve<string>(td.OrderLineDesc) != "Galvanized nails x100")
				Fail();
			if(!c.Move(1))
				Fail();
			if(c.Retrieve<string>(td.OrderLineDesc) != "3mm staples x80")
				Fail();
			if(c.Move(1)) //should be past end
				Fail();
		}

		public static void Test(Session sess, Database db, TestData td)
		{
			using(var tr = new Transaction(sess))
			{
				using(var ol = new Cursor(td.OrderLine))
				{
					bool currency;

					//full key seek
					Key k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025), new Field(td.OrderLineSeq, 1) });
					currency = ol.Seek(k);
					string s = ol.Retrieve<string>(td.OrderLineDesc);
					if(ol.Retrieve<string>(td.OrderLineDesc) != "Galvanized nails x100")
						Fail();

					//direct seek
					currency = ol.Seek(td.OrderLineOrder, 2025, td.OrderLineSeq, 2);
					if(ol.Retrieve<string>(td.OrderLineDesc) != "3mm staples x80")
						Fail();

					//first line in order 2025
					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardStart);
					currency = ol.Seek(new KeyPosition(k, SeekRel.GE));
					if(ol.Retrieve<int>(td.OrderLineSeq) != 0)
						Fail();

					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025) });
					currency = ol.Seek(new KeyPosition(k, SeekRel.GE));
					if(ol.Retrieve<int>(td.OrderLineSeq) != 0)
						Fail();


					//2nd line in 2025
					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025), new Field(td.OrderLineSeq, 0) });
					currency = ol.Seek(new KeyPosition(k, SeekRel.GT));
					if(ol.Retrieve<int>(td.OrderLineSeq) != 1)
						Fail();


					//last line in order 2025
					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardEnd);
					currency = ol.Seek(new KeyPosition(k, SeekRel.LE));
					if(ol.Retrieve<int>(td.OrderLineSeq) != 2)
						Fail();

					//using fields
					currency = ol.Seek(td.OrderLineOrder, 2025, Match.WildcardEnd, SeekRel.LE);
					if(ol.Retrieve<int>(td.OrderLineSeq) != 2)
						Fail();

					//with large value instead of wildcard
					currency = ol.Seek(td.OrderLineOrder, 2025, td.OrderLineSeq, int.MaxValue, Match.WildcardEnd, SeekRel.LE);
					if(ol.Retrieve<int>(td.OrderLineSeq) != 2)
						Fail();


					//range of all lines in 2025
					Limitable l1 = new KeyPosition(new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025), new Field(td.OrderLineSeq, 0) }), SeekRel.GE);
					Limitable l2 = new KeyPosition(new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025), new Field(td.OrderLineSeq, 2) }), SeekRel.LE);
					currency = ol.ForwardRange(l1, l2);
					ScrollOrderLines2025(ol, td);

					//same, with automatic wildcards
					Field[] fields = new Field[] { new Field(td.OrderLineOrder, 2025) };
					currency = ol.ForwardRangeInclusive(fields, fields);
					ScrollOrderLines2025(ol, td);

					//try cancel
					currency = ol.ForwardRangeInclusive(fields, fields);
					ol.CancelRange();

					//with explicit wildcards and seek relations
					currency = ol.ForwardRange(
							new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
							new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
					ScrollOrderLines2025(ol, td);

					//same, with discrete calls
					currency = ol.Seek(td.OrderLineOrder, 2025, td.OrderLineSeq, 0);
					currency = ol.SetUpperLimit(new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025), new Field(td.OrderLineSeq, 2) }, Match.WildcardEnd, SeekRel.LE));
					ScrollOrderLines2025(ol, td);

					//via keys
					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardStart);
					currency = ol.Seek(new KeyPosition(k, SeekRel.GE));
					k = new Key(ol, new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardEnd);
					currency = ol.SetUpperLimit(new KeyPosition(k, SeekRel.LE));
					ScrollOrderLines2025(ol, td);

					//with FieldPositions
					currency = ol.ForwardRange(
						new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
						new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
					ScrollOrderLines2025(ol, td);

					//count
					currency = ol.ForwardRange(
						new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
						new FieldPosition(new Field[] { new Field(td.OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
					if(ol.ForwardRecordCount() != 3)
						Fail();


					//key with null test
					using(var tr2 = new Transaction(sess))
					{
						var nkt = Table.Create(db, new Table.CreateOptions
						{
							Name = "nkt",
							Columns = new Column.CreateOptions[]
							{
								new Column.CreateOptions {Name = "C1", Type = Column.Type.Long},
								new Column.CreateOptions {Name = "C2", Type = Column.Type.Long}
							},
							Indexes = new Index.CreateOptions[]
							{
								Index.CreateOptions.NewPrimary("PK", "+C1.+C2", true)
							}
						});

						var nkcsr = new Cursor(nkt);
						using(var u = nkcsr.BeginInsert())
						{
							u.Set(new Column(nkt, "C1"), 3);
							//implicitly setting C2 to null
							u.Complete();
						}
						using(var u = nkcsr.BeginInsert())
						{
							u.Set(new Column(nkt, "C1"), 3);
							u.Set(new Column(nkt, "C2"), 4);
							u.Complete();
						}
						using(var u = nkcsr.BeginInsert())
						{
							u.Set(new Column(nkt, "C1"), 2);
							u.Set(new Column(nkt, "C2"), 0);
							u.Complete();
						}
						
						nkcsr.MoveFirst();
						//currency = nkcsr.Seek(new Column(nkt, "C1"), 3);
						currency = nkcsr.Seek(new Column(nkt, "C1"), 3, new Column(nkt, "C2"), null);

						Console.Write("Seek with null: ");
						Console.Write(currency);
						Console.Write(" ");
						if(currency)
							Console.WriteLine(nkcsr.Retrieve<int>(new Column(nkt, "C1")));
						else
							Console.WriteLine();
					}
				}
			}
		}
	}
}
