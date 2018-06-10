/* 
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
 */

/*
 * nsocaml.c -- Interface to Objective Caml language
 *
 *
 */

#define USE_TCL8X

#include "ns.h"
#include "nsd.h"
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#define NSOCAML_VERSION  "0.2"

NS_EXPORT Ns_ModuleInitProc Ns_ModuleInit;

static Ns_OpProc OCAMLHandler;
static Ns_TclTraceProc OCAMLInterpInit;

//static int OCAMLHandler(void *arg,Ns_Conn *conn);
//static int OCAMLInterpInit(Tcl_Interp *interp,void *context);
static int OCAMLCmd(void *context,Tcl_Interp *interp,int objc,Tcl_Obj * const objv[]);

static Ns_Mutex ocamlLock;
static value *ocamlLoader;

NS_EXPORT int Ns_ModuleVersion = 1;

NS_EXPORT int
Ns_ModuleInit(const char *server, const char *module)
{
    Ns_DString ds;
    NsServer *servPtr;
    char *argv[] = { 0, 0, 0 };
    const char *path;

    path = Ns_ConfigGetPath(server,module,NULL);
    // Initialize OCaml dynamic loader
    Ns_DStringInit(&ds);
    Ns_DStringPrintf(&ds,"%s/bin/nsocaml.so",Ns_InfoHomePath());
    argv[0] = argv[1] = ds.string;
    caml_main(argv);
    // Locate OCaml loader function
    if(!(ocamlLoader = caml_named_value("ns_ocaml_load"))) {
      Ns_Log(Error,"nsocaml: ns_ocaml_load function is not found");
      return TCL_ERROR;
    }
    // OCaml object files handler
    if((servPtr = NsGetServer(server))) {
      Ns_RegisterRequest(server,"GET","*.cmo",OCAMLHandler,0,servPtr,0);
      Ns_RegisterRequest(server,"POST","*.cmo",OCAMLHandler,0,servPtr,0);
    }
    // Initialize Tcl interpreter
    Ns_TclRegisterTrace(server, OCAMLInterpInit, 0, NS_TCL_TRACE_CREATE);
    return NS_OK;
}

static int
OCAMLInterpInit(Tcl_Interp *interp, const void *context)
{
    Tcl_CreateObjCommand(interp,"ns_ocaml",OCAMLCmd,(void *)context,NULL);
    return NS_OK;
}

static int
OCAMLCmd(ClientData UNUSED(clientData), Tcl_Interp *interp,int objc,Tcl_Obj * const objv[])
{
    int cmd;
    value *fn, res, arg = Val_unit;
    enum commands {
        cmdCall, cmdLoad
    };
      
    static const char *sCmd[] = {
        "call", "load",
        0
    };

    if(objc < 2) {
      Tcl_AppendResult(interp, "wrong # args: should be ns_ocaml command ?args ...?",0);
      return TCL_ERROR;
    }
    if(Tcl_GetIndexFromObj(interp,objv[1],sCmd,"command",TCL_EXACT,(int *)&cmd) != TCL_OK)
      return TCL_ERROR;

    switch(cmd) {
     case cmdLoad:
         if(objc < 3) {
           Tcl_WrongNumArgs(interp,2,objv,"filename");
           return TCL_ERROR;
         }
         arg = copy_string(Tcl_GetString(objv[2]));
         Ns_MutexLock(&ocamlLock);
         res = callback_exn(*ocamlLoader,arg);
         Ns_MutexUnlock(&ocamlLock);
         if(Is_exception_result(res)) {
           Tcl_AppendResult(interp,format_caml_exception(Extract_exception(res)),free);
           return TCL_ERROR;
         }
         break;

     case cmdCall:
         if(objc < 3) {
           Tcl_WrongNumArgs(interp,2,objv,"function ?arg?");
           return TCL_ERROR;
         }
         if(!(fn = caml_named_value(Tcl_GetString(objv[2])))) {
           Tcl_AppendResult(interp,Tcl_GetString(objv[2])," function is not defined",0);
           return TCL_ERROR;
         }
         if(objc > 3) arg = copy_string(Tcl_GetString(objv[2]));
         Ns_MutexLock(&ocamlLock);
         res = callback_exn(*fn,arg);
         Ns_MutexUnlock(&ocamlLock);
         if(Is_exception_result(res)) {
           Tcl_AppendResult(interp,format_caml_exception(Extract_exception(res)),free);
           return TCL_ERROR;
         }
         break;
    }
    return TCL_OK;
}

static Ns_ReturnCode
OCAMLHandler(const void *arg, Ns_Conn *conn)
{
   value res,file;
   Ns_DString ds;
   const NsServer *servPtr = arg;

   Ns_DStringInit(&ds);
   Ns_MakePath(&ds,servPtr->fastpath.pageroot,conn->request.url,NULL);
   if(access(ds.string,R_OK) != 0) goto notfound;
   file = copy_string(ds.string);
   Ns_MutexLock(&ocamlLock);
   res = callback_exn(*ocamlLoader,file);
   Ns_MutexUnlock(&ocamlLock);
   if(Is_exception_result(res)) {
     const char *msg = format_caml_exception(Extract_exception(res));

     Ns_Log(Error,"nsocaml: %s: %s",ds.string,msg);
     free((char *)msg);
     return TCL_ERROR;
   }
   // OCaml module id not produce any HTTP response, return internal error then
   if(Ns_ConnResponseStatus(conn) == 0) {
     Ns_Log(Error,"nsocaml: %s did not provide any valid HTTP response",ds.string);
     Ns_ConnReturnInternalError(conn);
   }
   return TCL_OK;
notfound:
   Ns_DStringFree(&ds);
   return Ns_ConnReturnNotFound(conn);
}

