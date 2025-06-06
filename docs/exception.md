[**\< Index**](/docs/README.md)

## When to expect an exception

sqlpp23 connectors throw the majority of `sqlpp::exception`s so check your connector's [documentation](/docs/connectors.md).
Generally, you should expect an exception when:

-  Connecting to a database
-  Preparing a statement
-  Executing a statement
-  Retrieving and iterating through result rows

Each of the [connectors](/docs/connectors.md) also provides their own exception class(es) that provide access to
native error information from the underlying library.

[**\< Index**](/docs/README.md)
