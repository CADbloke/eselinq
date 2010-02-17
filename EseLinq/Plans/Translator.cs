using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

using EseObjects;

namespace EseLinq.Plans
{
	/// <summary>
	/// ToPlans from a LINQ Expression to an EseLinq Plan.
	/// </summary>
	class Translator
	{
		internal enum RowPlanType
		{
			Invalid = 0,
			Calc,
			Column,
			Table,
			Tables
		}

		internal struct Channel
		{
			internal readonly RowPlanType rptype;

			//only some are populated, based on rptype
			internal readonly CalcPlan cplan;
			internal readonly Plan plan;
			internal readonly Table table;
			internal readonly Column column;
			internal readonly Type type;

			internal Channel(CalcPlan cplan, Type type)
			{
				this.cplan = cplan;
				this.type = type;
				rptype = RowPlanType.Calc;

				this.plan = null;
				this.table = null;
				this.column = null;
			}

			internal Channel(Plan plan, Type type)
			{
				this.plan = plan;
				this.type = type;

				if(plan.tables.Length == 1)
				{
					this.table = plan.tables[0];
					rptype = RowPlanType.Table;
				}
				else
				{
					this.table = null;
					rptype = RowPlanType.Tables;
				}

				this.cplan = null;
				this.column = null;
			}

			internal Channel(Plan plan, Table table, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.table = table;
				rptype = RowPlanType.Table;

				this.cplan = null;
				this.column = null;
			}

			internal Channel(Plan plan, Table table, Column column, Type type)
			{
				this.plan = plan;
				this.table = table;
				this.column = column;
				this.type = type;
				rptype = RowPlanType.Column;

				this.cplan = null;
			}

			internal Channel(Plan plan, string colname, Type type)
			{
				Column found_col = null;
				Table found_table = null;

				foreach(Table t in plan.tables)
					foreach(Column c in t.Columns)
						if(c.Name == colname)
						{
							if(found_col == null)
							{
								found_col = c;
								found_table = t;
							}
							else
								throw new AmbiguousMatchException(string.Format("Multiple candidates for column {0}, including tables {1} and {2}", colname, found_table.Name, t.Name));
						}

				this.plan = plan;
				table = found_table;
				column = found_col;
				this.type = type;
				rptype = RowPlanType.Column;

				this.cplan = null;
			}

			internal CalcPlan AsCalcPlan(Type type)
			{
				switch(rptype)
				{
				case RowPlanType.Calc:
					return cplan;

				case RowPlanType.Column:
					return new Retrieve(plan, table, column, type);

				case RowPlanType.Table:
					return MakeObject.AutoCreate(plan, type);

				case RowPlanType.Tables:
					throw new ApplicationException("TODO: fixme");

				default:
					throw new ArgumentException("Invalid rptype");
				}
			}

			internal CalcPlan AsCalcPlan()
			{
				return AsCalcPlan(type);
			}
		}

		internal struct Downstream
		{
			internal Dictionary<string, Channel> env;
			internal Plan context;

			///<summary>use this constructor when about to make modifications; copies mutable members</summary>
			internal Downstream(Downstream from)
			{
				this.env = new Dictionary<string, Channel>(from.env);
				this.context = from.context;
			}

			internal void Init()
			{
				env = new Dictionary<string, Channel>(16);
			}
		}

		internal struct Upstream
		{
			/// <summary>
			/// Immediate value returned in plan from translating expression.
			/// </summary>
			internal Channel chan;
			/// <summary>
			/// Plan used for scrolling, if applicable.
			/// Separate from Channel.plan and important to carry plans back up the call stack even the immediate value isn't a plan.
			/// </summary>
			internal Plan plan;

			/// <summary>
			/// Use for an explicitly separate plan.
			/// </summary>
			internal Upstream(Channel chan, Plan plan)
			{
				this.chan = chan;
				this.plan = plan;
			}

			/// <summary>
			/// Use when the only plan would be from the channel.
			/// </summary>
			internal Upstream(Channel chan)
			{
				this.chan = chan;
				this.plan = chan.plan;
			}

			internal Upstream Merge(Upstream other)
			{
				Upstream r = this;

				if(r.plan == null)
					r.plan = other.plan;
				
				return r;
			}

			internal Upstream WithChannel(Channel chan)
			{
				Upstream r = this;

				r.chan = chan;
				if(chan.plan != null)
					r.plan = chan.plan;

				return r;
			}
		}


		internal static Upstream Translate(UnaryExpression exp, Downstream downs)
		{
			switch(exp.NodeType)
			{
			//don't care about quotes
			case ExpressionType.Quote:
				return Translate(exp.Operand, downs);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(BinaryExpression exp, Downstream downs)
		{
			switch(exp.NodeType)
			{
			case ExpressionType.Equal:
				{
					var left = Translate(exp.Left, downs);
					var right = Translate(exp.Right, downs);

					return left.Merge(right).WithChannel(new Channel(new BinaryCalc(exp.NodeType, left.chan.AsCalcPlan(), right.chan.AsCalcPlan()), exp.Type));
				}

			default:
				throw IDontKnowWhatToDoWithThis(exp);
			}
		}

		internal static Upstream Translate(ConditionalExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(ConstantExpression exp, Downstream downs)
		{
			var planned = exp.Value as PrePlanned;
			if(planned != null)
				return new Upstream(new Channel(planned.plan, exp.Type));

			return new Upstream(new Channel(new Constant(exp.Value), exp.Value.GetType()));
		}

		internal static Upstream Translate(InvocationExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(LambdaExpression exp, Downstream downs)
		{
			//assuming one parameter

			//copy value for upstream use
			downs = new Downstream(downs);

			Plan bodyp = downs.context;

			downs.env.Add(exp.Parameters[0].Name, new Channel(bodyp, exp.Type));

			return Translate(exp.Body, downs);
		}

		internal static Upstream Translate(ListInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberExpression exp, Downstream downs)
		{
			var parm = (ParameterExpression)exp.Expression;
			var field = (FieldInfo)exp.Member;
			var tplan = downs.env[parm.Name];

			switch(tplan.rptype)
			{
			case RowPlanType.Table:
				return new Upstream(new Channel(tplan.plan, field.Name, exp.Type));
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MethodCallExpression exp, Downstream downs)
		{
			switch(exp.Method.Name)
			{
			case "Where":
				{
					var data = Translate(exp.Arguments[0], downs);
					downs.context = data.plan;
					var pred = Translate(exp.Arguments[1], downs);
					var body = new Filter(data.plan, pred.chan.AsCalcPlan());

					return data.Merge(pred).WithChannel(new Channel(body, exp.Type));
				}
			case "Select":
				{
					var data = Translate(exp.Arguments[0], downs);
					downs.context = data.plan;
					var body = Translate(exp.Arguments[1], downs);

					data = data.Merge(body);
					data.chan = body.chan;

					return data;
				}
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(NewArrayExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(NewExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(ParameterExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(TypeBinaryExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(Expression exp, Downstream downs)
		{
			{
				var tyexp = exp as UnaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as BinaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as TypeBinaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}

			{
				var tyexp = exp as ConstantExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as ConditionalExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MemberExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MethodCallExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as LambdaExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as NewExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MemberInitExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as ListInitExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as NewArrayExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as InvocationExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Exception IDontKnowWhatToDoWithThis(Expression exp)
		{
			return new ArgumentException("Unknown expression " + exp.Type + ":" + exp.NodeType);
		}
	}
}
