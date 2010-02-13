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

				Provider pro = new Provider(d);
				Query<ABC2> q = new Query<ABC2>(pro, tab);

				IEnumerable<ABC2> query;

				//query = from abc in q where abc.a == 3;
				query = q.Where(abc => abc.a == 3);
				
				//query = from abc in q select abc;

				foreach(ABC2 abc in q)
					Console.WriteLine(abc.a);

				var query2 = q.Where<ABC2>(abc => abc.a == 3).Select<ABC2, int>(abc => abc.a);

				foreach(int x in query2)
					Console.WriteLine(x);

				var query3 = from abc in q where abc.a == 3 select abc.b;

				foreach(var x in query3)
					Console.WriteLine(x);
			}
		}
	}
}
