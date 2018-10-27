package com.towersmatrix.rwav;

import android.content.res.AssetManager;
import android.view.Surface;

public class Native
{
    public native static long InitOgre(Surface surface, long nativeGvrContext, AssetManager assetManager);
    public native static void Render();
    public native static void UpdateModel(byte[] data);
    public native static void UpdateMeta(byte[] data);
    public native static boolean HasQueuedDownloads();
    public native static String GetNextDownload();
    public native static void DownloadFinished(String uri, byte[] data);;
}
