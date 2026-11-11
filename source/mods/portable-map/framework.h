#include "../shop-framework/client.h"
#include "../hints-framework/sdk/client.h"
#ifdef FW_TEMPLATE
#include "../equipment-framework/sdk/client.h"
static const MoonEquipment *fw_equipment;
#endif
static const MoonShop *fw_shop;
static const MoonHints *fw_hints;
static int fw_started;
static int fw_context(PTR world,PTR walker){int ok=1;return world==dget(m.world)&&walker==dget(m.walker)&&malive(world)&&!boolean(m.restoring,world,&ok)&&ok;}
static unsigned fw_state(PTR world,PTR walker){
 if(!fw_context(world,walker))return 0;
 unsigned state=MS_AVAILABLE|(m.owned?MS_OWNED:0);
 if(!m.root[0]||m.invalid_store||m.authority_lost||!host())state|=MS_BLOCKED;
 return state;
}
static int fw_purchase(PTR world,PTR walker){
 unsigned state=fw_state(world,walker);if(state!=MS_AVAILABLE)return 0;
 int ok=1;if(!boolean(m.focused,0,&ok)||boolean(m.typing,0,&ok)||!ok)return 0;
 purchase(FW_PRICE);return m.owned?1:0;
}
#ifdef FW_TEMPLATE
static int fw_visual(PTR definition,PTR world){
 (void)world;PTR field=get_field(object_class(definition),"visualScale");if(!field)return 0;
 void (*set)(PTR,PTR,void *)=(void(*)(PTR,PTR,void *))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_field_set_value");
 if(!set)return 0;float hidden=0;set(definition,field,&hidden);return 1;
}
static int fw_owned(PTR world,PTR walker){return fw_context(world,walker)&&m.owned;}
static int fw_equipped(PTR walker){return fw_equipment&&m.owned&&fw_equipment->is_equipped(MOD_ID,walker);}
#endif
static void fw_clear(void){if(fw_hints)fw_hints->clear(MOD_ID);}
static int fw_bind(PTR domain,PTR game){
 (void)num;fw_shop=ms_connect(domain,game);fw_hints=mh_connect(domain,game);if(!fw_shop||!fw_hints)return 0;
 MoonShopDefinition offer={MS_ABI,sizeof(offer),MOD_ID,FW_NAME,FW_DESC,FW_PRICE,fw_state,fw_purchase};
 if(!fw_shop->register_item(&offer))return 0;
#ifdef FW_TEMPLATE
 fw_equipment=me_connect(domain,game);if(!fw_equipment)return 0;
 MoonEquipmentDefinition item={ME_ABI,sizeof(item),MOD_ID,FW_NAME,FW_DESC,FW_TEMPLATE,fw_owned,fw_visual};
 if(!fw_equipment->register_item(&item))return 0;
#endif
 return 1;
}
