#include <assert.h>
#include <stdio.h>
#include "../registry.h"
#include "../context.h"
/* Finite menu fixture: native rows and descriptor duplicates are interleaved
 * with registered providers. Native producers retain row.active state during
 * refresh/navigation; ShopAction must independently enforce the context. */
typedef struct {int role,active;} Offer;
static Offer offers[]={{MS_ROLE_VANILLA,0},{MS_ROLE_DESCRIPTOR,0},{MS_ROLE_REGISTERED,0},{MS_ROLE_VANILLA,0},{MS_ROLE_REGISTERED,0}};
static int native_buys,provider_buys;
static unsigned available(void *world,void *walker){assert(world&&walker);return MS_AVAILABLE;}
static int buy(void *world,void *walker){assert(world&&walker);provider_buys++;return 1;}
static void select_context(int modded){for(unsigned i=0;i<5;i++)offers[i].active=ms_context_allows(modded,offers[i].role);}
static void action(int modded,unsigned index){
 if(!ms_context_allows(modded,offers[index].role))return;
 if(offers[index].role==MS_ROLE_REGISTERED){int world,walker;assert(ms_purchase("tool",&world,&walker)==1);}else native_buys++;
}
static void native_refresh_and_navigation(int modded){
 /* Mirrored source-pinned native activeSelf gate at RefreshShop0x36343f and
  * SelectOffer0x365604: hidden rows are skipped, not made active. */
 for(unsigned i=0;i<5;i++)if(offers[i].active)assert(ms_context_allows(modded,offers[i].role));
}
int main(void){
 MoonShopDefinition d={MS_ABI,sizeof(d),"tool","Tool","A test provider",100,available,buy};assert(ms_register(&d));
 select_context(1);native_refresh_and_navigation(1);action(1,0);action(1,1);assert(native_buys==0&&provider_buys==0); /* Stale hidden vanilla/descriptor selection. */
 action(1,2);assert(provider_buys==1);native_refresh_and_navigation(1);assert(!offers[0].active&&!offers[1].active&&!offers[3].active);
 action(1,3);assert(native_buys==0); /* Purchase refresh cannot authorize a stale original row. */
 select_context(0);native_refresh_and_navigation(0);action(0,2);action(0,1);assert(provider_buys==1&&native_buys==0);action(0,0);assert(native_buys==1);
 select_context(1);native_refresh_and_navigation(1);action(1,4);assert(provider_buys==2&&native_buys==1);
 assert(!ms_context_allows(0,MS_ROLE_INVALID)&&!ms_context_allows(1,MS_ROLE_INVALID));assert(ms_get(0)&&ms_count()==1);puts("shop context sequence: passed");
}
