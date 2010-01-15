///////////////////////////////////////////////////////////////////////////////
// Project     :  EseLinq http://code.google.com/p/eselinq/
// Copyright   :  (c) 2009 Christopher Smith
// Maintainer  :  csmith32@gmail.com
// Module      :  Storage.Flat
///////////////////////////////////////////////////////////////////////////////
//
//This software is licenced under the terms of the MIT License:
//
//Copyright (c) 2009 Christopher Smith
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Reflection;
using EseObjects;

using System.Runtime.Serialization.Formatters.Binary;

namespace EseLinq.Storage
{
	public class Flat<T> : IRecordLevelBridge<T>
	{
		//base linkage to .NET member
		protected abstract class MemberLink
		{
			public Type MemberType;

			public MemberLink(Type MemberType)
			{
				this.MemberType = MemberType;
			}

			public abstract object Get(object obj);
			public abstract void Set(object obj, object value);
		}

		//linkage to a field member
		protected class FieldLink : MemberLink
		{
			public FieldInfo Fi;

			public FieldLink(FieldInfo Fi) :
				base(Fi.FieldType)
			{
				this.Fi = Fi;
			}

			public override object Get(object obj)
			{
				return Fi.GetValue(obj);
			}
			public override void Set(object obj, object value)
			{
				Fi.SetValue(obj, value);
			}
		}

		//linkage to a property member
		protected class PropertyLink : MemberLink
		{
			public MethodInfo GetMi;
			public MethodInfo SetMi;

			public PropertyLink(PropertyInfo Pi) :
				base(Pi.PropertyType)
			{
				GetMi = Pi.GetGetMethod();
				SetMi = Pi.GetSetMethod();
			}

			public override object Get(object obj)
			{
				return GetMi.Invoke(obj, null);
			}
			public override void Set(object obj, object value)
			{
				object[] args = new object[1];
				args[0] = value;
				SetMi.Invoke(obj, args);
			}
		}

		//linkage to a ESE column. Defaults to a direct (bridged) linkage
		protected class ColumnLink
		{
			public MemberLink Ml;
			public Column Col;

			public ColumnLink(MemberLink Ml, Column Col)
			{
				this.Ml = Ml;
				this.Col = Col;
			}

			public virtual void SaveField(object obj, IWriteRecord wr)
			{
				wr.Set(Col, Ml.Get(obj));
			}

			public virtual void LoadField(object obj, IReadRecord rr)
			{
				Ml.Set(obj, rr.Retrieve(Col, Ml.MemberType));
			}
		}

		protected class BinaryColumnLink : ColumnLink
		{
			public BinaryFormatter formatter;

			public BinaryColumnLink(MemberLink Ml, Column Col) :
				base(Ml, Col)
			{
				formatter = new BinaryFormatter();
			}

			public override void SaveField(object obj, IWriteRecord wr)
			{
				System.IO.MemoryStream stream = new System.IO.MemoryStream();
				formatter.Serialize(stream, Ml.Get(obj));
				
				wr.Set(Col, stream.ToArray());
			}
			public override void LoadField(object obj, IReadRecord rr)
			{
				System.IO.MemoryStream stream = new System.IO.MemoryStream(rr.Retrieve<byte[]>(Col));

				Ml.Set(obj, formatter.Deserialize(stream));
			}
		}

		protected class XmlColumnLink : ColumnLink
		{
			public System.Xml.Serialization.XmlSerializer xmls;

			public XmlColumnLink(MemberLink Ml, Column Col) :
				base(Ml, Col)
			{
				xmls = (new System.Xml.Serialization.XmlSerializerFactory()).CreateSerializer(Ml.MemberType);
			}

			public override void SaveField(object obj, IWriteRecord wr)
			{
				System.IO.MemoryStream stream = new System.IO.MemoryStream();
				xmls.Serialize(stream, Ml.Get(obj));

				wr.Set(Col, System.Text.Encoding.UTF8.GetString(stream.ToArray(), 0, (int)stream.Position));
			}
			public override void LoadField(object obj, IReadRecord rr)
			{
				string str = rr.Retrieve<string >(Col);
				byte[] arr = System.Text.Encoding.UTF8.GetBytes(str);
				System.IO.MemoryStream stream = new System.IO.MemoryStream(arr);

				Ml.Set(obj, xmls.Deserialize(stream));
			}
		}

		protected class ExpandedColumnLink : ColumnLink
		{
			public ColumnLink[] Links;

			public ExpandedColumnLink(MemberLink Ml, string ColName, Table table) :
				base(Ml, null)
			{
				Links = LoadLinks(table, Ml.MemberType, ColName + ",");
			}

			public override void SaveField(object obj, IWriteRecord wr)
			{
				object member_obj = Ml.Get(obj);

				foreach(ColumnLink l in Links)
					l.SaveField(member_obj, wr);
			}

			public override void LoadField(object obj, IReadRecord rr)
			{
				object member_obj = Ml.Get(obj);

				foreach(ColumnLink l in Links)
					l.LoadField(member_obj, rr);

				Ml.Set(obj, member_obj); //set back in case the member is a value type
			}
		}

		protected class MultivaluedColumnLink : ColumnLink
		{
			IWriteRecord.SetOptions so;
			
			public MultivaluedColumnLink(MemberLink Ml, Column Col, Table table) :
				base(Ml, Col)
			{
				so.TagSequence = 0; //0 for append. Sets will always append values
			}

			public override void SaveField(object obj, IWriteRecord wr)
			{
				object[] arr = (object[])obj;

				//append all values
				foreach(object o in arr)
					wr.Set(Col, o, so);
			}

			public override void LoadField(object obj, IReadRecord rr)
			{
				Ml.Set(obj, rr.RetrieveAllValues<object>(Col)); //set back in case the member is a value type
			}
		}

		protected static ColumnLink MakeColumnLink(Table table, MemberInfo mi, MemberLink ml, string Prefix)
		{
			Attribute ColNameAtt = Attribute.GetCustomAttribute(mi, typeof(ColumnNameAttribute));
			string ColName;

			ColName = ColNameAtt != null ?
				((ColumnNameAttribute)ColNameAtt).Name :
				string.Concat(Prefix, mi.Name);

			if(Attribute.GetCustomAttribute(mi, typeof(ExpandFieldAttribute)) != null)
				return new ExpandedColumnLink(ml, ColName, table);

			Column Col = new Column(table, ColName);

			if(Attribute.GetCustomAttribute(mi, typeof(FieldMultivaluedAttribute)) != null)
				return new MultivaluedColumnLink(ml, Col, table);

			if(Attribute.GetCustomAttribute(mi, typeof(BinaryFieldSerializationAttribute)) != null)
				return new BinaryColumnLink(ml, Col);

			if(Attribute.GetCustomAttribute(mi, typeof(XmlFieldSerializationAttribute)) != null)
				return new XmlColumnLink(ml, Col);

			return new ColumnLink(ml, Col);
		}

		//creates linkages from metadata
		protected static ColumnLink[] LoadLinks(Table table, Type ty, string Prefix)
		{
			List<ColumnLink > links = new List<ColumnLink >();

			foreach(FieldInfo fi in ty.GetFields(BindingFlags.Public | BindingFlags.Instance))
				if(null == Attribute.GetCustomAttribute(fi, typeof(NonpersistentFieldAttribute))) //don't save if nonpersistent
					links.Add(MakeColumnLink(table, fi, new FieldLink(fi), Prefix));

			foreach(PropertyInfo pi in ty.GetProperties(BindingFlags.Public | BindingFlags.Instance))
				if(null == Attribute.GetCustomAttribute(pi, typeof(PersistentPropertyAttribute))) //only save if a persistent property
					links.Add(MakeColumnLink(table, pi, new PropertyLink(pi), Prefix));

			return links.ToArray();
		}


		protected ColumnLink[] Links;

		public Flat(Table table)			
		{
			Links = LoadLinks(table, typeof(T), string.Empty);
		}

		///<summary>Writes a single record using metadata associated with the object.</summary>
		public void Write(IWriteRecord wr, T obj)
		{
			foreach(ColumnLink l in Links)
				l.SaveField(obj, wr);
		}

		///<summary>Reads a single record using metadata associated with the object.</summary>
		public T Read(IReadRecord rr)
		{
			object obj = FormatterServices.GetUninitializedObject(typeof(T));
			
			foreach(ColumnLink l in Links)
				l.LoadField(obj, rr);

			return (T)(obj);
		}

		protected static void MakeColumnCreateOptions(ICollection<Column.CreateOptions> CreateOpts, MemberInfo mi, Type type, string Prefix, IDictionary<Type, Column.Type> TypeMap)
		{
			Attribute ColNameAtt = Attribute.GetCustomAttribute(mi, typeof(ColumnNameAttribute));
			string ColName;

			ColName = ColNameAtt != null ?
				((ColumnNameAttribute)ColNameAtt).Name :
				string.Concat(Prefix, mi.Name);

			Column.CreateOptions cco;

			//short path if explicit create options specified
			var explicit_cco = Attribute.GetCustomAttribute(mi, typeof(ColumnCreateOptionsAttribute)) as ColumnCreateOptionsAttribute;
			if(explicit_cco != null)
			{
				cco = new Column.CreateOptions
				{
					Name = ColName,
					Type = (Column.Type)explicit_cco.ColumnType,
					CP = (Column.CodePage)explicit_cco.CodePage,
					MaxLength = explicit_cco.MaxLength,
					Fixed = explicit_cco.Fixed,
					Tagged = explicit_cco.Tagged,
					Version = explicit_cco.Version,
					Autoincrement = explicit_cco.Autoincrement,
					MultiValued = explicit_cco.MultiValued,
					EscrowUpdate = explicit_cco.EscrowUpdate,
					Unversioned = explicit_cco.Unversioned,
					DeleteOnZero = explicit_cco.DeleteOnZero
				};

				if(explicit_cco._NullabilitySpecified)
					cco.NotNull = explicit_cco.NotNull;
				else
					cco.NotNull = type.IsValueType;

				CreateOpts.Add(cco);
				return;
			}

			//if expanded, sub members need to be added from here
			if(Attribute.GetCustomAttribute(mi, typeof(ExpandFieldAttribute)) != null)
			{
				string MemberPrefix = ColName + ",";

				foreach(FieldInfo fi in type.GetFields(BindingFlags.Public | BindingFlags.Instance))
					if(null == Attribute.GetCustomAttribute(fi, typeof(NonpersistentFieldAttribute))) //don't save if nonpersistent
						MakeColumnCreateOptions(CreateOpts, fi, fi.FieldType, MemberPrefix, TypeMap);

				foreach(PropertyInfo pi in type.GetProperties(BindingFlags.Public | BindingFlags.Instance))
					if(null == Attribute.GetCustomAttribute(pi, typeof(PersistentPropertyAttribute))) //only save if a persistent property
						MakeColumnCreateOptions(CreateOpts, pi, pi.PropertyType, MemberPrefix, TypeMap);

				return;
			}
	
			cco = new Column.CreateOptions();

			if(Attribute.GetCustomAttribute(mi, typeof(FieldMultivaluedAttribute)) != null)
			{
				cco.Tagged = true;
				cco.MultiValued = true;
			}

			if(Attribute.GetCustomAttribute(mi, typeof(BinaryFieldSerializationAttribute)) != null)
				cco.Type = Column.Type.LongBinary;
			else if(Attribute.GetCustomAttribute(mi, typeof(XmlFieldSerializationAttribute)) != null)
				cco.Type = Column.Type.LongText;
			else
				cco.Type = TypeMap[type];

			cco.NotNull = type.IsValueType;

			CreateOpts.Add(cco);
		}

		public static Table.CreateOptions CreateTableOptionsForFlat(Type type, IDictionary<Type, Column.Type> TypeMap)
		{
			Attribute TabNameAtt = Attribute.GetCustomAttribute(type, typeof(TableNameAttribute));
			string TabName = TabNameAtt != null ?
				((TableNameAttribute)TabNameAtt).Name :
				type.Name;
			
			var tco = Table.CreateOptions.NewWithLists(TabName);

			foreach(FieldInfo fi in type.GetFields(BindingFlags.Public | BindingFlags.Instance))
				if(null == Attribute.GetCustomAttribute(fi, typeof(NonpersistentFieldAttribute))) //don't save if nonpersistent
					MakeColumnCreateOptions(tco.Columns, fi, fi.FieldType, "", TypeMap);

			foreach(PropertyInfo pi in type.GetProperties(BindingFlags.Public | BindingFlags.Instance))
				if(null == Attribute.GetCustomAttribute(pi, typeof(PersistentPropertyAttribute))) //only save if a persistent property
					MakeColumnCreateOptions(tco.Columns, pi, pi.PropertyType, "", TypeMap);

			foreach(IndexAttribute ixa in Attribute.GetCustomAttributes(type, typeof(IndexAttribute)))
			{
				//var ixa = (IndexAttribute)a;

				var ixco = new Index.CreateOptions
				{
					Primary = ixa is PrimaryIndexAttribute,
					Name = ixa.Name,
					KeyColumns = ixa.KeyColumns,
					Density = ixa.Density,
					LCID = ixa.LCID,
					VarSegMac = ixa.VarSegMac,
					KeyMost = ixa.KeyMost,
					Unique = ixa.Unique,
					DisallowNull = ixa.DisallowNull,
					IgnoreAllNull = ixa.IgnoreAllNull,
					IgnoreAnyNull = ixa.IgnoreAnyNull,
					IgnoreFirstNull = ixa.IgnoreFirstNull,
					LazyFlush = ixa.LazyFlush,
					Unversioned = ixa.Unversioned,
					SortNullHigh = ixa.SortNullHigh,
					Tuples = ixa.Tuples,
					CrossProduct = ixa.CrossProduct,
					DisallowTruncation = ixa.DisallowTruncation
				};

				tco.Indexes.Add(ixco);
			}

			return tco;
		}
	}
}
