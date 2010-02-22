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
	using OperatorMap = Dictionary<Plan, Operator>;
	
	//split into TablePlan and MemoryPlan
	abstract class Plan : IDisposable
	{
		/// <summary>
		/// Tables underlying this plan node. Multiple tables are possible after a join.
		/// </summary>
		internal readonly Table[] tables;
		internal readonly MemoryTable[] mtables;

		protected Plan(Plan copy)
		{
			this.tables = copy.tables;
			this.mtables = copy.mtables;
		}

		protected Plan(Table[] tables, MemoryTable[] mtables)
		{
			this.tables = tables;
			this.mtables = mtables;
		}

		internal abstract Operator ToOperator(OperatorMap om);

		public virtual void Dispose()
		{}
	}

	internal abstract class Operator : IDisposable
	{
		/// <summary>
		/// Cursors underlying this operator. Multiple cursors are possible after a join.
		/// Should be the same count and order as Plan.tables;
		/// </summary>
		internal readonly Cursor[] cursors;
		internal readonly MemoryCursor[] mcursors;

		protected Operator(Cursor[] cursors, MemoryCursor[] mcursors)
		{
			this.cursors = cursors;
			this.mcursors = mcursors;
		}

		protected Operator(Operator copy)
		{
			this.cursors = copy.cursors;
			this.mcursors = copy.mcursors;
		}

		internal abstract Plan plan
		{
			get;
		}

		internal Cursor CorrespondingCursor(Table search)
		{
			Table[] tables = plan.tables;

			for(int i = 0; i < tables.Length; i++)
				if(tables[i] == search)
					return cursors[i];

			throw new ApplicationException("No corresponding cursor found for table");
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

	interface MemoryTable : IDisposable
	{
		MemoryCursor ToMemoryCursor(OperatorMap om);

		Type ObjType
		{
			get;
		}
		Type ElemType
		{
			get;
		}
	}

	interface MemoryCursor : IDisposable
	{
		MemoryTable MTable
		{
			get;
		}

		IEnumerable CurrentEnumerable
		{
			get;
		}
		IEnumerator CurrentEnumerator
		{
			get;
		}
	}

	internal class Filter : Plan
	{
		internal readonly Plan src;
		internal readonly CalcPlan predicate;

		internal Filter(Plan src, CalcPlan predicate)
			: base(src)
		{
			this.src = src;
			this.predicate = predicate;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			var src_op = src.ToOperator(om);

			var op = new Op(this, src_op, om);

			om.Add(this, op);

			return op;
		}

		internal class Op : Operator
		{
			Filter filter;
			Operator src;
			Calc predicate;

			internal Op(Filter filter, Operator src, OperatorMap om)
				: base(src)
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