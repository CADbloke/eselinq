using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;

using EseObjects;

namespace EseLinq.Plans
{
	/// <summary>
	/// ToPlans from a LINQ Expression to an EseLinq Plan.
	/// </summary>
	class Translator
	{
		internal enum ChannelType
		{
			/// <summary>
			/// Default. All fields null.
			/// </summary>
			Invalid = 0,
			/// <summary>
			/// Result of a calculation. Associated plan controls position, if present. Agg. fields by type.
			/// </summary>
			Calc,
			/// <summary>
			/// Single column selected from a ESE table. Agg. fields determined from Type.
			/// </summary>
			Column,
			/// <summary>
			/// Single ESE table. Agg. fields determined by Type.
			/// </summary>
			Table,
			/// <summary>
			/// Mapping of fields after New or selection or spool into a nonsource composite. Agg. fields by type.
			/// </summary>
			Collection
		}

		internal struct Channel
		{
			internal readonly ChannelType chtype;
			
			//only some are populated, based on rptype
			internal readonly CalcPlan cplan;
			internal readonly Plan plan;
			internal readonly Table table;
			internal readonly int column_index;
			internal readonly Type type;
			internal readonly Column.Type coltyp;
			internal readonly Dictionary<string, Channel> fields;
			internal readonly ConstructorInfo coninfo;
			internal readonly string name;

			/// <summary>
			/// Calculation without associated plan.
			/// </summary>
			internal Channel(CalcPlan cplan, Type type)
			{
				this.cplan = cplan;
				this.type = type;
				chtype = ChannelType.Calc;

				this.plan = null;
				this.table = null;
				this.column_index = 0;
				this.coltyp = Column.Type.Nil;
				this.fields = null;
				this.coninfo = null;
				this.name = null;
			}

			/// <summary>
			/// Plan with a specific single table.
			/// </summary>
			internal Channel(Plan plan, Table table, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.table = table;
				chtype = ChannelType.Table;

				this.cplan = null;
				this.column_index = 0;
				this.coltyp = Column.Type.Nil;
				this.fields = null;
				this.coninfo = null;
				this.name = table.Name;
			}

			/// <summary>
			/// Plan with a specific column.
			/// </summary>
			internal Channel(Plan plan, Table table, int column_index, Column.Type coltyp, Type type)
			{
				if(coltyp == Column.Type.Nil)
					throw new ArgumentException();

				this.plan = plan;
				this.table = table;
				this.column_index = column_index;
				this.coltyp = coltyp;
				this.type = type;
				this.name = table.Columns[column_index].Name;
				chtype = ChannelType.Column;

				this.cplan = null;
				this.fields = null;
				this.coninfo = null;
			}

			/// <summary>
			/// Plan with a specific column.
			/// </summary>
			internal Channel(Plan plan, int column_index, Column.Type coltyp, Type type)
			{
				if(coltyp == Column.Type.Nil)
					throw new ArgumentException();

				this.plan = plan;
				this.column_index = column_index;
				this.coltyp = coltyp;
				this.type = type;
				chtype = ChannelType.Column;

				this.name = null;
				this.cplan = null;
				this.table = null;
				this.fields = null;
				this.coninfo = null;
			}

			/// <summary>
			/// Plan with a specific column, selected by name.
			/// </summary>
			internal Channel(Plan plan, string colname, Type type)
			{
				//not necessairily in here, could be using ExpandFieldAttribute
				var columns = plan.table.Columns;

				for(int i = 0; i < columns.Count; i++)
					if(columns[i].Name == colname)
					{
						this.plan = plan;
						table = plan.table;
						column_index = i;
						this.type = type;
						this.name = table.Columns[i].Name;
						this.coltyp = columns[i].ColumnType;
						chtype = ChannelType.Column;

						this.cplan = null;
						this.fields = null;
						this.coninfo = null;

						return;
					}

				throw new ApplicationException("Column not found:" + colname);
			}

			/// <summary>
			/// Plan with specific calculation.
			/// </summary>
			internal Channel(Plan plan, CalcPlan cplan, Type type)
			{
				this.plan = plan;
				this.type = type;
				this.cplan = cplan;
				chtype = ChannelType.Calc;
				
				this.table = null;
				this.column_index = 0;
				this.fields = null;
				this.coltyp = Column.Type.Nil;
				this.coninfo = null;
				this.name = null;
			}

			internal Channel(Plan plan, Dictionary<string, Channel> fields, ConstructorInfo coninfo)
			{
				this.plan = plan;
				this.fields = fields;
				this.coninfo = coninfo;
				this.type = coninfo.DeclaringType;
				chtype = ChannelType.Collection;

				this.cplan = null;
				this.table = null;
				this.column_index = 0;
				this.coltyp = Column.Type.Nil;
				this.name = null;
			}

			internal Channel(Plan plan, Dictionary<string, Channel> fields, Type type)
			{
				this.plan = plan;
				this.fields = fields;
				this.type = type;
				chtype = ChannelType.Collection;

				this.cplan = null;
				this.table = null;
				this.column_index = 0;
				this.coltyp = Column.Type.Nil;
				this.coninfo = null;
				this.name = null;
			}

			internal CalcPlan AsCalcPlan(Type type)
			{
				switch(chtype)
				{
				case ChannelType.Calc:
					return cplan;

				case ChannelType.Column:
					return new Retrieve(plan, column_index, type);

				case ChannelType.Table:
					return MakeObject.AutoCreate(plan, type);

				case ChannelType.Collection:
					{
						CalcPlan[] arguments = (from chan in fields.Values
											   select chan.AsCalcPlan()).ToArray();

						if(coninfo == null)
							return MakeObject.AutoCreate(plan, type);
						else
							return new MakeObjectFromConstructor(arguments, coninfo);
					}

				default:
					throw new ArgumentException("Invalid rptype");
				}
			}

			internal WriterPlan AsWriterPlan(int dst_index)
			{
				switch(chtype)
				{
				case ChannelType.Calc:
					throw new NotImplementedException();

				case ChannelType.Column:
					return new DirectCopy(plan, column_index, dst_index);

				case ChannelType.Table:
					throw new NotImplementedException();

				case ChannelType.Collection:
					throw new NotImplementedException();

				default:
					throw new ArgumentException("Invalid rptype");
				}
			}

			internal Dictionary<string, Channel> AsFieldList(Downstream opts)
			{
				switch(chtype)
				{
				case ChannelType.Calc:
					{
						var ret = new Dictionary<string, Channel>();

						//special case for atomic types
						if(opts.type_map.ContainsKey(type))
						{
							ret.Add("#SINGLETON", this);
							return ret;
						}
					
						foreach(FieldInfo fi in type.GetFields(BindingFlags.Public | BindingFlags.Instance))
							if(null == Attribute.GetCustomAttribute(fi, typeof(Storage.NonpersistentFieldAttribute))) //don't save if nonpersistent
								ret.Add(fi.Name, new Channel(new FieldAccess(cplan, fi), fi.FieldType));

						//TODO: needs property access adapter
						//foreach(PropertyInfo pi in type.GetProperties(BindingFlags.Public | BindingFlags.Instance))
						//    if(null == Attribute.GetCustomAttribute(pi, typeof(PersistentPropertyAttribute))) //only save if a persistent property
						//        MakeColumnCreateOptions(tco.Columns, pi, pi.PropertyType, "", TypeMap);

						return ret;
					}

				case ChannelType.Column:
					{
						var ret = new Dictionary<string, Channel>();

						ret.Add("#SINGLETON", this);

						return ret;
					}

				case ChannelType.Table:
					{
						var ret = new Dictionary<string, Channel>();

						IList<Column> cols = table.Columns;

						for(int i = 0; i < cols.Count; i++)
							ret.Add(cols[i].Name, new Channel(plan, table, i, cols[i].ColumnType, opts.coltyp_map[cols[i].ColumnType]));

						return ret;
					}

				case ChannelType.Collection:
					return fields;

				default:
					throw new ArgumentException("Invalid rptype");
				}
			}

			internal Column.CreateOptions AsCreateColumn(Downstream opts)
			{
				switch(chtype)
				{
				case ChannelType.Calc:
					return new Column.CreateOptions
					{
						Type = opts.type_map[type]
					};

				case ChannelType.Column:
					return new Column.CreateOptions
					{
						Type = coltyp,
						CP = coltyp == Column.Type.LongText ? Column.CodePage.Unicode : Column.CodePage.Zero
					};

				case ChannelType.Table:
				case ChannelType.Collection:
					//expand into?
					throw new NotImplementedException();

				default:
					throw new ArgumentException("Invalid rptype");
				}
			}

			internal CalcPlan AsCalcPlan()
			{
				return AsCalcPlan(type);
			}
		}

		internal struct Downstream
		{
			internal Dictionary<string, Channel> env;
			internal Channel[] context;
			internal IDictionary<Type, Column.Type> type_map;
			internal IDictionary<Column.Type, Type> coltyp_map;
			internal Session session;

			///<summary>use this constructor when about to make modifications; copies mutable members</summary>
			internal Downstream(Downstream from)
			{
				this.env = new Dictionary<string, Channel>(from.env);
				if(from.context != null)
					this.context = from.context.ToArray();
				else
					this.context = null;
				this.type_map = from.type_map;
				this.coltyp_map = from.coltyp_map;
				this.session = from.session;
			}

			internal void Init()
			{
				env = new Dictionary<string, Channel>(16);
			}
		}

		internal struct Upstream
		{
			/// <summary>
			/// Immediate value returned in plan from translating expression.
			/// </summary>
			internal Channel chan;
			/// <summary>
			/// Plan used for scrolling, if applicable.
			/// Separate from Channel.plan and important to carry plans back up the call stack even the immediate value isn't a plan.
			/// </summary>
			internal Plan plan;

			/// <summary>
			/// Use for an explicitly separate plan.
			/// </summary>
			internal Upstream(Channel chan, Plan plan)
			{
				this.chan = chan;
				this.plan = plan;
			}

			/// <summary>
			/// Use when the only plan would be from the channel.
			/// </summary>
			internal Upstream(Channel chan)
			{
				this.chan = chan;
				this.plan = chan.plan;
			}

			internal Upstream WithChannel(Channel chan)
			{
				return new Upstream(chan, plan);
			}

			internal Upstream WithPlan(Plan plan)
			{
				return new Upstream(chan, plan);
			}
		}

		/// <summary>
		/// Not intended to be called directly; used in an expression to inject upstream state.
		/// </summary>
		public static IQueryable<T> InjectQueryClone<T>(Query<T> q)
		{
			return q;
		}

		internal static Upstream CloneQuery(QueryProperties query)
		{
			var cm = new CloneMap();
			var plan = query.plan.Clone(cm);

			if(query.table != null)
				return new Upstream(new Channel(plan, query.table, query.type), plan);

			return new Upstream(new Channel(plan, query.cplan, query.type), plan);
		}

		internal static Upstream TableSort(Downstream downs, Channel[] key, Channel nonkey, Plan src_plan, Type row_type)
		{
			//NEXT: for selections that are the same from the keys, no need to store again in body
			var nonkey_fields_dict = nonkey.AsFieldList(downs);
			var nonkey_fields = nonkey_fields_dict.ToArray();

			//columns indexes exist in blocks in this order
			int first_col = 0, first_nonkey_col = key.Length, seq_col = key.Length + nonkey_fields.Length;

			var col_creates = new Column.CreateOptions[seq_col + 1]; //one extra for seq itself
			var wplans = new WriterPlan[seq_col]; //don't need to write seq specifically, SortSpool handles that
			
			for(int i = 0; i < first_nonkey_col; i++)
			{
				col_creates[i] = key[i].AsCreateColumn(downs);
				col_creates[i].TTKey = true;
				//TODO: asc/desc
				wplans[i] = key[i].AsWriterPlan(i);
			}

			for(int i = first_nonkey_col; i < seq_col; i++)
			{
				col_creates[i] = nonkey_fields[i - first_nonkey_col].Value.AsCreateColumn(downs);
				wplans[i] = nonkey_fields[i - first_nonkey_col].Value.AsWriterPlan(i);
			}

			//add key field as ascending sequency to break unique ties (and maintain sort stability)
			//required since keys must be unique
			col_creates[key.Length + nonkey_fields.Length] = new Column.CreateOptions
			{
				TTKey = true,
				Type = Column.Type.Long
			};

			var ctto = new Table.CreateTempOptions
			{
				Columns = col_creates
			};

			var spool_plan = new SortSpool(downs.session, ctto, src_plan, new CompositeWriter(wplans), key.Length + nonkey_fields.Length);
			Channel spool_chan;

			//special case for single column nonkey value
			if(nonkey_fields_dict.ContainsKey("#SINGLETON"))
			{
				var single_field = nonkey_fields_dict["#SINGLETON"];
				var sf_coltyp = single_field.coltyp == Column.Type.Nil ? downs.type_map[single_field.type] : single_field.coltyp;
				var sf_type = single_field.type;
				spool_chan = new Channel(spool_plan, first_nonkey_col, sf_coltyp, sf_type);
			}
			else
			{
				var fields = new Dictionary<string, Channel>();

				for(int i = first_nonkey_col; i < seq_col; i++)
					fields.Add(nonkey_fields[i - first_nonkey_col].Key, new Channel(spool_plan, i, col_creates[i].Type, nonkey_fields[i - first_nonkey_col].Value.type));

				spool_chan = new Channel(spool_plan, fields, row_type);
			}

			return new Upstream(spool_chan, spool_plan);
		}

		internal static Upstream Translate(UnaryExpression exp, Downstream downs)
		{
			switch(exp.NodeType)
			{
			//don't care about quotes
			case ExpressionType.Quote:
				return Translate(exp.Operand, downs);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(BinaryExpression exp, Downstream downs)
		{
			switch(exp.NodeType)
			{
			default:
				{
					var left = Translate(exp.Left, downs);
					var right = Translate(exp.Right, downs);
					
					return new Upstream(new Channel(new BinaryCalc(exp.NodeType, left.chan.type, right.chan.type, exp.Type, left.chan.AsCalcPlan(), right.chan.AsCalcPlan()), exp.Type));
				}
			}
		}

		internal static Upstream Translate(ConditionalExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(ConstantExpression exp, Downstream downs)
		{
			return new Upstream(new Channel(new Constant(exp.Value), exp.Value.GetType()));
		}

		internal static Upstream Translate(InvocationExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(LambdaExpression exp, Downstream downs)
		{
			//copy value for upstream use
			downs = new Downstream(downs);

			for(int i = 0; i < exp.Parameters.Count; i++)
				downs.env.Add(exp.Parameters[i].Name, downs.context[i]);

			return Translate(exp.Body, downs);
		}

		internal static Upstream Translate(ListInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberExpression exp, Downstream downs)
		{
			{
				//TODO: should be runtime?
				var as_const = exp.Expression as ConstantExpression;
				if(as_const != null)
				{
					var field = (FieldInfo)exp.Member;
					object val = field.GetValue(as_const.Value);

					var query = val as QueryProperties;
					if(query != null)
						return CloneQuery(query);
					
					return new Upstream(new Channel(new Constant(val), exp.Type));
				}
			}

			{
				var as_parm = exp.Expression as ParameterExpression;
				if(as_parm != null)
				{
					var field = (FieldInfo)exp.Member;
					var chan = downs.env[as_parm.Name];

					switch(chan.chtype)
					{
					case ChannelType.Table:
						return new Upstream(new Channel(chan.plan, field.Name, exp.Type));

					case ChannelType.Calc:
						return new Upstream(new Channel(new FieldAccess(chan.cplan, field), exp.Type));

					case ChannelType.Column:
						//would need to access comma delimited names like ExpandFieldAttribute
						return new Upstream(new Channel(chan.plan, chan.name + "," + field.Name, exp.Type));

					case ChannelType.Collection:
						return new Upstream(chan.fields[field.Name]);
					
					default:
						throw IDontKnowWhatToDoWithThis(exp);
					}
				}
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MemberInitExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(MethodCallExpression exp, Downstream downs)
		{
			switch(exp.Method.DeclaringType.FullName)
			{
			case "System.Linq.Queryable":
				switch(exp.Method.Name)
				{
				case "Where":
					{
						var data = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { data.chan };
						var pred = Translate(exp.Arguments[1], downs);
						var body = new Filter(data.plan, pred.chan.AsCalcPlan());

						return data.WithPlan(body);
					}
				case "Select":
					{
						var data = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { data.chan };
						var body = Translate(exp.Arguments[1], downs);

						return new Upstream(body.chan, data.plan);
					}
				case "SelectMany":
					{
						var outer = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { outer.chan};

						var inner = Translate(exp.Arguments[1], downs);
						downs.context = new Channel[] { outer.chan, inner.chan };
						
						var body = Translate(exp.Arguments[2], downs);

						var product = new Product(outer.plan, inner.plan);

						return new Upstream(body.chan, product);
					}
				case "Distinct":
					{
						var data = Translate(exp.Arguments[0], downs);
						var elems = data.chan.AsCalcPlan();
						var hashtab = new MemoryHashDisctinctPlan(data.plan, elems, exp.Type);

						return new Upstream(new Channel((Plan)hashtab, (CalcPlan)hashtab, exp.Type));
					}
				case "Join":
					{
						var outer = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { outer.chan };
						var outer_key = Translate(exp.Arguments[2], downs);

						var inner = Translate(exp.Arguments[1], downs);
						downs.context = new Channel[] { inner.chan };
						var inner_key = Translate(exp.Arguments[3], downs);

						var join = new MemoryHashJoinPlan(inner.plan, inner_key.chan.AsCalcPlan(), inner.chan.AsCalcPlan(), outer.plan, outer_key.chan.AsCalcPlan());

						downs.context = new Channel[] { new Channel(join, exp.Arguments[1].Type), outer.chan };

						var body = Translate(exp.Arguments[4], downs);

						return new Upstream(body.chan, join);
					}
				case "OrderBy":
					{
						var body = Translate(exp.Arguments[0], downs);
						downs.context = new Channel[] { body.chan };
						var key0 = Translate(exp.Arguments[1], downs);

						return TableSort(downs, new Channel[] { key0.chan }, body.chan, body.plan, null);
					}
						
				default:
					throw IDontKnowWhatToDoWithThis(exp);
				}

			case "EseLinq.Plans.Translator":
				switch(exp.Method.Name)
				{
				case "InjectQueryClone":
					{
						var cons = (ConstantExpression)exp.Arguments[0];

						return CloneQuery((QueryProperties)cons.Value);
					}
				default:
					throw IDontKnowWhatToDoWithThis(exp);
				}
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(NewArrayExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(NewExpression exp, Downstream downs)
		{
			var fields = new Dictionary<string, Channel>(exp.Arguments.Count);

			for(int i = 0; i < exp.Arguments.Count; i++)
				fields.Add(exp.Members[i].Name, Translate(exp.Arguments[i], downs).chan);

			return new Upstream(new Channel(null, fields, exp.Constructor));
		}

		internal static Upstream Translate(ParameterExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(TypeBinaryExpression exp, Downstream downs)
		{
			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Upstream Translate(Expression exp, Downstream downs)
		{
			{
				var tyexp = exp as UnaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as BinaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as TypeBinaryExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as ConstantExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as ConditionalExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MemberExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MethodCallExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as LambdaExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as NewExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as MemberInitExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as ListInitExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as NewArrayExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}
			{
				var tyexp = exp as InvocationExpression;
				if(tyexp != null)
					return Translate(tyexp, downs);
			}

			throw IDontKnowWhatToDoWithThis(exp);
		}

		internal static Exception IDontKnowWhatToDoWithThis(Expression exp)
		{
			return new ArgumentException("Unknown expression " + exp.Type + ":" + exp.NodeType);
		}
	}
}
