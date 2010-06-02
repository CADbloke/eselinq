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
	
	interface Plan : IDisposable
	{
		/// <summary>
		/// Tables underlying this plan node. Multiple tables are possible after a join.
		/// </summary>
		Table table
		{
			get;
		}
		Operator ToOperator(OperatorMap om);
		Plan Clone(CloneMap cm);
	}

	internal interface Operator : IDisposable
	{
		/// <summary>
		/// Cursors underlying this operator. Multiple cursors are possible after a join.
		/// Should be the same count and order as Plan.tables;
		/// </summary>
		Cursor cursor
		{
			get;
		}
		IList<Column> columns
		{
			get;
		}
		Plan plan
		{
			get;
		}
		bool Advance();
		void Reset();
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

	internal interface Writer : IDisposable 
	{
		void Write(IWriteRecord wr);
		Column[] columns
		{
			set;
		}
	}

	internal interface WriterPlan : IDisposable 
	{
		Writer ToWriter(OperatorMap om);
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

		public Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(src), om);
		}

		public Plan Clone(CloneMap cm)
		{
			return new Filter(cm.Demand(src), predicate);
		}

		public Table table
		{
			get
			{
				return null;
			}
		}

		public void Dispose()
		{
			src.Dispose();
			predicate.Dispose();
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

			public bool Advance()
			{
				while(src.Advance())
					if(predicate.value.Equals(true)) //found a matching record
						return true;

				//ran out of rows
				return false;
			}

			public Plan plan
			{
				get
				{
					return filter;
				}
			}

			public Cursor cursor
			{
				get
				{
					return null;
				}
			}

			public IList<Column> columns
			{
				get
				{
					return null;
				}
			}

			public void Reset()
			{
				src.Reset();
			}

			public void Dispose()
			{
				src.Dispose();
				predicate.Dispose();
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

		public Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(outer), om.Demand(inner));
		}

		public Plan Clone(CloneMap cm)
		{
			return new Product(cm.Demand(outer), cm.Demand(inner));
		}

		public Table table
		{
			get
			{
				return null;
			}
		}

		public void Dispose()
		{
			throw new NotImplementedException();
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

			public Plan plan
			{
				get
				{
					return product;
				}
			}

			public bool Advance()
			{
				if(!inner.Advance())
				{
					inner.Reset();
					return outer.Advance();
				}

				return true;
			}

			public Cursor cursor
			{
				get
				{
					return null;
				}
			}

			public IList<Column> columns
			{
				get
				{
					return null;
				}
			}

			public void Reset()
			{
				inner.Reset();
				outer.Reset();

				outer.Advance(); //incr to 1st element
			}

			public void Dispose()
			{
				inner.Dispose();
				outer.Dispose();
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