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
	using OperatorMap = Dictionary<Plan, Operator>;

	internal class Scan : Plan
	{
		internal readonly Table src;

		internal Scan(Table table)
			: base(new Table[] { table }, new MemoryTable[0])
		{
			src = table;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			var csr = new Cursor(src);
			var op = new Op(this, csr);

			om.Add(this, op);

			return op;
		}

		public override void Dispose()
		{
			src.Dispose();
		}

		internal class Op : Operator
		{
			internal readonly Scan scan;
			internal readonly Cursor cursor;

			internal Op(Plans.Scan scab, Cursor cursor)
				: base(new Cursor[] { cursor }, new MemoryCursor[0])
			{
				this.scan = scab;
				this.cursor = cursor;
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