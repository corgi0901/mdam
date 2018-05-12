#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HLINE "----------------------------------------------------------\n"
#define LABEL_FORMET "%-18s%-14s%-18s%s\n"
#define PRINT_FORMAT "%-18p%-14ld%-18p%s\n"
#define LOG_PATH "MDAM_LOG"

static void *(*orig_malloc)(size_t);          /* malloc */
static void (*orig_free)(void *);             /* free */
static void *(*orig_calloc)(size_t, size_t);  /* calloc */
static void *(*orig_realloc)(void *, size_t); /* realloc */

typedef struct mem_info
{
    void *ptr;
    size_t size;
    void *return_addr;
    char *caller;
    struct mem_info *next;
} mem_info;

static mem_info *head = NULL;
static mem_info *tail = NULL;
static int islock = 0;

static void mdam_init() __attribute__((constructor));
static void mdam_exit() __attribute__((destructor));
static void *get_org_func(const char *name);
static void mem_add(void *ptr, size_t size, void *return_addr, const char *caller);
static void mem_remove(void *ptr);

static void mdam_init()
{
    orig_malloc = get_org_func("malloc");
    orig_free = get_org_func("free");
    orig_calloc = get_org_func("calloc");
    orig_realloc = get_org_func("realloc");
}

static void mdam_exit()
{
    mem_info *info;
    char *log = getenv(LOG_PATH);
    FILE *fp = stderr;
    islock = 1;

    if (log)
    {
        fp = fopen(log, "w");
        if (!fp)
        {
            fprintf(stderr, "Error : failed to open %s\n", log);
            return;
        }
    }

    if (head)
    {
        fprintf(fp, HLINE);
        fprintf(fp, LABEL_FORMET, "Address", "Size (byte)", "Return address", "Caller");
        fprintf(fp, HLINE);
        for (info = head; info != NULL; info = info->next)
        {
            fprintf(fp, PRINT_FORMAT, info->ptr, info->size, info->return_addr, info->caller);
        }
    }
    else
    {
        fprintf(fp, "Any memory leak is not detected !\n");
    }

    if (log)
        fclose(fp);
}

static void *get_org_func(const char *name)
{
    void *func = dlsym(RTLD_NEXT, name);
    if (!func)
        fprintf(stderr, "Error : %s is not implemented\n", name);
    return func;
}

static void mem_add(void *ptr, size_t size, void *return_addr, const char *caller)
{
    if (islock)
        return;

    const char *_caller = caller ? caller : "(none)";
    mem_info *info = (mem_info *)orig_malloc(sizeof(mem_info));
    info->ptr = ptr;
    info->size = size;
    info->return_addr = return_addr;
    info->caller = (char *)orig_calloc(sizeof(char), strlen(_caller) + 1);
    strcpy(info->caller, _caller);
    info->next = NULL;

    if (head == NULL)
    {
        head = info;
        tail = info;
    }
    else
    {
        tail->next = info;
        tail = info;
    }
}

static void mem_remove(void *ptr)
{
    if (islock)
        return;

    mem_info *info, *prev;
    for (info = head; info != NULL; info = info->next)
    {
        if (info->ptr == ptr)
        {
            if (info == head)
            {
                head = head->next;
            }
            else if (info == tail)
            {
                prev->next = NULL;
                tail = prev;
            }
            else
            {
                prev->next = info->next;
            }
            orig_free(info->caller);
            orig_free(info);
            break;
        }
        prev = info;
    }
}

void *malloc(size_t size)
{
    void *p = orig_malloc(size);
    Dl_info info;
    dladdr(__builtin_return_address(0), &info);
    mem_add(p, size, __builtin_return_address(0), info.dli_sname);
    return p;
};

void *calloc(size_t nmemb, size_t size)
{
    void *p = orig_calloc(nmemb, size);
    Dl_info info;
    dladdr(__builtin_return_address(0), &info);
    mem_add(p, nmemb * size, __builtin_return_address(0), info.dli_sname);
    return p;
}

void *realloc(void *ptr, size_t size)
{
    mem_remove(ptr);
    void *p = orig_realloc(ptr, size);
    Dl_info info;
    dladdr(__builtin_return_address(0), &info);
    mem_add(p, size, __builtin_return_address(0), info.dli_sname);
    return p;
}

void free(void *ptr)
{
    mem_remove(ptr);
    orig_free(ptr);
}