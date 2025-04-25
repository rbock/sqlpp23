[**\< Index**](/docs/README.md)

# Aggregate functions

Aggregate functions perform calculations across a range of rows, often
controlled by [`group_by`](/docs/select.md).

As they are often used as select columns, sqlpp23 provides respective names for
ease of use.

## `avg`

`avg` calculates the average value of its argument across a range of rows. The
argument has to be numeric. The result will be a
[nullable floating point number](/docs/data_types.md).

```c++
for (const auto& row : db(select(avg(foo.id).as(sqlpp::alias::avg_)))) {
    // use row.avg_;
}

for (const auto& row : db(select(avg(sqlpp::distinct, foo.id)
                                   .as(sqlpp::alias::distinct_avg_)))) {
    // use row.distinct_avg_;
}
```

## `count`

`count` simply counts the rows of a range. The argument can be anything with a
value (or `sqlpp::star` for non-distinct counts). The result will be an
[integer](/docs/data_types.md).

```c++
for (const auto& row : db(select(count(foo.id).as(sqlpp::alias::count_)))) {
    // use row.count_;
}

for (const auto& row : db(select(count(1).as(sqlpp::alias::count_)))) {
    // use row.count_;
}

for (const auto& row : db(select(count(sqlpp::star).as(sqlpp::alias::count_)))) {
    // use row.count_;
}

for (const auto& row : db(select(count(sqlpp::distinct, foo.id)
                                   .as(sqlpp::alias::distinct_count_)))) {
    // use row.distinct_count_;
}
```

## `max`

`max` calculates the maximum value of its argument across a range of rows. The
argument has to be comparable (e.g. numeric or text). The result will be the
nullable equivalent of its argument.

```c++
for (const auto& row : db(select(max(foo.id).as(sqlpp::alias::max_)))) {
    // use row.max_;
}

for (const auto& row : db(select(max(sqlpp::distinct, foo.id)
                                   .as(sqlpp::alias::distinct_max_)))) {
    // use row.distinct_max_;
}
```

## `min`

`min` calculates the minimum value of its argument across a range of rows. The
argument has to be comparable (e.g. numeric or text). The result will be the
nullable equivalent of its argument.

```c++
for (const auto& row : db(select(min(foo.id).as(sqlpp::alias::min_)))) {
    // use row.min_;
}

for (const auto& row : db(select(min(sqlpp::distinct, foo.id)
                                   .as(sqlpp::alias::distinct_min_)))) {
    // use row.distinct_min_;
}
```

## `sum`

`sum` calculates the sum of its argument across a range of rows. The argument
has to be numeric. The result will be the nullable equivalent of its argument.

```c++
for (const auto& row : db(select(sum(foo.id).as(sqlpp::alias::sum_)))) {
    // use row.sum_;
}

for (const auto& row : db(select(sum(sqlpp::distinct, foo.id)
                                   .as(sqlpp::alias::distinct_sum_)))) {
    // use row.distinct_sum_;
}
```

## `over`

Aggregate functions return aggregate results. They must not be mixed with
non-aggregates in [`select`](/docs/select.md). If you *want* to mix them, then the
`over` function comes in handy. It returns the aggregate for each row.

As of this writing, `over` does not support any arguments.

```c++
for (const auto& row : db(select(foo.id,
                                 max(foo.id).over().as(sqlpp::alias::max_)))) {
    // use row.id;
    // use row.max_;
}
```

[**\< Index**](/docs/README.md)
