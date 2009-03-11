open Naviserver;;
open List;;

let set = ns_set_new "";;

ns_set_put set "key1" "value1";;
ns_set_put set "key2" "value2";;
ns_set_put set "key3" "value3";;

ns_log "Debug" ("get key2: " ^ ns_set_get set "key2");;

for i = 0 to (ns_set_size set)-1 do
  ns_log "Debug" ((ns_set_key set i) ^ ": " ^ (ns_set_value set i))
done;;

ns_return 200 "text/plain" "test completed.";;

ns_log "Debug" ("Header: " ^ (ns_conn "headers"));;

ns_set_print (ns_conn "headers");;

ns_log "Debug" ("Output Headers: " ^ (ns_conn "outputheaders"));;

ns_set_print (ns_conn "outputheaders");;
