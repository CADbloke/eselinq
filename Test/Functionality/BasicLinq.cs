using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using EseObjects;
using EseLinq;

namespace Test.Functionality
{
	public struct ABC2
	{
		public int a;
		public float b;
		public string c;
	}

	class BasicLinq
	{
		public static void RunTests(Session s, Database d)
		{
			using(var tr = new Transaction(s))
			{
				Column[] col;
				Index[] ix;

				var tab = Table.Create(s, d, new Table.CreateOptions
				{
					Name = "ABC",
					Columns = new Column.CreateOptions[]
					{
						new Column.CreateOptions("a", Column.Type.Long),
						new Column.CreateOptions("b", Column.Type.SingleFloat),
						new Column.CreateOptions("c", Column.Type.Text, Column.CodePage.English) 
					},
					Indexes = new Index.CreateOptions[]
					{
						Index.CreateOptions.NewPrimary("PK", "+a", true)
					}
				}, out col, out ix);

				var csr = new Cursor(tab);

				using(var u = csr.BeginInsert())
				{
					u.Set(col[0], 3);
					u.Set(col[1], 5.55);
					u.Set(col[2], "foo");
					u.Complete();
				}
				using(var u = csr.BeginInsert())
				{
					u.Set(col[0], 4);
					u.Set(col[1], 4.444);
					u.Set(col[2], "bar");
					u.Complete();
				}

				Provider pro = new Provider(d);
				Query<ABC2> src = new Query<ABC2>(pro, tab);

				IEnumerable<ABC2> q1 = src.Where(abc => abc.a == 3);
				
				Console.WriteLine("A");
				foreach(ABC2 abc in q1)
					Console.WriteLine(abc.a);

				var q2 = src.Where<ABC2>(abc => abc.a == 3).Select<ABC2, int>(abc => abc.a);

				Console.WriteLine("B");
				foreach(int x in q2)
					Console.WriteLine(x);

				var q3 = from abc in src where abc.a == 3 select abc.b;

				Console.WriteLine("C");
				foreach(var x in q3)
					Console.WriteLine(x);
			}
		}
	}
}
