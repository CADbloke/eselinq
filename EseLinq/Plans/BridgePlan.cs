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
	internal class MakeObject
	{
		public static CalcPlan AutoCreate<T>(Plan src)
		{
			return new MakeObjectFromBridge<T>(src, new Storage.Flat<T>(src.table));
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

		public Calc ToCalc(OperatorMap om)
		{
			Operator src_op = om[src];

			return new Op(this, src_op.cursor); //single cursor
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

			public object value
			{
				get
				{
					return make_object.bridge.Read(cursor);
				}
			}

			public void Dispose()
			{
			}
		}

		public void Dispose()
		{
			src.Dispose();
		}
	}

	internal class MakeObjectFromConstructor : CalcPlan
	{
		CalcPlan[] sources;
		ConstructorInfo ctor;

		public MakeObjectFromConstructor(CalcPlan[] sources, ConstructorInfo ctor)
		{
			this.sources = sources;
			this.ctor = ctor;
		}

		public Calc ToCalc(OperatorMap om)
		{
			Calc[] calc_sources = new Calc[sources.Length];

			for(int i = 0; i < sources.Length; i++)
				calc_sources[i] = sources[i].ToCalc(om);

			return new Op(this, calc_sources);
		}

		public void Dispose()
		{
			foreach(var i in sources)
				i.Dispose();
		}

		internal class Op : Calc
		{
			MakeObjectFromConstructor plan;
			Calc[] sources;

			public Op(MakeObjectFromConstructor plan, Calc[] sources)
			{
				this.plan = plan;
				this.sources = sources;
			}

			public object value
			{
				get
				{
					object[] fields = new object[sources.Length];

					for(int i = 0; i < sources.Length; i++)
						fields[i] = sources[i].value;

					return plan.ctor.Invoke(fields);
				}
			}

			public void Dispose()
			{
				foreach(var i in sources)
					i.Dispose();
			}
		}

	}

}