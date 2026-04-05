DROP TYPE IF EXISTS shape CASCADE;
CREATE TYPE shape AS ENUM ('circle', 'square', 'triangle');

DROP TABLE IF EXISTS tab_enums;
CREATE TABLE tab_enums (
  -- cpp_type:animal
  animal bigint,
  -- cpp_type:shape
  cage shape
);
