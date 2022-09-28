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

#define USE_TCL8X

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include "ns.h"
#include "nsd.h"

static Ns_ThreadArgProc ThreadArgProc;

static value
copy_string2(const char *str)
{
   return copy_string(str ? str : "");
}

static const char *
GetServer()
{
   Ns_Conn *conn = Ns_GetConn();

   if(conn) return Ns_ConnServer(conn);
   return nsconf.servers.string;
}

static NsInterp *
GetInterp()
{
   Ns_Conn *conn = Ns_GetConn();
   if(conn) return NsGetInterpData(Ns_GetConnInterp(conn));
   return NsGetInterpData(Ns_TclAllocateInterp(GetServer()));
}

static void
ThreadArgProc(Tcl_DString *dsPtr, Ns_ThreadProc proc, const void *arg)
{
    Ns_GetProcInfo(dsPtr, (Ns_Callback *)proc, arg);
}

CAMLprim value
Ns_Eval_OCaml(value oscript)
{
    CAMLparam1(oscript);
    CAMLlocal1(retval);
    const char *result = "";
    NsInterp *itPtr;

    if((itPtr = GetInterp())) {
      if(Tcl_EvalEx(itPtr->interp,String_val(oscript),-1,0) != TCL_OK)
        result = Ns_TclLogErrorInfo(itPtr->interp, "\n(context: eval OCaml)");
      else
        result = (char *)Tcl_GetStringResult(itPtr->interp);
    }
    retval = copy_string(result);
    if(itPtr) Ns_TclDeAllocateInterp(itPtr->interp);
    CAMLreturn(retval);
}

CAMLprim value
Ns_Log_OCaml(value olevel,value ostr)
{
    CAMLparam2(olevel, ostr);
    int clevel = 0;
    char *level = String_val(olevel);
    char *str = String_val(ostr);

    if(!strcasecmp(level,"Debug")) clevel = Debug; else
    if(!strcasecmp(level,"Error")) clevel = Error; else
    if(!strcasecmp(level,"Notice")) clevel = Notice; else
    if(!strcasecmp(level,"Warning")) clevel = Warning; else
    if(!strcasecmp(level,"Fatal")) clevel = Fatal;
    Ns_Log(clevel, "%s", str);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_Info_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    Tcl_DString ds;
    NsServer *servPtr;
    const char *result = "";

    static CONST char *cmds[] = {
        "address", "argv0", "boottime", "builddate", "callbacks",
        "config", "home", "hostname", "label", "locks", "log",
        "major", "minor", "name", "nsd", "pageroot", "patchlevel",
        "pid", "platform", "pools", "scheduled", "server", "servers",
        "sockcallbacks", "tag", "tcllib", "threads", "uptime",
        "version", "winnt", 0 };
    enum {
        IAddressIdx, IArgv0Idx, IBoottimeIdx, IBuilddateIdx, ICallbacksIdx,
        IConfigIdx, IHomeIdx, hostINameIdx, ILabelIdx, ILocksIdx, ILogIdx,
        IMajorIdx, IMinorIdx, INameIdx, INsdIdx, IPageRootIdx, IPatchLevelIdx,
        IPidIdx, IPlatformIdx, IPoolsIdx, IScheduledIdx, IServerIdx, IServersIdx,
        sockICallbacksIdx, ITagIdx, ITclLibIdx, IThreadsIdx, IUptimeIdx,
        IVersionIdx, IWinntIdx, INoneIdx
    } opt;

    for(opt = 0;cmds[opt];opt++)
      if(!strcmp(cmds[opt],String_val(oname))) break;

    Tcl_DStringInit(&ds);
    switch(opt) {
     case INoneIdx:
        break;
     case IAddressIdx:
        if(!(result = Ns_InfoAddress())) result = "";
        break;
     case IArgv0Idx:
        if(!(result = nsconf.argv0)) result = "";
        break;
     case IBoottimeIdx:
        Ns_DStringPrintf(&ds,"%ld",Ns_InfoBootTime());
        result = ds.string;
        break;
     case IBuilddateIdx:
        result = Ns_InfoBuildDate();
        break;
     case ICallbacksIdx:
        NsGetCallbacks(&ds);
        result = ds.string;
        break;
     case IConfigIdx:
        if(!(result = Ns_InfoConfigFile())) result = "";
        break;
     case IHomeIdx:
        if(!(result = Ns_InfoHomePath())) result = "";
        break;
     case hostINameIdx:
        if(!(result = Ns_InfoHostname())) result = "";
        break;
     case ILabelIdx:
        if(!(result = Ns_InfoTag())) result = "";
        break;
     case ILocksIdx:
        Ns_MutexList(&ds);
        result = ds.string;
        break;
     case ILogIdx:
        if(!(result = Ns_InfoErrorLog())) result = "";
        break;
     case IMajorIdx:
        Ns_DStringPrintf(&ds,"%d",NS_MAJOR_VERSION);
        result = ds.string;
        break;
     case IMinorIdx:
        Ns_DStringPrintf(&ds,"%d",NS_MINOR_VERSION);
        result = ds.string;
        break;
     case INameIdx:
        if(!(result = Ns_InfoServerName())) result = "";
        break;
     case INsdIdx:
        if(!(result = nsconf.nsd)) result = "";
        break;
     case IPageRootIdx:
        if((servPtr = NsGetServer(GetServer()))) result = servPtr->fastpath.pageroot;
        break;
     case IPatchLevelIdx:
        result = NS_PATCH_LEVEL;
        break;
     case IPidIdx:
        Ns_DStringPrintf(&ds,"%d",Ns_InfoPid());
        result = ds.string;
        break;
     case IPlatformIdx:
        if(!(result = Ns_InfoPlatform())) result = "";
        break;
     case IPoolsIdx:
        break;
     case IScheduledIdx:
        NsGetScheduled(&ds);
        result = ds.string;
        break;
     case IServerIdx:
        if((servPtr = NsGetServer(GetServer()))) result = servPtr->server;
        break;
     case IServersIdx:
        result = GetServer();
        break;
     case sockICallbacksIdx:
        NsGetSockCallbacks(&ds);
        result = ds.string;
        break;
     case ITagIdx:
        if(!(result = Ns_InfoTag())) result = "";
        break;
     case ITclLibIdx:
        if(!(result = Ns_TclLibrary(GetServer()))) result = "";
        break;
     case IThreadsIdx:
        Ns_ThreadList(&ds,ThreadArgProc);
        result = ds.string;
        break;
     case IUptimeIdx:
        Ns_DStringPrintf(&ds,"%ld",Ns_InfoUptime());
        result = ds.string;
        break;
     case IVersionIdx:
        result = NS_VERSION;
        break;
     case IWinntIdx:
#ifdef _WIN32
        result = "true";
#else
	result = "false";
#endif
        break;
    }
    retval = copy_string(result);
    Tcl_DStringFree(&ds);
    CAMLreturn(retval);
}


static void
AppendConn(Tcl_DString *dsPtr, const Conn *connPtr, const char *state, bool checkforproxy)
{
    Ns_Time now, diff;

    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(state != NULL);

    /*
     * An annoying race condition can be lethal here.
     *
     * In the state "waiting", we have never a connPtr->reqPtr, therefore we
     * can't even determine the peer address, nor the request method or the
     * request URL. Furthermore, there is no way to honor the "checkforproxy"
     * flag.
     */
    if (connPtr != NULL) {
        Tcl_DStringStartSublist(dsPtr);

        if (connPtr->reqPtr != NULL) {
            Tcl_DStringAppendElement(dsPtr, connPtr->idstr);

            /*
             * The settings of (connPtr->flags & NS_CONN_CONFIGURED) is
             * protected via the mutex connPtr->poolPtr->tqueue.lock from the
             * caller, so the protected members can't be changed from another
             * thread.
             */
            if ((connPtr->flags & NS_CONN_CONFIGURED) != 0u) {
                const char *p;

                if ( checkforproxy ) {
                    /*
                     * When the connection is NS_CONN_CONFIGURED, the headers
                     * have to be always set.
                     */
                    assert(connPtr->headers != NULL);
                    p = Ns_SetIGet(connPtr->headers, "X-Forwarded-For");

                    if (p == NULL || (*p == '\0') || strcasecmp(p, "unknown") == 0) {
                        /*
                         * Lookup of header field failed, use upstream peer
                         * address.
                         */
                        p = Ns_ConnPeerAddr((const Ns_Conn *) connPtr);
                    }
                } else {
                    p = Ns_ConnPeerAddr((const Ns_Conn *) connPtr);
                }
                Tcl_DStringAppendElement(dsPtr, p);
            } else {
                /*
                 * The request is not configured, the headers might not be
                 * fully processed. In this situation we can determine the
                 * peer address, but not the header fields.
                 */
                if (checkforproxy ) {
                    /*
                     * The user requested "checkforproxy", but we can't. Since
                     * we assume that the user uses this option typically when
                     * running behind a proxy, we do not want to return here
                     * the peer address, which might be incorrect. So we
                     * append "unknown" as in other semi-processed cases.
                     */
                    Ns_Log(Notice, "Connection is not configured, we can't check for the proxy yet");
                    Tcl_DStringAppendElement(dsPtr, "unknown");
                } else {
                    /*
                     * Append the peer address, which is part of the reqPtr
                     * and unrelated with the configured state.
                     */
                    Tcl_DStringAppendElement(dsPtr, Ns_ConnPeerAddr((const Ns_Conn *) connPtr));
                }
            }
        } else {
            /*
             * connPtr->reqPtr == NULL. Having no connPtr->reqPtr is normal
             * for "queued" requests but not for "running" requests. Report this in the error log.
             */
            Tcl_DStringAppendElement(dsPtr, "unknown");
            if (*state == 'r') {
                Ns_Log(Notice,
                       "AppendConn state '%s': request not available, can't determine peer address",
                       state);
            }
        }

        Tcl_DStringAppendElement(dsPtr, state);

        if (connPtr->request.line != NULL) {
            Tcl_DStringAppendElement(dsPtr, (connPtr->request.method != NULL) ? connPtr->request.method : "?");
            Tcl_DStringAppendElement(dsPtr, (connPtr->request.url    != NULL) ? connPtr->request.url : "?");
        } else {
            /* Ns_Log(Notice, "AppendConn: no request in state %s; ignore conn in output", state);*/
            Tcl_DStringAppendElement(dsPtr, "unknown");
            Tcl_DStringAppendElement(dsPtr, "unknown");
        }
        Ns_GetTime(&now);
        Ns_DiffTime(&now, &connPtr->requestQueueTime, &diff);
        Ns_DStringNAppend(dsPtr, " ", 1);
        Ns_DStringAppendTime(dsPtr, &diff);
        Ns_DStringPrintf(dsPtr, " %" PRIuz, connPtr->nContentSent);

        Tcl_DStringEndSublist(dsPtr);
    }
}
static void
AppendConnList(Tcl_DString *dsPtr, const Conn *firstPtr, const char *state, bool checkforproxy)
{
    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(state != NULL);

    while (firstPtr != NULL) {
        AppendConn(dsPtr, firstPtr, state, checkforproxy);
        firstPtr = firstPtr->nextPtr;
    }
}
static void
ServerListActive(Tcl_DString *dsPtr, ConnPool *poolPtr, bool checkforproxy)
{
    int i;

    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(poolPtr != NULL);

    Ns_MutexLock(&poolPtr->tqueue.lock);
    for (i = 0; i < poolPtr->threads.max; i++) {
        const ConnThreadArg *argPtr = &poolPtr->tqueue.args[i];

        if (argPtr->connPtr != NULL) {
            AppendConnList(dsPtr, argPtr->connPtr, "running", checkforproxy);
        }
    }
    Ns_MutexUnlock(&poolPtr->tqueue.lock);
}

static void
ServerListQueued(Tcl_DString *dsPtr, ConnPool *poolPtr)
{
    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(poolPtr != NULL);

    Ns_MutexLock(&poolPtr->wqueue.lock);
    AppendConnList(dsPtr, poolPtr->wqueue.wait.firstPtr, "queued", NS_FALSE);
    Ns_MutexUnlock(&poolPtr->wqueue.lock);
}
static int
ServerListActiveCmd(Tcl_DString *dsPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv,
                 ConnPool *poolPtr, int nargs)
{
    int         result = TCL_OK, checkforproxy = (int)NS_FALSE;
    Ns_ObjvSpec opts[] = {
        {"-checkforproxy", Ns_ObjvBool, &checkforproxy, INT2PTR(NS_TRUE)},
        {NULL, NULL,  NULL, NULL}
    };

    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(interp != NULL);
    NS_NONNULL_ASSERT(objv != NULL);
    NS_NONNULL_ASSERT(poolPtr != NULL);

    if (Ns_ParseObjv(opts, NULL, interp, objc-nargs, objc, objv) != NS_OK) {
        result = TCL_ERROR;
    } else {
        ServerListActive(dsPtr, poolPtr, (bool)checkforproxy);
    }
    return result;
}

static int
ServerListQueuedCmd(Tcl_DString *dsPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv,
                 ConnPool *poolPtr, int nargs)
{
    int result = TCL_OK;

    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(interp != NULL);
    NS_NONNULL_ASSERT(objv != NULL);
    NS_NONNULL_ASSERT(poolPtr != NULL);

    if (Ns_ParseObjv(NULL, NULL, interp, objc-nargs, objc, objv) != NS_OK) {
        result = TCL_ERROR;
    } else {
        ServerListQueued(dsPtr, poolPtr);
    }
    return result;
}

static int
ServerListAllCmd(Tcl_DString *dsPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const* objv,
                 ConnPool *poolPtr, int nargs)
{
    int         result = TCL_OK, checkforproxy = (int)NS_FALSE;
    Ns_ObjvSpec opts[] = {
        {"-checkforproxy", Ns_ObjvBool, &checkforproxy, INT2PTR(NS_TRUE)},
        {NULL, NULL,  NULL, NULL}
    };

    NS_NONNULL_ASSERT(dsPtr != NULL);
    NS_NONNULL_ASSERT(interp != NULL);
    NS_NONNULL_ASSERT(objv != NULL);
    NS_NONNULL_ASSERT(poolPtr != NULL);

    if (Ns_ParseObjv(opts, NULL, interp, objc-nargs, objc, objv) != NS_OK) {
        result = TCL_ERROR;
    } else {
        ServerListActive(dsPtr, poolPtr, (bool)checkforproxy);
        ServerListQueued(dsPtr, poolPtr);
    }
    return result;
}

CAMLprim value
Ns_Server_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    static CONST char *cmds[] = {
         "active", "all", "connections", "keepalive",
         "pools", "queued", "threads", "waiting", 0
    };
    enum {
         SActiveIdx, SAllIdx, SConnectionsIdx, SKeepaliveIdx,
         SPoolsIdx, SQueuedIdx, SThreadsIdx, SWaitingIdx, INoneIdx
    } opt;
    char buf[100];
    Conn *connPtr;
    Tcl_DString ds, *dsPtr = &ds;
    NsServer *servPtr;
    ConnPool *poolPtr;
    char *result = "";
    Ns_Time now, diff;

    for(opt = 0;cmds[opt];opt++)
      if(!strcmp(cmds[opt],String_val(oname))) break;

    Tcl_DStringInit(&ds);
    switch(opt) {
     case INoneIdx:
        break;
    case SActiveIdx:
        if (ServerListActiveCmd(dsPtr, GetInterp()->interp, 0, NULL, poolPtr, 0) == NS_OK) {
	  result = ds.string;
	}
        break;
    case SQueuedIdx:
        if (ServerListQueuedCmd(dsPtr, GetInterp()->interp, 0, NULL, poolPtr, 0) == NS_OK) {
	  result = ds.string;
	}
        break;

    case SAllIdx:
        if (ServerListAllCmd(dsPtr, GetInterp()->interp, 0, NULL, poolPtr, 0) == NS_OK) {
	  result = ds.string;
	}
        break;	
     case SConnectionsIdx:
        if(!(servPtr = NsGetServer(GetServer()))) break;
        Ns_MutexLock(&servPtr->pools.lock);
        Ns_DStringPrintf(&ds,"%lu",servPtr->pools.nextconnid);
        Ns_MutexUnlock(&servPtr->pools.lock);
        result = ds.string;
        break;
     case SKeepaliveIdx:
        Ns_DStringPrintf(&ds,"%d",0);
        result = ds.string;
        break;
     case SPoolsIdx:
        if(!(servPtr = NsGetServer(GetServer()))) break;
        Ns_MutexLock(&servPtr->pools.lock);
        for(poolPtr = servPtr->pools.firstPtr;poolPtr;poolPtr = poolPtr->nextPtr)
          Ns_DStringPrintf(&ds,"%s ",poolPtr->pool);
        Ns_MutexUnlock(&servPtr->pools.lock);
        result = ds.string;
        break;
     case SThreadsIdx:
        if(!(servPtr = NsGetServer(GetServer()))) break;
        Ns_MutexLock(&servPtr->pools.lock);
        sprintf(buf, "min %d max %d current %d idle %d",
                servPtr->pools.defaultPtr->threads.min,
                servPtr->pools.defaultPtr->threads.max,
                servPtr->pools.defaultPtr->threads.current,
                servPtr->pools.defaultPtr->threads.idle);
        Ns_MutexUnlock(&servPtr->pools.lock);
        result = buf;
        break;
     case SWaitingIdx:
        if(!(servPtr = NsGetServer(GetServer()))) break;
        Ns_MutexLock(&servPtr->pools.lock);
        Ns_DStringPrintf(&ds,"%d",servPtr->pools.defaultPtr->wqueue.wait.num);
        Ns_MutexUnlock(&servPtr->pools.lock);
        result = ds.string;
        break;
    }
    retval = copy_string(result);
    Tcl_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_Conn_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    int idx;
    Ns_Set *form;
    Conn *connPtr;
    Ns_Conn *conn;
    Tcl_DString ds;
    NsInterp *itPtr;
    const char *result = "";
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;

    static CONST char *cmds[] = {
	 "authpassword", "authuser", "close", "content", "contentlength",
	 "copy", "driver", "encoding", "files", "fileoffset",
	 "filelength", "fileheaders", "flags", "form", "headers",
	 "host", "id", "isconnected", "location", "method",
	 "outputheaders", "peeraddr", "peerport", "port", "protocol",
	 "query", "request", "server", "sock", "start", "status",
	 "url", "urlc", "urlencoding", "urlv", "version", //"write_encoded",
         0
    };
    enum ISubCmdIdx {
	 CAuthPasswordIdx, CAuthUserIdx, CCloseIdx, CContentIdx,
	 CContentLengthIdx, CCopyIdx, CDriverIdx, CEncodingIdx,
	 CFilesIdx, CFileOffIdx, CFileLenIdx, CFileHdrIdx, CFlagsIdx,
	 CFormIdx, CHeadersIdx, CHostIdx, CIdIdx, CIsConnectedIdx,
	 CLocationIdx, CMethodIdx, COutputHeadersIdx, CPeerAddrIdx,
	 CPeerPortIdx, CPortIdx, CProtocolIdx, CQueryIdx, CRequestIdx,
	 CServerIdx, CSockIdx, CStartIdx, CStatusIdx, CUrlIdx,
	 CUrlcIdx, CUrlEncodingIdx, CUrlvIdx, CVersionIdx, //CWriteEncodedIdx,
         INoneIdx
    } opt;

    for(opt = 0;cmds[opt];opt++)
      if(!strcmp(cmds[opt],String_val(oname))) break;

    conn = Ns_GetConn();
    connPtr = (Conn*)conn;

    if(opt != CIsConnectedIdx && !connPtr) CAMLreturn(copy_string(result));

    Tcl_DStringInit(&ds);
    switch(opt) {
     case INoneIdx:
        break;

     case CIsConnectedIdx:
        result = connPtr ? "true" : "false";
	break;
		
     case CUrlvIdx:
        Ns_DStringPrintf(&ds,"%s ",connPtr->request.urlv);
        break;

     case CAuthUserIdx:
        result = Ns_ConnAuthUser(conn);
	break;
	    
     case CAuthPasswordIdx:
	result = Ns_ConnAuthPasswd(conn);
        break;

     case CContentIdx:
        result = Ns_ConnContent(conn);
        break;
	    
     case CContentLengthIdx:
	Ns_DStringPrintf(&ds,"%lu",conn->contentLength);
        result = ds.string;
        break;

     case CEncodingIdx:
	if(connPtr->outputEncoding) result = (char *)Tcl_GetEncodingName(connPtr->outputEncoding);
        break;
	
     case CUrlEncodingIdx:
        if(connPtr->urlEncoding) result = (char *)Tcl_GetEncodingName(connPtr->urlEncoding);
        break;
	
     case CPeerAddrIdx:
	result = Ns_ConnPeerAddr(conn);
        break;
	
     case CPeerPortIdx:
        Ns_DStringPrintf(&ds,"%d",Ns_ConnPeerPort(conn));
        result = ds.string;
        break;

     case CHeadersIdx:
        itPtr = GetInterp();
        if(!(itPtr->nsconn.flags & CONN_TCLHDRS)) {
          Tcl_ResetResult(itPtr->interp);
          Ns_TclEnterSet(itPtr->interp,connPtr->headers,NS_TCL_SET_STATIC);
	  strcpy(itPtr->nsconn.hdrs,Tcl_GetStringResult(itPtr->interp));
	  itPtr->nsconn.flags |= CONN_TCLHDRS;
	}
        result = itPtr->nsconn.hdrs;
	break;
	
     case COutputHeadersIdx:
        itPtr = GetInterp();
	if(!(itPtr->nsconn.flags & CONN_TCLOUTHDRS)) {
          Tcl_ResetResult(itPtr->interp);
          Ns_TclEnterSet(itPtr->interp,connPtr->outputheaders,NS_TCL_SET_STATIC);
	  strcpy(itPtr->nsconn.outhdrs,Tcl_GetStringResult(itPtr->interp));
	  itPtr->nsconn.flags |= CONN_TCLOUTHDRS;
	}
        result = itPtr->nsconn.outhdrs;
	break;
	
     case CFormIdx:
        itPtr = GetInterp();
	if(!(itPtr->nsconn.flags & CONN_TCLFORM)) {
	  form = Ns_ConnGetQuery(itPtr->interp, conn, NULL, NULL); /* ignoring encoding errors */
	  if(form == NULL) {
	    itPtr->nsconn.form[0] = '\0';
	  } else {
            Tcl_ResetResult(itPtr->interp);
            Ns_TclEnterSet(itPtr->interp,form,NS_TCL_SET_STATIC);
	    strcpy(itPtr->nsconn.form,Tcl_GetStringResult(itPtr->interp));
	  }
	  itPtr->nsconn.flags |= CONN_TCLFORM;
	}
        result = itPtr->nsconn.form;
	break;

     case CFilesIdx:
	hPtr = Tcl_FirstHashEntry(&connPtr->files, &search);
	while(hPtr) {
	  Ns_DStringPrintf(&ds,"%s ",Tcl_GetHashKey(&connPtr->files, hPtr));
	  hPtr = Tcl_NextHashEntry(&search);
	}
        result = ds.string;
	break;

     case CFileOffIdx:
     case CFileLenIdx:
     case CFileHdrIdx:
        break;

     case CCopyIdx:
        break;

	//case CWriteEncodedIdx:
        //result = Ns_ConnGetWriteEncodedFlag(conn) ? "true" : "false";
        //break;

     case CRequestIdx:
	result = conn->request.line;
        break;

     case CMethodIdx:
	result = conn->request.method;
	break;

     case CProtocolIdx:
	result = conn->request.protocol;
	break;

     case CHostIdx:
        result = Ns_ConnHost(conn);
	break;
	
     case CPortIdx:
        Ns_DStringPrintf(&ds,"%hu",Ns_ConnPort(conn));
        result = ds.string;
        break;

     case CUrlIdx:
	result = conn->request.url;
        break;
	
     case CQueryIdx:
	result = conn->request.query;
	break;
	
     case CUrlcIdx:
	Ns_DStringPrintf(&ds,"%d",conn->request.urlc);
        result = ds.string;
        break;
	
     case CVersionIdx:
	Ns_DStringPrintf(&ds,"%.2f",conn->request.version);
        result = ds.string;
        break;

     case CLocationIdx:
        result = Ns_ConnLocationAppend(conn, &ds);
	result = ds.string;
        break;

     case CDriverIdx:
	result = Ns_ConnDriverName(conn);
        break;
    
     case CServerIdx:
	result = Ns_ConnServer(conn);
        break;

     case CStatusIdx:
	Ns_DStringPrintf(&ds,"%d",Ns_ConnResponseStatus(conn));
        result = ds.string;
        break;

     case CSockIdx:
	Ns_DStringPrintf(&ds,"%d",Ns_ConnSock(conn));
        result = ds.string;
        break;
	
     case CIdIdx:
	Ns_DStringPrintf(&ds,"%" PRIiPTR,Ns_ConnId(conn));
        result = ds.string;
        break;
	
     case CFlagsIdx:
	Ns_DStringPrintf(&ds,"%d",connPtr->flags);
        result = ds.string;
        break;

     case CStartIdx:
	Ns_DStringPrintf(&ds,"%ld",connPtr->requestQueueTime.sec);
        result = ds.string;
        break;

     case CCloseIdx:
	Ns_ConnClose(conn);
        break;
    }
    retval = copy_string2(result);
    Tcl_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_ReturnRedirect_OCaml(value ourl)
{
    CAMLparam1(ourl);
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnRedirect(conn,String_val(ourl));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_ReturnNotFound_OCaml()
{
    CAMLparam0();
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnNotFound(conn);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_ReturnForbidden_OCaml()
{
    CAMLparam0();
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnForbidden(conn);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_ReturnUnauthorized_OCaml()
{
    CAMLparam0();
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnUnauthorized(conn);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_ReturnInternalError_OCaml()
{
    CAMLparam0();
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnInternalError(conn);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_Return_OCaml(value ostatus,value otype,value odata)
{
    CAMLparam3(ostatus,otype,odata);
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnData(conn,Int_val(ostatus),String_val(odata),strlen(String_val(odata)),String_val(otype));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_ReturnFile_OCaml(value ostatus,value otype,value ofile)
{
    CAMLparam3(ostatus,otype,ofile);
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnReturnFile(conn,Int_val(ostatus),String_val(otype),String_val(ofile));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_Write_OCaml(value ostr)
{
    CAMLparam1(ostr);
    Ns_Conn *conn = Ns_GetConn();
    if(conn) Ns_ConnPuts(conn,String_val(ostr));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_QueryExists_OCaml(value ostr)
{
    CAMLparam1(ostr);
    int result = -1;
    Ns_Conn *conn = Ns_GetConn();
    Ns_Set *form = conn ? Ns_ConnGetQuery(NULL, conn, NULL, NULL) : 0;
    if(form) result = Ns_SetIFind(form,String_val(ostr));
    CAMLreturn(Val_int((result >= 0)));
}

CAMLprim value
Ns_QueryGet_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal1(retval);
    char *result = "";
    Ns_Conn *conn = Ns_GetConn();
    Ns_Set *form = conn ? Ns_ConnGetQuery(NULL, conn, NULL, NULL) : 0;
    if(form) result = Ns_SetIGet(form,String_val(ostr));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_QueryGetAll_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal3(result,nrec,orec);
    int i;
    Ns_Conn *conn = Ns_GetConn();
    Ns_Set *form = conn ? Ns_ConnGetQuery(NULL, conn, NULL, NULL) : 0;

    result = Val_int(0); /* [] */
    for(i = 0;form && i < form->size;i++) {
      if(!strcasecmp(String_val(ostr),form->fields[i].name) && form->fields[i].value) {
        orec = result;
        result = alloc_small(2,0);
        nrec = copy_string(form->fields[i].value);
        Field(result,0) = nrec;
        Field(result,1) = orec;
      }
    }
    CAMLreturn(result);
}

CAMLprim value
Ns_UrlEncode_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal1(retval);
    Ns_DString ds;

    Ns_DStringInit(&ds);

    Ns_UrlQueryEncode(&ds, String_val(ostr), NS_utf8Encoding);
    retval = copy_string(ds.string);
    Ns_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_UrlDecode_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal1(retval);
    Ns_DString ds;

    Ns_DStringInit(&ds);
    Ns_UrlQueryDecode(&ds,String_val(ostr),NS_utf8Encoding);
    retval = copy_string(ds.string);
    Ns_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_Config_OCaml(value osection,value okey)
{
    CAMLparam2(osection,okey);
    CAMLlocal1(retval);
    const char *result = Ns_ConfigGetValue(String_val(osection),String_val(okey));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_GuessType_OCaml(value otype)
{
    CAMLparam1(otype);
    CAMLlocal1(retval);
    const char *result = Ns_GetMimeType(String_val(otype));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_QuoteHtml_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal1(retval);
    Ns_DString ds;

    Ns_DStringInit(&ds);
    Ns_QuoteHtml(&ds,String_val(ostr));
    retval = copy_string(ds.string);
    Ns_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_StripHtml_OCaml(value ostr)
{
    CAMLparam1(ostr);
    CAMLlocal1(retval);
    int inTag = 0, inSpec = 0;
    char *sPtr, *inPtr, *outPtr, *ePtr;

    inPtr = outPtr = sPtr = ns_strdup(String_val(ostr));
    while(*inPtr != '\0') {
      if(*inPtr == '<') {
        inTag = 1;
      } else
      if(inTag && (*inPtr == '>')) {
        inTag = 0;
      } else
      if(inSpec && (*inPtr == ';')) {
        inSpec = 0;
      } else
      if(!inTag && !inSpec) {
        if(*inPtr == '&') {
          ePtr = inPtr;
          if(*ePtr == '&') ePtr++;
          while(*ePtr && *ePtr != ' ' && *ePtr != ';' && *ePtr != '&') ePtr++;
          inSpec = (*ePtr == ';');
        }
        if(!inSpec) *outPtr++ = *inPtr;
      }
      ++inPtr;
    }
    *outPtr = 0;
    retval = copy_string(sPtr);
    ns_free(sPtr);
    CAMLreturn(retval);
}

/* 
 * The following represent the valid combinations of
 * NS_TCL_SET flags
 */
 
#define SET_DYNAMIC         'd'
#define SET_STATIC          't'
#define SET_SHARED_DYNAMIC  's'
#define SET_SHARED_STATIC   'p'
#define IS_DYNAMIC(id)      (*(id) == SET_DYNAMIC || *(id) == SET_SHARED_DYNAMIC)
#define IS_SHARED(id)       (*(id) == SET_SHARED_DYNAMIC || *(id) == SET_SHARED_STATIC)

CAMLprim value
Ns_SetCleanup_OCaml()
{
    CAMLparam0();
    NsInterp *itPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;
    Tcl_HashTable *tablePtr;

    if(!(itPtr = GetInterp())) CAMLreturn(Val_unit);

    tablePtr = &itPtr->sets;
    hPtr = Tcl_FirstHashEntry(tablePtr, &search);
    while(hPtr != NULL) {
      char *key = Tcl_GetHashKey(tablePtr, hPtr);
      if(IS_DYNAMIC(key)) Ns_SetFree((Ns_Set*)Tcl_GetHashValue(hPtr));
      hPtr = Tcl_NextHashEntry(&search);
    }
    Tcl_DeleteHashTable(tablePtr);
    Tcl_InitHashTable(tablePtr, TCL_STRING_KEYS);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetList_OCaml()
{
    CAMLparam0();
    CAMLlocal3(result,nrec,orec);
    NsInterp *itPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;
    Tcl_HashTable *tablePtr;

    result = Val_int(0); /* [] */
    if(!(itPtr = GetInterp()) || !(tablePtr = &itPtr->sets)) CAMLreturn(result);
    hPtr = Tcl_FirstHashEntry(tablePtr,&search);
    while(hPtr) {
      orec = result;
      result = alloc_small(2,0);
      nrec = copy_string(Tcl_GetHashKey(tablePtr,hPtr));
      Field(result,0) = nrec;
      Field(result,1) = orec;
      hPtr = Tcl_NextHashEntry(&search);
    }
    CAMLreturn(result);
}

CAMLprim value
Ns_SetNew_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp())) {
      set = Ns_SetCreate(String_val(oname));
      Tcl_ResetResult(itPtr->interp);
      Ns_TclEnterSet(itPtr->interp,set,NS_TCL_SET_DYNAMIC);
      result = (char*)Tcl_GetStringResult(itPtr->interp);
    }
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetCopy_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp()) && (set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) {
      Tcl_ResetResult(itPtr->interp);
      Ns_TclEnterSet(itPtr->interp,Ns_SetCopy(set),NS_TCL_SET_DYNAMIC);
      result = (char*)Tcl_GetStringResult(itPtr->interp);
    }
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetSplit_OCaml(value oname)
{
    CAMLparam1(oname);
    int i;
    NsInterp *itPtr;
    Ns_Set *set, **sets;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_unit);

    sets = Ns_SetSplit(set,'.');
    for(i = 0; sets[i]; i++) Ns_TclEnterSet(itPtr->interp,sets[i],NS_TCL_SET_DYNAMIC);
    ns_free(sets);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetArray_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal3(result,nrec,orec);
    int i;
    Ns_Set *set;
    NsInterp *itPtr;

    result = Val_int(0); /* [] */
    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(result);

    for(i = 0; i < Ns_SetSize(set); ++i) {
      orec = result;
      result = alloc_small(2,0);
      nrec = copy_string(Ns_SetKey(set,i));
      Field(result,0) = nrec;
      Field(result,1) = orec;
      orec = result;
      result = alloc_small(2,0);
      nrec = copy_string2(Ns_SetValue(set,i));
      Field(result,0) = nrec;
      Field(result,1) = orec;
    }
    CAMLreturn(result);
}

CAMLprim value
Ns_SetSize_OCaml(value oname)
{
    CAMLparam1(oname);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_int(0));
    CAMLreturn(Val_int(Ns_SetSize(set)));
}

CAMLprim value
Ns_SetName_OCaml(value oname)
{
    CAMLparam1(oname);
    CAMLlocal1(retval);
    NsInterp *itPtr;
    Ns_Set *set;

    if((itPtr = GetInterp())) set = Ns_TclGetSet(itPtr->interp,String_val(oname));
    retval = copy_string(set && set->name ? set->name : "");
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetPrint_OCaml(value oname)
{
    CAMLparam1(oname);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_unit);
    Ns_SetPrint(set);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetFree_OCaml(value oname)
{
    CAMLparam1(oname);
    NsInterp *itPtr;

    if((itPtr = GetInterp())) Ns_TclFreeSet(itPtr->interp,String_val(oname));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetFind_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_int(0));
    CAMLreturn(Val_int(Ns_SetFind(set,String_val(okey))));
}

CAMLprim value
Ns_SetIFind_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_int(0));
    CAMLreturn(Val_int(Ns_SetIFind(set,String_val(okey))));
}

CAMLprim value
Ns_SetUnique_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_int(0));
    CAMLreturn(Val_int(Ns_SetUnique(set,String_val(okey))));
}

CAMLprim value
Ns_SetIUnique_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_int(0));
    CAMLreturn(Val_int(Ns_SetIUnique(set,String_val(okey))));
}

CAMLprim value
Ns_SetDelKey_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_unit);
    Ns_SetDeleteKey(set,String_val(okey));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetIDelKey_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    NsInterp *itPtr;
    Ns_Set *set;

    if(!(itPtr = GetInterp()) ||
       !(set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) CAMLreturn(Val_unit);
    Ns_SetIDeleteKey(set,String_val(okey));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetGet_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp()) && (set = Ns_TclGetSet(itPtr->interp,String_val(oname))))
      result = Ns_SetGet(set,String_val(okey));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetIGet_OCaml(value oname,value okey)
{
    CAMLparam2(oname,okey);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp()) && (set = Ns_TclGetSet(itPtr->interp,String_val(oname))))
      result = Ns_SetIGet(set,String_val(okey));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetValue_OCaml(value oname,value oidx)
{
    CAMLparam2(oname,oidx);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       Int_val(oidx) < Ns_SetSize(set))
      result = Ns_SetValue(set,Int_val(oidx));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_SetIsNull_OCaml(value oname,value oidx)
{
    CAMLparam2(oname,oidx);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       Int_val(oidx) < Ns_SetSize(set))
      CAMLreturn(Int_val(Ns_SetValue(set,Int_val(oidx)) ? 1 : 0));
    CAMLreturn(Int_val(0));
}

CAMLprim value
Ns_SetKey_OCaml(value oname,value oidx)
{
    CAMLparam2(oname,oidx);
    CAMLlocal1(retval);
    Ns_Set *set;
    NsInterp *itPtr;
    char *result = "";

    if((itPtr = GetInterp()) && (set = Ns_TclGetSet(itPtr->interp,String_val(oname))))
      result = Ns_SetKey(set,Int_val(oidx));
    retval = copy_string2(result);
    CAMLreturn(retval);
}


CAMLprim value
Ns_SetDelete_OCaml(value oname,value oidx)
{
    CAMLparam2(oname,oidx);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       Int_val(oidx) < Ns_SetSize(set))
      Ns_SetDelete(set,Int_val(oidx));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetTrunc_OCaml(value oname,value oidx)
{
    CAMLparam2(oname,oidx);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       Int_val(oidx) < Ns_SetSize(set))
      Ns_SetTrunc(set,Int_val(oidx));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetUpdate_OCaml(value oname,value okey,value ovalue)
{
    CAMLparam3(oname,okey,ovalue);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) {
      Ns_SetDeleteKey(set,String_val(okey));
      Ns_SetPut(set,String_val(okey),String_val(ovalue));
    }
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetICPut_OCaml(value oname,value okey,value ovalue)
{
    CAMLparam3(oname,okey,ovalue);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) {
      int i = Ns_SetIFind(set,String_val(okey));
      if(i < 0) i = Ns_SetPut(set,String_val(okey),String_val(ovalue));
    }
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetCPut_OCaml(value oname,value okey,value ovalue)
{
    CAMLparam3(oname,okey,ovalue);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname)))) {
      int i = Ns_SetFind(set,String_val(okey));
      if(i < 0) i = Ns_SetPut(set,String_val(okey),String_val(ovalue));
    }
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetPut_OCaml(value oname,value okey,value ovalue)
{
    CAMLparam3(oname,okey,ovalue);
    Ns_Set *set;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))))
      Ns_SetPut(set,String_val(okey),String_val(ovalue));
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetMerge_OCaml(value oname,value oname2)
{
    CAMLparam2(oname,oname2);
    Ns_Set *set, *set2;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       (set2 = Ns_TclGetSet(itPtr->interp,String_val(oname2))))
      Ns_SetMerge(set,set2);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_SetMove_OCaml(value oname,value oname2)
{
    CAMLparam2(oname,oname2);
    Ns_Set *set, *set2;
    NsInterp *itPtr;

    if((itPtr = GetInterp()) &&
       (set = Ns_TclGetSet(itPtr->interp,String_val(oname))) &&
       (set2 = Ns_TclGetSet(itPtr->interp,String_val(oname2))))
      Ns_SetMove(set,set2);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_NormalizePath_OCaml(value opath)
{
    CAMLparam1(opath);
    CAMLlocal1(retval);
    Ns_DString ds;

    Ns_DStringInit(&ds);
    Ns_NormalizePath(&ds,String_val(opath));
    retval = copy_string2(ds.string);
    Ns_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_Url2File_OCaml(value opath)
{
    CAMLparam1(opath);
    CAMLlocal1(retval);
    Ns_DString ds;
    NsInterp *itPtr = GetInterp();

    Ns_DStringInit(&ds);
    if(itPtr) NsUrlToFile(&ds,itPtr->servPtr,String_val(opath));
    retval = copy_string2(ds.string);
    Ns_DStringFree(&ds);
    CAMLreturn(retval);
}

CAMLprim value
Ns_Time_OCaml()
{
    CAMLparam0();
    CAMLreturn(Int_val(time(0)));
}

CAMLprim value
Ns_FmtTime_OCaml(value otime,value ofmt)
{
    CAMLparam2(otime,ofmt);
    CAMLlocal1(retval);
    char result[512];
    time_t time = Int_val(otime);

    strftime(result,sizeof(result),String_val(ofmt),ns_localtime(&time));
    retval = copy_string2(result);
    CAMLreturn(retval);
}

/*
 *  nsv_ implementation copied from tclvar.c due to static declaration
 */

typedef struct Bucket {
    Ns_Mutex lock;
    Tcl_HashTable arrays;
} Bucket;
 
typedef struct Array {
    Bucket *bucketPtr;
    Tcl_HashEntry *entryPtr;
    Tcl_HashTable vars;
} Array;

#define UnlockArray(arrayPtr) Ns_MutexUnlock(&((arrayPtr)->bucketPtr->lock));

static Array *
LockArray(char *array,int create)
{
    NsInterp *itPtr = GetInterp();
    Bucket *bucketPtr;
    Tcl_HashEntry *hPtr;
    Array *arrayPtr;
    register char *p = array;
    register unsigned int result = 0;
    int i, new;

    if(!itPtr) return 0;
    while(1) {
      if((i = *p++) == 0) break;
      result += (result<<3) + i;
    }
    i = result % itPtr->servPtr->nsv.nbuckets;
    bucketPtr = &itPtr->servPtr->nsv.buckets[i];
    Ns_MutexLock(&bucketPtr->lock);
    if(create) {
      hPtr = Tcl_CreateHashEntry(&bucketPtr->arrays, array, &new);
      if(!new) {
        arrayPtr = Tcl_GetHashValue(hPtr);
      } else {
       arrayPtr = ns_malloc(sizeof(Array));
       arrayPtr->bucketPtr = bucketPtr;
       arrayPtr->entryPtr = hPtr;
       Tcl_InitHashTable(&arrayPtr->vars, TCL_STRING_KEYS);
       Tcl_SetHashValue(hPtr, arrayPtr);
      }
    } else {
      if(!(hPtr = Tcl_FindHashEntry(&bucketPtr->arrays, array))) {
        Ns_MutexUnlock(&bucketPtr->lock);
        return NULL;
      }
      arrayPtr = Tcl_GetHashValue(hPtr);
    }
    return arrayPtr;
}

static void
UpdateVar(Tcl_HashEntry *hPtr,char *val,int append)
{
    char *ostr, *nstr;
    int olen = 0,nlen;

    if(!val || !*val) return;
    nlen = strlen(val);
    ostr = Tcl_GetHashValue(hPtr);
    if(append) {
      if(ostr) olen = strlen(ostr);
      nlen += olen;
    }
    nstr = ns_realloc(ostr,(size_t)(nlen+1));
    nstr[olen] = 0;
    strcat(nstr,val);
    Tcl_SetHashValue(hPtr,nstr);
}

static void
SetVar(Array *arrayPtr,char *key,char *value)
{
    int new;
    Tcl_HashEntry *hPtr;

    hPtr = Tcl_CreateHashEntry(&arrayPtr->vars,key,&new);
    UpdateVar(hPtr,value,0);
}

static void
FlushArray(Array *arrayPtr)
{
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;

    hPtr = Tcl_FirstHashEntry(&arrayPtr->vars, &search);
    while(hPtr != NULL) {
      ns_free(Tcl_GetHashValue(hPtr));
      Tcl_DeleteHashEntry(hPtr);
      hPtr = Tcl_NextHashEntry(&search);
    }
}

CAMLprim value
Ns_NsvGet_OCaml(value oarray,value oname)
{
    CAMLparam2(oarray,oname);
    CAMLlocal1(retval);
    Array *arrayPtr;
    Tcl_HashEntry *hPtr;
    char *result = "";

    if((arrayPtr = LockArray(String_val(oarray),0))) {
      hPtr = Tcl_FindHashEntry(&arrayPtr->vars,String_val(oname));
      if(hPtr) result = Tcl_GetHashValue(hPtr);
      UnlockArray(arrayPtr);
    }
    retval = copy_string2(result);
    CAMLreturn(retval);
}

CAMLprim value
Ns_NsvExists_OCaml(value oarray,value oname)
{
    CAMLparam2(oarray,oname);
    Array *arrayPtr;
    int result = 0;

    if((arrayPtr = LockArray(String_val(oarray),0))) {
      if(Tcl_FindHashEntry(&arrayPtr->vars,String_val(oname))) result = 1;
      UnlockArray(arrayPtr);
    }
    CAMLreturn(Val_int(result));
}

CAMLprim value
Ns_NsvSet_OCaml(value oarray,value oname,value ovalue)
{
    CAMLparam3(oarray,oname,ovalue);
    Array *arrayPtr;

    arrayPtr = LockArray(String_val(oarray),1);
    SetVar(arrayPtr,String_val(oname),String_val(ovalue));
    UnlockArray(arrayPtr);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_NsvIncr_OCaml(value oarray,value oname,value ovalue)
{
    CAMLparam3(oarray,oname,ovalue);
    Array *arrayPtr;
    char buf[32];
    int new,result = 0;
    Tcl_HashEntry *hPtr;

    arrayPtr = LockArray(String_val(oarray),1);
    hPtr = Tcl_CreateHashEntry(&arrayPtr->vars,String_val(oname),&new);
    if(!new) result = atoi(Tcl_GetHashValue(hPtr));
    result += Int_val(ovalue);
    sprintf(buf,"%d",result);
    UpdateVar(hPtr,buf,0);
    UnlockArray(arrayPtr);
    CAMLreturn(Val_int(result));
}

CAMLprim value
Ns_NsvAppend_OCaml(value oarray,value oname,value ovalue)
{
    CAMLparam3(oarray,oname,ovalue);
    Array *arrayPtr;
    int new;
    Tcl_HashEntry *hPtr;

    arrayPtr = LockArray(String_val(oarray),1);
    hPtr = Tcl_CreateHashEntry(&arrayPtr->vars,String_val(oname),&new);
    UpdateVar(hPtr,String_val(ovalue),1);
    UnlockArray(arrayPtr);
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_NsvUnset_OCaml(value oarray,value oname)
{
    CAMLparam2(oarray,oname);
    Tcl_HashEntry *hPtr = NULL;
    Array *arrayPtr;

    if(!(arrayPtr = LockArray(String_val(oarray),0))) CAMLreturn(Val_unit);
    if(!strcmp(String_val(oname),""))
      Tcl_DeleteHashEntry(arrayPtr->entryPtr);
    else {
      hPtr = Tcl_FindHashEntry(&arrayPtr->vars,String_val(oname));
      if(hPtr) {
        ns_free(Tcl_GetHashValue(hPtr));
        Tcl_DeleteHashEntry(hPtr);
      }
    }
    UnlockArray(arrayPtr);
    if(!strcmp(String_val(oname),"")) {
      FlushArray(arrayPtr);
      Tcl_DeleteHashTable(&arrayPtr->vars);
      ns_free(arrayPtr);
    }
    CAMLreturn(Val_unit);
}

CAMLprim value
Ns_NsvNames_OCaml(value oarray,value oname)
{
    CAMLparam2(oarray,oname);
    CAMLlocal3(result,nrec,orec);
    NsInterp *itPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;
    Bucket *bucketPtr;
    char *pattern, *key;
    int i;

    result = Val_int(0); /* [] */
    if(!(itPtr = GetInterp())) CAMLreturn(result);

    pattern = String_val(oarray);
    for(i = 0; i < itPtr->servPtr->nsv.nbuckets; i++) {
      bucketPtr = &itPtr->servPtr->nsv.buckets[i];
      Ns_MutexLock(&bucketPtr->lock);
      hPtr = Tcl_FirstHashEntry(&bucketPtr->arrays,&search);
      while(hPtr != NULL) {
        key = Tcl_GetHashKey(&bucketPtr->arrays,hPtr);
        if(!*pattern || Tcl_StringMatch(key,pattern)) {
          orec = result;
          result = alloc_small(2,0);
          nrec = copy_string(key);
          Field(result,0) = nrec;
          Field(result,1) = orec;
        }
        hPtr = Tcl_NextHashEntry(&search);
      }
      Ns_MutexUnlock(&bucketPtr->lock);
    }
    CAMLreturn(result);
}

CAMLprim value
Ns_NsvArrayNames_OCaml(value oarray,value oname)
{
    CAMLparam2(oarray,oname);
    CAMLlocal3(result,nrec,orec);
    Array *arrayPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;
    char *pattern, *key;

    result = Val_int(0); /* [] */
    arrayPtr = LockArray(String_val(oarray),0);
    if(arrayPtr != NULL) {
      pattern = String_val(oname);
      hPtr = Tcl_FirstHashEntry(&arrayPtr->vars, &search);
      while(hPtr != NULL) {
        key = Tcl_GetHashKey(&arrayPtr->vars, hPtr);
        if(!*pattern || Tcl_StringMatch(key,pattern)) {
          orec = result;
          result = alloc_small(2,0);
          nrec = copy_string(key);
          Field(result,0) = nrec;
          Field(result,1) = orec;
        }
        hPtr = Tcl_NextHashEntry(&search);
      }
      UnlockArray(arrayPtr);
    }
    CAMLreturn(result);
}
