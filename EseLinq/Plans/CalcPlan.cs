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
				csr = om[plan].CorrespondingCursor(table)
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

		public Calc ToCalc(Dictionary<Plan, Operator> om)
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
		internal readonly ExpressionType func;
		internal readonly CalcPlan left;
		internal readonly CalcPlan right;

		internal BinaryCalc(ExpressionType func, CalcPlan left, CalcPlan right)
		{
			this.func = func;
			this.left = left;
			this.right = right;
		}

		public Calc ToCalc(Dictionary<Plan, Operator> om)
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
					switch(plan.func)
					{
					case ExpressionType.Equal:
						return left.value.Equals(right.value);

					default:
						throw new ArgumentException("Unknown calculation type " + plan.func.ToString());
					}
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