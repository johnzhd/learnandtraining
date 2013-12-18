GLOBLE_VERSION = "1.0.0.1"
GLOBLE_TRIGGER_TYPE = "MAIN"

function Load_DB_Mysql()
InsertNewPlugins("db1.lua");
end

function Load_DB_DB2()
InsertNewPlugins("db2.lua");
end

function Init_Lua(a,b,c)
InsertNewPlugins("domain.lua");
InsertNewPlugins("normal.lua<|>normal2.lua<|>");
Load_DB_Mysql()
--Load_DB_DB2()
return "","","";
end