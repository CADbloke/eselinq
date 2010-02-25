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
	internal class Retrieve : CalcPlan
	{
		internal readonly Plan plan;
		internal readonly Table table;
		internal readonly Column col;
		internal readonly Type type;

		internal Retrieve(Plan plan, Table table, Column col, Type type)
		{
			if(plan == null || table == null || col == null || type == null)
				throw new NullReferenceException();

			this.plan = plan;
			this.table = table;
			this.col = col;
			this.type = type;
		}

		public Calc ToCalc(OperatorMap om)
		{
			return new Op
			{
				plan = this,
				csr = om[plan].cursor
			};
		}

		internal class Op : Calc
		{
			internal Retrieve plan;
			internal Cursor csr;

			public object value
			{
				get
				{
					return csr.Retrieve(plan.col, plan.type);
				}
			}

			public void Dispose()
			{
				csr.Dispose();
			}
		}

		public void Dispose()
		{
			plan.Dispose();
		}
	}

	internal class Constant : CalcPlan
	{
		internal readonly object value;

		internal Constant(object value)
		{
			this.value = value;
		}

		public Calc ToCalc(OperatorMap om)
		{
			return new Op
			{
				plan = this
			};
		}

		internal class Op : Calc
		{
			internal Constant plan;

			public object value
			{
				get
				{
					return plan.value;
				}
			}

			public void Dispose()
			{
			}
		}

		public void Dispose()
		{
		}
	}

	internal class BinaryCalc : CalcPlan
	{
		internal readonly Delegate func;
		internal readonly CalcPlan left;
		internal readonly CalcPlan right;

		static bool StaticObjEqual(object x, object y)
		{
			return x.Equals(y);
		}

		internal BinaryCalc(ExpressionType expr_ty, Type ltype, Type rtype, Type type, CalcPlan left, CalcPlan right)
		{
			this.left = left;
			this.right = right;

			var x = Expression.Parameter(ltype, "x");
			var y = Expression.Parameter(rtype, "y");

			var body = Expression.MakeBinary(expr_ty, x, y);

			this.func = Expression.Lambda(body, new ParameterExpression[] {x, y}).Compile();
		}

		public Calc ToCalc(OperatorMap om)
		{
			return new Op
			{
				plan = this,
				left = left.ToCalc(om),
				right = right.ToCalc(om)
			};
		}

		internal class Op : Calc
		{
			internal BinaryCalc plan;
			internal Calc left;
			internal Calc right;

			public object value
			{
				get
				{
					return plan.func.DynamicInvoke(new object[] { left.value, right.value });
				}
			}

			public void Dispose()
			{
				left.Dispose();
				right.Dispose();
			}
		}

		public void Dispose()
		{
			left.Dispose();
			right.Dispose();
		}
	}
}