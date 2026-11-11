#include <assert.h>
#include <string.h>
#include "../sdk.h"
typedef void *PTR;
static PTR ms_show_chips=(PTR)1,ms_set_action=(PTR)2;
static int hide_calls,action_calls,fail_chips;
static unsigned expected;
static int ms_can_buy(unsigned state){return (state&(MS_AVAILABLE|MS_OWNED|MS_BLOCKED))==MS_AVAILABLE;}
static PTR ms_field(PTR shop,const char *name){assert(shop==(PTR)3&&!strcmp(name,"actionButton"));return (PTR)4;}
static PTR string(const char *text){return (PTR)text;}
static struct {PTR (*string)(const char *);} su={string};
static PTR call(PTR method,PTR instance,void **arguments,int *ok){
    if(!*ok)return 0;
    if(method==ms_show_chips){
        assert(instance==(PTR)3&&!arguments[0]);assert(*(int *)arguments[1]==-1);
        assert(!*(unsigned char *)arguments[2]&&!*(unsigned char *)arguments[3]);hide_calls++;
        if(fail_chips)*ok=0;
    }else{
        assert(method==ms_set_action&&!instance&&arguments[0]==(PTR)4);
        assert(!strcmp(arguments[1],expected&MS_OWNED?"Owned":"Buy"));
        assert(*(unsigned char *)arguments[2]==ms_can_buy(expected));action_calls++;
    }return 0;
}
#include "action-view-runtime.h"
int main(void){
    const unsigned states[]={MS_AVAILABLE,MS_AVAILABLE|MS_OWNED,MS_AVAILABLE|MS_BLOCKED,0};
    for(unsigned i=0;i<4;i++){expected=states[i];assert(ms_details_controls((PTR)3,expected));}
    assert(hide_calls==4&&action_calls==4);fail_chips=1;
    assert(!ms_details_controls((PTR)3,MS_AVAILABLE));assert(hide_calls==5&&action_calls==4);return 0;
}
