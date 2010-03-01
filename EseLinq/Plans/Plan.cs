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
	internal class OperatorMap : Dictionary<Plan, Operator>
	{
		public Operator Demand(Plan p)
		{
			Operator o;

			if(!TryGetValue(p, out o))
			{
				o = p.ToOperator(this);
				Add(p, o);
			}

			return o;
		}
	}

	internal class CloneMap : Dictionary<Plan, Plan>
	{
		public Plan Demand(Plan p)
		{
			Plan r;

			if(!TryGetValue(p, out r))
			{
				r = p.Clone(this);
				Add(p, r);
			}

			return r;
		}
	}
	
	abstract class Plan : IDisposable
	{
		/// <summary>
		/// Tables underlying this plan node. Multiple tables are possible after a join.
		/// </summary>
		internal readonly Table table;

		protected Plan()
		{
			this.table = null;
		}

		protected Plan(Table table)
		{
			this.table = table;
		}

		//TODO: should only be called from OperatorMap
		internal abstract Operator ToOperator(OperatorMap om);
		internal abstract Plan Clone(CloneMap cm);

		public virtual void Dispose()
		{}
	}

	internal abstract class Operator : IDisposable
	{
		/// <summary>
		/// Cursors underlying this operator. Multiple cursors are possible after a join.
		/// Should be the same count and order as Plan.tables;
		/// </summary>
		internal readonly Cursor cursor;

		protected Operator()
		{
			this.cursor = null;
		}

		protected Operator(Cursor cursor)
		{
			this.cursor = cursor;
		}

		internal abstract Plan plan
		{
			get;
		}

		internal abstract bool Advance();
		internal virtual void Reset()
		{}
		public virtual void Dispose()
		{}
	}

	internal interface Calc : IDisposable
	{
		object value
		{
			get;
		}
	}

	internal interface CalcPlan : IDisposable
	{
		Calc ToCalc(OperatorMap om);
	}

	internal class Filter : Plan
	{
		internal readonly Plan src;
		internal readonly CalcPlan predicate;

		internal Filter(Plan src, CalcPlan predicate)
		{
			this.src = src;
			this.predicate = predicate;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(src), om);
		}

		internal override Plan Clone(CloneMap cm)
		{
			return new Filter(cm.Demand(src), predicate);
		}

		internal class Op : Operator
		{
			Filter filter;
			Operator src;
			Calc predicate;

			internal Op(Filter filter, Operator src, OperatorMap om)
			{
				this.filter = filter;
				this.src = src;
				this.predicate = filter.predicate.ToCalc(om);
			}

			internal override bool Advance()
			{
				while(src.Advance())
					if(predicate.value.Equals(true)) //found a matching record
						return true;

				//ran out of rows
				return false;
			}

			internal override Plan plan
			{
				get
				{
					return filter;
				}
			}
		}
	}

	internal class Product : Plan
	{
		public readonly Plan outer;
		public readonly Plan inner;

		public Product(Plan outer, Plan inner)
		{
			this.outer = outer;
			this.inner = inner;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(outer), om.Demand(inner));
		}

		internal override Plan Clone(CloneMap cm)
		{
			return new Product(cm.Demand(outer), cm.Demand(inner));
		}

		internal class Op : Operator
		{
			public readonly Product product;
			public readonly Operator outer;
			public readonly Operator inner;

			public Op(Product product, Operator outer, Operator inner)
			{
				this.product = product;
				this.outer = outer;
				this.inner = inner;

				outer.Advance(); //incr to 1st element
			}

			internal override Plan plan
			{
				get
				{
					return product;
				}
			}

			internal override bool Advance()
			{
				if(!inner.Advance())
				{
					inner.Reset();
					return outer.Advance();
				}

				return true;
			}
		}
	}


	public static class ArrayUtil
	{
		public static T[] Append<T>(this T[] src, T elem)
		{
			T[] copy = new T[src.Length + 1];
			src.CopyTo(copy, 0);

			copy[copy.Length-1] = elem;

			return copy;
		}
	}
}