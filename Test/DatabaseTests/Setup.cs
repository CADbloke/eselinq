///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTests.Setup
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
using NUnit.Framework;
using System.IO;
using EseObjects;

namespace Test.DatabaseTests
{
	[SetUpFixture]
	///<summary>Setup environment</summary>
	public class E
	{
		public const string DatabaseName = "Test.edb";
		public static string FileDirectory;

		public static Instance I;
		public static Session S;
		public static Database D;

		[SetUp]
		public static void Setup()
		{
			try
			{
				InstanceSetup();
				SessionSetup();
				DatabaseSetup();
			}
			catch(Exception)
			{
				Teardown(); //want this to run regardless
				throw;
			}
		}

		[TearDown]
		public static void Teardown()
		{
			if(D != null)
				D.Dispose();

			if(S != null)
				S.Dispose();

			if(I != null)
				I.Dispose();

			if(FileDirectory != null)
				Directory.Delete(FileDirectory, true);
		}

		static void InstanceSetup()
		{
			string path = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
			Directory.CreateDirectory(path);
			FileDirectory = path + "\\";

			I = new Instance();
			I.NoInformationEvent = true;
			I.CircularLog = true;
			I.TempPath = FileDirectory;
			I.LogFilePath = FileDirectory;
			I.SystemPath = FileDirectory;
			I.InitGlobal();
		}

		static void SessionSetup()
		{
			S = new Session(I);
		}

		static void DatabaseSetup()
		{
			var dco = new Database.CreateOptions{FileName = Path.Combine(FileDirectory, "test.edb"), OverwriteExisting = true};
			D = Database.Create(S, dco);
		}
	}
}
