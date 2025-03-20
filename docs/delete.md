# Delete

Deleting rows from tables is straight forward:

```C++
db(delete_from(tab).where(tab.alpha == 35));
```

Note that the library requires a `where` clause. If you really want to delete all rows of a table using a `delete` statement you have to explictly say so with `.where(true)`. But see also the next section.

## Truncate

If you want to delete all rows from a table, the truncate function might be the faster option.

```c++
db(truncate(tab));
```
## The `using` clause

Some backends support `where` conditions that use additional tables. These are supplied to the statement via the `using` clause.

```C++
test_sqlpp::Users usr;
test_sqlpp::UsersForms usr_forms;
test_sqlpp::Forms form_;

db(remove_from(usr_forms).using_(usr, form_, usr_forms).where(
    usr_forms.iduser == usr.id
    and usr.username == username
    and usr_forms.idform == form_.id
    and form_.name == form_name
    ));
```


