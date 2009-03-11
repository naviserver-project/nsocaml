open Naviserver;;
open List;;

let logger cmd =
    ns_log "Debug" (cmd ^ " " ^ (ns_conn cmd));;

let qlogger cmd =
    ns_log "Debug" ("Query list all = " ^ cmd);;

let cmds = [ "authpassword"; "authuser"; "close"; "content"; "contentlength";
     	     "copy"; "driver"; "encoding"; "files"; "fileoffset";
	     "filelength"; "fileheaders"; "flags"; "form"; "headers";
	     "host"; "id"; "isconnected"; "location"; "method";
	     "outputheaders"; "peeraddr"; "peerport"; "port"; "protocol";
	     "query"; "request"; "server"; "sock"; "start"; "status";
             "url"; "urlc"; "urlencoding"; "urlv"; "version"; "write_encoded" ];;

ns_log "Debug" "Testing ns_conn...";;


ns_log "Debug" ("Query cmd = " ^ ns_queryget "cmd");;
ns_log "Debug" ("Query name = " ^ ns_queryget "name");;

iter qlogger (ns_querygetall "all");;

ns_return 200 "text/plain" "test completed.";;

iter logger cmds;;

