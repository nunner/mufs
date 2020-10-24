BEGIN TRANSACTION;
CREATE TABLE "FILES" (
	"Path"		TEXT NOT NULL,
	"Title"		TEXT NOT NULL,
	"Album"		TEXT NOT NULL,
	"Artist"	TEXT NOT NULL,
	"Genre"		TEXT NOT NULL,
	"Track"		INT  NOT NULL,
	PRIMARY KEY("Path","Title","Album","Artist","Genre","Track")
);
COMMIT;
