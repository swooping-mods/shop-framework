/* Exercise the actual mapping-provider adapter against the production shop
 * registry. Managed world/input, wallet IO and equipment selection are mocked;
 * this qualifies registration/payment routing, not a rendered Unity menu. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../registry.h"
#include "../../equipment-framework/sdk/equipment.h"
#include "../../hints-framework/sdk/hints.h"
#define MOON_SHOP_FRAMEWORK_CLIENT_H
#define MOON_EQUIPMENT_CLIENT_H
#define MOON_HINTS_CLIENT_H
typedef void *PTR;
#include "provider-config.h"
static struct {PTR world,walker,restoring,focused,typing;char root[192];int owned,invalid_store,authority_lost;} m;
static int is_host=1,focused=1,typing,restoring,selected,charges,charged,cleared;
static MoonEquipmentDefinition equipment;
static PTR dget(PTR p){return p;}
static int malive(PTR p){return p!=0;}
static int boolean(PTR method,PTR receiver,int *ok){(void)receiver;*ok=1;return method==m.restoring?restoring:method==m.focused?focused:typing;}
static int host(void){return is_host;}
static void purchase(int price){assert(!m.owned);charges++;charged+=price;m.owned=1;}
static PTR object_class(PTR p){return p;}
static PTR get_field(PTR klass,const char *name){(void)klass;(void)name;return 0;}
static PTR GetModuleHandleA(const char *name){(void)name;return 0;}
static PTR GetProcAddress(PTR module,const char *name){(void)module;(void)name;return 0;}
static void num(void){}
static int register_equipment(const MoonEquipmentDefinition *d){equipment=*d;return 1;}
static int equipped(const char *key,PTR walker){assert(!strcmp(key,MOD_ID)&&walker==m.walker);return selected;}
static void clear(const char *key){assert(!strcmp(key,MOD_ID));cleared++;}
static const MoonShop shop={MS_ABI,sizeof(MoonShop),ms_register,ms_count,ms_get,ms_state,ms_purchase,0};
static const MoonEquipment tools={ME_ABI,sizeof(MoonEquipment),register_equipment,equipped,0};
static const MoonHints hints={MH_ABI,sizeof(MoonHints),0,clear};
static const MoonShop *ms_connect(PTR domain,PTR game){(void)domain;(void)game;return &shop;}
static const MoonEquipment *me_connect(PTR domain,PTR game){(void)domain;(void)game;return &tools;}
static const MoonHints *mh_connect(PTR domain,PTR game){(void)domain;(void)game;return &hints;}
#ifdef MAP_PROVIDER
#include "../../portable-map/framework.h"
#else
#include "../../flag-finder/framework.h"
#endif
int main(void){
 int world,walker,other; m.world=&world;m.walker=&walker;
 m.restoring=(PTR)1;m.focused=(PTR)2;m.typing=(PTR)3;strcpy(m.root,"private-test-world");
 (void)num;(void)fw_started;
 assert(fw_bind(0,0)&&ms_count()==1);
 const MoonShopDefinition *d=ms_get(0);assert(!strcmp(d->key,EXPECTED_KEY)&&d->price==EXPECTED_PRICE);
 assert(!strcmp(equipment.key,MOD_ID)&&!strcmp(equipment.name,d->name));
 assert(!ms_purchase(MOD_ID,&other,&walker)&&!ms_purchase(MOD_ID,&world,&other));
 restoring=1;assert(!ms_purchase(MOD_ID,&world,&walker));restoring=0;
 is_host=0;assert(!ms_purchase(MOD_ID,&world,&walker));is_host=1;
 m.authority_lost=1;assert(!ms_purchase(MOD_ID,&world,&walker));m.authority_lost=0;
 m.invalid_store=1;assert(!ms_purchase(MOD_ID,&world,&walker));m.invalid_store=0;
 m.root[0]=0;assert(!ms_purchase(MOD_ID,&world,&walker));strcpy(m.root,"private-test-world");
 focused=0;assert(!ms_purchase(MOD_ID,&world,&walker));focused=1;
 typing=1;assert(!ms_purchase(MOD_ID,&world,&walker));typing=0;
 assert(charges==0&&!equipment.owned(&world,&walker));
 selected=1;assert(!fw_equipped(&walker));
 assert(ms_purchase(MOD_ID,&world,&walker)==1&&charges==1&&charged==FW_PRICE);
 assert(equipment.owned(&world,&walker)&&fw_equipped(&walker));
 assert(!equipment.owned(&other,&walker)&&!equipment.owned(&world,&other));
 assert(!ms_purchase(MOD_ID,&world,&walker)&&charges==1);
 selected=0;assert(!fw_equipped(&walker));fw_clear();assert(cleared==1);
 puts(MOD_ID ": custom shop registration, guarded purchase and equipment ownership passed");
}
