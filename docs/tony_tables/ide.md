[**\< Differences**](/docs/differences_to_sqlpp11.md)

# Before and after: IDE code completion

<table>
<tr>
<th align="left">sqlpp11</th><th align="left">sqlpp23</th>
</tr>
</tr>
<tr><td colspan=2>

  **IDE suggestions for a select statement**

</td></tr>
<tr>
<td  valign="top">

Code:
```c++
auto select = sqlpp::select()
                .columns(all_of(t))
                .flags(sqlpp::all)
                .from(t)
                .where(t.alpha > 0)
                .group_by(t.alpha)
                .order_by(t.gamma.asc())
                .having(t.gamma)
                .offset(19u)
                .limit(7u);
select.<cursor>
```
Suggested members:
```diff
! ----------------------------------------------
!        as(const AliasProvider &aliasProvider)|
!        asc() const                           |
!        desc() const                          |
!        for_update() const                    |
!        from                                  |
!        get_dynamic_names() const             |
!        get_no_of_result_columns() const      |
!        get_selected_columns()                |
!        group_by                              |
!        having                                |
!        in(T t...) const                      |
!        is_not_null() const                   |
!        is_null() const                       |
!        limit                                 |
!        not_in(T t...) const                  |
!        no_for_update                         |
!        no_union                              |
!        no_with                               |
!        offset                                |
!        order(sort_type s) const              |
!        order_by                              |
!        selected_columns                      |
!        select_flags                          |
!        statement_name                        |
!        union_all(Rhs rhs) const              |
!        union_distinct(Rhs rhs) const         |
!        where                                 |
!        _can_be_used_as_table()               |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_member(T t)                      |
!        _get_no_of_parameters() const         |
!        _get_statement() const                |
!        _get_static_no_of_parameters()        |
!        _prepare(Database &db) const          |
!        _run(Database &db) const              |
! ----------------------------------------------
```
</td>
<td  valign="top">

Code:
```c++
auto select = sqlpp::select()
                .columns(all_of(t))
                .flags(sqlpp::all)
                .from(t)
                .where(t.id > 0)
                .group_by(t.id)
                .order_by(t.boolNn.asc())
                .having(max(t.boolNn) > 0)
                .offset(19u)
                .limit(7u);
select.<cursor>
```
Suggested members:
```diff
! -----------------------------------------------------
!        as(Statement &&self, const NameTagProvider &)|
!        for_update(Statement &&self)                 |
!        union_all(Statement &&self, Rhs rhs)         |
!        union_distinct(Statement &&self, Rhs rhs)    |
! -----------------------------------------------------
```

</td>
</tr>
<tr><td colspan=2>

  **IDE suggestions for a prepared statement**

</td></tr>
<tr>
<td  valign="top">

Code:
```c++
auto prep = db.prepare(my_statement);
prep.<cursor>
```
Suggested members:
```diff
! ---------------------------
!        params             |
!        _bind_params()     |
!        _dynamic_names     |
!        _prepared_statement|
!        _run(MockDb &db)   |
! ---------------------------
```
</td>
<td  valign="top">

Code:
```c++
auto prep = db.prepare(my_statement);
prep.<cursor>
```
Suggested members:
```diff
! ------------------
!        parameters|
! ------------------
```

</td>
</tr>
<tr><td colspan=2>

  **IDE suggestions for a connection**

</td></tr>
<tr>
<td  valign="top">

Code:
```c++
db.<cursor>
```
Suggested members:
```diff
! ------------------------------------------------------------
!   commit_transaction()                                     |
!   connectUsing(const _config_ptr_t &config)                |
!   escape(const std::string &s) const                       |
!   execute(…)                                               |
!   get_config()                                             |
!   get_handle()                                             |
!   insert(const Insert &i)                                  |
!   is_connected() const                                     |
!   is_transaction_active()                                  |
!   is_valid() const                                         |
!   native_handle()                                          |
!   ping_server() const                                      |
!   prepare(const T &t)                                      |
!   prepare_insert(Insert &i)                                |
!   prepare_remove(Remove &r)                                |
!   prepare_select(Select &s)                                |
!   prepare_update(Update &u)                                |
!   remove(const Remove &r)                                  |
!   report_rollback_failure(const std::string &message)      |
!   rollback_transaction(bool report)                        |
!   run(const T &t)                                          |
!   run_prepared_insert(const PreparedInsert &i)             |
!   run_prepared_remove(const PreparedRemove &r)             |
!   run_prepared_select(const PreparedSelect &s)             |
!   run_prepared_update(const PreparedUpdate &u)             |
!   select(const Select &s)                                  |
!   start_transaction()                                      |
!   update(const Update &u)                                  |
!   _interpret_interpretable(const T &t, _context_t &context)|
!   _prepare(…)                                              |
!   _run(…)                                                  |
!   _serialize_interpretable(const T &t, _context_t &context)|
! ------------------------------------------------------------
```
</td>
<td  valign="top">

Code:
```c++
db.<cursor>
```
Suggested members:
```diff
! ------------------------------------------------------
!   commit_transaction()                               |
!   connect_using(const _config_ptr_t &config)         |
!   escape(const std::string_view &s) const            |
!   get_config()                                       |
!   is_connected() const                               |
!   is_transaction_active()                            |
!   native_handle()                                    |
!   ping_server() const                                |
!   prepare(const T &t)                                |
!   report_rollback_failure(const std::string &message)|
!   rollback_transaction()                             |
!   start_transaction()                                |
! ------------------------------------------------------
```

</td>
</tr>
</table>

[**\< Differences**](/docs/differences_to_sqlpp11.md)
