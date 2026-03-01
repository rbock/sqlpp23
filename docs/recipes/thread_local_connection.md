[**\< Recipes**](/docs/recipes.md)

# Thread-local database connections

In this document we describe a simple pattern that allows us to make thread-safe database queries using thread-unsafe database connections.

## Thread safety in the database connectors

At the time of this writing, database connections created by the three main databases supported by sqlpp23 (MySQL, PostgreSQL, and SQLite3) are not thread-safe. The sqlpp23 library is thread-agnostic, which means that it does not add any requirements or guarantees to the thread safety of the underlying database objects and operations. So it is up to the library user to ensure the thread safety of the database operations performed through these database connections.

## Making thread-safe queries using thread-unsafe connectors

The pattern is based on the idea that each user thread is given its own database connection. When a thread wants to execute a database query, it uses its own database connection to execute the query, thus avoiding the need to implement complex and potentially expensive thread synchronization.

We define a database connection class called `lazy_connection`, which mimics the regular database connections provided by sqlpp23 and lets the user execute database queries, pretty much like a regular sqlpp connection does. In fact, our lazy connection creates an underlying sqlpp database connection and forwards all database queries to the sqlpp connection, but that sqlpp connection is not
created immediately in the constructor of the lazy connection. Instead, its creation is postponed until the moment when the user tries to execute their first query through our lazy connection object, which is why our connection class is called "lazy".

Then we create a thread-local variable of type `lazy_connection` called g_dbc at global scope:
```
thread_local lazy_connection g_dbc{g_pool};
```

As you can see from the definition of g_dbc, it is defined as `thread_local`, which means that each user thread gets its own copy of `g_dbc`, stored in the thread's TLS (Thread Local Storage). By using the `thread_local` keyword, we offload all the thread-related chores to the C++ compiler and runtime. When a user thread tries to execute a query through `g_dbc`, the C++ runtime automatically gets a thread-local lazy connection, creating it if necessary. The lazy connection in turn gets a new sqlpp connection from the connection pool and uses it to execute the query. The connection pool is merely an implementation detail; strictly speaking, we could skip the connection pool and create a new connection inside `lazy_connection`, but we use the connection pool for performance reasons.

## Why not make the connection object local?

One might be tempted to make our instance of `lazy_connection` local; after all, a local variable can also be declared as `thread_local`. So why did we make `g_dbc` local? It is because making it local does not work the way one might expect. Let's say that we try to define and use the thread-local lazy connection in block scope:
```
sqlpp::postgresql::connection_pool g_pool{...};

int main()
{
    thread_local dbc{&g_pool};
    std::thread t{[&] {
        dbc(...);
    }};
    t.join ();
}
```

Attempting to use our lazy connection in this fashion will cause a runtime error, because the newly spawned thread uses an uninitialized copy of the lazy connection. While global thread-local variables are guaranteed to be initialized the moment when a thread tries to use them, the local thread-local variables are only initialized when execution passes through their definition. The newly spawned thread never actually entered the `main()` function, so its thread-local copy of the database connection was never initialized, and the attempt to use the uninitialized lazy connection caused the runtime error.

## Sample code

The sample source code, implementing this pattern, is available [here](/tests/postgresql/recipes/thread_local_connection.cpp).

[**\< Recipes**](/docs/recipes.md)
