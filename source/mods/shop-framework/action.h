#ifndef MOON_SHOP_ACTION_H
#define MOON_SHOP_ACTION_H
#include "registry.h"
#include "context.h"
enum {MS_ACTION_PASS,MS_ACTION_DENY,MS_ACTION_PROVIDER};
/* Native Enter reaches EquipSelectedTool, while the Buy button reaches
 * ShopAction. Inventory equip and ordinary shop behavior remain native. */
static int ms_action_route(int keyboard,int modded,int inventory,int role){
    if(inventory)return keyboard?MS_ACTION_PASS:role==MS_ROLE_REGISTERED?MS_ACTION_DENY:MS_ACTION_PASS;
    if(keyboard&&!modded)return MS_ACTION_PASS;
    if(!ms_context_allows(modded,role))return MS_ACTION_DENY;
    return role==MS_ROLE_REGISTERED?MS_ACTION_PROVIDER:MS_ACTION_PASS;
}
typedef struct {unsigned frame;const MoonShopDefinition *definition;int seen,result;} MSActionGate;
static int ms_action_once(MSActionGate *gate,const MoonShopDefinition *definition,unsigned frame,void *world,void *walker,int *fresh){
    *fresh=0;if(!definition||!world||!walker)return 0;
    /* EventSystem submit and the native keyboard path may see the same press.
     * Mark before invoking the provider so recursive UI dispatch also declines. */
    if(gate->seen&&gate->frame==frame&&gate->definition==definition)return gate->result;
    gate->seen=1;gate->frame=frame;gate->definition=definition;gate->result=0;*fresh=1;
    gate->result=ms_purchase(definition->key,world,walker);return gate->result;
}
#endif
