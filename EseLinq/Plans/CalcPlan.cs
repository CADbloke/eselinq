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
		internal readonly Plan src;
		internal readonly int col_index;
		internal readonly Type type;

		internal Retrieve(Plan src, int col_index, Type type)
		{
			this.src = src;
			this.col_index = col_index;
			this.type = type;
		}

		public Calc ToCalc(OperatorMap om)
		{
			Operator src_op = om.Demand(src);

			return new Op(this, src_op.cursor, src_op.columns[col_index]);
		}

		internal class Op : Calc
		{
			internal readonly Retrieve plan;
			internal readonly Cursor csr;
			internal readonly Column col;

			internal Op(Retrieve plan, Cursor csr, Column col)
			{
				this.plan = plan;
				this.csr = csr;
				this.col = col;
			}

			public object value
			{
				get
				{
					return csr.Retrieve(col, plan.type);
				}
			}

			public void Dispose()
			{
				csr.Dispose();
			}
		}

		public void Dispose()
		{
			src.Dispose();
		}
	}

	internal class FieldAccess : CalcPlan
	{
		internal readonly CalcPlan src;
		internal readonly FieldInfo field_info;

		internal FieldAccess(CalcPlan src, FieldInfo field_info)
		{
			this.src = src;
			this.field_info = field_info;
		}

		public Calc ToCalc(OperatorMap om)
		{
			return new Op(src.ToCalc(om), this);
		}

		internal class Op : Calc
		{
			readonly Calc src;
			readonly FieldAccess plan;

			internal Op(Calc src, FieldAccess plan)
			{
				this.src = src;
				this.plan = plan;
			}

			public object value
			{
				get
				{
					return plan.field_info.GetValue(src.value);
				}
			}

			public void Dispose()
			{
				src.Dispose();
			}
		}

		public void Dispose()
		{
			src.Dispose();
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