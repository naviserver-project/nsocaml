open Naviserver;;
open List;;

let logger cmd =
    ns_log "Debug" (cmd ^ " " ^ (ns_server cmd));;

let cmds = [ "active"; "all"; "connections"; "keepalive";
             "pools"; "queued"; "threads"; "waiting" ];;

ns_log "Debug" "Testing ns_server...";;

ns_return 200 "text/plain" "test completed.";;

iter logger cmds;;
