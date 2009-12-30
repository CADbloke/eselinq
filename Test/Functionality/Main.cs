using System;
using System.Collections.Generic;
using System.Text;
using EseObjects;

namespace Test.Functionality
{
	class Main
	{
		public static void RunTests()
		{
			Console.WriteLine("Creating instance");
			var n = new Instance();

			n.NoInformationEvent = true;
			n.CircularLog = true;
			n.TempPath = "C:\\Temp\\Edb\\";
			n.LogFilePath = "C:\\Temp\\Edb\\";
			n.InitGlobal();
			
			Console.WriteLine("Creating session");
			var sess = new Session(n);

			Console.WriteLine("Creating database");
			var ndb = new Database.CreateOptions("C:\\Temp\\Edb\\test.edb");
			ndb.OverwriteExisting = true;

			var d = Database.Create(sess, ndb);

			EseObjects.Table t = null;

			try
			{
				Console.WriteLine("Temp table testing");
				Functionality.TempTable.Test(sess, d);

				Console.WriteLine("Creating test data");
				using(var td = new Functionality.TestData(sess, d))
				{
					Console.WriteLine("Bookmark test");
					Functionality.BookmarkTest.Test(sess, d, td);
					
					Console.WriteLine("Key test");
					Functionality.KeyTest.Test(sess, d, td);

					Console.WriteLine("Dump test");
					Functionality.EnumTest.DumpContents(sess, d, Console.WriteLine);
				}

				Console.WriteLine("Invalid create table test");
				InvalidCreateTable.Test(sess, d);
			}
			finally
			{
				if(t != null)
					t.Dispose();
			}
			Console.WriteLine("Done!");
			Console.ReadLine();
		}

		public static void DumpDB(string Path, string Name, string Output)
		{
			Console.WriteLine("Creating instance");
			var n = new Instance();

			n.NoInformationEvent = true;
			n.TempPath = Path;
			n.SystemPath = Path;
			n.LogFilePath = Path;

			n.DatabasePageSize = 8192;
			n.InitGlobal();

			Console.WriteLine("Creating session");
			var sess = new Session(n);

			Console.WriteLine("Opening database");
			var db = Database.AttachDatabase(sess, Name);

			Console.WriteLine("Opening output file");
			var f = new System.IO.StreamWriter(Output);

			Console.WriteLine("Dumping contents");
			Functionality.EnumTest.DumpContents(sess, db, f.WriteLine);
		}
	}
}
