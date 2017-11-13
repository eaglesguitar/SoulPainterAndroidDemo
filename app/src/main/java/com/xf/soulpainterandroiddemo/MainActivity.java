package com.xf.soulpainterandroiddemo;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.ttpic.soulpainter.SoulPaintDetector;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    private SoulPaintDetector soulPaintDetector = new SoulPaintDetector();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        try {
            initSoulPaintDetector();

            int n = detectAssetsImage("full_2Fnumpy_bitmap_Fbeard_6508.jpg");
            Log.e(TAG, "soulPaintDetector.detect beard: " + n);

            n = detectAssetsImage("full_Fnumpy_bitmap_Fbear_6915.jpg");
            Log.e(TAG, "soulPaintDetector.detect bear: " + n);

            n = detectAssetsImage("full_Fnumpy_bitmap_Fcat_6547.jpg");
            Log.e(TAG, "soulPaintDetector.detect cat: " + n);

            n = detectAssetsImage("full_Fnumpy_bitmap_Fdog_6587.jpg");
            Log.e(TAG, "soulPaintDetector.detect dog: " + n);

            n = detectAssetsImage("full_Fnumpy_bitmap_Feyeglasses_6978.jpg");
            Log.e(TAG, "soulPaintDetector.detect eyeglasses: " + n);

            Bitmap b = Bitmap.createBitmap(28, 28, Bitmap.Config.ARGB_8888);
            n = soulPaintDetector.detect(b);
            Log.d(TAG, "soulPaintDetector.detect empty bitmap ==> " + n);

        } catch (IOException e) {
            Log.e("MainActivity", "initSqueezeNcnn error");
        }
    }

    private void initSoulPaintDetector() throws IOException {
        byte[] param_bin;
        byte[] model_bin;
        {
            InputStream assetsInputStream = getAssets().open("lenet.param.bin");
            int available = assetsInputStream.available();
            param_bin = new byte[available];
            assetsInputStream.read(param_bin);
            assetsInputStream.close();
        }
        {
            InputStream assetsInputStream = getAssets().open("lenet.bin");
            int available = assetsInputStream.available();
            model_bin = new byte[available];
            assetsInputStream.read(model_bin);
            assetsInputStream.close();
        }
        int r = soulPaintDetector.init(model_bin, param_bin);
        if (r != 0) {
            Log.e(TAG, "soulPaintDetector.init error: " + r);
        }
    }

    private int detectAssetsImage(String file) {
        InputStream in = null;
        try {
            in = getAssets().open("image/" + file);
            Bitmap number = BitmapFactory.decodeStream(in);
            if (number.getConfig() != Bitmap.Config.ARGB_8888) {
                number = number.copy(Bitmap.Config.ARGB_8888, false);
            }
            return soulPaintDetector.detect(number);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return -1;
    }

}
