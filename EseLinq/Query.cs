using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Linq.Expressions;

using EseObjects;
using EseLinq.Storage;

namespace EseLinq
{
	public class Query<T> : IQueryable<T>, IQueryable //, IOrderedQueryable<T>, IOrderedQueryable
	{
		readonly Expression exp;
		readonly Provider provider;
		readonly Expression<Func<IQueryable<T>>> exec;
		IEnumerable<T> enumerable;

		IEnumerable<T> GetEnumerable()
		{
			if(enumerable == null)
				enumerable = exec.Compile()();

			return enumerable;
		}

		public Query(Provider provider, Expression<Func<IQueryable<T>>> exp)
		{
			var args = new ParameterExpression[0];

			this.provider = provider;
			this.exp = Expression.Invoke(exp, args);
			this.exec = exp;
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

		IEnumerator IEnumerable.GetEnumerator()
		{
			return GetEnumerable().GetEnumerator();
		}

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			return GetEnumerable().GetEnumerator();
		}
	}
}
