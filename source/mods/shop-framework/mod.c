/* Registered offers use the game's own rows, details, navigation and Buy button. */
#include "../../runtime/native/gameplay.h"
#include "../../runtime/native/services.h"
#include "../../runtime/native/input.h"
#include "unity.h"
#include "registry.h"
#include "context.h"
#include "action.h"
int _fltused=0;
#define MS_KIND 5
#define MS_SHOPS 8
static int ms_started,ms_busy,ms_filtering,ms_opening;
static DWORD ms_thread;
static PTR ms_log,ms_add_offer,ms_refresh,ms_tmp_text,ms_category,ms_open,ms_tool_offers,ms_tool_id,ms_set_action,ms_show_chips,ms_frame;
static void (*ms_barrier)(PTR,PTR *,PTR);
static short (*ms_key)(int);
static PTR (*ms_foreground)(void);
static DWORD (*ms_window_pid)(PTR,DWORD *);

typedef struct {SURoot shop,title,offers[MS_LIMIT];unsigned added;int modded;MSActionGate action;} MSShop;
static MSShop ms_shops[MS_SHOPS];
static PTR ms_field(PTR obj,const char *name){PTR value=0;if(obj){PTR f=get_field(object_class(obj),name);if(f)field_value(obj,f,&value);}return value;}
static int ms_int(PTR obj,const char *name){int value=0;if(obj){PTR f=get_field(object_class(obj),name);if(f)field_value(obj,f,&value);}return value;}
static void ms_text(PTR text,const char *value){if(!text)return;int ok=1;void *a[]={su.string(value)};call(ms_tmp_text,text,a,&ok);}
static void ms_label(PTR shop,const char *name,const char *value){ms_text(ms_field(shop,name),value);}
static void ms_title(MSShop *s,PTR shop,int modded){
    PTR label=ms_field(shop,"pageTitle");int ok=1;
    /* First-open category callbacks can precede creation of the title widget.
     * Update reapplies the context once native construction finishes. */
    if(!su_alive(label))return;
    if(modded){if(!s->title)s->title=su_hold(call(method("TMPro","TMP_Text","get_text",0),label,0,&ok));ms_text(label,"MODDED SHOP");}
    else if(s->title){void *a[]={su_get(s->title)};call(ms_tmp_text,label,a,&ok);su_drop(&s->title);}
}
static void ms_event(const char *value){if(!ms_log)return;char b[200],*p=cat(b,"{\"event\":\"");p=cat(p,value);p=cat(p,"\"}\n");DWORD n=0;WriteFile(ms_log,b,(DWORD)(p-b),&n,0);}
static MSShop *ms_slot(PTR shop){
    for(unsigned i=0;i<MS_SHOPS;i++)if(su_get(ms_shops[i].shop)==shop)return ms_shops+i;
    for(unsigned i=0;i<MS_SHOPS;i++)if(!su_alive(su_get(ms_shops[i].shop))){
        MSShop *s=ms_shops+i;su_drop(&s->shop);su_drop(&s->title);for(unsigned j=0;j<MS_LIMIT;j++)su_drop(s->offers+j);s->added=0;s->modded=0;s->action=(MSActionGate){0};s->shop=su_hold(shop);return s->shop?s:0;
    }
    return 0;
}

static PTR ms_world(PTR shop){int ok=1;return call(method("MoonCleaners","MoonUpgradeShop","get_World",0),shop,0,&ok);}
static unsigned ms_offer_state(PTR shop,const MoonShopDefinition *d){return d?ms_state(d->key,ms_world(shop),ms_field(shop,"walker")):0;}
static const char *ms_status(unsigned state){return state&MS_OWNED?"Owned":!(state&MS_AVAILABLE)?"Unavailable":state&MS_BLOCKED?"Locked":"Available";}
static int ms_can_buy(unsigned state){return (state&(MS_AVAILABLE|MS_OWNED|MS_BLOCKED))==MS_AVAILABLE;}
static void ms_price(char *out,const MoonShopDefinition *d,unsigned state){char *p=num(cat(out,"$"),(unsigned)d->price);p=cat(p,"  |  ");cat(p,ms_status(state));}
static void ms_rows(PTR shop){MSShop *s=ms_slot(shop);if(!s)return;for(unsigned i=0;i<s->added;i++){
    const MoonShopDefinition *d=ms_get(i);PTR offer=su_get(s->offers[i]);if(!d||!offer)continue;char b[160];ms_price(b,d,ms_offer_state(shop,d));ms_text(ms_field(offer,"line"),b);
}}
static int ms_details_controls(PTR shop,unsigned state){
    int ok=1,container=-1;unsigned char no=0,enabled=(unsigned char)ms_can_buy(state);
    void *chips[]={0,&container,&no,&no};call(ms_show_chips,shop,chips,&ok);
    PTR button=ms_field(shop,"actionButton");
    /* SetAction updates the native ActionTag caption, key layout, colors and
     * interactability together. The first TMP child is only the ENTER keycap. */
    void *action[]={button,su.string(state&MS_OWNED?"Owned":"Buy"),&enabled};
    call(ms_set_action,0,action,&ok);return ok;
}
static void ms_details(PTR shop,const MoonShopDefinition *d){
    unsigned state=ms_offer_state(shop,d);char price[160];ms_price(price,d,state);
    ms_label(shop,"detailCategory","MODDED EQUIPMENT");ms_label(shop,"detailName",d->name);ms_label(shop,"detailSummary",d->description);ms_label(shop,"detailStatus",price);ms_label(shop,"chipCaption","");
    PTR captions=ms_field(shop,"statCaptions"),values=ms_field(shop,"statValues");
    for(U64 i=0;captions&&i<su.array_length(captions)&&i<16;i++)ms_text(*(PTR *)su.array_address(captions,8,i),"");
    for(U64 i=0;values&&i<su.array_length(values)&&i<16;i++)ms_text(*(PTR *)su.array_address(values,8,i),"");
    PTR art=ms_field(shop,"detailArt"),symbol=ms_field(shop,"detailSymbol");
    su_bool(method("UnityEngine","Behaviour","set_enabled",1),art,0);su_bool(method("UnityEngine","Behaviour","set_enabled",1),symbol,0);
    if(!ms_details_controls(shop,state)){
        su_bool(method("UnityEngine.UI","Selectable","set_interactable",1),ms_field(shop,"actionButton"),0);ms_event("native-action-view-failed");
    }
}
static void ms_insert(PTR shop){
    if(ms_busy||ms_filtering||!ms_field(shop,"shopContent")||!ms_field(shop,"offers"))return;
    MSShop *s=ms_slot(shop);if(!s)return;ms_busy=1;
    while(s->added<ms_count_value){
        unsigned i=s->added;const MoonShopDefinition *d=ms_get(i);int kind=MS_KIND,index=(int)i,category=0,ok=1;float rotation=0,scale=1;
        void *a[]={&kind,&index,&category,su.string(d->name),0,&rotation,&scale};PTR offer=call(ms_add_offer,shop,a,&ok);
        if(!ok||!offer){ms_event("offer-insertion-failed");break;}
        s->offers[i]=su_hold(offer);if(!s->offers[i])break;s->added++;
    }ms_busy=0;
}
/* Only a temporary managed list changes. Offer kinds, tool descriptors,
 * catalog arrays and saved inventory identifiers stay exactly native. */
static int ms_reference(PTR obj,const char *name,PTR value){PTR f=obj?get_field(object_class(obj),name):0;int offset=f?su.field_offset(f):0;if(offset<16)return 0;ms_barrier(obj,(PTR *)((unsigned char *)obj+offset),value);return 1;}
static int ms_registered(MSShop *s,PTR offer){for(unsigned i=0;i<s->added;i++)if(su_get(s->offers[i])==offer)return 1;return 0;}
static int ms_duplicate(PTR shop,PTR offer){
    if(ms_int(offer,"kind")!=0)return 0;
    int ok=1,index=ms_int(offer,"index");PTR array=call(ms_tool_offers,shop,0,&ok);U64 n=array?su.array_length(array):0;
    if(!ok||!array||n>1024||index<0||(U64)index>=n)return -1;
    PTR descriptor=*(PTR *)su.array_address(array,8,(U64)index),tool=ms_field(descriptor,"tool");
    PTR id=tool?call(ms_tool_id,tool,0,&ok):0;if(!ok||!id)return -1;
    int length=su.string_length(id);if(length<1||length>63)return 0;
    char key[64];su_string(id,key,sizeof(key));return ms_find(key)!=0;
}
static const MoonShopDefinition *ms_provider(MSShop *s,PTR offer){if(!s||!offer)return 0;for(unsigned i=0;i<s->added;i++)if(su_get(s->offers[i])==offer)return ms_get(i);return 0;}
static int ms_role(PTR shop,MSShop *s,PTR offer){if(!s||!offer)return MS_ROLE_INVALID;if(ms_registered(s,offer))return MS_ROLE_REGISTERED;int duplicate=ms_duplicate(shop,offer);return duplicate<0?MS_ROLE_INVALID:duplicate?MS_ROLE_DESCRIPTOR:MS_ROLE_VANILLA;}
static void ms_row_visible(PTR offer,int visible){int ok=1;PTR row=ms_field(offer,"row"),obj=row?call(method("UnityEngine","Component","get_gameObject",0),row,0,&ok):0;if(obj)su_bool(method("UnityEngine","GameObject","SetActive",1),obj,visible);}
static int ms_filter(PTR shop,int category){
    ms_insert(shop);MSShop *s=ms_slot(shop);PTR full=ms_field(shop,"offers"),items=ms_field(full,"_items");int count=ms_int(full,"_size");
    if(!s||!full||!items||count<0||count>512||su.array_length(items)<(U64)count)return 0;
    SURoot original=su_hold(full),filtered=su_hold(su.new_object(object_class(full)));int ok=original&&filtered;PTR list=su_get(filtered);
    if(!ok){su_drop(&original);su_drop(&filtered);return 0;}
    call(get_method(object_class(full),".ctor",0),list,0,&ok);PTR add=get_method(object_class(full),"Add",1);
    for(int i=0;i<count&&ok;i++){
        PTR offer=*(PTR *)su.array_address(items,8,(U64)i);if(!offer){ok=0;break;}
        int role=ms_role(shop,s,offer);if(role==MS_ROLE_INVALID){ok=0;break;}
        if(ms_context_allows(s->modded,role)){void *a[]={offer};call(add,list,a,&ok);}
    }
    if(!ok){su_drop(&filtered);su_drop(&original);return 0;}
    /* Native filtering only visits included rows; explicitly hide exclusions
     * before the native category producer owns layout and first selection. */
    for(int i=0;i<count;i++)ms_row_visible(*(PTR *)su.array_address(items,8,(U64)i),0);
    if(!ms_reference(shop,"offers",list)){su_drop(&filtered);su_drop(&original);return 0;}
    ms_filtering=1;int selected=s->modded?0:category;void *a[]={&selected};call(ms_category,shop,a,&ok);ms_filtering=0;
    if(ms_field(shop,"offers")==list)ms_reference(shop,"offers",su_get(original));else{ok=0;ms_event("foreign-offer-list-writer");}
    su_drop(&filtered);su_drop(&original);
    if(s->modded){ms_title(s,shop,1);ms_rows(shop);}
    return ok;
}
static PTR ms_find_menu(PTR walker,int *ok){
    PTR type=su_type("MoonCleaners","MoonUpgradeShop");void *a[]={type};PTR shop=call(su_method("UnityEngine","Component","GetComponent","System.Type"),walker,a,ok);
    if(!shop&&*ok){PTR camera=ms_field(walker,"viewCamera");void *camera_args[]={camera};shop=call(method("MoonCleaners","MoonUpgradeShop","ForCamera",1),0,camera_args,ok);}return *ok?shop:0;
}
static int ms_open_modded(void *walker){
    if(!walker||!ms_thread||ms_thread!=GetCurrentThreadId())return 0;int ok=1;PTR world=ms_field(walker,"world");
    if(!su_alive(world)||boolean(method("MoonCleaners","MoonWorld","get_IsRestoringRendering",0),world,&ok)||!ok)return 0;
    PTR shop=ms_find_menu(walker,&ok);
    MSShop *s=shop&&ok?ms_slot(shop):0;if(!s)return 0;s->modded=1;
    /* OpenInventory(false) chooses the native Shop page, not the inventory page. */
    ms_opening=1;su_bool(method("MoonCleaners","MoonUpgradeShop","OpenInventory",1),shop,0);ms_opening=0;
    int tools=0;void *category_args[]={&tools};call(ms_category,shop,category_args,&ok);return ok&&s->modded&&boolean(method("MoonCleaners","MoonUpgradeShop","get_IsOpen",0),shop,&ok)&&ok;
}
static void ms_shortcut(PTR walker){
    int ok=1;DWORD pid=0;PTR window=ms_foreground();if(window)ms_window_pid(window,&pid);
    int allowed=pid==GetCurrentProcessId()&&boolean(method("UnityEngine","Application","get_isFocused",0),0,&ok)&&!boolean(method("MoonCleaners","MoonInput","get_Typing",0),0,&ok)&&ok;
    MoonInputState input=moon_input_action("modded-shop.toggle","!Alt+Ctrl+Shift+F2",allowed);if(!input.pressed)return;
    PTR shop=ms_find_menu(walker,&ok);if(!shop||!ok)return;MSShop *s=ms_slot(shop);if(!s)return;
    int opened=boolean(method("MoonCleaners","MoonUpgradeShop","get_IsOpen",0),shop,&ok);if(!ok)return;
    if(opened&&s->modded){su_bool(ms_open,shop,0);return;}
    if(!opened&&(boolean(method("MoonCleaners","MoonWalker","get_MenuBlocked",0),walker,&ok)||!boolean(method("MoonCleaners","MoonWalker","get_IsFirstPerson",0),walker,&ok)||!ok))return;
    ms_open_modded(walker);
}
static int ms_before(int id,Context *c){
    if(id==0){if(!ms_thread)ms_thread=GetCurrentThreadId();if(ms_thread!=GetCurrentThreadId())return 0;ms_sealed=1;ms_shortcut((PTR)c->rcx);
        for(unsigned i=0;i<MS_SHOPS;i++){PTR shop=su_get(ms_shops[i].shop);int ok=1;if(su_alive(shop)&&!ms_int(shop,"inventoryPage")&&boolean(method("MoonCleaners","MoonUpgradeShop","get_IsOpen",0),shop,&ok)&&integer(method("MoonCleaners","MoonUpgradeShop","get_SelectedCategory",0),shop,&ok)==0&&ok){if(ms_shops[i].modded){ms_title(ms_shops+i,shop,1);}ms_rows(shop);const MoonShopDefinition *d=ms_provider(ms_shops+i,ms_field(shop,"shownOffer"));if(d)ms_details(shop,d);}}return 0;}
    if(ms_thread&&ms_thread!=GetCurrentThreadId())return 0;
    PTR shop=(PTR)c->rcx;
    if(id==1){ms_insert(shop);return 0;}
    if(id==5){MSShop *s=ms_slot(shop);if(s&&!ms_opening){ms_title(s,shop,0);s->modded=0;}return 0;}
    if(id==4){if(ms_filtering)return 0;if(!ms_filter(shop,(int)c->rdx)){ms_event("native-shop-filter-failed");su_bool(ms_open,shop,0);}return 1;}

    MSShop *s=ms_slot(shop);PTR selected=ms_field(shop,id==2?"shownOffer":"selectedOffer");int role=ms_role(shop,s,selected);
    if(id==3||id==6){
        int route=ms_action_route(id==6,s?s->modded:0,ms_int(shop,"inventoryPage"),role);
        if(route==MS_ACTION_PASS)return 0;
        c->rax=0;
        if(route==MS_ACTION_DENY){ms_event("out-of-context-purchase-denied");return 1;}
        const MoonShopDefinition *d=ms_provider(s,selected);int ok=1,fresh=0;
        unsigned frame=(unsigned)integer(ms_frame,0,&ok);
        if(!ok){ms_event("native-action-frame-failed");return 1;}
        int result=ms_action_once(&s->action,d,frame,ms_world(shop),ms_field(shop,"walker"),&fresh);
        c->rax=result>0;
        if(!fresh)return 1;
        ms_event(id==6?"native-enter-action":"native-button-action");
        void *a[]={su.string(result>0?"Purchased":result<0?"Purchase failed":"Purchase unavailable")};call(method("MoonCleaners","MoonUpgradeShop","set_LastMessage",1),shop,a,&ok);
        ms_event(result>0?"purchased":"purchase-denied");call(ms_refresh,shop,0,&ok);ms_rows(shop);ms_details(shop,d);return 1;
    }
    if(id==2&&ms_int(shop,"inventoryPage"))return 0;
    if(id==2&&!ms_context_allows(s?s->modded:0,role)){
        ms_label(shop,"detailName","Select an item");ms_label(shop,"detailSummary","");ms_label(shop,"detailStatus","");su_bool(method("UnityEngine.UI","Selectable","set_interactable",1),ms_field(shop,"actionButton"),0);
        return 1;
    }
    const MoonShopDefinition *d=ms_provider(s,selected);if(!d)return 0;
    if(id==2){ms_details(shop,d);return 1;}
    return 0;
}
static unsigned ms_api_state(const char *key,void *world,void *walker){return ms_thread&&ms_thread==GetCurrentThreadId()?ms_state(key,world,walker):0;}
static int ms_api_purchase(const char *key,void *world,void *walker){return ms_thread&&ms_thread==GetCurrentThreadId()?ms_purchase(key,world,walker):0;}
static const MoonShop ms_api={MS_ABI,sizeof(MoonShop),ms_register,ms_count,ms_get,ms_api_state,ms_api_purchase,ms_open_modded};
__declspec(dllexport) const MoonShop *MoonLab_Shop_v1(unsigned abi,unsigned size){return ms_started&&abi==MS_ABI&&(size==sizeof(MoonShop)||size==sizeof(MoonShop)-sizeof(void *))?&ms_api:0;}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
    if(ms_started)return 1;
    if(!bind_runtime(domain,game)||!su_bind(game))return 0;
    char path[1100];DWORD n=GetEnvironmentVariableA("MOONLAB_EVIDENCE",path,1000);if(!n||n>=1000)return 0;cat(path+n,"\\shop-framework.jsonl");ms_log=CreateFileA(path,0x40000000,1,0,1,0x80,0);if(ms_log==(PTR)(U64)-1){ms_log=0;return 0;}
    ms_add_offer=method("MoonCleaners","MoonUpgradeShop","AddOffer",7);ms_refresh=method("MoonCleaners","MoonUpgradeShop","Refresh",0);ms_tmp_text=method("TMPro","TMP_Text","set_text",1);
    ms_category=method("MoonCleaners","MoonUpgradeShop","SelectCategory",1);ms_open=method("MoonCleaners","MoonUpgradeShop","SetOpen",1);ms_tool_offers=method("MoonCleaners","MoonUpgradeShop","get_ToolOffers",0);ms_tool_id=method("MoonCleaners","MoonToolDefinition","get_Id",0);
    ms_set_action=method("MoonCleaners","MoonSurveyKit","SetAction",3);ms_show_chips=method("MoonCleaners","MoonUpgradeShop","ShowChips",4);ms_frame=method("UnityEngine","Time","get_frameCount",0);
    if(!ms_set_action||method_token(ms_set_action)!=100671033||!ms_show_chips||method_token(ms_show_chips)!=100666916||!ms_frame)return 0;
    ms_barrier=(void(*)(PTR,PTR *,PTR))GetProcAddress(game,"il2cpp_gc_wbarrier_set_field");
    PTR user=LoadLibraryA("user32.dll");if(!user)return 0;ms_key=(short(*)(int))GetProcAddress(user,"GetAsyncKeyState");ms_foreground=(PTR(*)(void))GetProcAddress(user,"GetForegroundWindow");ms_window_pid=(DWORD(*)(PTR,DWORD *))GetProcAddress(user,"GetWindowThreadProcessId");
    if(!ms_barrier||!ms_key||!ms_foreground||!ms_window_pid||!ms_category||!ms_open||!ms_tool_offers||method_token(ms_tool_offers)!=100666873||!ms_tool_id||method_token(ms_tool_id)!=100671161)return 0;
    if(!ms_add_offer||method_token(ms_add_offer)!=100666896||!ms_refresh||method_token(ms_refresh)!=100666884||!ms_tmp_text)return 0;
    if(!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game)||!prepare_hook(method("MoonCleaners","MoonUpgradeShop","RefreshShop",0),100666912,1,game)||!prepare_hook(method("MoonCleaners","MoonUpgradeShop","RefreshDetails",0),100666913,2,game)||!prepare_hook(method("MoonCleaners","MoonUpgradeShop","ShopAction",0),100666904,3,game)||!prepare_hook(ms_category,100666901,4,game)||!prepare_hook(ms_open,100666880,5,game)||!prepare_hook(method("MoonCleaners","MoonUpgradeShop","EquipSelectedTool",0),100666854,6,game))return 0;
    before_hook=ms_before;if(!install_hooks())return 0;if(moon_runtime_loader_present()){
        const MoonRuntimeApiV1 *runtime=moon_runtime_connect();
        const MoonServiceLimitV1 limits[]={{"max_items",MS_LIMIT}};
        MoonServicePublicationLimitsV1 publication={.base={.size=sizeof(publication),.service_id="shop",.abi=MS_ABI,.api_size=sizeof(ms_api),.api=&ms_api},.limit_count=sizeof(limits)/sizeof(limits[0]),.reserved=0,.limits=limits};
        if(!runtime||!runtime->publish(&publication.base))return 0;
    }
    ms_started=1;ms_event("ready");return 1;
}
