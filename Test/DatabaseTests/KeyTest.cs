///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.KeyTest
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
	public class KeyTest : WithBasicData
	{
		public void ScrollOrderLines2025(Cursor c)
		{
			Assert.That(c.Retrieve<string>(OrderLineDesc), Is.EqualTo("Silver hammer with claw"));
			Assert.That(c.Move(1), Is.True);
			Assert.That(c.Retrieve<string>(OrderLineDesc), Is.EqualTo("Galvanized nails x100"));
			Assert.That(c.Move(1), Is.True);
			Assert.That(c.Retrieve<string>(OrderLineDesc), Is.EqualTo("3mm staples x80"));
			Assert.That(c.Move(1), Is.False); //should be past end
		}

		[Test]
		public void FullKeySeek()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025), new Field(OrderLineSeq, 1) });
				Assert.That(ol.Seek(k));
				Assert.That(ol.Retrieve<string>(OrderLineDesc), Is.EqualTo("Galvanized nails x100"));
			}
		}

		[Test]
		public void DirectSeek()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Assert.That(ol.Seek(OrderLineOrder, 2025, OrderLineSeq, 2));
				if(ol.Retrieve<string>(OrderLineDesc) != "3mm staples x80")
					Assert.Fail();
			}
		}

		[Test]
		public void FirstLineInOrder2025ByKeyPositionWildcard()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardStart);
				Assert.That(ol.Seek(new KeyPosition(k, SeekRel.GE)));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(0));
			}
		}

		[Test]
		public void FirstLineInOrder2025ByKeyPositionExact()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025) });
				Assert.That(ol.Seek(new KeyPosition(k, SeekRel.GE)));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(0));
			}
		}

		[Test]
		public void SecondLineInOrder2025()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025), new Field(OrderLineSeq, 0) });
				Assert.That(ol.Seek(new KeyPosition(k, SeekRel.GT)));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(1));
			}
		}

		[Test]
		public void LastLineInOrder2025ByWildcardKey()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardEnd);
				Assert.That(ol.Seek(new KeyPosition(k, SeekRel.LE)));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(2));
			}
		}

		[Test]
		public void LastLineInOrder2025ByWildcardDirect()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Assert.That(ol.Seek(OrderLineOrder, 2025, Match.WildcardEnd, SeekRel.LE));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(2));
			}
		}

		[Test]
		public void LastLineInOrder2025ByRelativeLargeValueDirect()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Assert.That(ol.Seek(OrderLineOrder, 2025, OrderLineSeq, int.MaxValue, Match.WildcardEnd, SeekRel.LE));
				Assert.That(ol.Retrieve<int>(OrderLineSeq), Is.EqualTo(2));
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByKeyPositions()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Limitable l1 = new KeyPosition(new Key(ol, new Field[] { new Field(OrderLineOrder, 2025), new Field(OrderLineSeq, 0) }), SeekRel.GE);
				Limitable l2 = new KeyPosition(new Key(ol, new Field[] { new Field(OrderLineOrder, 2025), new Field(OrderLineSeq, 2) }), SeekRel.LE);
				Assert.That(ol.ForwardRange(l1, l2));
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByForwardRangeInclusive()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Field[] fields = new Field[] { new Field(OrderLineOrder, 2025) };
				Assert.That(ol.ForwardRangeInclusive(fields, fields));
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void CancelRange()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Field[] fields = new Field[] { new Field(OrderLineOrder, 2025) };
				ol.ForwardRangeInclusive(fields, fields);
				ol.CancelRange();
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByExplicitWildcardsAndSeekRelations()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				bool has_currency = ol.ForwardRange(
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
				Assert.That(has_currency);
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByExplicitWildcardsAndSeekRelationsWithSeparateCalls()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Assert.That(ol.Seek(OrderLineOrder, 2025, OrderLineSeq, 0));
				Assert.That(ol.SetUpperLimit(new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025), new Field(OrderLineSeq, 2) }, Match.WildcardEnd, SeekRel.LE)));
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByExplicitWildcardsAndSeekRelationsWithSeparateCallsByKey()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				Key k;
				k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardStart);
				Assert.That(ol.Seek(new KeyPosition(k, SeekRel.GE)));
				k = new Key(ol, new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardEnd);
				Assert.That(ol.SetUpperLimit(new KeyPosition(k, SeekRel.LE)));
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void RangeOfAllLinesIn2025ByExplicitWildcardsAndSeekRelationsByFieldPosition()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				bool has_currency = ol.ForwardRange(
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
				Assert.That(has_currency);
				ScrollOrderLines2025(ol);
			}
		}

		[Test]
		public void RangeCountOfAllLinesIn2025ByExplicitWildcardsAndSeekRelationsByFieldPosition()
		{
			using(var tr = new Transaction(E.S))
			using(var ol = new Cursor(OrderLine))
			{
				bool has_currency = ol.ForwardRange(
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardStart, SeekRel.GE),
					new FieldPosition(new Field[] { new Field(OrderLineOrder, 2025) }, Match.WildcardEnd, SeekRel.LE));
				Assert.That(has_currency);
				Assert.That(ol.ForwardRecordCount(), Is.EqualTo(3));
			}
		}
	}

	[TestFixture]
	public class KeyWithNullTests
	{
		Transaction trans;
		Table table;

		[TestFixtureSetUp]
		public void Setup()
		{
			trans = new Transaction(E.S);
			table = Table.Create(E.D, new Table.CreateOptions
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

			var nkcsr = new Cursor(table);
			using(var u = nkcsr.BeginInsert())
			{
				u.Set(new Column(table, "C1"), 3);
				//implicitly setting C2 to null
				u.Complete();
			}
			using(var u = nkcsr.BeginInsert())
			{
				u.Set(new Column(table, "C1"), 3);
				u.Set(new Column(table, "C2"), 4);
				u.Complete();
			}
			using(var u = nkcsr.BeginInsert())
			{
				u.Set(new Column(table, "C1"), 2);
				u.Set(new Column(table, "C2"), 0);
				u.Complete();
			}
		}

		[TestFixtureTearDown]
		public void Teardown()
		{
			trans.Rollback();
		}

		[Test]
		public void NullSeekTest()
		{
			using(var tr = new Transaction(E.S))
			using(var nkcsr = new Cursor(table))
			{
				nkcsr.MoveFirst();
				Assert.That(nkcsr.Seek(new Column(nkcsr, "C1"), 3, new Column(nkcsr, "C2"), null));

				Assert.That(nkcsr.Retrieve<int>(new Column(nkcsr, "C1")), Is.EqualTo(3));
			}
		}
	}
}
