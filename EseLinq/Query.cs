using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Linq.Expressions;

using EseObjects;

namespace EseLinq
{
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
		}

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
		}
	}
}
