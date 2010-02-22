using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Linq.Expressions;

using EseLinq.Plans;
using EseObjects;

namespace EseLinq
{
	using OperatorMap = Dictionary<Plan, Operator>;

	internal interface PrePlanned
	{
		Plan plan
		{
			get;
		}
		Translator.Channel chan
		{
			get;
		}
	}

	public class Query<T> : IQueryable<T>, IQueryable, PrePlanned
	{
		Plan plan;
		CalcPlan calc;
		Expression exp;
		Provider provider;
		Table table;

		public Query(Provider pro, Table table)
		{
			plan = new Scan(table);
			exp = Expression.Constant(this);
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

			var op = plan.ToOperator(om);
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

		Plan PrePlanned.plan
		{
			get
			{
				return plan;
			}
		}

		//TODO: should be more direct
		Translator.Channel PrePlanned.chan
		{
			get
			{
				if(table != null)
					return new Translator.Channel(plan, table, typeof(T));
				else
					return new Translator.Channel(plan, calc, typeof(T));
			}
		}
	}
}
