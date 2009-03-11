(* OCaml module for NaviServer
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis,WITHOUT WARRANTY OF ANY KIND,either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * Alternatively,the contents of this file may be used under the terms
 * of the GNU General Public License(the "GPL"),in which case the
 * provisions of GPL are applicable instead of those above.  If you wish
 * to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * License,indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above,a recipient may use your
 * version of this file under either the License or the GPL.
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 * 
 *
 *)

open Naviserver;;

(*----- Define OCaml functions -----*)

let ns_ocaml_load name =
  try
    Dynlink.loadfile name;
  with
    Dynlink.Error (e) ->
      ns_log "Error" (Dynlink.error_message e);;

(*----- Register OCaml callbacks -----*)

Callback.register "ns_ocaml_load" ns_ocaml_load;;

(*----- Initialize Dynlink library. -----*)

Dynlink.init ();;
Dynlink.allow_unsafe_modules true;;

ns_log "Notice" ("OCaml " ^
                 Sys.ocaml_version ^
                 " module for NaviServer " ^
                 (ns_info "version") ^
                 " started");;

