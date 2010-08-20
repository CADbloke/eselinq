///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.BasicTestData.SerializationTest
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
using EseLinq.Storage;

namespace Test.DatabaseTests
{
	[Serializable]
	struct XYZ : IRecordSerializable
	{
		public int x;
		public string y;
		public double z;

		#region IRecordSerializable Members

		public XYZ(IReadRecord rr)
		{
			x = rr.Retrieve<int>(new Column(rr.AsTable, "X"));
			y = rr.Retrieve<string>(new Column(rr.AsTable, "Y"));
			z = rr.Retrieve<double>(new Column(rr.AsTable, "Z"));
		}

		public void WriteToRecord(IWriteRecord wr)
		{
			wr.Set(new Column(wr.Read.AsTable, "X"), x);
			wr.Set(new Column(wr.Read.AsTable, "Y"), y);
			wr.Set(new Column(wr.Read.AsTable, "Z"), z);
		}

		#endregion
	}

	[MemberwiseStorageAttribute]
	public struct ABC
	{
		public int a;
		public string b;
		public double c;

		[TableCreateOptionsAttribute]
		public static readonly Table.CreateOptions TableCreateOptions = new Table.CreateOptions
		{
			Name = "FieldSerial",
			Columns = new Column.CreateOptions[]
			{
				new Column.CreateOptions("A", Column.Type.Long),
				new Column.CreateOptions("B", Column.Type.LongText, Column.CodePage.Unicode),
				new Column.CreateOptions("C", Column.Type.DoubleFloat)
			},
			Indexes = new Index.CreateOptions[]
			{
				new Index.CreateOptions { Name = "PK", KeyColumns = "+A", Unique = true, Primary = true }
			}
		};
	}

	[MemberwiseStorageAttribute]
	[PrimaryIndex("PK", "+d")]
	struct DEF
	{
		public int d;
		[XmlFieldSerializationAttribute]
		public string e;
		[BinaryFieldSerializationAttribute]
		public double f;
		[ExpandFieldAttribute]
		public XYZ xyz;
	}

	[TestFixture]
	class SerializationTest
	{
		[Test]
		public static void RowBinSerial()
		{
			XYZ a, b;
			a.x = 33;
			a.y = "RS232 - 9600 baud";
			a.z = Math.E;

			using(new Transaction(E.S))
			{
				Column[] cols;
				Index[] ixs;
				var BinSerial = Table.Create(E.D, new Table.CreateOptions
				{
					Name = "BinSerial",
					Columns = new Column.CreateOptions[]
					{
						new Column.CreateOptions("K", Column.Type.Long),
						new Column.CreateOptions("Data", Column.Type.LongBinary)
					},
					Indexes = new Index.CreateOptions[]
					{
						new Index.CreateOptions { Name = "PK", KeyColumns = "+K", Unique = true, Primary = true }
					}
				}, out cols, out ixs);

				using(var csr = new Cursor(BinSerial))
				{
					using(var u = csr.BeginInsert())
					{
						u.Set(cols[0], 0);
						u.Set(cols[1], a);
						u.Complete();
					}

					csr.Seek(cols[0], 0);

					b = csr.Retrieve<XYZ>(cols[1]);

					Assert.AreEqual(a.x, b.x);
					Assert.AreEqual(a.y, b.y);
					Assert.AreEqual(a.z, b.z);
				}
			}
		}

		[Test]
		public void FieldBinSerial()
		{
			XYZ a, b;
			a.x = 33;
			a.y = "RS232 - 9600 baud";
			a.z = Math.E;

			using(new Transaction(E.S))
			{
				Column[] cols;
				Index[] ixs;
				var FieldSerial = Table.Create(E.D, new Table.CreateOptions
				{
					Name = "FieldSerial",
					Columns = new Column.CreateOptions[]
					{
						new Column.CreateOptions("X", Column.Type.Long),
						new Column.CreateOptions("Y", Column.Type.LongText, Column.CodePage.Unicode),
						new Column.CreateOptions("Z", Column.Type.DoubleFloat)
					},
					Indexes = new Index.CreateOptions[]
					{
						new Index.CreateOptions { Name = "PK", KeyColumns = "+X", Unique = true, Primary = true }
					}
				}, out cols, out ixs);

				using(var trans_mod = new Transaction(E.S))
				{
					using(var csr = new Cursor(FieldSerial))
					{
						var rle = new Flat<XYZ>(FieldSerial);

						using(var u = csr.BeginInsert())
						{
							rle.Write(u, a);
							u.Complete();
						}

						csr.MoveFirst();
						b = rle.Read(csr);

						Assert.AreEqual(a.x, b.x);
						Assert.AreEqual(a.y, b.y);
						Assert.AreEqual(a.z, b.z);
					}

					trans_mod.Rollback();
				}
			}
		}

		[Test]
		public void FieldwiseFlatTest()
		{
			XYZ a;
			a.x = 33;
			a.y = "RS232 - 9600 baud";
			a.z = Math.E;
	
			using(var trans_mod = new Transaction(E.S))
			{
				DEF d1 = new DEF {d = 5, e = "qx", f = Math.PI, xyz = a};
				DEF d2;
				
				Column[] cols;
				Index[] ixs;
				//var table = Table.Create(sess, db, new Table.CreateOptions
				//{
				//    Name = "FieldSerial",
				//    Columns = new Column.CreateOptions[]
				//    {
				//        new Column.CreateOptions("D", Column.Type.Long),
				//        new Column.CreateOptions("E", Column.Type.LongText, Column.CodePage.Unicode),
				//        //new Column.CreateOptions("E", Column.Type.LongText, Column.CodePage.English),
				//        //new Column.CreateOptions("F", Column.Type.DoubleFloat)
				//        new Column.CreateOptions("F", Column.Type.LongBinary),
				//        new Column.CreateOptions("xyz,x", Column.Type.Long),
				//        new Column.CreateOptions("xyz,y", Column.Type.Text),
				//        new Column.CreateOptions("xyz,z", Column.Type.DoubleFloat)
				//    },
				//    Indexes = new Index.CreateOptions[]
				//    {
				//        new Index.CreateOptions { Name = "PK", KeyColumns = "+D", Unique = true, Primary = true }
				//    }
				//}, out cols, out ixs);
			
				var table = Table.Create(E.D, Flat<DEF>.CreateTableOptionsForFlat(), out cols, out ixs);

				using(var csr = new Cursor(table))
				{
					var srr = new Flat<DEF>(table);
					using(var u = csr.BeginInsert())
					{
						srr.Write(u, d1);
						u.Complete();
					}

					csr.MoveFirst();
					d2 = srr.Read(csr);

					Field[] values = csr.RetrieveAllFields();

					Assert.AreEqual(d1.d, d2.d);
					Assert.AreEqual(d1.e, d2.e);
					Assert.AreEqual(d1.f, d2.f);
					Assert.AreEqual(d1.xyz.x, d2.xyz.x);
					Assert.AreEqual(d1.xyz.y, d2.xyz.y);
					Assert.AreEqual(d1.xyz.z, d2.xyz.z);
				}

				trans_mod.Rollback();				
			}
		}
	}
}
