///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.Main
///////////////////////////////////////////////////////////////////////////////
//
//This software is licenced under the terms of the MIT License:
//
//Copyright (c) 2009 Christopher Smith
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
