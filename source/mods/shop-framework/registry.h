#ifndef MOON_SHOP_REGISTRY_H
#define MOON_SHOP_REGISTRY_H
#include "sdk.h"
static MoonShopDefinition ms_definitions[MS_LIMIT];
static unsigned ms_count_value;
static int ms_sealed,ms_purchasing;
static int ms_equal(const char *a,const char *b){if(!a||!b)return 0;while(*a&&*a==*b){a++;b++;}return *a==*b;}
static int ms_string_valid(const char *s,unsigned max,int key){
    if(!s||!*s)return 0;
    for(unsigned i=0;i<=max;i++){
        unsigned char c=(unsigned char)s[i];if(!c)return i!=0;
        if(i==max||c<32||c>126)return 0;
        if(key&&!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='.'))return 0;
    }return 0;
}
static const MoonShopDefinition *ms_find(const char *key){for(unsigned i=0;i<ms_count_value;i++)if(ms_equal(key,ms_definitions[i].key))return ms_definitions+i;return 0;}
static int ms_register(const MoonShopDefinition *d){
    if(!d||d->abi!=MS_ABI||d->size!=sizeof(*d)||!ms_string_valid(d->key,63,1)||!ms_string_valid(d->name,95,0)||!ms_string_valid(d->description,767,0)||d->price<0||!d->state||!d->purchase)return 0;
    const MoonShopDefinition *old=ms_find(d->key);
    /* Repeated dependency startup is safe; an ID cannot change providers. */
    if(old)return old->state==d->state&&old->purchase==d->purchase&&old->price==d->price&&ms_equal(old->name,d->name)&&ms_equal(old->description,d->description);
    if(ms_sealed||ms_count_value==MS_LIMIT)return 0;
    ms_definitions[ms_count_value++]=*d;return 1;
}
static unsigned ms_count(void){return ms_count_value;}
static const MoonShopDefinition *ms_get(unsigned index){return index<ms_count_value?ms_definitions+index:0;}
static unsigned ms_state(const char *key,void *world,void *walker){const MoonShopDefinition *d=ms_find(key);return d&&world&&walker?d->state(world,walker)&(MS_AVAILABLE|MS_OWNED|MS_BLOCKED):0;}
static int ms_purchase(const char *key,void *world,void *walker){
    const MoonShopDefinition *d=ms_find(key);if(!d||!world||!walker||ms_purchasing)return 0;
    unsigned state=ms_state(key,world,walker);if(!(state&MS_AVAILABLE)||(state&(MS_OWNED|MS_BLOCKED)))return 0;
    ms_purchasing=1;int result=d->purchase(world,walker);ms_purchasing=0;return result;
}
#endif
