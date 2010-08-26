///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.ColumnTest
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
	class ColumnTest
	{
		[Test]
		public void CreateColumn()
		{
			var tc = Table.CreateOptions.NewWithLists("TestTable");
			tc.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));

			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, tc))
			{
				var col1 = Column.Create(tab, new Column.CreateOptions("string", Column.Type.Text, Column.CodePage.English));

				Assert.That(col1.Name, Is.EqualTo("string"));
				Assert.That(col1.ColumnType, Is.EqualTo(Column.Type.Text));
				Assert.That(col1.CP, Is.EqualTo(Column.CodePage.English));

				var col2 = new Column(tab, "string");

				Assert.That(col2.Name, Is.EqualTo("string"));
				Assert.That(col2.ColumnType, Is.EqualTo(Column.Type.Text));
				Assert.That(col2.CP, Is.EqualTo(Column.CodePage.English));
			}
		}

		[Test]
		public void DeleteColumn()
		{
			var tc = Table.CreateOptions.NewWithLists("TestTable");
			tc.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));
			tc.Columns.Add(new Column.CreateOptions("delete_me", Column.Type.DoubleFloat));

			using(var tr = new Transaction(E.S))
			using(var tab = Table.Create(E.D, tc))
			{
				Column.Delete(tab, "delete_me");

				try
				{
					var col = new Column(tab, "delete_me");
					Assert.Fail("Exception expected");
				}
				catch(EseException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errColumnNotFound"));
				}
			}
		}

		[Test]
		public void RenameColumn()
		{
			if(!E.I.IsVersionAtLeast(5, 1))
				Assert.Ignore("Requries 5.1+");

			var tc = Table.CreateOptions.NewWithLists("RenameColumnTable");
			tc.Columns.Add(new Column.CreateOptions("id", Column.Type.Long));
			tc.Columns.Add(new Column.CreateOptions("old_name", Column.Type.DoubleFloat));

			using(var tab = Table.Create(E.D, tc))
			{
				{
					var col1 = new Column(tab, "old_name");
					col1.RenameColumn(tab, "new_name");
				}

				var col2 = new Column(tab, "new_name");

					try
				{
					var col = new Column(tab, "old_name");
				}
				catch(EseException e)
				{
					Assert.That(e.Symbol, Is.EqualTo("JET_errColumnNotFound"));
				}
			}
		}

		[Test]
		public void ColumnTypes()
		{
			var tc = Table.CreateOptions.NewWithLists("TryAllColumnTypes");
			tc.Columns.Add(new Column.CreateOptions("bit", Column.Type.Bit));
			tc.Columns.Add(new Column.CreateOptions("ubyte", Column.Type.UnsignedByte));
			tc.Columns.Add(new Column.CreateOptions("short", Column.Type.Short));
			tc.Columns.Add(new Column.CreateOptions("long", Column.Type.Long));
			tc.Columns.Add(new Column.CreateOptions("currency", Column.Type.Currency));
			tc.Columns.Add(new Column.CreateOptions("singlefloat", Column.Type.SingleFloat));
			tc.Columns.Add(new Column.CreateOptions("doublefloat", Column.Type.DoubleFloat));
			tc.Columns.Add(new Column.CreateOptions("datetime", Column.Type.DateTime));
			tc.Columns.Add(new Column.CreateOptions("binary", Column.Type.Binary));
			tc.Columns.Add(new Column.CreateOptions("ascii", Column.Type.Text, Column.CodePage.English));
			tc.Columns.Add(new Column.CreateOptions("ucs", Column.Type.Text, Column.CodePage.Unicode));
			tc.Columns.Add(new Column.CreateOptions("longbinary", Column.Type.LongBinary));
			tc.Columns.Add(new Column.CreateOptions("longtext", Column.Type.LongText));

			if(E.I.IsVersionAtLeast(6))
			{
				tc.Columns.Add(new Column.CreateOptions("ulong", Column.Type.UnsignedLong));
				tc.Columns.Add(new Column.CreateOptions("longlong", Column.Type.LongLong));
				tc.Columns.Add(new Column.CreateOptions("guid", Column.Type.GUID));
				tc.Columns.Add(new Column.CreateOptions("ushort", Column.Type.UnsignedShort));
			}

			using(var tr = new Transaction(E.S))
				Table.Create(E.D, tc).Dispose();
		}
	}
}