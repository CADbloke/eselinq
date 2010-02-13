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
	}

	public class Query<T> : IQueryable<T>, IQueryable, PrePlanned
	{
		Plan plan;
		ScalarPlan scalar_plan;
		Expression exp;
		Provider provider;

		public Query(Provider pro, Table table)
		{
			plan = new Scan(table);
			exp = Expression.Constant(this);
			provider = pro;
			this.scalar_plan = Plans.MakeObject.AutoCreate<T>(plan);
		}

		internal Query(Plan plan, object value_plan, Expression exp, Provider pro)
		{
			this.plan = plan;
			this.exp = exp;
			this.provider = pro;

			//use existing scalar if availaible, else build an object from the base plan
			this.scalar_plan = value_plan as ScalarPlan;
			if(this.scalar_plan == null)
				this.scalar_plan = Plans.MakeObject.AutoCreate<T>(plan);
		}

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			OperatorMap om = new OperatorMap();

			return new Executor(this, plan.ToOperator(om), scalar_plan.ToScalar(om));
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			OperatorMap om = new OperatorMap();

			var op = plan.ToOperator(om);
			var scalar = scalar_plan.ToScalar(om);

			return new Executor(this, op, scalar);
		}

		internal class Executor : IEnumerator<T>
		{
			Query<T> query;
			Operator top;
			Scalar scalar;

			public Executor(Query<T> query, Operator top, Scalar scalar)
			{
				this.query = query;
				this.top = top;
				this.scalar = scalar;
			}

			public T Current
			{
				get
				{
					return (T)scalar.value;
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
	}
}
