# Introduction #

There are certain conventions used in the interface to user code that can be helpful to understand when using EseObjects.

# Object Construction #

Most new objects are created through direct calls to constructors or static factory methods. Constructors are only used for transient resources, like handles. Static factory methods are always used when creating objects that correspond to persistent database elements like tables. Static factory methods are also sometimes used to provide a specific way to open an object.

Classes have a static `Create` method for creating a new instance of it in the database, as opposed to opening a preexisting instance. Persistent objects are also only deleted with explicit methods, never Dispose or Finalize.

For example, to open an already attached database, you can use `Database db = new Database(session, "db_name");` but to create a new database, you must call `Database db = Database.Create(session, create_options);`

The following classes follow this convention of open with constructor, create and delete with static method:
  * `Table`
  * `Column`
  * `Index`
  * `Database` - Static methods for create, attach and detach.

The following classes are not persistent in a database, and even if they hold ESE resource handles, are not created permanently. These classes are opened by constructor and freed either by Dispose (if implementing IDisposable) or implicitly by the garbage collector if no ESE resources are held.

  * `Instance`
  * `Session`
  * `Transaction`
  * `Bookmark`
  * `SecondaryBookmark`
  * `Key`
  * `Column`

The underlying convention being that while a constructor may allocate resources, it does not change or create anything in the database. Likewise, destructors free resources but do not delete anything in the database.

The closest exception this are temporary tables: they are automatically deleted when the table object is disposed.

## Parameter Structs ##

Objects in ESE often have a large number of parameters that can be specified. To improve maintenance and reduce complexity, fully public value structs are used to hold the parameters. There are constructors and static factory methods included with these create structs intended to cover common usage, but are not required to be used. All members set to their .NET value type defaults (0 or null) represent a default or not present state. Look for a `CreateOptions` subtype in the class you want to build. Depending on the .NET language used, there are several ways these structs can be populated.

Here are some examples of populating `Table.CreateOptions`, the most complex create struct, since it has nested `Column` and `Index` collections to specify the initial columns and indexes.

In C# with inline initializers:
```
var nkt = Table.Create(sess, db, new Table.CreateOptions
{
	Name = "nkt",
	Columns = new Column.CreateOptions[]
	{
		new Column.CreateOptions {Name = "C1", Type = Column.Type.Long},
		new Column.CreateOptions {Name = "C2", Type = Column.Type.Long}
	},
	Indexes = new Index.CreateOptions[]
	{
		Index.CreateOptions.NewPrimary("PK", "+C1.+C2", true)
	}
});
```

In C# with factory methods, constructors and ICollection.Add:
```
var tco = Table.CreateOptions.NewWithLists("Order");
tco.Columns.Add(new Column.CreateOptions("ID", Column.Type.Long));
tco.Columns.Add(new Column.CreateOptions("OpenDate", Column.Type.DateTime));
tco.Columns.Add(new Column.CreateOptions("Customer", Column.Type.Long));

var ixco = Index.CreateOptions.NewPrimary("PK", "+ID", true);
tco.Indexes.Add(ixco);

ixco = Index.CreateOptions.NewSecondary("OpenDateIx", "-OpenDate", false);
tco.Indexes.Add(ixco);

ixco = Index.CreateOptions.NewSecondary("CustomerIx", "+Customer", false);
tco.Indexes.Add(ixco);

Order = Table.Create(sess, db, tco);
```

## Object Lifetime ##

The classes that implement IDisposable should be disposed when no longer needed. They will be cleaned up when finalized, but that can take an arbitrary amount of time to occur. Recommend use of C# using syntax whenever possible. Some objects (transactions and cursor updates) will cancel the operation if they haven't already been committed. This should be the desired behavior when an exit occurs due to an exception.

# Enum Types #

Enum types are only used for discrete choices. There are no bitfield enums in EseObjects. Instead, create structs with boolean parameters are provided. Flag sets often come packaged with other types of options, and with a option struct these can all be packaged in a uniform way and without introducing another type and namespace.

# Access to Unmanaged Resources #

EseObjects should normally insulate you from the need to touch any unmanaged resources, but there are a few cases where you may want to access unmanaged resouces more directly for performance or interoperability (e.g. with ManagedEsent).

Objects that have ESE handles provide a `Jet*` property that retrieves the value of the handle:

| **Object** | **Handle property** |
|:-----------|:--------------------|
| Cursor     | JetTableID          |
| Table      | JetTableID          |
| Column     | JetColumnID         |
| Database   | JetDbID             |
| Session    | JetSesID            |
| Instance   | JetInstance         |

The binary data underlying bookmarks and keys is available as a byte array.

Implementing the data transfer function of a value level bridge can be done either by a byte array or an IntPtr to an unmanaged storage area. For storing values, EseObjects provides a temporary allocation mechanism.