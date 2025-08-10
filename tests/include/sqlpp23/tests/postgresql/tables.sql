DROP TABLE IF EXISTS tab_foo;

CREATE TABLE tab_foo
(
  id bigserial PRIMARY KEY,
  text_nn_d varchar(255) NOT NULL DEFAULT '',
  int_n bigint,
  double_n double precision,
  int_nn_u bigint UNIQUE,
  bool_n bool,
  blob_n bytea,
  UNIQUE(id, int_nn_u) -- to test multi-argument on_confict
);

DROP TABLE IF EXISTS tab_bar;

CREATE TABLE tab_bar
(
  id bigserial PRIMARY KEY,
  text_n varchar(255) NULL,
  bool_nn bool NOT NULL DEFAULT false,
  int_n int
);

DROP TABLE IF EXISTS tab_date_time;

CREATE TABLE tab_date_time
(
  date_n date,
  timestamp_n timestamp,
  time_n time,
  timestamp_n_tz timestamp with time zone,
  time_n_tz time with time zone
);

DROP TABLE IF EXISTS blob_sample;

CREATE TABLE blob_sample (
  id bigserial PRIMARY KEY,
  data bytea
);

DROP TABLE IF EXISTS tab_except;

CREATE TABLE tab_except (
  int_small_n_u smallint UNIQUE,
  text_short_n text CHECK( length(text_short_n) < 5 )
)

DROP TABLE IF EXISTS tab_department;

CREATE TABLE tab_department (
  id SERIAL PRIMARY KEY,
  name CHAR(100),
  division VARCHAR(255) NOT NULL DEFAULT 'engineering'
);
