#ifndef MOON_SHOP_CONTEXT_H
#define MOON_SHOP_CONTEXT_H
enum {MS_ROLE_INVALID=-1,MS_ROLE_VANILLA,MS_ROLE_REGISTERED,MS_ROLE_DESCRIPTOR};
/* A visible row and a Buy action use the same context boundary. In particular,
 * stale selection/hover events cannot authorize the native purchase fallback. */
static int ms_context_allows(int modded,int role){return modded?role==MS_ROLE_REGISTERED:role==MS_ROLE_VANILLA;}
#endif
