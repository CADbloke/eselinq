using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;

namespace Test.Functionality
{
	class TempTable
	{
		public static void Test(Session sess, Database db)
		{
			using(var tr = new Transaction(sess))
			{
				var cto = Table.CreateTempOptions.NewWithLists();

				cto.Columns.Add(new Column.CreateOptions{Type = Column.Type.Long, TTKey = true});

				Column[] cols;

				using(var csr = Table.CreateTemp(sess, cto, out cols))
				{
					using(var upd = csr.BeginInsert())
					{
						upd.Set(cols[0], 3);
						upd.Complete();
					}
					using(var upd = csr.BeginInsert())
					{
						upd.Set(cols[0], 4);
						upd.Complete();
					}

					csr.MoveFirst();

					int val = csr.Retrieve<int>(cols[0]);

					Console.WriteLine(val);

					csr.Move(1);

					Console.WriteLine(csr.Retrieve<int>(cols[0]));

					tr.Commit();
				}
			}
		}
	}
}
