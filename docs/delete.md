# Delete

Deleting rows from tables is straight forward:

```C++
db(delete_from(tab).where(tab.alpha == 35));
```

## Deleting rows using multiple tables in the condition

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

## Truncate

If you want to delete all rows from a table, the truncate function might be the faster option.

```c++
db(truncate(tab));
```

