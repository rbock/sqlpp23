[**\< Recipes**](/docs/recipes.md)

# Mapping to/from key-value store

Let's assume you have a key-value store that uses strings as keys and can store / retrieve
values of pretty much any data type.

# Storing SELECT results

For storing results into a key-value store, we need to iterate of the names and values of each column.
You could do this "manually"

```c++
for (const auto& row : db(sqlpp::select(all_of(tab)).from(tab))) {
  keyValue.store("id", row.id);
  keyValue.store("name", row.name);
```

Or you can use `get_sql_name_tuple` and `as_tuple` functions.
The former returns a tuple of `std::string_view` representing the names of the result columns.
The latter returns the respective values.

```c++
template <typename... Names, typename... Values, size_t... Idx>
void storeRowImpl(KeyValue& keyValue,
                  const std::tuple<Names...>& names,
                  const std::tuple<Values...>& values,
                  std::index_sequence<Idx...>) {
  ((keyValue.store(std::get<Idx>(names), std::get<Idx>(values)), ...);
}

template <typename... Names, typename... Values>
void storeRow(const std::tuple<Names...>& names,
              const std::tuple<Values...>& values) {
  storeRowImpl(names, values,
               std::make_index_sequence<sizeof...(Values)>());
}

void selectAndStore(sql::connection& db, KeyValue& keyValue) {
  for (const auto& row : db(sqlpp::select(all_of(tab)).from(tab))) {
    storeRow(get_sql_name_tuple(row), as_tuple(row));
  }
}
```

# INSERT or UPDATE from a key-value store

If you want to take values from a key-value store and insert them into a database, you have
options similar to the above.

You can do it manually:

```c++
insert_into(tab).set(
        tab.id = keyValue.get<int64_t>("id"),
        tab.name = keyValue.get<std::string_view>("name"),
        ...);
```

Alternatively, you can use a couple of helpers to perform the task more generically:

* `all_of(my_table)`  returns a tuple of all columns of a table
* `get_sql_name(column)` returns the name
* `data_type_of_t<Column>` yields the data type of the column.

The application of these functions could look like this:

```c++
auto assign(Column col, const KeyValue& keyValue) {
  return col = keyValue.
                  get_value<sqlpp::data_type_of_t<Column>>(
                          get_sql_name(col));
}

template <typename T, typename... Column>
auto update(const Table& t, const std::tuple<Column...>&, const KeyValue& keyValue) {
   return sqlpp::update(t).set(assign(Column{}, keyValue), ...);
}

template <typename T>
auto update(const Table& t, const KeyValue& keyValue) {
   return update(t, all_of(t), keyValue);
}
```

[**\< Recipes**](/docs/recipes.md)

