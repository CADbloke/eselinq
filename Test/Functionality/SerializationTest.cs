using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;
using EseLinq.Storage;

namespace Test.Functionality
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
	struct ABC
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

	class SerializationTest
	{
		public static void RunTests(Session sess, Database db)
		{
			XYZ a, b;
			a.x = 33;
			a.y = "RS232 - 9600 baud";
			a.z = Math.E;

			using(new Transaction(sess))
			{
				Column[] cols;
				Index[] ixs;
				var BinSerial = Table.Create(sess, db, new Table.CreateOptions
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

					if(a.x != b.x || a.y != b.y || a.z != b.z)
						throw new InvalidOperationException();
				}
			}

			using(new Transaction(sess))
			{
				Column[] cols;
				Index[] ixs;
				var FieldSerial = Table.Create(sess, db, new Table.CreateOptions
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

				using(var trans_mod = new Transaction(sess))
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

						if(a.x != b.x || a.y != b.y || a.z != b.z)
							throw new InvalidOperationException();
					}

					trans_mod.Rollback();
				}
			}

			using(var trans_mod = new Transaction(sess))
			{
				DEF d1 = new DEF {d = 5, e = "qx", f = Math.PI, xyz = a};
				DEF d2;
				
				//TODO: make implicit
				Column[] cols;
				Index[] ixs;
				var table = Table.Create(sess, db, new Table.CreateOptions
				{
					Name = "FieldSerial",
					Columns = new Column.CreateOptions[]
					{
						new Column.CreateOptions("D", Column.Type.Long),
						new Column.CreateOptions("E", Column.Type.LongText, Column.CodePage.Unicode),
						//new Column.CreateOptions("E", Column.Type.LongText, Column.CodePage.English),
						//new Column.CreateOptions("F", Column.Type.DoubleFloat)
						new Column.CreateOptions("F", Column.Type.LongBinary),
						new Column.CreateOptions("xyz,x", Column.Type.Long),
						new Column.CreateOptions("xyz,y", Column.Type.Text),
						new Column.CreateOptions("xyz,z", Column.Type.DoubleFloat)
					},
					Indexes = new Index.CreateOptions[]
					{
						new Index.CreateOptions { Name = "PK", KeyColumns = "+D", Unique = true, Primary = true }
					}
				}, out cols, out ixs);


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

					Field[] values = csr.RetreiveAllFields();

					if(d1.d != d2.d || d1.e != d2.e || d1.f != d2.f ||
						d1.xyz.x != d2.xyz.x || d1.xyz.y != d2.xyz.y || d1.xyz.z != d2.xyz.z)
						throw new InvalidOperationException();
				}

				trans_mod.Rollback();				
			}

		}
	}
}
