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

	abstract class MemoryTable : IDisposable
	{
		internal abstract MemoryCursor Instantiate();

		internal abstract Type obj_type
		{
			get;
		}
		internal abstract Type elem_type
		{
			get;
		}
		public virtual void Dispose()
		{
		}
	}

	abstract class MemoryCursor : IDisposable
	{
		internal readonly MemoryTable mtable;
		internal readonly IEnumerable src;
		internal readonly IEnumerator pos;

		internal MemoryCursor(MemoryTable mtable)
		{
			this.mtable = mtable;
		}

		internal abstract IEnumerable enumerable
		{
			get;
		}
		internal abstract IEnumerator enumerator
		{
			get;
		}
		public virtual void Dispose()
		{
		}
	}

	internal abstract class Operator : IDisposable
	{
		/// <summary>
		/// Cursors underlying this operator. Multiple cursors are possible after a join.
		/// Should be the same count and order as Plan.tables;
		/// </summary>
		internal readonly Cursor[] cursors;
		internal readonly MemoryCursor[] mcursors;

		protected Operator(Cursor[] cursors)
		{
			this.cursors = cursors;
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

	internal abstract class Calc : IDisposable
	{
		internal abstract object value
		{
			get;
		}
		public virtual void Dispose()
		{}
	}

	internal abstract class CalcPlan : IDisposable
	{
		internal abstract Calc ToCalc(OperatorMap om);
		public virtual void Dispose()
		{}
	}

	internal class Scan : Plan
	{
		internal readonly Table src;

		internal Scan(Table table)
			: base(new Table[] {table}, new MemoryTable[0])
		{
			src = table;
		}

		internal override Operator ToOperator(OperatorMap om)
		{
			var csr = new Cursor(src);
			var op = new Op(this, csr);

			om.Add(this, op);

			return op;
		}

		public override void Dispose()
		{
			src.Dispose();
		}

		internal class Op : Operator
		{
			internal readonly Scan scan;
			internal readonly Cursor cursor;

			internal Op(Plans.Scan scab, Cursor cursor)
				: base(new Cursor[] {cursor})
			{
				this.scan = scab;
				this.cursor = cursor;
				cursor.MoveFirst();
				cursor.Move(-1);
			}

			internal override bool Advance()
			{
				return cursor.Move(1);
			}

			internal override void Reset()
			{
				cursor.MoveFirst();
			}

			public override void Dispose()
			{
				cursor.Dispose();
			}

			internal override Plan plan
			{
				get
				{
					return scan;
				}
			}
		}
	}

	internal class Retrieve : CalcPlan
	{
		internal readonly Plan plan;
		internal readonly Table table;
		internal readonly Column col;
		internal readonly Type type;

		internal Retrieve(Plan plan, Table table, Column col, Type type)
		{
			if(plan == null || table == null || col == null || type == null)
				throw new NullReferenceException();

			this.plan = plan;
			this.table = table;
			this.col = col;
			this.type = type;
		}

		internal override Calc ToCalc(OperatorMap om)
		{
			return new Op
			{
				plan = this,
				csr = om[plan].CorrespondingCursor(table)
			};
		}

		internal class Op : Calc
		{
			internal Retrieve plan;
			internal Cursor csr;

			internal override object value
			{
				get
				{
					return csr.Retrieve(plan.col, plan.type);
				}
			}
		}
	}

	internal class Constant : CalcPlan
	{
		internal readonly object value;

		internal Constant(object value)
		{
			this.value = value;
		}

		internal override Calc ToCalc(Dictionary<Plan, Operator> om)
		{
			return new Op
			{
				plan = this
			};
		}

		internal class Op : Calc
		{
			internal Constant plan;

			internal override object value
			{
				get
				{
					return plan.value;
				}
			}
		}
	}

	internal class BinaryCalc : CalcPlan
	{
		internal readonly ExpressionType func;
		internal readonly CalcPlan left;
		internal readonly CalcPlan right;

		internal BinaryCalc(ExpressionType func, CalcPlan left, CalcPlan right)
		{
			this.func = func;
			this.left = left;
			this.right = right;
		}

		internal override Calc ToCalc(Dictionary<Plan, Operator> om)
		{
			return new Op
			{
				plan = this,
				left = left.ToCalc(om),
				right = right.ToCalc(om)
			};
		}

		internal class Op : Calc
		{
			internal BinaryCalc plan;
			internal Calc left;
			internal Calc right;

			internal override object value
			{
				get
				{
					switch(plan.func)
					{
					case ExpressionType.Equal:
						return left.value.Equals(right.value);

					default:
						throw new ArgumentException("Unknown calculation type " + plan.func.ToString());
					}
				}
			}
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
				: base(src.cursors)
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

	internal class MemoryHashDistinct<T> : MemoryTable
	{
		Plan src;

		internal MemoryHashDistinct(Plan src)
		{
			this.src = src;
		}

		internal override MemoryCursor Instantiate()
		{
			throw new NotImplementedException();
		}

		internal override Type obj_type
		{
			get
			{
				return typeof(HashSet<T>);
			}
		}

		internal override Type elem_type
		{
			get
			{
				return typeof(T);
			}
		}

		internal class Op : MemoryCursor
		{
			HashSet<T> hashset;
			HashSet<T>.Enumerator position;

			Plan src;

			Op(MemoryHashDistinct<T> mtable)
				: base(mtable)
			{
			}

			internal override IEnumerable enumerable
			{
				get
				{
					return hashset;
				}
			}

			internal override IEnumerator enumerator
			{
				get
				{
					return position;
				}
			}
		}
	}

	//internal class MemoryHashJoin<K, V> : MemoryTable
	//{
	//    internal override Type obj_type
	//    {
	//        get
	//        {
	//            return typeof(Dictionary<K, V>);
	//        }
	//    }

	//    internal override Type elem_type
	//    {
	//        get
	//        {
	//            return typeof(V);
	//        }
	//    }

	//    internal override MemoryCursor Instantiate()
	//    {
	//        throw new NotImplementedException();
	//    }

	//    internal class Op : MemoryCursor
	//    {
	//        Dictionary<K, V> dict;
	//        IEnumerator<V> position;



	//        internal override IEnumerable enumerable
	//        {
	//            get
	//            {
	//                return dict;
	//            }
	//        }

	//        internal override IEnumerator enumerator
	//        {
	//            get
	//            {
	//                return position;
	//            }
	//        }
	//    }
	//}

	internal class MakeObject
	{
		public static CalcPlan AutoCreate<T>(Plan src)
		{
			if(src.tables.Length == 1) //one single table
				return new MakeObjectFromBridge<T>(src, new Storage.Flat<T>(src.tables[0]));
			else
				return null; //TODO: memberwise input
		}

		internal static CalcPlan AutoCreate(Plan src, Type type)
		{
			return (CalcPlan)typeof(MakeObject)
				.GetMethod("AutoCreate")
				.MakeGenericMethod(type)
				.Invoke(null, new Object[] { src });
		}
	}

	internal class MakeObjectFromBridge<T> : CalcPlan
	{
		internal readonly Plan src;
		internal readonly IRecordBridge<T> bridge;

		internal MakeObjectFromBridge(Plan src, IRecordBridge<T> bridge)
		{
			this.src = src;
			this.bridge = bridge;
		}

		internal override Calc ToCalc(Dictionary<Plan, Operator> om)
		{
			Operator src_op = om[src];

			return new Op(this, src_op.cursors[0]); //single cursor
		}

		internal class Op : Calc
		{
			internal readonly MakeObjectFromBridge<T> make_object;
			internal readonly Cursor cursor;

			internal Op(MakeObjectFromBridge<T> make_object, Cursor cursor)
			{
				this.make_object = make_object;
				this.cursor = cursor;
			}

			internal override object value
			{
				get
				{
					return make_object.bridge.Read(cursor);
				}
			}
		}
	}
}