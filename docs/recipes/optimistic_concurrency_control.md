[**\< Recipes**](/docs/recipes.md)

# Optimistic concurrency control

## What is optimistic concurrency control?

**Optimistic concurrency control** (OCC), also known as optimistic locking, is a method for handling concurrent access to shared resources, that runs concurrent transactions without any explicit locks and relies on the underlying database for the detection and resolution of any conflicts arising from access to the shared resources. If automatic resolution of a conflict fails, then one of the conflicting transactions is aborted and retried one or more times, until it succeeds.

The lack of explicit locking allows OCC to provide high performance if there is little contention between the parallel transactions. On the other hand, if  there is high contention, the performance may (and likely will) degrade significantly.

In contrast to OCC, the other popular method for handling concurrent access to shared resources is **pessimistic concurrency control** (PCC), also known as pessimistic locking. It relies on the user placing explicit locks as well as the database placing implicit locks on the shared resources, which prevents the conflicts from occurring in the first place.

PCC provides lower performance if there is little contention for the shared data, but its performance does not degrade as much when there is high contention for the shared data.

## When should optimistic concurrency control be used?

As mentioned in the previous section, the two main strategies for the synchonization of concurrent access to data are OCC and PCC. The following table summarizes the properties of the two methods for concurrency control:

|                                | OCC     | PCC    |
|--------------------------------|---------|--------|
| Uses transactions              | Yes     | Yes    |
| Retries transactions           | Yes     | No     |
| Uses locks                     | No[^1]  | Yes    |
| Performance on low contention  | High    | Medium |
| Performance on high contention | Low     | Medium |
| Easy to reason about and use   | Yes[^2] | No     |

[^1]: The underlying database may lock implicitly some rows or tables.
[^2]: May require some knowledge of the database-specific serialization failures.

The main conclusion that we can draw from this table is that OCC is suitable when there is little contention for shared data (fewer conflicts). For example, in an application which mostly reads from the shared data and rarely modifies the shared data. In cases where OCC and PCC provide similar performance, OCC can still be a better candidate, because it doesn't require the user to place explicit locks on the shared data, thus eliminating the risk of deadlocks and simplifying the application architecture in general.

## How to implement optimistic concurrency control?

The following pseudocode outlines the implementation of a custom function `tx`, that applies optimistic concurrency control to a user-provided transaction handler.

1. Let `handler` be a user-provided callable value (e.g. function or lambda), that receives a database connection as a parameter and runs a complete database transaction using the received database connection. On success, it returns a value of an arbitrary type; on failure, it throws an arbitrary exception.
2. Get a database connection in `dbc`.
3. Start a transaction at the desired isolation level.
4. Run the following sub-steps in a `try` block.
   - Call `handler(dbc)` and assign the result to `result`. The type of `result` should be the same as the return type of `handler` and can possibly be `void`.
   - Commit the transaction.
   - Return `result` to the caller.
5. If an exception has occurred in step 4, then
   - Roll back the transaction if it has not already been rolled back automatically by the database.
   - If the exception is for a retryable SQL error, e.g. `Serialization Failure` or `Deadlock Detected`, then go to step 3.
   - Otherwise (the exception is not for a retryable SQL error), retrow the exception and let it bubble up. The caller of `tx` can handle it if they want.

## Sample code

Please check [this sample program](/tests/postgresql/recipes/optimistic_concurrency_control.cpp) that inserts a zero value in a database table and then runs in parallel two threads that read the value, increase it by one and then write it back.

[**\< Recipes**](/docs/recipes.md)
