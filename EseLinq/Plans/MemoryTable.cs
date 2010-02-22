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

	internal class MemoryHashDisctinctPlan : Plan, MemoryTable, CalcPlan
	{
		readonly Plan src;
		readonly CalcPlan value_src;
		readonly Type elem_type;

		internal MemoryHashDisctinctPlan(Plan src, CalcPlan value_src, Type elem_type)
			: base(src.tables, src.mtables.Append(null))
		{
			mtables[mtables.Length-1] = this; //since "this" wasn't availaible one line up

			this.src = src;
			this.value_src = value_src;
			this.elem_type = elem_type;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			Op op = new Op(this, src.ToOperator(om), value_src.ToCalc(om));

			om.Add(this, op);

			return op;
		}

		public MemoryCursor ToMemoryCursor(OperatorMap om)
		{
			return (MemoryCursor)om[this];
		}

		public Calc ToCalc(Dictionary<Plan, Operator> om)
		{
			return (Calc)om[this];
		}

		public Type ObjType
		{
			get
			{
				return elem_type;
			}
		}

		public Type ElemType
		{
			get
			{
				return typeof(HashSet<object>);
			}
		}

		internal class Op : Operator, MemoryCursor, Calc
		{
			readonly MemoryHashDisctinctPlan mtable;

			HashSet<object> hashset;
			HashSet<object>.Enumerator position;

			readonly Operator src;
			readonly Calc value;

			internal Op(MemoryHashDisctinctPlan mtable, Operator src, Calc value)
				: base(src.cursors, src.mcursors.Append(null))
			{
				mcursors[mcursors.Length-1] = this; //since "this" wasn't availaible one line up

				this.mtable = mtable;
				this.src = src;
				this.value = value;
			}

			void Populate()
			{
				if(hashset == null)
				{
					hashset = new HashSet<object>();

					while(src.Advance())
						hashset.Add(value.value);

					src.Dispose(); //all results extracted: no longer needed

					position = hashset.GetEnumerator();
				}
			}

			public IEnumerable CurrentEnumerable
			{
				get
				{
					Populate();
					return hashset;
				}
			}

			public IEnumerator CurrentEnumerator
			{
				get
				{
					Populate();
					return position;
				}
			}

			internal override Plan plan
			{
				get
				{
					return mtable;
				}
			}

			public MemoryTable MTable
			{
				get
				{
					return mtable;
				}
			}

			internal override bool Advance()
			{
				Populate();
				return position.MoveNext();
			}

			object Calc.value
			{
				get
				{
					return position.Current;
				}
			}
		}

		void IDisposable.Dispose()
		{
			src.Dispose();
		}
	}
}