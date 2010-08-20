///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.BasicTestData
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
	public abstract class WithBasicData
	{
		public Table Order;
		public Column OrderID;
		public Column OrderOpenDate;
		public Column OrderCustomer;
		public Index OrderIx;
		public Index OrderOpenDateIx;
		public Index OrderCustomerIx;

		public Table Customer;
		public Column CustomerID;
		public Column CustomerName;
		public Index CustomerIx;
		public Index CustomerNameIx;

		public Table OrderLine;
		public Column OrderLineOrder;
		public Column OrderLineSeq;
		public Column OrderLineDesc;
		public Column OrderLineAmount;
		public Index OrderLineIx;
		public Index OrderLineDescIx;

		[TestFixtureSetUp]
		public void Setup()
		{
			CreateTables();
			InsertTestData();
		}

		public void CreateTables()
		{
			using(var tr = new Transaction(E.S))
			{
				Column[] newcols;
				Index[] newixs;

				var tco = Table.CreateOptions.NewWithLists("Order");
				tco.Columns.Add(new Column.CreateOptions("ID", Column.Type.Long));
				tco.Columns.Add(new Column.CreateOptions("OpenDate", Column.Type.DateTime));
				tco.Columns.Add(new Column.CreateOptions("Customer", Column.Type.Long));

				var ixco = Index.CreateOptions.NewPrimary("PK", "+ID", true);
				tco.Indexes.Add(ixco);

				ixco = Index.CreateOptions.NewSecondary("OpenDateIx", "-OpenDate", false);
				tco.Indexes.Add(ixco);

				ixco = Index.CreateOptions.NewSecondary("CustomerIx", "+Customer", false);
				tco.Indexes.Add(ixco);

				Order = Table.Create(E.D, tco, out newcols, out newixs);
				OrderID = newcols[0];
				OrderOpenDate = newcols[1];
				OrderCustomer = newcols[2];
				OrderIx = newixs[0];
				OrderOpenDateIx = newixs[1];
				OrderCustomerIx = newixs[2];

				new Index(Order, "OpenDateIx");


				tco = Table.CreateOptions.NewWithLists("Customer");
				tco.Columns.Add(new Column.CreateOptions("ID", Column.Type.Long));
				tco.Columns.Add(new Column.CreateOptions("Name", Column.Type.Text, Column.CodePage.Unicode));

				ixco = Index.CreateOptions.NewPrimary("PK", "+ID", true);
				tco.Indexes.Add(ixco);

				ixco = Index.CreateOptions.NewSecondary("Name", "+Name", true);
				tco.Indexes.Add(ixco);

				Customer = Table.Create(E.D, tco, out newcols, out newixs);
				CustomerID = newcols[0];
				CustomerName = newcols[1];
				CustomerIx = newixs[0];
				CustomerNameIx = newixs[1];

				tco = Table.CreateOptions.NewWithLists("OrderLine");
				tco.Columns.Add(new Column.CreateOptions("Order", Column.Type.Long));
				tco.Columns.Add(new Column.CreateOptions("Seq", Column.Type.Long));
				tco.Columns.Add(new Column.CreateOptions("Desc", Column.Type.LongText, Column.CodePage.English));
				tco.Columns.Add(new Column.CreateOptions("Amount", Column.Type.Currency));

				ixco = Index.CreateOptions.NewPrimary("PK", "+Order.+Seq", true);
				tco.Indexes.Add(ixco);

				ixco = new Index.CreateOptions
				{
					Name = "Desc",
					Tuples = true,
					KeyColumns = "+Desc"
				};
				tco.Indexes.Add(ixco);

				OrderLine = Table.Create(E.D, tco, out newcols, out newixs);
				OrderLineOrder = newcols[0];
				OrderLineSeq = newcols[1];
				OrderLineDesc = newcols[2];
				OrderLineAmount = newcols[3];
				OrderLineIx = newixs[0];
				OrderLineDescIx = newixs[1];

				tr.Commit();
			}
		}

		public void InsertTestData()
		{
			using(var tr = new Transaction(E.S))
			{
				using(var c = new Cursor(Customer))
				{
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 100);
						u.Set(CustomerName, "Bob");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 103);
						u.Set(CustomerName, "Billie");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 104);
						u.Set(CustomerName, "Bub");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 105);
						u.Set(CustomerName, "Barb");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 106);
						u.Set(CustomerName, "Belle");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 101);
						u.Set(CustomerName, "Judy");
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(CustomerID, 102);
						u.Set(CustomerName, "Tom");
						u.Complete();
					}
				}

				using(var c = new Cursor(Order))
				{
					using(var u = c.BeginInsert())
					{
						u.Set(OrderID, 2025);
						u.Set(OrderCustomer, 100);
						u.Set(OrderOpenDate, new DateTime(2009, 10, 13));
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderID, 2026);
						u.Set(OrderCustomer, 100);
						u.Set(OrderOpenDate, new DateTime(2009, 10, 15));
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderID, 2027);
						u.Set(OrderCustomer, 100);
						u.Set(OrderOpenDate, new DateTime(2009, 11, 1));
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderID, 3063);
						u.Set(OrderCustomer, 101);
						u.Set(OrderOpenDate, new DateTime(2009, 11, 1));
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderID, 3095);
						u.Set(OrderCustomer, 102);
						u.Set(OrderOpenDate, new DateTime(2009, 11, 12));
						u.Complete();
					}
				}

				using(var c = new Cursor(OrderLine))
				{
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 2025);
						u.Set(OrderLineSeq, 0);
						u.Set(OrderLineDesc, "Silver hammer with claw");
						u.Set(OrderLineAmount, 1495);
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 2025);
						u.Set(OrderLineSeq, 1);
						u.Set(OrderLineDesc, "Galvanized nails x100");
						u.Set(OrderLineAmount, 350);
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 2025);
						u.Set(OrderLineSeq, 2);
						u.Set(OrderLineDesc, "3mm staples x80");
						u.Set(OrderLineAmount, 200);
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 2027);
						u.Set(OrderLineSeq, 0);
						u.Set(OrderLineDesc, "Corner support 3x5");
						u.Set(OrderLineAmount, 85);
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 3063);
						u.Set(OrderLineSeq, 0);
						u.Set(OrderLineDesc, "RG37 100ft");
						u.Set(OrderLineAmount, 4535);
						u.Complete();
					}
					using(var u = c.BeginInsert())
					{
						u.Set(OrderLineOrder, 3095);
						u.Set(OrderLineSeq, 0);
						u.Set(OrderLineDesc, "Vulcanized nails x100");
						u.Set(OrderLineAmount, 35);
						u.Complete();
					}
				}

				tr.Commit();
			}
		}

		[TestFixtureTearDown]
		public void Teardown()
		{
			using(var tr = new Transaction(E.S))
			{
				Order.Dispose();
				Customer.Dispose();
				OrderLine.Dispose();

				Table.Delete(E.D, "Order");
				Table.Delete(E.D, "Customer");
				Table.Delete(E.D, "OrderLine");

				tr.Commit();
			}
		}
	}
}