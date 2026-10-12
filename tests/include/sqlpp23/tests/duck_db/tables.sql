DROP TABLE IF EXISTS tab_foo;
DROP SEQUENCE IF EXISTS seq_tab_foo_id;

CREATE SEQUENCE seq_tab_foo_id START 1;

CREATE TABLE tab_foo
(
  id BIGINT PRIMARY KEY DEFAULT nextval('seq_tab_foo_id'),
  text_nn_d VARCHAR NOT NULL DEFAULT '',
  int_n BIGINT,
  double_n DOUBLE,
  u_int_n UBIGINT,
  bool_n BOOLEAN,
  blob_n BLOB
);

DROP TABLE IF EXISTS tab_bar;
DROP SEQUENCE IF EXISTS seq_tab_bar_id;

CREATE SEQUENCE seq_tab_bar_id START 1;

CREATE TABLE tab_bar
(
  id BIGINT PRIMARY KEY DEFAULT nextval('seq_tab_bar_id'),
  text_n VARCHAR NULL,
  bool_nn BOOLEAN NOT NULL DEFAULT false,
  int_n INTEGER
);

DROP TABLE IF EXISTS tab_date_time;
DROP SEQUENCE IF EXISTS seq_tab_date_time_id;

CREATE SEQUENCE seq_tab_date_time_id START 1;

CREATE TABLE tab_date_time
(
  id BIGINT PRIMARY KEY DEFAULT nextval('seq_tab_date_time_id'),
  date_n DATE,
  timestamp_n TIMESTAMP,
  date_timestamp_n_d TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  time_n TIME
);

DROP TABLE IF EXISTS tab_department;
DROP SEQUENCE IF EXISTS seq_tab_department_id;

CREATE SEQUENCE seq_tab_department_id START 1;

CREATE TABLE tab_department
(
  id BIGINT PRIMARY KEY DEFAULT nextval('seq_tab_department_id'),
  name VARCHAR(100),
  division VARCHAR(255) NOT NULL DEFAULT 'engineering'
);

DROP TABLE IF EXISTS tab_identity;

CREATE TABLE tab_identity
(
  id BIGINT PRIMARY KEY,
  text_n VARCHAR
);
