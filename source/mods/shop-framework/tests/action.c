#include <assert.h>
#include <stdio.h>
#include "../action.h"
static unsigned flags=MS_AVAILABLE,frame;
static int charges,grants,nested_fresh,nested_result;
static MSActionGate gate;
static const MoonShopDefinition *definition;
static unsigned state(void *world,void *walker){assert(world&&walker);return flags;}
static int purchase(void *world,void *walker){
    charges++;nested_result=ms_action_once(&gate,definition,frame,world,walker,&nested_fresh);grants++;return 1;
}
int main(void){
    int world=1,walker=2,fresh;
    MoonShopDefinition d={MS_ABI,sizeof(d),"repeatable","Repeatable","Actual provider dispatch",750,state,purchase};
    assert(ms_register(&d)&&ms_count()==1);definition=ms_get(0);
    assert(ms_action_route(1,1,0,MS_ROLE_REGISTERED)==MS_ACTION_PROVIDER);
    assert(ms_action_route(0,1,0,MS_ROLE_REGISTERED)==MS_ACTION_PROVIDER);
    assert(ms_action_route(1,0,0,MS_ROLE_REGISTERED)==MS_ACTION_PASS);
    assert(ms_action_route(1,1,1,MS_ROLE_REGISTERED)==MS_ACTION_PASS);
    assert(ms_action_route(0,1,1,MS_ROLE_REGISTERED)==MS_ACTION_DENY);
    for(int role=MS_ROLE_INVALID;role<=MS_ROLE_DESCRIPTOR;role++)if(role!=MS_ROLE_REGISTERED){
        assert(ms_action_route(1,1,0,role)==MS_ACTION_DENY);
        assert(ms_action_route(0,1,0,role)==MS_ACTION_DENY);
    }
    assert(ms_action_route(0,0,0,MS_ROLE_VANILLA)==MS_ACTION_PASS);
    assert(ms_action_route(1,0,1,MS_ROLE_VANILLA)==MS_ACTION_PASS);
    assert(ms_action_once(&gate,definition,frame,&world,&walker,&fresh)==1&&fresh);
    assert(charges==1&&grants==1&&!nested_fresh&&!nested_result);
    /* Native Button submit plus native Enter in one frame still debit once,
     * even for a repeatable provider that does not become Owned. */
    assert(ms_action_once(&gate,definition,frame,&world,&walker,&fresh)==1&&!fresh);
    assert(charges==1&&grants==1);
    frame++;assert(ms_action_once(&gate,definition,frame,&world,&walker,&fresh)==1&&fresh);
    assert(charges==2&&grants==2);
    const unsigned denied[]={MS_BLOCKED,MS_OWNED};
    for(unsigned i=0;i<2;i++){
        frame++;flags=MS_AVAILABLE|denied[i];
        assert(!ms_action_once(&gate,definition,frame,&world,&walker,&fresh)&&fresh);
        assert(charges==2&&grants==2);
    }
    frame++;flags=0;assert(!ms_action_once(&gate,definition,frame,&world,&walker,&fresh));assert(charges==2);
    frame++;flags=MS_AVAILABLE;
    assert(!ms_action_once(&gate,definition,frame,0,&walker,&fresh)&&!fresh);
    assert(!ms_action_once(&gate,definition,frame,&world,0,&fresh)&&!fresh);
    assert(!ms_action_once(&gate,0,frame,&world,&walker,&fresh)&&!fresh);
    puts("native button/Enter admission and single-frame purchase passed");return 0;
}
