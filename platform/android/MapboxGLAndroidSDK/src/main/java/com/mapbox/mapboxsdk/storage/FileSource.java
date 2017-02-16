package com.mapbox.mapboxsdk.storage;

import android.content.Context;
import android.support.annotation.NonNull;

import com.mapbox.mapboxsdk.offline.OfflineManager;

import java.lang.ref.WeakReference;

/**
 * Holds a central reference to the core's DefaultFileSource for as long as
 * there are active mapviews / offline managers
 */
public class FileSource {

  /**
   * This callback allows implementors to transform URLs before they are requested
   * from the internet. This can be used add or remove custom parameters, or reroute
   * certain requests to other servers or endpoints.
   */
  public interface ResourceTransformCallback {

    /**
     * Called whenever a URL needs to be transformed.
     *
     * @param kind The kind of URL to be transformed.
     * @return A URL that will now be downloaded.
     */
    String onURL(@Resource.Kind int kind, String url);

  }

  // Use weak reference to avoid blocking GC on this reference.
  // Should only block when mapview / Offline manager instances
  // are alive
  private static WeakReference<FileSource> INSTANCE;

  public static synchronized FileSource getInstance(Context context) {
    if (INSTANCE == null || INSTANCE.get() == null) {
      String cachePath = OfflineManager.getDatabasePath(context);
      String apkPath = context.getPackageCodePath();
      INSTANCE = new WeakReference<>(new FileSource(cachePath, apkPath));
    }

    return INSTANCE.get();
  }

  private long nativePtr;

  private FileSource(String cachePath, String apkPath) {
    initialize(cachePath, apkPath);
  }

  /**
   * Sets a callback for transforming URLs requested from the internet
   * <p>
   * The callback will be executed on the main thread once for every requested URL.
   * </p>
   *
   * @param callback the callback to be invoked
   */
  public native void setResourceTransform(@NonNull final ResourceTransformCallback callback);

  private native void initialize(String cachePath, String apkPath);

  @Override
  protected native void finalize() throws Throwable;

}
