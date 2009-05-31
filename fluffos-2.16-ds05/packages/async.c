#include "std.h"
#include "../lpc_incl.h"
#include "async.h"
#include "../function.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef F_ASYNC_GETDIR
#define _GNU_SOURCE
#include <sys/syscall.h>
#endif
#include "../config.h"
#include "../interpret.h"
#include "../file.h"

#if defined(F_ASYNC_READ) || defined(F_ASYNC_WRITE)

struct cb_mem{
    function_to_call_t cb;
    struct cb_mem *next;
} *cbs = 0;

struct aiob_mem{
    aiob aio;
    struct aiob_mem *next;
} *aiobs = 0;

struct req_mem{
    struct request req;
    struct req_mem *next;
} *reqms;

struct stuff{
    void (*func)(void *);
    void *data;
    struct stuff *next;
} *todo, *lasttodo;

struct stuff_mem{
    struct stuff stuff;
    struct stuff_mem *next;
} *stuffs;

pthread_mutex_t mem_mut;


struct stuff *get_stuff(){
    struct stuff *ret;
    if(stuffs){
        pthread_mutex_lock(&mem_mut);
        ret = &stuffs->stuff;
        stuffs = stuffs->next;
        ((struct stuff_mem *)ret)->next = 0;
        pthread_mutex_unlock(&mem_mut);
    }else{
        ret = MALLOC(sizeof(struct stuff_mem));
        ((struct stuff_mem *)ret)->next = 0;
    }
    return ret;
}

void free_stuff(struct stuff *stuff){
    struct stuff_mem *stufft = (struct stuff_mem *)stuff;
    pthread_mutex_lock(&mem_mut);
    stufft->next = stuffs;
    stuffs = stufft;
    pthread_mutex_unlock(&mem_mut);
}

pthread_mutex_t mut;
pthread_mutex_t work_mut;
int thread_started = 0;

void thread_func(void *mydata){
    while(1){
        pthread_mutex_lock(&mut);
        while(todo){
            pthread_mutex_lock(&work_mut);
            struct stuff *work = todo;
            todo = todo->next;
            if(!todo)
                lasttodo = NULL;
            pthread_mutex_unlock(&work_mut);
            work->func(work->data);
            free_stuff(work);
        }
    }
}

void do_stuff(void (*func)(void *), void *data){
    if(!thread_started){
        pthread_mutex_init(&mut, NULL);
        pthread_mutex_init(&mem_mut, NULL);
        pthread_mutex_init(&work_mut, NULL);
        pthread_mutex_lock(&mut);
        pthread_t t;
        pthread_create(&t, NULL, thread_func, NULL);
        thread_started = 1;
    }
    struct stuff *work = get_stuff();
    work->func = func;
    work->data = data;
    work->next = NULL;
    pthread_mutex_lock(&work_mut);
    if(lasttodo){
        lasttodo->next = work;
        lasttodo = work;
    } else {
        todo = lasttodo = work;
    }
    pthread_mutex_unlock(&work_mut);
    pthread_mutex_unlock(&mut);
}

function_to_call_t *get_cb(){
    function_to_call_t *ret;
    if(cbs){
        ret = &cbs->cb;
        cbs = cbs->next;
        ((struct cb_mem *)ret)->next = 0;
    }else{
        ret = MALLOC(sizeof(struct cb_mem));
        ((struct cb_mem *)ret)->next = 0;
    }
    memset(ret, 0, sizeof(function_to_call_t));
    return ret;
}

void free_cb(function_to_call_t *cb){
    struct cb_mem *cbt = (struct cb_mem *)cb;
    cbt->next = cbs;
    cbs = cbt;
}

aiob *get_aiob(){
    aiob *ret;
    if(aiobs){
        ret = &aiobs->aio;
        aiobs = aiobs->next;
        ((struct aiob_mem *)ret)->next = 0;
    }else{
        ret = MALLOC(sizeof(struct aiob_mem));
        ((struct aiob_mem *)ret)->next = 0;
    }
    return ret;
}

void free_aiob(aiob *aio){
    struct aiob_mem *aiobt = (struct aiob_mem *)aio;
    aiobt->next = aiobs;
    aiobs = aiobt;
}

struct request *get_req(){
    struct request *ret;
    if(reqms){
        ret = &reqms->req;
        reqms = reqms->next;
        ((struct req_mem *)ret)->next = 0;
    }else{
        ret = MALLOC(sizeof(struct req_mem));
        ((struct req_mem *)ret)->next = 0;
    }
    return ret;
}

void free_req(struct request *req){
    struct req_mem *reqt = (struct req_mem *)req;
    reqt->next = reqms;
    reqms = reqt;
}


static struct request *reqs = NULL;
static struct request *lastreq = NULL;
void add_req(struct request *req){
    if(lastreq){
        lastreq->next = req;
    } else {
        reqs = req;
    }
    req->next = NULL;
    lastreq = req;
}

#ifdef PACKAGE_COMPRESS
#include <zlib.h>

void *gzreadthread(void *data){
    aiob *aio = (aiob *)data;
    void *file = gzdopen(dup(aio->aio_fildes), "rb");
    aio->__return_value = gzread(file, (void *)(aio->aio_buf), aio->aio_nbytes);
    aio->__error_code = 0;
    gzclose(file);
    return NULL;
}

int aio_gzread(aiob *aio){
    pthread_t thread;
    aio->__error_code = EINPROGRESS;
    do_stuff(gzreadthread, aio);
    return 0;
}

void *gzwritethread(void *data){
    aiob *aio = (aiob *)data;
    void *file = gzdopen(dup(aio->aio_fildes), "wb");
    aio->__return_value = gzwrite(file, (void *)(aio->aio_buf), aio->aio_nbytes);
    aio->__error_code = 0;
    gzclose(file);
    return NULL;
}

int aio_gzwrite(aiob *aio){
    pthread_t thread;
    aio->__error_code = EINPROGRESS;
    do_stuff(gzwritethread, aio);
    return 0;
}
#endif

void *writethread(void *data){
    aiob *aio = (aiob *)data;
    aio->__return_value = write(aio->aio_fildes, (void *)(aio->aio_buf), aio->aio_nbytes);
    aio->__error_code = 0;
    return NULL;
}

int aio_write(aiob *aio){
    pthread_t thread;
    aio->__error_code = EINPROGRESS;
    do_stuff(writethread, aio);
    return 0;
}

void *readthread(void *data){
    aiob *aio = (aiob *)data;
    aio->__return_value = read(aio->aio_fildes, (void *)(aio->aio_buf), aio->aio_nbytes);
    aio->__error_code = 0;
    return NULL;
}

int aio_read(aiob *aio){
    pthread_t thread;
    aio->__error_code = EINPROGRESS;
    do_stuff(readthread, aio);
    return 0;
}



#ifdef F_ASYNC_GETDIR
void *getdirthread(void *data){
    aiob *aio = (aiob *)data;
    aio->__return_value = syscall(SYS_getdents, aio->aio_fildes, aio->aio_buf, aio->aio_nbytes);
    aio->__error_code = 0;
    return NULL;
}

int aio_getdir(aiob *aio){
    pthread_t thread;
    aio->__error_code = EINPROGRESS;
    do_stuff(getdirthread, aio);
    return 0;
}

#endif

int add_read(const char *fname, function_to_call_t *fun) {
    if (fname) {
        aiob *aio = get_aiob();
        memset(aio, 0, sizeof(aiob));
        //printf("fname: %s\n", fname);
        int fd = open(fname, O_RDONLY);
        aio->aio_fildes = fd;
        aio->aio_buf = (char *)MALLOC(READ_FILE_MAX_SIZE);
        aio->aio_nbytes = READ_FILE_MAX_SIZE;
        struct request *req = get_req();
        req->aio = aio;
        req->fun = fun;
        req->type = aread;
        add_req(req);
#ifdef PACKAGE_COMPRESS
        return aio_gzread(aio);
#else
        return aio_read(aio);
#endif
    }else
        error("permission denied\n");
    return 1;
}

#ifdef F_ASYNC_GETDIR
extern int max_array_size;
int add_getdir(const char *fname, function_to_call_t *fun) {
    if (fname) {
        aiob *aio= get_aiob();
        memset(aio, 0, sizeof(aiob));
        //printf("fname: %s\n", fname);
        int fd = open(fname, O_RDONLY);
        aio->aio_fildes = fd;
        aio->aio_buf = (char *)MALLOC(sizeof(struct dirent) * max_array_size);
        aio->aio_nbytes = sizeof(struct dirent) * max_array_size;
        struct request *req = get_req();
        req->aio = aio;
        req->fun = fun;
        req->type = agetdir;
        add_req(req);
        return aio_getdir(aio);
    }else
        error("permission denied\n");
    return 1;
}
#endif

int add_write(const char *fname, const char *buf, int size, char flags, function_to_call_t *fun) {
    if (fname) {
        aiob *aio = get_aiob();
        memset(aio, 0, sizeof(aiob));
        int fd = open(fname, flags & 1 ? O_CREAT|O_WRONLY
                : O_CREAT|O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG);
        aio->aio_fildes = fd;
        aio->aio_buf = buf;
        aio->aio_nbytes = size;
        struct request *req = get_req();
        req->aio = aio;
        req->fun = fun;
        req->type = awrite;
        assign_svalue_no_free(&req->tmp, sp-2);
        add_req(req);
#ifdef PACKAGE_COMPRESS
        if(flags & 2)
            return aio_gzwrite(aio);
        else
#endif
            return aio_write(aio);
    } else
        error("permission denied\n");
    return 1;
}

void handle_read(struct request *req, int val){
    aiob *aio = req->aio;
    close(aio->aio_fildes);
    if(val){
        FREE(aio->aio_buf);
        push_number(val);
        set_eval(max_cost);
        safe_call_efun_callback(req->fun, 1);
        return;
    }
    val = aio_return(aio);
    if(val < 0){
        FREE(aio->aio_buf);
        push_number(val);
        set_eval(max_cost);
        safe_call_efun_callback(req->fun, 1);
        return;
    }
    char *file = new_string(val, "read_file_async: str");
    memcpy(file, (char *)(aio->aio_buf), val);
    file[val]=0;
    push_malloced_string(file);
    FREE(aio->aio_buf);
    set_eval(max_cost);
    safe_call_efun_callback(req->fun, 1);
}

#ifdef F_ASYNC_GETDIR

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next dirent */
    unsigned short d_reclen;  /* Length of this dirent */
    char           d_name []; /* Filename (null-terminated) */
    /* length is actually (d_reclen - 2 -
       offsetof(struct linux_dirent, d_name) */
};


void handle_getdir(struct request *req, int val){
    aiob *aio = req->aio;
    close(aio->aio_fildes);
    val = aio_return(aio);
    array_t *ret = allocate_empty_array(val);
    int i;
    if(val > -1)
    {
        struct linux_dirent *de = (struct linux_dirent *)aio->aio_buf;
        for(i=0; ((char *)de) - (char *)(aio->aio_buf) < val; i++)
        {
            svalue_t *vp = &(ret->item[i]);
            vp->type = T_STRING;
            vp->subtype = STRING_MALLOC;
            //printf("%s ", de->d_name);
            vp->u.string = string_copy(de->d_name, "encode_stat");
            de = (struct linux_dirent *)(((char *)de) + de->d_reclen);
        }
    }
    ret = RESIZE_ARRAY(ret, i);
    ret->size = i;
    push_refed_array(ret);
    set_eval(max_cost);
    safe_call_efun_callback(req->fun, 1);
}
#endif


void handle_write(struct request *req, int val){
    aiob *aio = req->aio;
    close(aio->aio_fildes);
    free_svalue(&req->tmp, "handle_write");
    if(val){
        push_number(val);
        set_eval(max_cost);
        safe_call_efun_callback(req->fun, 1);
        return;
    }
    val = aio_return(aio);
    if(val < 0){
        push_number(val);
        set_eval(max_cost);
        safe_call_efun_callback(req->fun, 1);
        return;
    }
    push_undefined();
    set_eval(max_cost);
    safe_call_efun_callback(req->fun, 1);
}

void check_reqs() {
    while (reqs) {
        int val = aio_error(reqs->aio);
        if (val != EINPROGRESS) {
            enum atypes type =  (reqs->type);
            reqs->type = done;
            switch (type) {
                case aread:
                    handle_read(reqs, val);
                    break;
                case awrite:
                    handle_write(reqs, val);
                    break;
#ifdef F_ASYNC_GETDIR
                case agetdir:
                    handle_getdir(reqs, val);
                    break;
#endif
                case done:
                    //must have had an error while handling it before.
                    break;
                default:
                    fatal("unknown async type\n");
            }
            struct request *here = reqs;
            reqs = reqs->next;
            if(!reqs)
                lastreq = reqs;
            free_aiob(here->aio);
            free_funp(here->fun->f.fp);
            free_cb(here->fun);
            free_req(here);
        } else
            return;
    }
}

#ifdef F_ASYNC_READ

void f_async_read(){
    function_to_call_t *cb = get_cb();
    process_efun_callback(1, cb, F_ASYNC_READ);
    cb->f.fp->hdr.ref++;
    add_read(check_valid_path((sp-1)->u.string, current_object, "read_file", 0), cb);
    pop_2_elems();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write(){
    function_to_call_t *cb = get_cb();
    process_efun_callback(3, cb, F_ASYNC_WRITE);
    cb->f.fp->hdr.ref++;
    add_write(check_valid_path((sp-3)->u.string, current_object, "write_file", 1), (sp-2)->u.string, strlen((sp-2)->u.string), (sp-1)->u.number, cb);
    pop_n_elems(4);
}
#endif

#ifdef F_ASYNC_GETDIR
void f_async_getdir(){
    function_to_call_t *cb = get_cb();
    process_efun_callback(1, cb, F_ASYNC_READ);
    cb->f.fp->hdr.ref++;
    add_getdir(check_valid_path((sp-1)->u.string, current_object, "get_dir", 0), cb);
    pop_2_elems();
}
#endif
#ifdef F_ASYNC_DB_EXEC
void f_async_db_exec(){
    array_t *info;
    db_t *db;
    info = allocate_empty_array(1);
    info->item[0].type = T_STRING;
    info->item[0].subtype = STRING_MALLOC;
    info->item[0].u.string = string_copy((sp-1)->u.string, "f_db_exec");
    valid_database("exec", info);

    db = find_db_conn((sp-1)->u.number);
    if (!db) {
        error("Attempt to exec on an invalid database handle\n");
    }

    if (db->type->cleanup) {
        db->type->cleanup(&(db->c));
    }

    function_to_call_t *cb = get_cb();
    process_efun_callback(2, cb, F_ASYNC_READ);
    cb->f.fp->hdr.ref++;

    add_db_exec(db, (sp-1)->u.string, cb);
    pop_2_elems();
}
#endif
#endif
