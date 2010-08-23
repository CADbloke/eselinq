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
	class CreateTableTest
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
			}
		}

		[Test]
		public void SpecificColumnErrors()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("ErrorTable");

				nt.Columns.Add(new Column.CreateOptions("ok", Column.Type.Long));
				nt.Columns.Add(new Column.CreateOptions("invalid_name...", Column.Type.Bit));

				try
				{
					Table.Create(E.D, nt).Dispose();
				}
				catch(CreateTableException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errInvalidName"));
					Assert.That(e.SpecificColumnErrors[0].Symbol, Is.EqualTo("JET_errSuccess")); //column ok
					Assert.That(e.SpecificColumnErrors[1].Symbol, Is.EqualTo("JET_errInvalidName")); //column invalid_name...
				}
			}
		}

		[Test]
		public void SpecificIndexErrors()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("ErrorTable");

				nt.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));

				nt.Indexes.Add(Index.CreateOptions.NewPrimary("pk", "+id", true));

				nt.Indexes.Add(new Index.CreateOptions
				{
					Name = "bad_columns",
					KeyColumns = "+not_present"
				});

				try
				{
					Table.Create(E.D, nt).Dispose();
				}
				catch(CreateTableException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errColumnNotFound"));
					Assert.That(e.SpecificIndexErrors[0].Symbol, Is.EqualTo("JET_errSuccess")); //column pk
					Assert.That(e.SpecificIndexErrors[1].Symbol, Is.EqualTo("JET_errColumnNotFound")); //index bad_columns
				}
			}
		}

		[Test]
		public void NoColumnsOrIndexes()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = new Table.CreateOptions
				{
					Name = "NoColmns"
				};

				using(var tab = Table.Create(E.D, nt))
				{
					Assert.That(tab.Columns.Count, Is.EqualTo(0));
					Assert.That(tab.Indexes.Length, Is.EqualTo(1)); //one implicitly created index
				}
			}
		}

		[Test]
		public void CreatedWithName()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				using(var tab = Table.Create(E.D, nt))
					Assert.That(tab.Name, Is.EqualTo("TestTable"));
			}
		}

		[Test]
		public void DefaultColumnValue()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("DefaultColumnValue");

				nt.Columns.Add(new Column.CreateOptions
				{
					Name = "defaulted",
					Type = Column.Type.Text,
					CP = Column.CodePage.English,
					DefaultValue = "foobarbaz"
				});

				using(var tab = Table.Create(E.D, nt))
				using(var csr = new Cursor(tab))
				{
					using(var u = csr.BeginInsert())
						u.Complete(); //everything as default

					csr.MoveFirst();
					Assert.That(csr["defaulted"], Is.EqualTo("foobarbaz"));
				}
			}
		}

		[Test]
		public void ColumnsReturned()
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

				nt.Indexes.Add(Index.CreateOptions.NewPrimary("pk", "+text", true));

				Column[] cols;
				Index[] ixs;

				using(var tab = Table.Create(E.D, nt, out cols, out ixs))
				{
					Assert.That(cols[0].Name, Is.EqualTo("text"));
					Assert.That(cols[0].ColumnType, Is.EqualTo(Column.Type.Text));
					Assert.That(cols[0].CP, Is.EqualTo(Column.CodePage.Unicode));
					Assert.That(cols[0].Fixed, Is.EqualTo(true));
					Assert.That(cols[0].MaxLength, Is.EqualTo(32));
					Assert.That(cols[0].NotNull, Is.EqualTo(true));

					Assert.That(cols[1].Name, Is.EqualTo("tagged"));
					Assert.That(cols[1].ColumnType, Is.EqualTo(Column.Type.DateTime));
					Assert.That(cols[1].Tagged, Is.EqualTo(true));
					Assert.That(cols[1].MultiValued, Is.EqualTo(true));

					Assert.That(cols[2].Name, Is.EqualTo("auto_int"));
					Assert.That(cols[2].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[2].Autoincrement, Is.EqualTo(true));

					Assert.That(cols[3].Name, Is.EqualTo("escrow_int"));
					Assert.That(cols[3].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[3].EscrowUpdate, Is.EqualTo(true));

					Assert.That(cols[4].Name, Is.EqualTo("version_int"));
					Assert.That(cols[4].ColumnType, Is.EqualTo(Column.Type.Long));
					Assert.That(cols[4].Version, Is.EqualTo(true));
				}
			}
		}

		[Test]
		public void IndexShortStruct()
		{
			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				nt.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));
				nt.Indexes.Add(Index.CreateOptions.NewPrimary("pk", "+id", true));

				Column[] cols;
				Index[] ixs;

				using(var tab = Table.Create(E.D, nt, out cols, out ixs))
				{
					Assert.That(ixs[0].IndexName, Is.EqualTo("pk"));
					Assert.That(ixs[0].ColumnCount, Is.EqualTo(1));
					
					var kcols = new List<Index.KeyColumn>();
					foreach(var kcol in ixs[0].KeyColumns)
						kcols.Add(kcol);

					Assert.That(kcols.Count, Is.EqualTo(1));
					Assert.That(kcols[0].Name, Is.EqualTo("id"));
				}
			}
		}

		[Test]
		public void IndexLongStruct()
		{
			if(E.I.VersionMajor < 6)
				Assert.Ignore("Needs 6.0+");

			using(var tr = new Transaction(E.S))
			{
				var nt = Table.CreateOptions.NewWithLists("TestTable");

				nt.Columns.Add(new Column.CreateOptions("text", Column.Type.Text));

				var ixcreate = Index.CreateOptions.NewSecondary("tuple", "+text", false);
				ixcreate.TupleLimits = new Index.TupleLimits
				{
					CharStart = 1,
					CharsToIncrement = 1,
					CharsToIndexMax = 1000,
					LengthMax = 10,
					LengthMin = 3
				};

				nt.Indexes.Add(ixcreate);

				Column[] cols;
				Index[] ixs;

				using(var tab = Table.Create(E.D, nt, out cols, out ixs))
				{
					Assert.That(ixs[0].IndexName, Is.EqualTo("tuple"));
					Assert.That(ixs[0].ColumnCount, Is.EqualTo(1));

					var kcols = new List<Index.KeyColumn>();
					foreach(var kcol in ixs[0].KeyColumns)
						kcols.Add(kcol);

					Assert.That(kcols.Count, Is.EqualTo(1));
					Assert.That(kcols[0].Name, Is.EqualTo("text"));
				}
			}
		}

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
	}
}
