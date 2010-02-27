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
	internal class MemoryHashDisctinctPlan : Plan, MemoryTable, CalcPlan
	{
		readonly Plan src;
		readonly CalcPlan value_src;
		readonly Type elem_type;

		internal MemoryHashDisctinctPlan(Plan src, CalcPlan value_src, Type elem_type)
		{
			this.src = src;
			this.value_src = value_src;
			this.elem_type = elem_type;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(src), value_src.ToCalc(om));
		}

		internal override Plan Clone(CloneMap cm)
		{
			return new MemoryHashDisctinctPlan(cm.Demand(src), value_src, elem_type);
		}

		public MemoryCursor ToMemoryCursor(OperatorMap om)
		{
			return (MemoryCursor)om[this];
		}

		public Calc ToCalc(OperatorMap om)
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
			{
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
					value.Dispose();

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
			value_src.Dispose();
		}
	}

	/// <summary>
	/// Builds a hash map (Collections.Dictionary) from the inner source and matches with the outer source.
	/// </summary>
	internal class MemoryHashJoinPlan : Plan, CalcPlan
	{
		readonly Plan inner_plan;
		readonly CalcPlan inner_key;
		readonly CalcPlan inner_value;

		readonly Plan outer_plan;
		readonly CalcPlan outer_key;

		internal MemoryHashJoinPlan(Plan inner_plan, CalcPlan inner_key, CalcPlan inner_value, Plan outer_plan, CalcPlan outer_key)
		{
			this.inner_plan = inner_plan;
			this.outer_plan = outer_plan;
			this.inner_key = inner_key;
			this.inner_value = inner_value;
			this.outer_key = outer_key;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(inner_plan), inner_key.ToCalc(om), inner_value.ToCalc(om), om.Demand(outer_plan), outer_key.ToCalc(om));
		}

		internal override Plan Clone(CloneMap cm)
		{
			return new MemoryHashJoinPlan(cm.Demand(inner_plan), inner_key, inner_value, cm.Demand(outer_plan), outer_key);
		}

		public Calc ToCalc(OperatorMap om)
		{
			return (Calc)om.Demand(this);
		}

		internal class Op : Operator, Calc
		{
			readonly MemoryHashJoinPlan mtable;

			Dictionary<object, List<object>> hashtable;
			List<object>.Enumerator position;

			readonly Operator inner_operator;
			readonly Calc inner_key;
			readonly Calc inner_value;
			
			readonly Operator outer_operator;
			readonly Calc outer_key;

			internal Op(MemoryHashJoinPlan mtable, Operator inner_operator, Calc inner_key, Calc inner_value, Operator outer_operator, Calc outer_key)
			{
				this.mtable = mtable;
				this.inner_operator = inner_operator;
				this.inner_key = inner_key;
				this.inner_value = inner_value;
				this.outer_operator = outer_operator;
				this.outer_key = outer_key;
			}

			void Insert(object k, object v)
			{
				List<object> list;

				if(!hashtable.TryGetValue(k, out list))
					hashtable.Add(k, list = new List<object>());

				list.Add(v);
			}

			void Populate()
			{
				hashtable = new Dictionary<object, List<object>>(64);

				while(inner_operator.Advance())
					Insert(inner_key.value, inner_value.value);

				inner_operator.Dispose(); //all results extracted: no longer needed
				inner_key.Dispose();
				inner_value.Dispose();
			}

			internal override Plan plan
			{
				get
				{
					return mtable;
				}
			}

			bool NextKey()
			{
				List<object> list;

				while(outer_operator.Advance())
				{
					if(hashtable.TryGetValue(outer_key.value, out list))
					{
						position = list.GetEnumerator();
						return true;
					}
				}				

				return false; //noting left in outer operator
			}

			internal override bool Advance()
			{
				if(hashtable == null)
				{
					Populate();
					if(!NextKey())
						return false; //no results
				}

				if(position.MoveNext()) //for each in the inner rows
					return true;
				else
					return NextKey() && position.MoveNext(); //for each in the outer rows
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
			inner_plan.Dispose();
			inner_key.Dispose();
			inner_value.Dispose();
			outer_plan.Dispose();
			outer_key.Dispose();
		}
	}
}