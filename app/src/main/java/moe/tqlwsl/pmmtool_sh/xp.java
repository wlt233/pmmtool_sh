package moe.tqlwsl.pmmtool_sh;

import android.app.Application;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

import java.util.List;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XC_MethodReplacement;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class xp implements IXposedHookLoadPackage {
    ClassLoader mclassloader = null;
    Context mcontext = null;

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {

        XposedBridge.log(lpparam.packageName);

        if (lpparam.packageName.equals("com.android.nfc")) {

            XposedBridge.log("Inside " + lpparam.packageName);

            XposedHelpers.findAndHookMethod("com.android.nfc.NfcApplication",
                    lpparam.classLoader, "onCreate", new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {
                            XposedBridge.log("Inside com.android.nfc.NfcApplication#onCreate");
                            super.beforeHookedMethod(param);
                            Application application = (Application) param.thisObject;
                            mcontext = application.getApplicationContext();
                            XposedBridge.log("Got context");
                        }
                    });


            XposedHelpers.findAndHookMethod("android.nfc.cardemulation.NfcFCardEmulation",
                lpparam.classLoader, "isValidSystemCode", String.class, new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {
                    super.afterHookedMethod(param);
                    XposedBridge.log("Inside android.nfc.cardemulation.NfcFCardEmulation#isValidSystemCode");
                    mclassloader = mcontext.getClassLoader();
                    XposedBridge.log("Got classloader");
                    String path = getSoPath();
                    XposedBridge.log("So path = " + path);
                    int version = android.os.Build.VERSION.SDK_INT;
                    try {
                        if (!path.equals("")) {
                            XposedBridge.log("Start injecting libpmm.so");
                            if (version >= 28) {
                                XposedHelpers.callMethod(Runtime.getRuntime(),
                                        "nativeLoad", path + "libshadowhook.so", mclassloader);
                                XposedHelpers.callMethod(Runtime.getRuntime(),
                                        "nativeLoad", path + "libpmm.so", mclassloader);
                            } else {
                                XposedHelpers.callMethod(Runtime.getRuntime(),
                                        "doLoad", path + "libshadowhook.so", mclassloader);
                                XposedHelpers.callMethod(Runtime.getRuntime(),
                                        "doLoad", path + "libpmm.so", mclassloader);
                            }
                            XposedBridge.log("Injected libpmm.so");
                        }
                    } catch (Exception e) {
                        XposedBridge.log(e);
                        e.printStackTrace();
                    }

                    param.setResult(true);
                }
            });


            XposedHelpers.findAndHookMethod("android.nfc.cardemulation.NfcFCardEmulation", lpparam.classLoader,
            "isValidNfcid2", String.class, new XC_MethodReplacement() {
                @Override
                protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
                    return true;
                }
            });
        }
    }

    private String getSoPath() {
        try {
            String text = "";
            PackageManager pm = mcontext.getPackageManager();
            List<PackageInfo> pkgList = pm.getInstalledPackages(0);
            if (pkgList.size() > 0) {
                for (PackageInfo pi: pkgList) {
                    if (pi.applicationInfo.publicSourceDir.contains("moe.tqlwsl.pmmtool_sh")) {
                        text = pi.applicationInfo.publicSourceDir.replace("base.apk", "lib/arm64/");
                        return text;
                    }
                }
            }
        } catch (Exception e) {
            XposedBridge.log(e);
            e.printStackTrace();
        }
        return "";
    }

}
