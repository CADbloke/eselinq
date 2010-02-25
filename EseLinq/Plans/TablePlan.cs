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
		internal Scan(Table table)
			: base(table, null)
		{}

		internal override Operator ToOperator(OperatorMap om)
		{
			return new Op(this, new Cursor(table));
		}

		internal override Plan Clone(CloneMap cm)
		{
			return new Scan(table);
		}

		public override void Dispose()
		{
			table.Dispose();
		}

		internal class Op : Operator
		{
			internal readonly Scan scan;

			internal Op(Plans.Scan scab, Cursor cursor)
				: base(cursor, null)
			{
				this.scan = scab;
				cursor.MoveFirst();
				cursor.Move(-1);
			}

			internal override bool Advance()
			{
				return cursor.Move(1);
			}

			internal override void Reset()
			{
				cursor.MoveFirst();
			}

			public override void Dispose()
			{
				cursor.Dispose();
			}

			internal override Plan plan
			{
				get
				{
					return scan;
				}
			}
		}
	}
}