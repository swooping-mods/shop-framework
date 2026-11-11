#include <assert.h>
#include <string.h>
typedef void *PTR;
typedef unsigned long long SURoot;
typedef struct {SURoot title;} MSShop;
static PTR current_label;
static int reads,writes,drops;
static PTR field(PTR shop,const char *name){assert(shop==(PTR)1&&!strcmp(name,"pageTitle"));return current_label;}
#define ms_field field
static PTR method(const char *ns,const char *type,const char *name,int arity){
    assert(!strcmp(ns,"TMPro")&&!strcmp(type,"TMP_Text")&&!strcmp(name,"get_text")&&!arity);return (PTR)2;
}
static PTR call(PTR fn,PTR instance,void **args,int *ok){
    assert(*ok&&instance==(PTR)3);
    if(fn==(PTR)2){assert(!args);reads++;return (PTR)4;}
    assert(fn==(PTR)5&&args&&args[0]==(PTR)4);writes++;return 0;
}
static void text(PTR label,const char *value){assert(label==(PTR)3&&!strcmp(value,"MODDED SHOP"));writes++;}
#define ms_text text
static PTR ms_tmp_text=(PTR)5;
static int alive(PTR p){return p==(PTR)3;}
static SURoot hold(PTR p){assert(p==(PTR)4);return 9;}
static PTR get(SURoot r){assert(r==9);return (PTR)4;}
static void drop(SURoot *r){assert(*r==9);*r=0;drops++;}
static struct {int (*alive)(PTR);SURoot (*hold)(PTR);PTR (*get)(SURoot);void (*drop)(SURoot *);} su={alive,hold,get,drop};
#define su_get(r) su.get(r)
#define su_hold(p) su.hold(p)
#define su_drop(p) su.drop(p)
#define su_alive(p) su.alive(p)
#include "title-runtime.h"
int main(void){
    MSShop state={0};
    ms_title(&state,(PTR)1,1);assert(!reads&&!writes&&!state.title);
    current_label=(PTR)3;ms_title(&state,(PTR)1,1);assert(reads==1&&writes==1&&state.title==9);
    ms_title(&state,(PTR)1,1);assert(reads==1&&writes==2);
    current_label=0;ms_title(&state,(PTR)1,0);assert(!drops&&state.title==9);
    current_label=(PTR)3;ms_title(&state,(PTR)1,0);assert(drops==1&&!state.title&&writes==3);
    return 0;
}
