///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2010 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.DatabaseTest.InvalidCreateTable
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
using System.IO;
using NUnit.Framework;
using EseObjects;

namespace Test.NonDatabaseTests
{
	[TestFixture]
	public class InstanceTests
	{
		public static string FileDirectory;

		[TestFixtureSetUp]
		public void Setup()
		{
			string path = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
			Directory.CreateDirectory(path);
			FileDirectory = path + "\\";
		}

		[TestFixtureTearDown]
		public void Teardown()
		{
			if(FileDirectory != null)
				Directory.Delete(FileDirectory, true);
		}

		/// <summary>
		/// Standard but necessary instance parameters.
		/// </summary>
		void SetStandardInstanceParms(Instance I)
		{
			I.NoInformationEvent = true;
			I.CircularLog = true;
			I.TempPath = FileDirectory;
			I.LogFilePath = FileDirectory;
			I.SystemPath = FileDirectory;
		}

		[Test]
		public void InitGlobalTermComplete()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Global));
				inst.InitGlobal();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Initialized));
				inst.TerminateComplete();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Disposed));
			}
		}

		[Test]
		public void InitGlobalTermAbrupt()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Global));
				inst.InitGlobal();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Initialized));
				inst.TerminateAbrubt();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Disposed));
			}
		}

		[Test]
		public void InitGlobalTermAbruptStopBackup()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Global));
				inst.InitGlobal();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Initialized));
				inst.TerminateAbruptStopBackup();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Disposed));
			}
		}

		[Test]
		public void InitTerm()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Global));
				inst.Create("test", "test db");
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Created));
				inst.Init();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Initialized));
				inst.TerminateComplete();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Disposed));
			}
		}

		[Test]
		public void InitWithOptsTerm()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Global));
				inst.Create("test", "test db");
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Created));
				inst.Init(true, false, true, true);
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Initialized));
				inst.TerminateComplete();
				Assert.That(inst.State, Is.EqualTo(Instance.InstanceState.Disposed));
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailDoubleOnGlobalInit()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.InitGlobal();
				inst.InitGlobal();
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailCreateOnGlobalInit()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.InitGlobal();
				inst.Create("test", "test db");
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailInitOnGlobalInit()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.InitGlobal();
				inst.Init();
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailInitOnNotCreated()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Init();
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailInitWithOptsNotCreated()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Init(false, false, false, false);
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailCreateOnInitialized()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Create("test", "test db");
				inst.Init();
				inst.Create("test", "test db");
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailInitOnDisposed()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Create("test", "test db");
				inst.Init();
				inst.Dispose();
				inst.Init();
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailInitWithOptsOnDisposed()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Create("test", "test db");
				inst.Init();
				inst.Dispose();
				inst.Init(false, false, false, false);
			}
		}

		[Test]
		[ExpectedException(typeof(InvalidOperationException))]
		public void FailCreateOnDisposed()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.Create("test", "test db");
				inst.Init();
				inst.Dispose();
				inst.Create("test", "test db");
			}
		}

		[Test]
		public void LongParameter()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.PageFragment = 16;
				Assert.That(inst.PageFragment, Is.EqualTo(16));
			}
		}

		[Test]
		public void StringParameter()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.EventSource = "ESELinq NUnit tests";
				Assert.That(inst.EventSource, Is.EqualTo("ESELinq NUnit tests"));
			}
		}

		[Test]
		public void BoolParameter()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.NoInformationEvent = true;
				Assert.That(inst.NoInformationEvent, Is.EqualTo(true));
			}
		}

		[Test]
		public void EnableMultiInstnaceMode()
		{
			using(var inst = new Instance())
			{
				SetStandardInstanceParms(inst);
				inst.EnableMultInstanceMode();
			}
		}
	}
}