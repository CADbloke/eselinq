///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.TableTest
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
	class TableTest
	{
		[Test]
		public void DeleteTable()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = new Table.CreateOptions
				{
					Name = "DeleteMe"
				};

				Table.Create(E.D, nt).Dispose();
				Table.Delete(E.D, "DeleteMe");

				try
				{
					new Table(E.D, "DeleteMe").Dispose();
				}
				catch(EseException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errObjectNotFound"));
				}
			}
		}

		[Test]
		public void ReadTableName()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				using(var tab = Table.Create(E.D, nt))
					Assert.That(tab.Name, Is.EqualTo("TestTable"));
			}
		}

		[Test]
		public void ChangeTableName()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = new Table.CreateOptions
				{
					Name = "OldName"
				};

				using(var tab = Table.Create(E.D, nt))
				{
					tab.Name = "NewName";
				}

				//try to open by old name
				try
				{
					new Table(E.D, "OldName").Dispose();
				}
				catch(EseException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errObjectNotFound"));
				}

				//open by new name
				new Table(E.D, "NewName").Dispose();
			}
		}

		[Test]
		public void DatabaseProperty()
		{
			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, new Table.CreateOptions{Name = "empty"}))
				Assert.That(tab.Database, Is.SameAs(E.D));
		}

		[Test]
		public void SessionProperty()
		{
			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, new Table.CreateOptions{Name = "empty"}))
				Assert.That(tab.Session, Is.SameAs(E.S));
		}

		[Test]
		public void BridgeProperty()
		{
			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, new Table.CreateOptions{Name = "empty"}))
				Assert.That(tab.Bridge, Is.SameAs(E.I.Bridge));
		}

		[Test]
		public void InfoProperties()
		{
			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, new Table.CreateOptions{Name = "empty"}))
			{
				Assert.That(tab.Bookmarkable, Is.EqualTo(true));
				Assert.That(tab.Rollbackable, Is.EqualTo(true));
				Assert.That(tab.Updatable, Is.EqualTo(true));
				Assert.That(tab.SystemTable, Is.EqualTo(false));
				Assert.That(tab.Derived, Is.EqualTo(false));
				Assert.That(tab.FixedDDL, Is.EqualTo(false));
				Assert.That(tab.ObjectTableNoFixedVarColumnsInDerivedTables, Is.EqualTo(false));
				Assert.That(tab.TemplateTable, Is.EqualTo(false));
				Assert.That(tab.RecordCount, Is.EqualTo(0));
				Assert.That(tab.PageCount, Is.EqualTo(0));
				Assert.That(tab.TargetDensity, Is.GreaterThan(0));
				Assert.That(tab.SpaceAvailaible, Is.EqualTo(0));
				Assert.That(tab.OwnedExtents, Is.GreaterThan(0));
			}
		}

		[Test]
		public void ColumnsProperty()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "text",
					Type = Column.Type.Text,
					CP = Column.CodePage.Unicode,
					Fixed = true,
					MaxLength = 32,
					NotNull = true
				});

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "tagged",
					Type = Column.Type.DateTime,
					Tagged = true,
					MultiValued = true
				});

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "auto_int",
					Type = Column.Type.Long,
					Autoincrement = true
				});

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "escrow_int",
					Type = Column.Type.Long,
					EscrowUpdate = true,
					DefaultValue = 0
				});

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "version_int",
					Type = Column.Type.Long,
					Version = true
				});

				nt.Indexes.Add(Index.CreateOptions.NewPrimary("pk", "+text"));

				using(var tab = Table.Create(E.D, nt))
				{
					var cols = new List<Column>(tab.Columns);

					//returned in a specific order
					Assert.That(cols[0].Name, Is.EqualTo("text"));
					Assert.That(cols[0].ColumnType, Is.EqualTo(Column.Type.Text));
					Assert.That(cols[0].CP, Is.EqualTo(Column.CodePage.Unicode));
					Assert.That(cols[0].Fixed, Is.EqualTo(true));
					Assert.That(cols[0].MaxLength, Is.EqualTo(32));
					Assert.That(cols[0].NotNull, Is.EqualTo(true));

					Assert.That(cols[1].Name, Is.EqualTo("auto_int"));
					Assert.That(cols[1].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[1].Autoincrement, Is.EqualTo(true));

					Assert.That(cols[2].Name, Is.EqualTo("escrow_int"));
					Assert.That(cols[2].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[2].EscrowUpdate, Is.EqualTo(true));

					Assert.That(cols[3].Name, Is.EqualTo("version_int"));
					Assert.That(cols[3].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[3].Version, Is.EqualTo(true));

					Assert.That(cols[4].Name, Is.EqualTo("tagged"));
					Assert.That(cols[4].ColumnType, Is.EqualTo(Column.Type.DateTime));
					Assert.That(cols[4].Tagged, Is.EqualTo(true));
					Assert.That(cols[4].MultiValued, Is.EqualTo(true));
				}
			}
		}

		[Test]
		public void IndexesProperty()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				nt.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));
				nt.Columns.Add(new Column.CreateOptions("string", Column.Type.Text, Column.CodePage.English));

				nt.Indexes.Add(Index.CreateOptions.NewPrimary("pk", "+id"));
				nt.Indexes.Add(Index.CreateOptions.NewSecondary("string_first", "+string.-id", false));
				nt.Indexes.Add(new Index.CreateOptions
				{
					Name = "tuple",
					KeyColumns = "+string",
					Tuples = true
				});

				using(var tab = Table.Create(E.D, nt))
				{
					var ixs = new List<Index>(tab.Indexes);
					List<Index.KeyColumn> kcols;

					Assert.That(ixs[0].IndexName, Is.EqualTo("pk"));
					Assert.That(ixs[0].ColumnCount, Is.EqualTo(1));
					Assert.That(ixs[0].Primary, Is.EqualTo(true));
					Assert.That(ixs[0].Unique, Is.EqualTo(true));
					kcols = new List<Index.KeyColumn>(ixs[0].KeyColumns);
						Assert.That(kcols.Count, Is.EqualTo(1));
						Assert.That(kcols[0].Name, Is.EqualTo("id"));
						Assert.That(kcols[0].Type, Is.EqualTo(Column.Type.Long));
						Assert.That(kcols[0].SortDescending, Is.EqualTo(false));
					
					Assert.That(ixs[1].IndexName, Is.EqualTo("string_first"));
					Assert.That(ixs[1].Primary, Is.EqualTo(false));
					Assert.That(ixs[1].Unique, Is.EqualTo(false));
					Assert.That(ixs[1].ColumnCount, Is.EqualTo(2));
					kcols = new List<Index.KeyColumn>(ixs[1].KeyColumns);
						Assert.That(kcols.Count, Is.EqualTo(2));
						Assert.That(kcols[0].Name, Is.EqualTo("string"));
						Assert.That(kcols[0].Type, Is.EqualTo(Column.Type.Text));
						Assert.That(kcols[0].CP, Is.EqualTo(Column.CodePage.English));
						Assert.That(kcols[0].SortDescending, Is.EqualTo(false));
						Assert.That(kcols[1].Name, Is.EqualTo("id"));
						Assert.That(kcols[1].Type, Is.EqualTo(Column.Type.Long));
						Assert.That(kcols[1].SortDescending, Is.EqualTo(true));
					
					Assert.That(ixs[2].IndexName, Is.EqualTo("tuple"));
					Assert.That(ixs[2].ColumnCount, Is.EqualTo(1));
					Assert.That(ixs[2].TupleIndex, Is.EqualTo(true));
					kcols = new List<Index.KeyColumn>(ixs[2].KeyColumns);
						Assert.That(kcols.Count, Is.EqualTo(1));
						Assert.That(kcols[0].Name, Is.EqualTo("string"));
						Assert.That(kcols[0].Type, Is.EqualTo(Column.Type.Text));
						Assert.That(kcols[0].CP, Is.EqualTo(Column.CodePage.English));
						Assert.That(kcols[0].SortDescending, Is.EqualTo(false));					
				}
			}
		}
	}
}