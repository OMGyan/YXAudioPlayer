package com.yx.yxaudioplayerlib.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import java.nio.ByteBuffer;

/**
 * Author by YX, Date on 2020/1/9.
 *
 * 用于OpenGL渲染
 */
public class YXGLSurfaceView extends GLSurfaceView {

    private YXImageRenderer renderer;

    public YXGLSurfaceView(Context context) {
        this(context,null);
    }

    public YXGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        //通知默认的EGLContextFactory和默认的EGLConfigChooser,选择哪个EGLContext客户端版本
        setEGLContextClientVersion(2);
        //设置与此视图关联的渲染器。同时启动那个线程,将调用渲染器，这又将导致渲染开始.
        renderer = new YXImageRenderer(context);
        setRenderer(renderer);
        //设置渲染模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width,int height,byte[] y,byte[] u,byte[] v){
        if(renderer!=null){
            renderer.setYUVRendererData(width,height,y,u,v);
            //手动渲染
            requestRender();
        }
    }

}
