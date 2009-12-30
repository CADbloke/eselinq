using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;

namespace Test.Functionality
{
	delegate void DumpLine(string s);

	class EnumTest
	{
		public static void DumpContents(Session sess, Database db, DumpLine WriteLine)
		{
			using(var tr = new Transaction(sess))
			{
				foreach(string tname in db.TableNames)
				{
					WriteLine("-------" + tname);
					using(var t = new Table(db, sess, tname))
					{
						var columns = t.Columns;
						var colstrs = new String[columns.Count];
						for(int i = 0; i < columns.Count; i++)
						{
							colstrs[i] = columns[i].Name;
						}
						WriteLine(String.Join("|", colstrs));
		
						using(var c = new Cursor(t))
						{
							bool has_current = c.MoveFirst();

							while(has_current)
							{
								var fields = c.RetreiveAllFields(0x1000);
								var fieldstrs = new String[fields.Length];
								for(uint i = 0; i < fields.Length; i++)
								{
									fieldstrs[i] = fields[i].Val.ToString();
								}

								WriteLine(String.Join("|", fieldstrs));
								has_current = c.Move(1);
							}
						}
					}
				}
			}
		}
	}
}
