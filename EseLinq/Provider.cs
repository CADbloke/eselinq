using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;

using EseObjects;

namespace EseLinq
{
	public class Provider : IQueryProvider
	{
		readonly Session sess;

		public Provider(Session sess)
		{
			this.sess = sess;
		}

		public IQueryable<T> CreateQuery<T>(Expression exp)
		{
			var args = new ParameterExpression[0];
			return Expression.Lambda<Func<IQueryable<T>>>(exp, args).Compile()();
		}

		public IQueryable CreateQuery(Expression exp)
		{
			try
			{
				return (IQueryable)typeof(Provider)
					.GetMethod("CreateQuery")
					.MakeGenericMethod(exp.Type)
					.Invoke(this, new Object[] { exp });
			}
			catch(System.Reflection.TargetInvocationException tie)
			{
				throw tie.InnerException;
			}
		}

		public T Execute<T>(Expression exp)
		{
			var args = new ParameterExpression[0];
			var lambda = Expression.Lambda<Func<T>>(exp, args);
			return lambda.Compile().Invoke();
		}

		public object Execute(Expression exp)
		{
			try
			{
				return (IQueryable)typeof(Provider)
					.GetMethod("Execute")
					.MakeGenericMethod(exp.Type)
					.Invoke(this, new Object[] { exp });
			}
			catch(System.Reflection.TargetInvocationException tie)
			{
				throw tie.InnerException;
			}
		}
	}
}
