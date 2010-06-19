///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Test.EnumTest
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
					using(var t = new Table(db, tname))
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
								var fields = c.RetrieveAllFields(0x1000);
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
