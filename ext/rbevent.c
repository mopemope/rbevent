#include "ruby.h"
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <event.h>


VALUE rb_mRubyEvent, rb_mRubyEventConst, rb_cEvent, rb_cSignalEvent, rb_cTimerEvent;

struct eventdata{
    struct event ev;
    VALUE proc;
    short evtype;
};

static const struct signals {
    const char *signm;
    int  signo;
} siglist [] = {
    {"EXIT", 0},
#ifdef SIGHUP
    {"HUP", SIGHUP},
#endif
    {"INT", SIGINT},
#ifdef SIGQUIT
    {"QUIT", SIGQUIT},
#endif
#ifdef SIGILL
    {"ILL", SIGILL},
#endif
#ifdef SIGTRAP
    {"TRAP", SIGTRAP},
#endif
#ifdef SIGIOT
    {"IOT", SIGIOT},
#endif
#ifdef SIGABRT
    {"ABRT", SIGABRT},
#endif
#ifdef SIGEMT
    {"EMT", SIGEMT},
#endif
#ifdef SIGFPE
    {"FPE", SIGFPE},
#endif
#ifdef SIGKILL
    {"KILL", SIGKILL},
#endif
#ifdef SIGBUS
    {"BUS", SIGBUS},
#endif
#ifdef SIGSEGV
    {"SEGV", SIGSEGV},
#endif
#ifdef SIGSYS
    {"SYS", SIGSYS},
#endif
#ifdef SIGPIPE
    {"PIPE", SIGPIPE},
#endif
#ifdef SIGALRM
    {"ALRM", SIGALRM},
#endif
#ifdef SIGTERM
    {"TERM", SIGTERM},
#endif
#ifdef SIGURG
    {"URG", SIGURG},
#endif
#ifdef SIGSTOP
    {"STOP", SIGSTOP},
#endif
#ifdef SIGTSTP
    {"TSTP", SIGTSTP},
#endif
#ifdef SIGCONT
    {"CONT", SIGCONT},
#endif
#ifdef SIGCHLD
    {"CHLD", SIGCHLD},
#endif
#ifdef SIGCLD
    {"CLD", SIGCLD},
#else
# ifdef SIGCHLD
    {"CLD", SIGCHLD},
# endif
#endif
#ifdef SIGTTIN
    {"TTIN", SIGTTIN},
#endif
#ifdef SIGTTOU
    {"TTOU", SIGTTOU},
#endif
#ifdef SIGIO
    {"IO", SIGIO},
#endif
#ifdef SIGXCPU
    {"XCPU", SIGXCPU},
#endif
#ifdef SIGXFSZ
    {"XFSZ", SIGXFSZ},
#endif
#ifdef SIGVTALRM
    {"VTALRM", SIGVTALRM},
#endif
#ifdef SIGPROF
    {"PROF", SIGPROF},
#endif
#ifdef SIGWINCH
    {"WINCH", SIGWINCH},
#endif
#ifdef SIGUSR1
    {"USR1", SIGUSR1},
#endif
#ifdef SIGUSR2
    {"USR2", SIGUSR2},
#endif
#ifdef SIGLOST
    {"LOST", SIGLOST},
#endif
#ifdef SIGMSG
    {"MSG", SIGMSG},
#endif
#ifdef SIGPWR
    {"PWR", SIGPWR},
#endif
#ifdef SIGPOLL
    {"POLL", SIGPOLL},
#endif
#ifdef SIGDANGER
    {"DANGER", SIGDANGER},
#endif
#ifdef SIGMIGRATE
    {"MIGRATE", SIGMIGRATE},
#endif
#ifdef SIGPRE
    {"PRE", SIGPRE},
#endif
#ifdef SIGGRANT
    {"GRANT", SIGGRANT},
#endif
#ifdef SIGRETRACT
    {"RETRACT", SIGRETRACT},
#endif
#ifdef SIGSOUND
    {"SOUND", SIGSOUND},
#endif
#ifdef SIGINFO
    {"INFO", SIGINFO},
#endif
    {NULL, 0}
};

static int
signm2signo(const char *nm)
{
    const struct signals *sigs;

    for (sigs = siglist; sigs->signm; sigs++)
	if (strcmp(sigs->signm, nm) == 0)
	    return sigs->signo;
    return 0;
}

static void 
frb_event_init(VALUE obj){
    event_init();
}

static void 
fevent_callback(int fd, short event, void *args)
{
    struct eventdata *data;
    VALUE obj, proc;
    obj  = (VALUE)args;
    Data_Get_Struct(obj, struct eventdata, data);
    proc = data->proc;
    data->evtype = event;
    if(!NIL_P(proc)){
        rb_funcall(proc, rb_intern("call"), 1, obj);
    }
}

static void 
free_event(struct eventdata *data)
{
    if(data){
        xfree(data);
    }
}

static VALUE
fevent_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, 0, free_event, 0);
}

static VALUE
fevent_get_evtype(VALUE obj)
{
    struct eventdata *data;
    short evtype;
    Data_Get_Struct(obj, struct eventdata, data);
    evtype = data->evtype;
    return INT2NUM(evtype);
}

static VALUE
fevent_get_fileno(VALUE obj)
{
    struct eventdata *data;
    int fd;
    Data_Get_Struct(obj, struct eventdata, data);
    fd = data->ev.ev_fd;
    return INT2NUM(fd);
}


//Event.new(fd, type, &block)
static VALUE
fevent_initialize(int argc, VALUE *argv, VALUE obj)
{
    //event
    struct eventdata *data;
    struct event ev;
    VALUE vident, vtype, vproc;

    rb_scan_args(argc, argv, "2&", &vident, &vtype, &vproc);
    int fd = NUM2INT(vident);
    int type = NUM2INT(vtype);
    
    //event_set(&ev, fd, type, fevent_callback, &obj);

    data = ALLOC(struct eventdata);
    DATA_PTR(obj) = data;
    data->ev = ev;
    data->proc = vproc;
    //printf("ev %d\n", ev.ev_fd);
    event_set(&(data->ev), fd, type, fevent_callback, (void *)obj);
    
    return obj;
}

//SignalEvent.new(fd, type, &block)
static VALUE
fsignal_event_initialize(int argc, VALUE *argv, VALUE obj)
{
    //event
    struct eventdata *data;
    struct event ev;
    const char *signm;
    int signo;
    VALUE vsig, vtype, vproc;

    rb_scan_args(argc, argv, "2&", &vsig, &vtype, &vproc);
    
	signm = SYMBOL_P(vsig) ? rb_id2name(SYM2ID(vsig)) : StringValuePtr(vsig);
	if (strncmp(signm, "SIG", 3) == 0) signm += 3;
	signo = signm2signo(signm);
	if (!signo) {
	    rb_raise(rb_eArgError, "unsupported name `SIG%s'", signm);
	}
    
    int type = NUM2INT(vtype);
    
    //event_set(&ev, fd, type, fevent_callback, &obj);

    data = ALLOC(struct eventdata);
    DATA_PTR(obj) = data;
    data->ev = ev;
    data->proc = vproc;
    //printf("ev %d\n", ev.ev_fd);
    event_set(&(data->ev), signo, type, fevent_callback, (void *)obj);
    
    return obj;
}


//TimerEvent(timout, &block)
static VALUE
ftimer_event_initialize(int argc, VALUE *argv, VALUE obj)
{
    //event
    struct eventdata *data;
    struct event ev;
    struct timeval time;
    struct timeval *ptime;
    VALUE vtimeout, vproc;

    rb_scan_args(argc, argv, "1&", &vtimeout, &vproc);
    //int sec = NUM2INT(vtimeout);
    
    //event_set(&ev, fd, type, fevent_callback, &obj);

    data = ALLOC(struct eventdata);
    DATA_PTR(obj) = data;
    data->ev = ev;
    data->proc = vproc;
    //printf("ev %d\n", ev.ev_fd);
	evtimer_set(&(data->ev), fevent_callback, (void *)obj);
    
    if(!NIL_P(vtimeout)){
        time.tv_sec = NUM2INT(vtimeout);
        time.tv_usec = 0;
        ptime = &time;
    }else{
        ptime = NULL;
    }
    event_add(&(data->ev), ptime);
        
    return obj;
}

static VALUE 
fevent_add(int argc, VALUE *argv,  VALUE obj)
{
    struct eventdata *data;
    struct timeval time;
    struct timeval *ptime = NULL;
    VALUE vtimeout;
    
    rb_scan_args(argc, argv, "01", &vtimeout);
    
    Data_Get_Struct(obj, struct eventdata, data);
    
    if(!NIL_P(vtimeout)){
        //printf("timout %d\n", NUM2INT(vtimeout));
        time.tv_sec = NUM2INT(vtimeout);
        time.tv_usec = 0;
        ptime = &time;
    }else{
        ptime = NULL;
    }
    event_add(&(data->ev), ptime);
    return Qnil;
}

static VALUE
fevent_del(VALUE obj)
{
    struct eventdata *data;
    Data_Get_Struct(obj, struct eventdata, data);
    event_del(&(data->ev));
    return Qnil;
}

static VALUE
frb_event_abort(VALUE obj)
{
    int result = event_loopbreak();
    return INT2NUM(result);
}

static VALUE
frb_event_dispatch(VALUE obj)
{
    int result;
    result = event_dispatch();
    return INT2NUM(result);
}

void
Init_rbevent(void){
    rb_mRubyEvent = rb_define_module("RubyEvent");
    rb_define_module_function(rb_mRubyEvent, "event_init", frb_event_init, 0);
    rb_define_module_function(rb_mRubyEvent, "event_dispatch", frb_event_dispatch, 0);
    rb_define_module_function(rb_mRubyEvent, "event_abort", frb_event_abort, 0);

    rb_mRubyEventConst = rb_define_module_under(rb_mRubyEvent, "Constants");
    //RubyEvent::Constants

    rb_define_const(rb_mRubyEventConst, "EVLIST_TIMEOUT", INT2FIX(EVLIST_TIMEOUT));
    rb_define_const(rb_mRubyEventConst, "EVLIST_INSERTED", INT2FIX(EVLIST_INSERTED));
    rb_define_const(rb_mRubyEventConst, "EVLIST_SIGNAL", INT2FIX(EV_SIGNAL));
    rb_define_const(rb_mRubyEventConst, "EVLIST_ACTIVE", INT2FIX(EVLIST_ACTIVE));
    rb_define_const(rb_mRubyEventConst, "EVLIST_INTERNAL", INT2FIX(EVLIST_INTERNAL));
    rb_define_const(rb_mRubyEventConst, "EVLIST_INIT", INT2FIX(EVLIST_INIT));
    
    rb_define_const(rb_mRubyEventConst, "EV_TIMEOUT", INT2FIX(EV_TIMEOUT));
    rb_define_const(rb_mRubyEventConst, "EV_READ", INT2FIX(EV_READ));
    rb_define_const(rb_mRubyEventConst, "EV_WRITE", INT2FIX(EV_WRITE));
    rb_define_const(rb_mRubyEventConst, "EV_SIGNAL", INT2FIX(EV_SIGNAL));
    rb_define_const(rb_mRubyEventConst, "EV_PERSIST", INT2FIX(EV_PERSIST));

    //#define EVBUFFER_READ		0x01
    //#define EVBUFFER_WRITE		0x02
    //#define EVBUFFER_EOF		0x10
    //#define EVBUFFER_ERROR		0x20
    //#define EVBUFFER_TIMEOUT	0x40

    //Event
    rb_cEvent = rb_define_class("Event", rb_cObject);
    rb_define_alloc_func(rb_cEvent, fevent_alloc);
    rb_define_method(rb_cEvent, "initialize", fevent_initialize, -1);
    rb_define_method(rb_cEvent, "add", fevent_add, -1);
    rb_define_method(rb_cEvent, "delete", fevent_del, 0);
    rb_define_method(rb_cEvent, "fileno", fevent_get_fileno, 0);
    rb_define_method(rb_cEvent, "evtype", fevent_get_evtype, 0);
    
    //Signal
    rb_cSignalEvent = rb_define_class("SignalEvent", rb_cObject);
    rb_define_alloc_func(rb_cSignalEvent, fevent_alloc);
    rb_define_method(rb_cSignalEvent, "initialize", fsignal_event_initialize, -1);
    rb_define_method(rb_cSignalEvent, "add", fevent_add, -1);
    rb_define_method(rb_cSignalEvent, "delete", fevent_del, 0);
    rb_define_method(rb_cSignalEvent, "signal", fevent_get_fileno, 0);
    rb_define_method(rb_cSignalEvent, "evtype", fevent_get_evtype, 0);
    
    //Timer
    rb_cTimerEvent = rb_define_class("TimerEvent", rb_cObject);
    rb_define_alloc_func(rb_cTimerEvent, fevent_alloc);
    rb_define_method(rb_cTimerEvent, "initialize", ftimer_event_initialize, -1);
    rb_define_method(rb_cTimerEvent, "delete", fevent_del, 0);
    rb_define_method(rb_cTimerEvent, "evtype", fevent_get_evtype, 0);

    
}

