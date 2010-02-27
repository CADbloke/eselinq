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
			/// <summary>
			/// Default. All fields null.
			/// </summary>
			Invalid = 0,
			/// <summary>
			/// Result of a calculation. Associated plan controls position, if present. Agg. fields by type.
			/// </summary>
			Calc,
			/// <summary>
			/// Single column selected from a ESE table. Agg. fields determined from Type.
			/// </summary>
			Column,
			/// <summary>
			/// Single ESE table. Agg. fields determined by Type.
			/// </summary>
			Table,
			/// <summary>
			/// Memory stored table. Agg. fields determined by table definition.
			/// </summary>
			MemoryTable,
			/// <summary>
			/// Mapping of fields after New or selection into a nonsource composite. Agg. fields by type.
			/// </summary>
			Collection
		}

		internal struct Channel
		{
			internal readonly RowPlanType rptype;

			//only some are populated, based on rptype
			internal readonly CalcPlan cplan;
			internal readonly Plan plan;
			internal readonly Table table;
			internal readonly MemoryTable mtable;
			internal readonly Column column;
			internal readonly Type type;
			internal readonly Dictionary<string, Channel> fields;

			/// <summary>
			/// Calculation without associated plan.
			/// </summary>
			internal Channel(CalcPlan cplan, Type type)
			{
				this.cplan = cplan;
				this.type = type;
				rptype = RowPlanType.Calc;

				this.plan = null;
				this.table = null;
				this.mtable = null;
				this.column = null;
				this.fields = null;
			}

			/// <summary>
			/// Plan with a specific single memory table.
			/// </summary>
			internal Channel(Plan plan, MemoryTable mtable, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.mtable = mtable;
				rptype = RowPlanType.Table;

				this.table = null;
				this.cplan = null;
				this.column = null;
				this.fields = null;
			}

			/// <summary>
			/// Plan with a specific single table.
			/// </summary>
			internal Channel(Plan plan, Table table, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.table = table;
				rptype = RowPlanType.Table;

				this.cplan = null;
				this.column = null;
				this.mtable = null;
				this.fields = null;
			}

			/// <summary>
			/// Plan with a specific column.
			/// </summary>
			internal Channel(Plan plan, Table table, Column column, Type type)
			{
				this.plan = plan;
				this.table = table;
				this.column = column;
				this.type = type;
				rptype = RowPlanType.Column;

				this.cplan = null;
				this.mtable = null;
				this.fields = null;
			}

			/// <summary>
			/// Plan with a specific column, selected by name.
			/// </summary>
			internal Channel(Plan plan, string colname, Type type)
			{
				Column found_col = null;

				//foreach(Table t in plan.tables)
					foreach(Column c in plan.table.Columns)
						if(c.Name == colname)
						{
							found_col = c;
							break;
						}

				this.plan = plan;
				table = plan.table;
				column = found_col;
				this.type = type;
				rptype = RowPlanType.Column;

				this.cplan = null;
				this.mtable = null;
				this.fields = null;
			}

			/// <summary>
			/// Plan with specific calculation.
			/// </summary>
			internal Channel(Plan plan, CalcPlan cplan, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.cplan = cplan;
				rptype = RowPlanType.Calc;
				
				this.table = null;
				this.mtable = null;
				this.column = null;
				this.fields = null;
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

				case RowPlanType.MemoryTable:
				case RowPlanType.Collection:
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
			//internal Plan context;
			internal Channel[] context;

			///<summary>use this constructor when about to make modifications; copies mutable members</summary>
			internal Downstream(Downstream from)
			{
				this.env = new Dictionary<string, Channel>(from.env);
				if(from.context != null)
					this.context = from.context.ToArray();
				else
					this.context = null;
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

			internal Upstream WithChannel(Channel chan)
			{
				return new Upstream(chan, plan);
			}

			internal Upstream WithPlan(Plan plan)
			{
				return new Upstream(chan, plan);
			}
		}

		/// <summary>
		/// Not intended to be called directly; used in an expression to inject upstream state.
		/// </summary>
		public static IQueryable<T> InjectQueryClone<T>(Query<T> q)
		{
			return q;
		}

		internal static Upstream CloneQuery(QueryProperties query)
		{
			var cm = new CloneMap();
			var plan = query.plan.Clone(cm);

			if(query.table != null)
				return new Upstream(new Channel(plan, query.table, query.type), plan);

			return new Upstream(new Channel(plan, query.cplan, query.type), plan);
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
			default:
				{
					var left = Translate(exp.Left, downs);
					var right = Translate(exp.Right, downs);
					
					return new Upstream(new Channel(new BinaryCalc(exp.NodeType, left.chan.type, right.chan.type, exp.Type, left.chan.AsCalcPlan(), right.chan.AsCalcPlan()), exp.Type));
				}
			}
		}

		internal static Upstream Translate(ConditionalExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(ConstantExpression exp, Downstream downs)
		{
			return new Upstream(new Channel(new Constant(exp.Value), exp.Value.GetType()));
		}

		internal static Upstream Translate(InvocationExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(LambdaExpression exp, Downstream downs)
		{
			//copy value for upstream use
			downs = new Downstream(downs);

			for(int i = 0; i < exp.Parameters.Count; i++)
				downs.env.Add(exp.Parameters[i].Name, downs.context[i]);

			return Translate(exp.Body, downs);
		}

		internal static Upstream Translate(ListInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberExpression exp, Downstream downs)
		{
			{
				//TODO: should be runtime?
				var as_const = exp.Expression as ConstantExpression;
				if(as_const != null)
				{
					var field = (FieldInfo)exp.Member;
					object val = field.GetValue(as_const.Value);

					var query = val as QueryProperties;
					if(query != null)
					{
						return CloneQuery(query);
					}
					
					return new Upstream(new Channel(new Constant(val), exp.Type));
				}
			}

			{
				var as_parm = exp.Expression as ParameterExpression;
				if(as_parm != null)
				{
					var field = (FieldInfo)exp.Member;
					var tplan = downs.env[as_parm.Name];

					switch(tplan.rptype)
					{
					case RowPlanType.Table:
						return new Upstream(new Channel(tplan.plan, field.Name, exp.Type));
					}
				}
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MethodCallExpression exp, Downstream downs)
		{
			switch(exp.Method.DeclaringType.FullName)
			{
			case "System.Linq.Queryable":
				switch(exp.Method.Name)
				{
				case "Where":
					{
						var data = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { data.chan };
						var pred = Translate(exp.Arguments[1], downs);
						var body = new Filter(data.plan, pred.chan.AsCalcPlan());

						return data.WithPlan(body);
					}
				case "Select":
					{
						var data = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { data.chan };
						var body = Translate(exp.Arguments[1], downs);

						return new Upstream(body.chan, data.plan);
					}
				case "SelectMany":
					{
						var outer = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { outer.chan};

						var inner = Translate(exp.Arguments[1], downs);
						downs.context = new Channel[] { outer.chan, inner.chan };
						
						var body = Translate(exp.Arguments[2], downs);

						var product = new Product(outer.plan, inner.plan);

						return new Upstream(body.chan, product);
					}
				case "Distinct":
					{
						var data = Translate(exp.Arguments[0], downs);
						var elems = data.chan.AsCalcPlan();
						var hashtab = new MemoryHashDisctinctPlan(data.plan, elems, exp.Type);

						return new Upstream(new Channel((Plan)hashtab, (CalcPlan)hashtab, exp.Type));
					}
				default:
					throw IDontKnowWhatToDoWithThis(exp);
				}

			case "EseLinq.Plans.Translator":
				switch(exp.Method.Name)
				{
				case "InjectQueryClone":
					{
						var cons = (ConstantExpression)exp.Arguments[0];

						return CloneQuery((QueryProperties)cons.Value);
					}
				default:
					throw IDontKnowWhatToDoWithThis(exp);
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
			CalcPlan[] arguments = new CalcPlan[exp.Arguments.Count];

			for(int i = 0; i < exp.Arguments.Count; i++)
				arguments[i] = Translate(exp.Arguments[i], downs).chan.AsCalcPlan();

			var calc = new MakeObjectFromConstructor(arguments, exp.Constructor);

			return new Upstream(new Channel(calc, exp.Type));
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
