GLOBLE_VERSION = "1.0.0.1"
GLOBLE_UNIONID = "RS_001"
GLOBLE_TRIGGER_TYPE = "DOMAIN||URL=JSP||"


function Init_Lua( url, request, reponse )
-- do something
print(string.format("db1.lua %s [%d] (%d)\n", url, string.len(request), string.len(reponse)));
return "", "", "";
end

