using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;

namespace Test
{
	class InvalidCreateTable
	{
		/// <summary>
		/// 
		/// </summary>
		/// <param name="sess"></param>
		/// <param name="db"></param>
		public static void Test(Session sess, Database db)
		{
			using(var tr = new Transaction(sess))
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

				try
				{
					using(var tab = Table.Create(sess, db, nt)) {} //nothing to do since this should fail
					tr.Commit();
					throw new InvalidOperationException("Create table with bad column type");
				}
				catch(EseException e)
				{
					if(e.Symbol != "JET_errInvalidColumnType") //expected error
						throw e;
				}

				Columns[0] = new Column.CreateOptions
				{
					Name = new String('q', 100), //name too long
					Type = Column.Type.Long
				};
				
				try
				{
					using(var tab = Table.Create(sess, db, nt)) {} //nothing to do since this should fail
					tr.Commit();
					throw new InvalidOperationException("Create table with bad column type");
				}
				catch(EseException e)
				{
					if(e.Symbol != "JET_errInvalidName") //expected error
						throw e;
				}
				
			}
		}
	}
}
