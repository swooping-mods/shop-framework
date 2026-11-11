#include <assert.h>
#include <stdio.h>
#include "../registry.h"
static unsigned current=MS_AVAILABLE;
static int charges,grants,recursive;
static unsigned state(void *world,void *walker){assert(world&&walker);return current;}
static int purchase(void *world,void *walker){charges++;recursive=ms_purchase("vacuum",world,walker);grants++;current=MS_AVAILABLE|MS_OWNED;return 1;}
int main(void){
    int world,walker;MoonShopDefinition d={MS_ABI,sizeof(d),"vacuum","Vacuum","A dust collector",120,state,purchase};
    MoonShopDefinition bad=d;bad.abi++;assert(!ms_register(&bad));bad=d;bad.key="../vacuum";assert(!ms_register(&bad));bad=d;bad.price=-1;assert(!ms_register(&bad));
    assert(ms_register(&d));assert(ms_register(&d));assert(ms_count()==1);assert(!ms_get(1));bad=d;bad.price++;assert(!ms_register(&bad));assert(ms_get(0)->price==120);
    current=MS_AVAILABLE|MS_BLOCKED;assert(!ms_purchase("vacuum",&world,&walker));assert(charges==0);
    current=MS_AVAILABLE;assert(!ms_purchase("vacuum",0,&walker));assert(ms_purchase("vacuum",&world,&walker)==1);assert(charges==1&&grants==1&&recursive==0);
    assert(!ms_purchase("vacuum",&world,&walker));assert(charges==1);assert(ms_state("missing",&world,&walker)==0);
    ms_sealed=1;bad=d;bad.key="late";assert(!ms_register(&bad));assert(ms_register(&d));ms_sealed=0;
    static char keys[MS_LIMIT][16];
    for(unsigned i=1;i<MS_LIMIT;i++){snprintf(keys[i],sizeof(keys[i]),"item-%u",i);bad=d;bad.key=keys[i];assert(ms_register(&bad));}
    bad=d;bad.key="overflow";assert(ms_count()==MS_LIMIT&&!ms_register(&bad));puts("shop registry: passed");
}
