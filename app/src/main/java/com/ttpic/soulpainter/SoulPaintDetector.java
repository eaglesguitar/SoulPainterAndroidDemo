package com.ttpic.soulpainter;

import android.graphics.Bitmap;
import android.util.Log;

/**
 * Created by Gerald on 13/11/2017.
 */

public class SoulPaintDetector {

    private static final String TAG = SoulPaintDetector.class.getSimpleName();

    public SoulPaintDetector() {
    }

    public int init(byte[] model_bin, byte[] param_bin) {
        return nativeInit(model_bin, param_bin);
    }

    public int detect(Bitmap bitmap) {
        return nativeDetect(bitmap);
    }

    public native int nativeInit(byte[] model_bin, byte[] param_bin);

    public native int nativeDetect(Bitmap bitmap);

    static {
        try {
            System.loadLibrary("soulpainterdetector");
            Log.d(TAG, "library loaded!");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
