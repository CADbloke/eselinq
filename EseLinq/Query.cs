using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Linq.Expressions;

using EseLinq.Plans;
using EseObjects;

namespace EseLinq
{
	using OperatorMap = OperatorMap;

	internal interface QueryProperties
	{
		Plan plan
		{
			get;
		}
		Table table
		{
			get;
		}
		CalcPlan cplan
		{
			get;
		}
		Type type
		{
			get;
		}
	}

	public class Query<T> : IQueryable<T>, IQueryable, IOrderedQueryable<T>, IOrderedQueryable, QueryProperties
	{
		Plan plan;
		CalcPlan calc;
		Expression exp;
		Provider provider;
		Table table;

		internal Expression MakeInject()
		{
			return Expression.Call
			(
				typeof(Translator),
				"InjectQueryClone",
				new Type[]
				{
					typeof(T)
				},
				new Expression[]
				{
					Expression.Constant(this)
				}
			);
		}

		public Query(Provider pro, Table table)
		{
			plan = new Scan(table);
			var chan = new Translator.Channel(plan, table, typeof(T));
			exp = MakeInject();
			provider = pro;
			this.calc = Plans.MakeObject.AutoCreate<T>(plan);
			this.table = table;
		}

		internal Query(Plan plan, Plans.Translator.Channel chan, Expression exp, Provider pro)
		{
			this.plan = plan;
			this.exp = exp;
			this.provider = pro;
			this.calc = chan.AsCalcPlan(typeof(T));
			this.table = chan.table;
		}

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			OperatorMap om = new OperatorMap();
			
			var op = om.Demand(plan);
			var scalar = calc.ToCalc(om);

			return new Executor(this, op, scalar);
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			OperatorMap om = new OperatorMap();

			var op = plan.ToOperator(om);
			var scalar = calc.ToCalc(om);

			return new Executor(this, op, scalar);
		}

		internal class Executor : IEnumerator<T>
		{
			Query<T> query;
			Operator top;
			Calc value;

			public Executor(Query<T> query, Operator top, Calc scalar)
			{
				this.query = query;
				this.top = top;
				this.value = scalar;
			}

			public T Current
			{
				get
				{
					return (T)value.value;
				}
			}

			void IDisposable.Dispose()
			{
				top.Dispose();
			}

			object IEnumerator.Current
			{
				get
				{
					return this.Current;
				}
			}

			bool IEnumerator.MoveNext()
			{
				return top.Advance();
			}

			void IEnumerator.Reset()
			{
				top.Reset();
			}
		}

		public Type ElementType
		{
			get
			{
				return typeof(T);
			}
		}

		public System.Linq.Expressions.Expression Expression
		{
			get
			{
				return exp;
			}
		}

		public IQueryProvider Provider
		{
			get
			{
				return provider;
			}
		}

		Plan QueryProperties.plan
		{
			get
			{
				return plan;
			}
		}

		Table QueryProperties.table
		{
			get
			{
				return table;
			}
		}

		CalcPlan QueryProperties.cplan
		{
			get
			{
				return calc;
			}
		}

		Type QueryProperties.type
		{
			get
			{
				return typeof(T);
			}
		}
	}
}
