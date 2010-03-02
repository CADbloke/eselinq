using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

using EseObjects;
using EseLinq.Storage;

namespace EseLinq.Plans
{
	internal class Scan : Plan
	{
		readonly Table table;

		internal Scan(Table table)
		{
			this.table = table;
		}

		public Operator ToOperator(OperatorMap om)
		{
			return new Op(this, new Cursor(table));
		}

		public Plan Clone(CloneMap cm)
		{
			return new Scan(table);
		}

		Table Plan.table
		{
			get
			{
				return table;
			}
		}

		public void Dispose()
		{
			table.Dispose();
		}

		internal class Op : Operator
		{
			readonly Cursor cursor;
			internal readonly Scan scan;

			internal Op(Plans.Scan scab, Cursor cursor)
			{
				this.cursor = cursor;
				this.scan = scab;
				cursor.MoveFirst();
				cursor.Move(-1);
			}

			public bool Advance()
			{
				return cursor.Move(1);
			}

			public void Reset()
			{
				cursor.MoveFirst();
			}

			public void Dispose()
			{
				cursor.Dispose();
			}

			public Plan plan
			{
				get
				{
					return scan;
				}
			}

			Cursor Operator.cursor
			{
				get
				{
					return cursor;
				}
			}
		}
	}
}