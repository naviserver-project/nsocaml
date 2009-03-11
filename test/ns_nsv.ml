open Naviserver;;
open List;;

nsv_set "1" "key1" "value1";;
nsv_set "1" "key2" "value2";;
nsv_set "1" "key3" "value3";;
nsv_set "1" "counter" "1";

ns_log "Debug" ("get key2: " ^ nsv_get "1" "key2");;
ns_log "Debug" ("get counter: " ^ nsv_get "1" "counter");;

nsv_append "1" "key2" "___12345";;
nsv_incr "1" "counter" 2;

ns_log "Debug" ("get counter: " ^ nsv_get "1" "counter");;

ns_return 200 "text/plain" "test completed.";;

let logger key =
    ns_log "Debug" (key ^ " " ^ (nsv_get "1" key));;

iter logger (nsv_array_names "1" "");;

