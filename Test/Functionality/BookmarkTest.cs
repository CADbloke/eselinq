///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.BookmarkTest
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
	class BookmarkTest
	{
		public static void Test(Session sess, Database db, TestData td)
		{
			//save a bookmark and then seek to it it
			using(var tr = Transaction.BeginReadonly(sess))
			{
				Bookmark bkmk1;
				byte[] bkmk1bytes;

				int OrderID;

				using(var c = new Cursor(td.Order))
				{
					c.MoveFirst();
					c.Move(1);
					OrderID = c.Retrieve<int>(td.OrderID);

					bkmk1 = new Bookmark(c);
					bkmk1bytes = bkmk1.ToByteArray();
				}

				using(var c = new Cursor(td.Order))
				{
					c.Seek(bkmk1);
					if(c.Retrieve<int>(td.OrderID) != OrderID)
						throw new Exception("Bookmark seek test failed");
				}

				using(var c = new Cursor(td.Order))
				{
					Bookmark fromBin = new Bookmark(bkmk1bytes);
					c.Seek(fromBin);
					if(c.Retrieve<int>(td.OrderID) != OrderID)
						throw new Exception("Bookmark seek from binary test failed");
				}
			}

			//create an auxillary relation of tagged saved bookmarks, retrieve them
			using(var tr = new Transaction(sess))
			{
				//add a temp column that will be implicitly rolled back later
				var cco = new Column.CreateOptions
				{
					Name = "Lines",
					Type = Column.Type.Binary,
					Tagged = true
				};

				var Lines = Column.Create(td.Order, cco);

				//set up fields
				using(var order = new Cursor(td.Order))
				{
					using(var order_line = new Cursor(td.OrderLine))
					{
						bool has_current = order_line.MoveFirst();

						while(has_current)
						{
							order.Seek(td.OrderID, order_line.Retrieve<int>(td.OrderLineOrder));
							using(var u = order.BeginReplace())
							{
								IWriteRecord.SetOptions so = new IWriteRecord.SetOptions {TagSequence = 0};
								u.Set(Lines, new Bookmark(order_line).ToByteArray(), so);
								u.Complete();
							}
							has_current = order_line.Move(1);
						}
					}
				}

				//locate corresponding fields
				using(var order = new Cursor(td.Order))
				{
					using(var order_line = new Cursor(td.OrderLine))
					{
						bool has_current = order.MoveFirst();

						Console.WriteLine("Bookmark tag test");
						//correct results based on test data. 4 prefix spaces indiciate a detail record.
						string[] reference = { "2025", "    0", "    1", "    2", "2026", "2027", "    0", "3063", "    0", "3095", "    0" };
						int ref_ix = 0;

						while(has_current)
						{
							Console.WriteLine(order.Retrieve<int>(td.OrderID));
							if(reference[ref_ix] != order.Retrieve<int>(td.OrderID).ToString())
								throw new ApplicationException("Discrepancy in bookmark tag test results");
							ref_ix++;

							foreach(byte[] bkmk_bytes in order.RetrieveAllValues<byte []>(Lines))
							{
								var bkmk = new Bookmark(bkmk_bytes);
								order_line.Seek(bkmk);

								string prt_str = "    " + order_line.Retrieve<int>(td.OrderLineSeq);
								Console.WriteLine(prt_str);

								if(reference[ref_ix] != prt_str)
									throw new ApplicationException("Discrepancy in bookmark tag test results");
								ref_ix++;
							}

							has_current = order.Move(1);
						}
					}
				}
			}

			//create a secondary bookmark and seek to it
			using(var tr = new Transaction(sess))
			{
				using(var cust = new Cursor(td.Customer))
				{
					cust.CurrentIndex = td.CustomerNameIx;

					cust.Seek(td.CustomerName, "Judy");

					if(cust.Retrieve<int>(td.CustomerID) != 101)
						throw new ApplicationException("Unexpected seek results");

					var judy = new SecondaryBookmark(cust);
					var judy_pk = new Bookmark(cust);

					cust.MoveFirst();

					cust.Seek(new CombinedBookmarkPosition(judy_pk, judy));

					if(cust.Retrieve<int>(td.CustomerID) != 101)
						throw new ApplicationException("Unexpected seek results after secondary bookmark seek");

					cust.Seek(judy);

					if(cust.Retrieve<int>(td.CustomerID) != 101)
						throw new ApplicationException("Unexpected seek results after secondary bookmark seek");
				}
			}
		}
	}
}
