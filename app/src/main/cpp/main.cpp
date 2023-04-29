
#include "main.h"
#include "shadowhook.h"

char buff[30];
char pmm_str[30];
//char target_pmm[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
char target_pmm[8] = {0x00, 0xf1, 0x00, 0x00, 0x00, 0x01, 0x43, 0x00};

void *(*old_func)(u_int8_t, u_int8_t *, int) = nullptr;

void *new_func(u_int8_t a1, u_int8_t *a2, int a3) {

    __android_log_print(6, "pmmtool_sh", "hook _Z23nfa_dm_check_set_confighPhb arg0->%x arg1->%x", a1, a2);

    // if (a1 == 0x1d) { // hardcoded arg pattern
    // 40 0a [syscode] [IDm] 53 02 01 00 55 01 01 51 08 [PMm]
    // if (a1 == 0x1b) { // another type hardcoded arg pattern
    // 40 12 [syscode] [IDm] [PMm] 53 02 01 00 55 01 01

    // handmade hexdump
    for (int i = 0x0; i < 0x10; ++i)
        sprintf(buff + i * 3, "%02x ", *(char *)(a2 + i));
    __android_log_print(6, "pmmtool_sh", "[%x]: %s", a2, buff);
    for (int i = 0x0; i < 0x10; ++i)
        sprintf(buff + i * 3, "%02x ", *(char *)(a2 + 0x10 + i));
    __android_log_print(6, "pmmtool_sh", "[%x]: %s", a2 + 0x10, buff);

    // look for 51 08 (set pmm command) for type 0x1d
    for (int i = 0x0; i < 0x20; ++i) {
        if (*(char *)(a2 + i) == 0x51 && *(char *)(a2 + i + 1) == 0x08) {

            for (int j = 0; j < 8; ++j)
                sprintf(pmm_str + j * 3, "%02x ", *(char *)(a2 + i + 2 + j));
            __android_log_print(6, "pmmtool_sh", "[1] old PMm: %s", pmm_str);

            // set
            for (int j = 0; j < 8; ++j)
                *(char *)(a2 + i + 2 + j) = target_pmm[j];

            for (int j = 0; j < 8; ++j)
                sprintf(pmm_str + j * 3, "%02x ", *(char *)(a2 + i + 2 + j));
            __android_log_print(6, "pmmtool_sh", "[1] new PMm: %s", pmm_str);
        }
    }

    // look for FF FF FF FF FF FF FF FF (pmm itself)
    for (int i = 0x0; i < 0x20; ++i) {
        if (*(char *)(a2 + i) == 0xff && *(char *)(a2 + i + 1) == 0xff
            && *(char *)(a2 + i + 2) == 0xff && *(char *)(a2 + i + 3) == 0xff
            && *(char *)(a2 + i + 4) == 0xff && *(char *)(a2 + i + 5) == 0xff
            && *(char *)(a2 + i + 6) == 0xff && *(char *)(a2 + i + 7) == 0xff) {

            for (int j = 0; j < 8; ++j)
                sprintf(pmm_str + j * 3, "%02x ", *(char *)(a2 + i  + j));
            __android_log_print(6, "pmmtool_sh", "[2] old PMm: %s", pmm_str);

            // set
            for (int j = 0; j < 8; ++j)
                *(char *)(a2 + i + j) = target_pmm[j];

            for (int j = 0; j < 8; ++j)
                sprintf(pmm_str + j * 3, "%02x ", *(char *)(a2 + i + j));
            __android_log_print(6, "pmmtool_sh", "[2] new PMm: %s", pmm_str);
        }
    }
    //}
    //__android_log_print(6, "pmmtool_sh", "load old func");
    void *result = old_func(a1, a2, a3);
    //__android_log_print(6, "pmmtool_sh", "hook result -> %x", result);
    return result;
}

void do_hook() {
    int init_result = shadowhook_init(SHADOWHOOK_MODE_UNIQUE, true);
    __android_log_print(6, "pmmtool_sh", "ShadowHook init result -> %d", init_result)

    //void *stub = shadowhook_hook_sym_name("libnfc-nci.so", "_Z23nfa_dm_check_set_confighPhb",
    void *stub = shadowhook_hook_sym_name("libnfc-nci.so", "_Z23nfa_dm_check_set_confighPhb",
                                               (void *)new_func,(void **)&old_func);
    if(stub == NULL)
    {
        int err_num = shadowhook_get_errno();
        const char *err_msg = shadowhook_to_errmsg(err_num);
        __android_log_print(6, "pmmtool_sh", "Hook error %d - %s", err_num, err_msg);
    }
}

jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    __android_log_print(6, "pmmtool_sh", "Inside JNI_OnLoad");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        __android_log_print(6, "pmmtool_sh", "Start hooking");
        do_hook();
        __android_log_print(6, "pmmtool_sh", "Hooked");
        return JNI_VERSION_1_6;
    }
    return 0;
}
