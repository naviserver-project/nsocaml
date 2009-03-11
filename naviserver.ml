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
 *)

(*----- Declare external functions -----*)

external ns_eval : string -> string = "Ns_Eval_OCaml"

external ns_log : string -> string -> unit = "Ns_Log_OCaml"

external ns_info : string -> string = "Ns_Info_OCaml"

external ns_conn : string -> string = "Ns_Conn_OCaml"

external ns_server : string -> string = "Ns_Server_OCaml"

external ns_write : string -> unit = "Ns_Write_OCaml"

external ns_returnredirect : string -> unit = "Ns_ReturnRedirect_OCaml"

external ns_returnnotfound : unit -> unit = "Ns_ReturnNotFound_OCaml"

external ns_returnforbidden : unit -> unit = "Ns_ReturnForbidden_OCaml"

external ns_returnunauthorized : unit -> unit = "Ns_ReturnUnauthorized_OCaml"

external ns_returninternalerror : unit -> unit = "Ns_ReturnInternalError_OCaml"

external ns_return : int -> string -> string -> unit = "Ns_Return_OCaml"

external ns_returnfile : int -> string -> string -> unit = "Ns_ReturnFile_OCaml"

external ns_queryexists : string -> int = "Ns_QueryExists_OCaml"

external ns_queryget : string -> string = "Ns_QueryGet_OCaml"

external ns_querygetall : string -> string list = "Ns_QueryGetAll_OCaml"

external ns_urlencode : string -> string = "Ns_UrlEncode_OCaml"

external ns_urldecode : string -> string = "Ns_UrlDecode_OCaml"

external ns_config : string -> string -> string = "Ns_Config_OCaml"

external ns_guesstype : string -> string = "Ns_GuessType_OCaml"

external ns_quotehtml : string -> string = "Ns_QuoteHtml_OCaml"

external ns_striphtml : string -> string = "Ns_StripHtml_OCaml"

external ns_set_cleanup : unit -> unit = "Ns_SetCleanup_OCaml"

external ns_set_array : unit -> string list = "Ns_SetArray_OCaml"

external ns_set_list : unit -> string list = "Ns_SetList_OCaml"

external ns_set_split : string -> unit = "Ns_SetSplit_OCaml"

external ns_set_copy : string -> string = "Ns_SetCopy_OCaml"

external ns_set_new : string -> string = "Ns_SetNew_OCaml"

external ns_set_create : string -> string = "Ns_SetNew_OCaml"

external ns_set_size : string -> int = "Ns_SetSize_OCaml"

external ns_set_name : string -> string = "Ns_SetName_OCaml"

external ns_set_print : string -> unit = "Ns_SetPrint_OCaml"

external ns_set_free : string -> unit = "Ns_SetFree_OCaml"

external ns_set_find: string -> string -> int = "Ns_SetFind_OCaml"

external ns_set_ifind: string -> string -> int = "Ns_SetIFind_OCaml"

external ns_set_unique: string -> string -> int = "Ns_SetUnique_OCaml"

external ns_set_iunique: string -> string -> int = "Ns_SetIUnique_OCaml"

external ns_set_delkey: string -> string -> unit = "Ns_SetDelKey_OCaml"

external ns_set_idelkey: string -> string -> unit = "Ns_SetIDelKey_OCaml"

external ns_set_get: string -> string -> string = "Ns_SetGet_OCaml"

external ns_set_iget: string -> string -> string = "Ns_SetIGet_OCaml"

external ns_set_value: string -> int -> string = "Ns_SetValue_OCaml"

external ns_set_isnull: string -> int -> int = "Ns_SetIsNull_OCaml"

external ns_set_key: string -> int -> string = "Ns_SetKey_OCaml"

external ns_set_delete: string -> int -> unit = "Ns_SetDelete_OCaml"

external ns_set_truncate: string -> int -> unit = "Ns_SetTrunc_OCaml"

external ns_set_update: string -> string ->string -> unit = "Ns_SetUpdate_OCaml"

external ns_set_cput: string -> string ->string -> unit = "Ns_SetCPut_OCaml"

external ns_set_icput: string -> string ->string -> unit = "Ns_SetICPut_OCaml"

external ns_set_put: string -> string ->string -> unit = "Ns_SetPut_OCaml"

external ns_set_merge : string -> string -> unit = "Ns_SetMerge_OCaml"

external ns_set_move : string -> string -> unit = "Ns_SetMove_OCaml"

external ns_normalizepath : string -> string = "Ns_NormalizePath_OCaml"

external ns_url2file : string -> string = "Ns_Url2File_OCaml"

external ns_time : unit -> int = "Ns_Time_OCaml"

external ns_fmttime : int -> string -> string = "Ns_FmtTime_OCaml"

external nsv_get : string -> string -> string = "Ns_NsvGet_OCaml"

external nsv_exists : string -> string -> int = "Ns_NsvExists_OCaml"

external nsv_set : string -> string -> string -> unit = "Ns_NsvSet_OCaml"

external nsv_append : string -> string -> string -> unit = "Ns_NsvAppend_OCaml"

external nsv_incr : string -> string -> int -> unit = "Ns_NsvIncr_OCaml"

external nsv_unset : string -> string -> unit = "Ns_NsvUnset_OCaml"

external nsv_names : string -> string list = "Ns_NsvNames_OCaml"

external nsv_array_names : string -> string -> string list = "Ns_NsvArrayNames_OCaml"

