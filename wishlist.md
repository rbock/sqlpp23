If you want to help with the development of the library, you might want to
consider one of these items:

## Async support

Obtain results in an asynchronous fashion, see #35, for instance.

## Suppress export of symbols

Some compilers tend to export all the generated symbols, which is a bit annoying
in case of template-heavy libraries like sqlpp23 (leads to larger files and
longer compile/link/startup times, I believe). There are ways to suppress this
in most compilers, afaik.
