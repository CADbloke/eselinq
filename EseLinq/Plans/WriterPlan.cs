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
	class DirectCopy : WriterPlan
	{
		internal readonly Plan src;
		internal readonly int src_index;
		internal readonly int dst_index;

		internal DirectCopy(Plan src, int src_index, int dst_index)
		{
			this.src = src;
			this.src_index = src_index;
			this.dst_index = dst_index;
		}

		public void Dispose()
		{
			src.Dispose();
		}

		public Writer ToWriter(OperatorMap om)
		{
			Operator src_op = om.Demand(src);

			return new Op(this, src_op.cursor, src_op.columns[src_index], dst_index);
		}

		internal class Op : Writer
		{
			internal readonly DirectCopy plan;
			internal readonly Cursor src_csr;
			internal readonly Column src_col;
			internal readonly int dst_index;
			internal Column dst_col;

			internal Op(DirectCopy plan, Cursor src_csr, Column src_col, int dst_index)
			{
				this.plan = plan;
				this.src_csr = src_csr;
				this.src_col = src_col;
				this.dst_index = dst_index;
			}

			public void Write(IWriteRecord wr)
			{
				wr.Set(dst_col, src_csr, src_col);
			}

			public Column[] columns
			{
				set
				{
					this.dst_col = value[dst_index];
				}
			}

			public void Dispose()
			{
				src_csr.Dispose();
			}
		}
	}

	class CompositeWriter : WriterPlan
	{
		internal readonly WriterPlan[] wplans;

		internal CompositeWriter(WriterPlan[] wplans)
		{
			this.wplans = wplans;
		}

		public Writer ToWriter(OperatorMap om)
		{
			var writers = new Writer[wplans.Length];

			for(int i = 0; i < wplans.Length; i++)
				writers[i] = wplans[i].ToWriter(om);

			return new Op(writers);
		}

		public void Dispose()
		{
			foreach(var wplan in wplans)
				wplan.Dispose();
		}
		
		internal class Op : Writer
		{
			internal readonly Writer[] writers;

			internal Op(Writer[] writers)
			{
				this.writers = writers;
			}

			public void Write(IWriteRecord wr)
			{
				foreach(var wtr in writers)
					wtr.Write(wr);
			}

			public Column[] columns
			{
				set
				{
					foreach(var wtr in writers)
						wtr.columns = value;
				}
			}

			public void Dispose()
			{
				foreach(var wtr in writers)
					wtr.Dispose();
			}
		}
	}
}