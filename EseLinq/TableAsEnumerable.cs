using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;

using EseObjects;
using EseLinq.Storage;

namespace EseLinq
{
	/// <summary>
	/// Provides extension methods to EseObjects for Enumerable and Queryable interfaces.
	/// </summary>
	public static class TableAsEnumerableExt
	{
		/// <summary>
		/// Provides IEnumerable interface for a table by scanning its contents using a default row bridge.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		public static TableAsEnumerable<T> AsEnumerable<T>(this Table table)
		{
			return new TableAsEnumerable<T>(table);
		}

		/// <summary>
		/// Provides IEnumerable interface for a table by scanning its contents using a specified row bridge.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		/// <param name="bridge">Bridge to use for retrieving rows.</param>
		public static TableAsEnumerable<T> AsEnumerable<T>(this Table table, IRecordBridge<T> bridge)
		{
			return new TableAsEnumerable<T>(table, bridge);
		}

		/// <summary>
		/// Provides IQueryable interface for a table by scanning its contents into LINQ to Objects and a default row bridge.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		/// <returns>LINQ to Objects based queryable object.</returns>
		public static IQueryable<T> AsQueryable<T>(this Table table)
		{
			return table.AsEnumerable<T>().AsQueryable<T>();
		}

		/// <summary>
		/// Provides IQueryable interface for a table by scanning its contents into LINQ to Objects and a specific row bridge.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		/// <param name="bridge">Specified bridge for retrieving rows.</param>
		/// <returns>LINQ to Objects based queryable object.</returns>
		public static IQueryable<T> AsQueryable<T>(this Table table, IRecordBridge<T> bridge)
		{
			return table.AsEnumerable<T>(bridge).AsQueryable<T>();
		}

		/// <summary>
		/// Provides IQueryable interface for a table using EseLinq.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		/// <param name="provider">Instance of EseLinq provider.</param>

		public static IQueryable<T> AsQueryable<T>(this Table table, Provider provider)
		{
			Expression<Func<IQueryable<T>>> exp = () => table.AsEnumerable<T>().AsQueryable<T>();

			return new Query<T>(provider, exp);
		}

		/// <summary>
		/// Provides IQueryable interface for a table using EseLinq.
		/// </summary>
		/// <typeparam name="T">Type to bridge rows into.</typeparam>
		/// <param name="table">Source table.</param>
		/// <param name="provider">Instance of EseLinq provider.</param>
		public static IQueryable<T> AsQueryable<T>(this Table table, Provider provider, IRecordBridge<T> bridge)
		{
			Expression<Func<IQueryable<T>>> exp = () => table.AsEnumerable<T>(bridge).AsQueryable<T>();

			return new Query<T>(provider, exp);			
		}
	}

	/// <summary>
	/// Provides an IEnumerable instance for a table by creating a cursor for the enuerator that scans the contents of the table.
	/// </summary>
	/// <typeparam name="T">Type to bridge retrieved rows into.</typeparam>
	public class TableAsEnumerable<T> : IEnumerable, IEnumerable<T>
	{
		readonly Table table;
		readonly IRecordBridge<T> bridge;

		public TableAsEnumerable(Table table)
		{
			this.table = table;
			this.bridge = new Flat<T>(table);
		}

		public TableAsEnumerable(Table table, IRecordBridge<T> bridge)
		{
			this.table = table;
			this.bridge = bridge;
		}

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			return new Enumerator(table, bridge);
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return new Enumerator(table, bridge);
		}

		internal class Enumerator : IEnumerator, IEnumerator<T>, IDisposable
		{
			readonly Cursor cursor;
			readonly IRecordBridge<T> bridge;

			internal Enumerator(Table tab, IRecordBridge<T> bridge)
			{
				this.cursor = new Cursor(tab);
				this.bridge = bridge;
				Reset();
			}

			T IEnumerator<T>.Current
			{
				get
				{
					return bridge.Read(cursor);
				}
			}

			object IEnumerator.Current
			{
				get
				{
					return bridge.Read(cursor);
				}
			}

			public bool MoveNext()
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
		}
	}

	/// <summary>
	/// Provides an IEnumerable instance for a table by creating a cursor for the enuerator that scans the contents of the table.
	/// </summary>
	/// <typeparam name="T">Type to bridge retrieved rows into.</typeparam>
	public class KeyRangeAsEnumerable<T> : IEnumerable, IEnumerable<T>
	{
		readonly Table table;
		readonly IRecordBridge<T> bridge;
		readonly Seekable start_key;
		readonly Limitable end_key;
		readonly int direction;

		KeyRangeAsEnumerable(Table table, Seekable start_key, Limitable end_key, int direction)
		{
			this.table = table;
			this.bridge = new Flat<T>(table);
			this.start_key = start_key;
			this.end_key = end_key;
			this.direction = direction;
		}

		KeyRangeAsEnumerable(Table table, Seekable start_key, Limitable end_key, int direction, IRecordBridge<T> bridge)
		{
			this.table = table;
			this.bridge = bridge;
			this.start_key = start_key;
			this.end_key = end_key;
			this.direction = direction;
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a forward-scrolling key range delimited by the specified keys.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewForward(Table table, Seekable start_key, Limitable end_key)
		{
			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, 1);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a forward-scrolling key range delimited by the specified keys.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewForward(Table table, Seekable start_key, Limitable end_key, IRecordBridge<T> bridge)
		{
			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, 1, bridge);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a forward-scrolling key range using a single field as the prefix to a wildcard in a range that includes all values starting with that key.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewForward(Table table, Column col, object k1)
		{
			var field = new Field(col, k1);
			var start_key = new FieldPosition(new Field[] { field }, Match.WildcardStart, SeekRel.GE);
			var end_key = new FieldPosition(new Field[] { field }, Match.WildcardEnd, SeekRel.LE);
			
			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, 1);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a forward-scrolling key range using a single field as the prefix to a wildcard in a range that includes all values starting with that key.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewForward(Table table, string col, object k1)
		{
			var field = new Field(new Column(table, col), k1);
			var start_key = new FieldPosition(new Field[] { field }, Match.WildcardStart, SeekRel.GE);
			var end_key = new FieldPosition(new Field[] { field }, Match.WildcardEnd, SeekRel.LE);

			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, 1);
		}


		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a backward-scrolling key range delimited by the specified keys.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewBackward(Table table, Seekable start_key, Limitable end_key)
		{
			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, -1);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a backward-scrolling key range delimited by the specified keys.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewBackward(Table table, Seekable start_key, Limitable end_key, IRecordBridge<T> bridge)
		{
			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, -1, bridge);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a backward-scrolling key range using a single field as the prefix to a wildcard in a range that includes all values starting with that key.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewBackward(Table table, Column col, object k1)
		{
			var field = new Field(col, k1);
			var start_key = new FieldPosition(new Field[] { field }, Match.WildcardStart, SeekRel.GE);
			var end_key = new FieldPosition(new Field[] { field }, Match.WildcardEnd, SeekRel.LE);

			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, -1);
		}

		/// <summary>
		/// Creates a KeyRangeAsEnumerable for a backward-scrolling key range using a single field as the prefix to a wildcard in a range that includes all values starting with that key.
		/// </summary>
		public static KeyRangeAsEnumerable<T> NewBackward(Table table, string col, object k1)
		{
			var field = new Field(new Column(table, col), k1);
			var start_key = new FieldPosition(new Field[] { field }, Match.WildcardStart, SeekRel.GE);
			var end_key = new FieldPosition(new Field[] { field }, Match.WildcardEnd, SeekRel.LE);

			return new KeyRangeAsEnumerable<T>(table, start_key, end_key, -1);
		}


		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			return new Enumerator(table, this);
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return new Enumerator(table, this);
		}

		internal class Enumerator : IEnumerator, IEnumerator<T>, IDisposable
		{
			readonly Cursor cursor;
			readonly KeyRangeAsEnumerable<T> parent;

			internal Enumerator(Table tab, KeyRangeAsEnumerable<T> parent)
			{
				this.cursor = new Cursor(tab);
				this.parent = parent;
				Reset();
			}

			T IEnumerator<T>.Current
			{
				get
				{
					return parent.bridge.Read(cursor);
				}
			}

			object IEnumerator.Current
			{
				get
				{
					return parent.bridge.Read(cursor);
				}
			}

			public bool MoveNext()
			{
				return cursor.Move(parent.direction);
			}

			public void Reset()
			{
				if(parent.direction > 0)
					cursor.ForwardRange(parent.start_key, parent.end_key);
				else
					cursor.BackwardRange(parent.start_key, parent.end_key);
				cursor.Move(-parent.direction);
			}

			public void Dispose()
			{
				cursor.Dispose();
			}
		}
	}
}
