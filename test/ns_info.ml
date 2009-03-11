open Naviserver;;
open List;;

let logger cmd =
    ns_log "Debug" (cmd ^ " " ^ (ns_info cmd));;

let cmds = [ "address"; "argv0"; "boottime"; "builddate"; "callbacks";
             "config"; "home"; "hostname"; "label"; "locks"; "log";
             "major"; "minor"; "name"; "nsd"; "pageroot"; "patchlevel";
             "pid"; "platform"; "pools"; "scheduled"; "server"; "servers";
             "sockcallbacks"; "tag"; "tcllib"; "threads"; "uptime";
             "version"; "winnt"];;

ns_log "Debug" "Testing ns_info...";;

ns_return 200 "text/plain" "test completed.";;

iter logger cmds;;

