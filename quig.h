void setPixel(SDL_Surface *target, int x, int y, Uint32 color);
SDL_Surface* generateFont(int mode);
void cleanup();
int do_key(int key);
int c_key(lua_State *LL);
int init_fn();
int step_fn();
void do_squ(int x, int y, double scale, int r, int g, int b);
int c_squ(lua_State *LL);
void do_rect(int x, int y, int w, int h, int r, int g, int b);
int c_rect(lua_State *LL);
void do_spr(int x, int y, double scale, int sx, int sy);
int c_spr(lua_State *LL);
void do_text(const char *str, int x, int y, double scale, int mode);
int c_text(lua_State *LL);
bool do_squcol(int x1, int y1, int s1, int x2, int y2, int s2);
int c_squcol(lua_State *LL);
void do_cls(int r, int g, int b);
int c_cls(lua_State *LL);
void registerLuaFn();
int main(int argc, char* argv[]);
int max2(int a, int b);
int max3(int a, int b, int c);
int min2(int a, int b);