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
		public readonly Database db;

		public Provider(Database db)
		{
			this.db = db;
		}

		public IQueryable<T> CreateQuery<T>(Expression exp)
		{

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

		public TResult Execute<TResult>(Expression expression)
		{
			throw new NotImplementedException();
		}

		public object Execute(Expression expression)
		{
			throw new NotImplementedException();
		}
	}
}
