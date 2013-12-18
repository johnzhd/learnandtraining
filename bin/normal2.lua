GLOBLE_VERSION = "1.0.0.1"
GLOBLE_UNIONID = "RS_001"
GLOBLE_TRIGGER_TYPE = "ONCE||URL=JSP||"


function Init_Lua( url, request, reponse )
-- do something
print("normal2.lua %s [%d] (%d)\n", url, strlen(request), strlen(reponse) );
return "", "", "";
end

