GLOBLE_VERSION = "1.0.0.1"
GLOBLE_UNIONID = "RS_001"
GLOBLE_TRIGGER_TYPE = "ONCE||URL=JSP||"


function Init_Lua( url, request, reponse )
-- do something
print("db2.lua %s [%d] (%d)\n", url, strlen(request), srelen(reponse) );
return "", "", "";
end

