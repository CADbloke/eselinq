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
	internal class Scan : Plan
	{
		readonly Table table;

		internal Scan(Table table)
		{
			this.table = table;
		}

		public Operator ToOperator(OperatorMap om)
		{
			return new Op(this, new Cursor(table));
		}

		public Plan Clone(CloneMap cm)
		{
			return new Scan(table);
		}

		Table Plan.table
		{
			get
			{
				return table;
			}
		}

		public void Dispose()
		{
			table.Dispose();
		}

		internal class Op : Operator
		{
			readonly Cursor cursor;
			internal readonly Scan scan;

			internal Op(Plans.Scan scab, Cursor cursor)
			{
				this.cursor = cursor;
				this.scan = scab;
				cursor.MoveFirst();
				cursor.Move(-1);
			}

			public bool Advance()
			{
				return cursor.Move(1);
			}

			public void Reset()
			{
				cursor.MoveFirst();
				cursor.Move(-1);
			}

			public void Dispose()
			{
				cursor.Dispose();
			}

			public Plan plan
			{
				get
				{
					return scan;
				}
			}

			Cursor Operator.cursor
			{
				get
				{
					return cursor;
				}
			}

			public IList<Column> columns
			{
				get
				{
					return cursor.AsTable.Columns;
				}
			}
		}
	}

	internal class Spool : Plan
	{
		readonly Session sess;
		readonly Table.CreateTempOptions create_opts;
		readonly Plan src;
		readonly WriterPlan wplan;

		internal Spool(Session sess, Table.CreateTempOptions create_opts, Plan src, WriterPlan wplan)
		{
			this.sess = sess;
			this.create_opts = create_opts;
			this.src = src;
			this.wplan = wplan;
		}

		Table Plan.table
		{
			get
			{
				return null;
			}
		}

		Operator Plan.ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(src), wplan.ToWriter(om));
		}

		virtual public Plan Clone(CloneMap cm)
		{
			return new Spool(sess, create_opts, src, wplan);
		}

		public void Dispose()
		{
			wplan.Dispose();
		}

		internal class Op : Operator
		{
			readonly Spool spool;
			readonly Operator src;
			readonly Writer writer;
			Cursor cursor;
			Column[] cols;

			internal Op(Spool spool, Operator src, Writer writer)
			{
				this.spool = spool;
				this.src = src;
				this.writer = writer;
			}

			void Populate()
			{
				if(cursor == null)
				{
					cursor = Table.CreateTemp(spool.sess, spool.create_opts, out cols);
					writer.columns = cols;

					while(src.Advance())
						using(var upd = cursor.BeginInsert())
						{
							writer.Write(upd);
							upd.Complete();
						}

					src.Dispose(); //all results extracted: no longer needed

					cursor.MoveFirst();
					cursor.Move(-1);
				}
			}

			Cursor Operator.cursor
			{
				get
				{
					return cursor;
				}
			}

			IList<Column> Operator.columns
			{
				get
				{
					return cols;
				}
			}

			public virtual Plan plan
			{
				get
				{
					return spool;
				}
			}

			public virtual bool Advance()
			{
				return cursor.Move(1);
			}

			public virtual void Reset()
			{
				if(cursor != null)
				{
					cursor.MoveFirst();
					cursor.Move(-1);
				}
			}

			public void Dispose()
			{
				if(cursor != null)
					cursor.Dispose();
				writer.Dispose();
			}
		}
	}

	internal class SortSpool : Plan
	{
		readonly Session sess;
		readonly Table.CreateTempOptions create_opts;
		readonly Plan src;
		readonly WriterPlan wplan;
		readonly int seq_col_ix;

		internal SortSpool(Session sess, Table.CreateTempOptions create_opts, Plan src, WriterPlan wplan, int seq_col_ix)
		{
			this.sess = sess;
			this.create_opts = create_opts;
			this.src = src;
			this.wplan = wplan;
			this.seq_col_ix = seq_col_ix;
		}

		Table Plan.table
		{
			get
			{
				return null;
			}
		}

		Operator Plan.ToOperator(OperatorMap om)
		{
			return new Op(this, om.Demand(src), wplan.ToWriter(om));
		}

		virtual public Plan Clone(CloneMap cm)
		{
			return new Spool(sess, create_opts, src, wplan);
		}

		public void Dispose()
		{
			wplan.Dispose();
		}

		internal class Op : Operator
		{
			readonly SortSpool spool;
			readonly Operator src;
			readonly Writer writer;
			Cursor cursor;
			Column[] cols;
			int seq;
			bool populated;

			internal Op(SortSpool spool, Operator src, Writer writer)
			{
				this.spool = spool;
				this.src = src;
				this.writer = writer;
				this.seq = 0;

				this.cursor = Table.CreateTemp(spool.sess, spool.create_opts, out cols);
				this.writer.columns = cols;

				this.populated = false;
			}

			void Populate()
			{
				if(!populated)
				{
					while(src.Advance())
						using(var upd = cursor.BeginInsert())
						{
							writer.Write(upd);
							upd.Set(cols[spool.seq_col_ix], seq);
							seq++;
							upd.Complete();
						}

					src.Dispose(); //all results extracted: no longer needed

					cursor.MoveFirst();
					cursor.Move(-1);

					populated = true;
				}
			}

			Cursor Operator.cursor
			{
				get
				{
					return cursor;
				}
			}

			IList<Column> Operator.columns
			{
				get
				{
					return cols;
				}
			}

			public virtual Plan plan
			{
				get
				{
					return spool;
				}
			}

			public virtual bool Advance()
			{
				Populate();
				return cursor.Move(1);
			}

			public virtual void Reset()
			{
				if(populated)
				{
					cursor.MoveFirst();
					cursor.Move(-1);
				}
			}

			public void Dispose()
			{
				if(cursor != null)
					cursor.Dispose();
				writer.Dispose();
			}
		}
	}
}