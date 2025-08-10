DROP TABLE IF EXISTS tab_foo;

CREATE TABLE tab_foo
(
  id bigint(20) AUTO_INCREMENT PRIMARY KEY,
  text_nn_d varchar(255) NOT NULL DEFAULT '',
  int_n bigint,
  double_n double precision,
  u_int_n bigint UNSIGNED,
  bool_n bool,
  blob_n blob
);

DROP TABLE IF EXISTS tab_bar;

CREATE TABLE tab_bar
(
  id bigint(20) AUTO_INCREMENT PRIMARY KEY,
  text_n varchar(255) NULL,
  bool_nn bool NOT NULL DEFAULT false,
  int_n int
);

DROP TABLE IF EXISTS tab_date_time;

CREATE TABLE tab_date_time (
  id bigint(20) AUTO_INCREMENT PRIMARY KEY,
  date_n date,
  timestamp_n datetime(3),
  date_timestamp_n_d datetime DEFAULT CURRENT_TIMESTAMP,
  time_n time(3)
);

DROP TABLE IF EXISTS tab_json;

CREATE TABLE tab_json (
  id bigint(20) AUTO_INCREMENT PRIMARY KEY,
  data JSON NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
