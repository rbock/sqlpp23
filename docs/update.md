# Update

You can update rows of a table using an `update` statement:

```C++
db(update(tab).
   .set(tab.gamma = false, tab.textN = std::nullopt)
   .where(tab.id = 17);
```

## `update`
The `update` function takes a single [raw table](tables.md) as its argument.

## `set`
The `set` function has to be called with one or more assigments as its arguments. The left sides of these assignments have to be columns of the table mentioned above.

## `where`
The `where` clause specifies which rows should be affected.

Note that the library forces you to specify a `where` clause. If you want to update **all** rows, you have to say so explicitly using `where(true)`.


