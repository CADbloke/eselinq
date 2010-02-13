using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

using EseObjects;
using EseLinq.Storage;

namespace EseLinq.Plans
{
	using OperatorMap = Dictionary<Plan, Operator>;
	
	abstract class Plan : IDisposable
	{
		/// <summary>
		/// Tables underlying this plan node. Multiple tables are possible after a join.
		/// </summary>
		internal readonly Table[] tables;

		protected Plan(Table[] tables)
		{
			this.tables = tables;
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

	internal abstract class Scalar : IDisposable
	{
		internal abstract object value
		{
			get;
		}
		public virtual void Dispose()
		{}
	}

	internal abstract class ScalarPlan : IDisposable
	{
		internal abstract Scalar ToScalar(OperatorMap om);

		public virtual void Dispose()
		{}
	}



	internal class Scan : Plan
	{
		internal readonly Table src;

		internal Scan(Table table)
			: base(new Table[] {table})
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

	internal class Retrieve : ScalarPlan
	{
		internal readonly Plan plan;
		internal readonly Table table;
		internal readonly Column col;
		internal readonly Type type;

		internal Retrieve(Plan plan, Table table, Column col, Type type)
		{
			this.plan = plan;
			this.table = table;
			this.col = col;
			this.type = type;
		}

		internal Retrieve(Plan plan, string colname, Type type)
		{
			Column found_col = null;
			Table found_table = null;

			foreach(Table t in plan.tables)
				foreach(Column c in t.Columns)
					if(c.Name == colname)
					{
						if(found_col == null)
						{
							found_col = c;
							found_table = t;
						}
						else
							throw new AmbiguousMatchException(string.Format("Multiple candidates for column {0}, including tables {1} and {2}", colname, found_table.Name, t.Name));
					}

			this.plan = plan;
			this.table = found_table;
			this.col = found_col;
			this.type = type;
		}

		internal override Scalar ToScalar(OperatorMap om)
		{
			return new Op
			{
				plan = this,
				csr = om[plan].CorrespondingCursor(table)
			};
		}

		internal class Op : Scalar
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

	internal class Constant : ScalarPlan
	{
		internal readonly object value;

		internal Constant(object value)
		{
			this.value = value;
		}

		internal override Scalar ToScalar(Dictionary<Plan, Operator> om)
		{
			return new Op
			{
				plan = this
			};
		}

		internal class Op : Scalar
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

	internal class BinaryCalc : ScalarPlan
	{
		internal readonly ExpressionType func;
		internal readonly ScalarPlan left;
		internal readonly ScalarPlan right;

		internal BinaryCalc(ExpressionType func, ScalarPlan left, ScalarPlan right)
		{
			this.func = func;
			this.left = left;
			this.right = right;
		}

		internal override Scalar ToScalar(Dictionary<Plan, Operator> om)
		{
			return new Op
			{
				plan = this,
				left = left.ToScalar(om),
				right = right.ToScalar(om)
			};
		}

		internal class Op : Scalar
		{
			internal BinaryCalc plan;
			internal Scalar left;
			internal Scalar right;

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
		internal readonly ScalarPlan predicate;

		internal Filter(Plan src, ScalarPlan predicate)
			: base(src.tables)
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
			Scalar predicate;

			internal Op(Filter filter, Operator src, OperatorMap om)
				: base(src.cursors)
			{
				this.filter = filter;
				this.src = src;
				this.predicate = filter.predicate.ToScalar(om);
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

	internal class MakeObject
	{
		internal static ScalarPlan AutoCreate<T>(Plan src)
		{
			if(src.tables.Length == 1) //one single table
				return new MakeObjectFromBridge<T>(src, new Storage.Flat<T>(src.tables[0]));
			else
				return null; //TODO: memberwise input
		}
	}

	internal class MakeObjectFromBridge<T> : ScalarPlan
	{
		internal readonly Plan src;
		internal readonly IRecordBridge<T> bridge;

		internal MakeObjectFromBridge(Plan src, IRecordBridge<T> bridge)
		{
			this.src = src;
			this.bridge = bridge;
		}

		internal override Scalar ToScalar(Dictionary<Plan, Operator> om)
		{
			Operator src_op = om[src];

			return new Op(this, src_op.cursors[0]); //single cursor
		}

		internal class Op : Scalar
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