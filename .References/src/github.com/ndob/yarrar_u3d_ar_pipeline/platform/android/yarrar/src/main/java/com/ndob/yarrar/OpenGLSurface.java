package com.ndob.yarrar;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLSurface extends GLSurfaceView {

    private class MyGLRenderer implements GLSurfaceView.Renderer {

        YarrarActivity mMain;
        MyGLRenderer(YarrarActivity main) {
            mMain = main;
        }

        @Override
        public void onSurfaceCreated(GL10 unused, EGLConfig config) {
            mMain.onOpenGLSurfaceCreated();
        }

        @Override
        public void onDrawFrame(GL10 unused) {
            mMain.onOpenGLDrawFrame();
        }

        @Override
        public void onSurfaceChanged(GL10 unused, int width, int height) {

        }
    }

    public OpenGLSurface(Context context) {
        super(context);
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setRenderer(new MyGLRenderer((YarrarActivity) context));
        getHolder().setFormat(PixelFormat.RGBA_8888);
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }
}
