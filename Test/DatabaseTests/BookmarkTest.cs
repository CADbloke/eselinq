///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.BookmarkTest
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
	public class BookmarkTest : WithBasicData
	{
		[Test]
		public void ScrollToCreateBookmarkThenSeek([Range(0, 4)] int scroll_to)
		{
			using(var tr = Transaction.BeginReadonly(E.S))
			{
				Bookmark bkmk1;

				int order_id;

				using(var c = new Cursor(Order))
				{
					c.MoveFirst();
					c.Move(scroll_to);
					order_id = c.Retrieve<int>(OrderID);

					bkmk1 = new Bookmark(c);
				}

				using(var c = new Cursor(Order))
				{
					Assert.That(c.Seek(bkmk1), Is.True);
					Assert.That(c.Retrieve<int>(OrderID), Is.EqualTo(order_id));
				}
			}
		}

		[Test]
		public void ScrollToCreateBoomarkThroughByteArrayThenSeek([Range(0, 4)] int scroll_to)
		{
			using(var tr = Transaction.BeginReadonly(E.S))
			{
				byte[] bkmk1bytes;

				int order_id;

				//scroll, save
				using(var c = new Cursor(Order))
				{
					c.MoveFirst();
					c.Move(scroll_to);
					order_id = c.Retrieve<int>(OrderID);

					var bkmk1 = new Bookmark(c);
					bkmk1bytes = bkmk1.ToByteArray();
				}

				//restore, seek
				using(var c = new Cursor(Order))
				{
					Bookmark fromBin = new Bookmark(bkmk1bytes);
					Assert.That(c.Seek(fromBin), Is.True);
					Assert.That(c.Retrieve<int>(OrderID), Is.EqualTo(order_id));
				}
			}
		}

		[Test]
		public void ScrollToCreateBoomarkThroughBinaryColumnAndInternalBridgeThenSeek([Range(0, 4)] int scroll_to)
		{
			Column[] storage_columns;

			var storage_def = Table.CreateTempOptions.NewWithLists();
			storage_def.ForwardOnlyIfPossible = true;
			storage_def.Columns.Add(new Column.CreateOptions(null, Column.Type.Binary));

			using(var tr = new Transaction(E.S))
			using(var storage = Table.CreateTemp(E.S, storage_def, out storage_columns))
			{
				int order_id;

				//scroll, save
				using(var c = new Cursor(Order))
				{
					c.MoveFirst();
					c.Move(scroll_to);
					order_id = c.Retrieve<int>(OrderID);

					var bkmk1 = new Bookmark(c);

					using(var u = storage.BeginInsert())
					{
						u[storage_columns[0]] = bkmk1;
						u.Complete();
					}
				}

				//restore, seek
				using(var c = new Cursor(Order))
				{
					storage.MoveFirst();

					Bookmark fromBin = storage.Retrieve<Bookmark>(storage_columns[0]);
					Assert.That(c.Seek(fromBin), Is.True);
					Assert.That(c.Retrieve<int>(OrderID), Is.EqualTo(order_id));
				}
			}
		}

		[Test]
		public void CreateBookmarkFromInsertThenSeek()
		{
			using(var tr = new Transaction(E.S))
			{
				Bookmark bkmk;

				using(var c = new Cursor(Order))
				using(var u = c.BeginInsert())
				{
					u.Set(OrderID, 5056);
					u.Set(OrderCustomer, 100);
					u.Set(OrderOpenDate, new DateTime(2009, 10, 13));

					bkmk = u.CompleteWithBookmark();
				}

				using(var c = new Cursor(Order))
				{
					Assert.That(c.Seek(bkmk), Is.True);
					Assert.That(c.Retrieve<int>(OrderID), Is.EqualTo(5056));
				}
			}
		}

		[Test]
		public void TestBookmarkRelationBinaryTagColumn()
		{
			//create an auxillary relation of tagged saved bookmarks, retrieve them
			using(var tr = new Transaction(E.S))
			{
				//add a temp column that will be implicitly rolled back later
				var cco = new Column.CreateOptions
				{
					Name = "Lines",
					Type = Column.Type.Binary,
					Tagged = true
				};

				var lines = Column.Create(Order, cco);

				//set up fields
				using(var order = new Cursor(Order))
				{
					using(var order_line = new Cursor(OrderLine))
					{
						bool has_current = order_line.MoveFirst();

						while(has_current)
						{
							order.Seek(OrderID, order_line.Retrieve<int>(OrderLineOrder));
							using(var u = order.BeginReplace())
							{
								IWriteRecord.SetOptions so = new IWriteRecord.SetOptions
								{
									TagSequence = 0
								};
								u.Set(lines, new Bookmark(order_line), so);
								u.Complete();
							}
							has_current = order_line.Move(1);
						}
					}
				}

				//locate corresponding fields
				using(var order = new Cursor(Order))
				{
					using(var order_line = new Cursor(OrderLine))
					{
						bool has_current = order.MoveFirst();

						Console.WriteLine("Bookmark tag test");
						//correct results based on test data. 4 prefix spaces indiciate a detail record.
						string[] reference = { "2025", "    0", "    1", "    2", "2026", "2027", "    0", "3063", "    0", "3095", "    0" };
						int ref_ix = 0;

						while(has_current)
						{
							Console.WriteLine(order.Retrieve<int>(OrderID));
							Assert.That(reference[ref_ix], Is.EqualTo(order.Retrieve<int>(OrderID).ToString()), "Discrepancy in bookmark tag test results");
							ref_ix++;

							foreach(var bkmk in order.RetrieveAllValues<Bookmark>(lines))
							{
								order_line.Seek(bkmk);

								string prt_str = "    " + order_line.Retrieve<int>(OrderLineSeq);
								Console.WriteLine(prt_str);

								Assert.That(reference[ref_ix], Is.EqualTo(prt_str));

								ref_ix++;
							}

							has_current = order.Move(1);
						}
					}
				}
			}
		}
	}
}
