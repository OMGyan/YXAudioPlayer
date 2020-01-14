package com.yx.yxaudioplayerlib.opengl;

import android.content.Context;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;


import com.yx.yxaudioplayerlib.R;


import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


/**
 * Author by YX, Date on 2020/1/10.
 */
public class YXImageRenderer implements GLSurfaceView.Renderer{

    private Context context;
    //创建顶点坐标缓存
    private FloatBuffer vertexBuffer;
    //创建纹理坐标缓存
    private FloatBuffer textureBuffer;
    private int program_yuv;
    private int av_position_yuv;
    private int af_position_yuv;
    private int textureId;
    //YUV取样器
    private int sampler_y;
    private int sampler_u;
    private int sampler_v;

    private int[] textureId_yuv;

    private int width_yuv;
    private int height_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;



    //创建顶点坐标数组(全屏)(GLES20.GL_TRIANGLE_STRIP)
    private final float[] vertexData = {
            -1f,-1f,
            1f,-1f,
            -1f,1f,
            1f,1f
    };

    //创建纹理坐标数组(对应顶点坐标系)
    private final float[] textureData = {
             0f,1f,
             1f,1f,
             0f,0f,
             1f,0f
    };

    public YXImageRenderer(Context context) {
        this.context = context;
        //分配一个新的直接字节缓冲区(每个单位为4字节)。
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                //修改此缓冲区的字节顺序。
                .order(ByteOrder.nativeOrder())
                //创建此字节缓冲区的视图作为浮点缓冲区。
                .asFloatBuffer()
                //把坐标放入
                .put(vertexData);
        //设置此缓冲区的位置。
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        textureBuffer.position(0);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initYUVRenderer();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //x，y 以像素为单位，指定了视口的左下角位置。
        //width，height 表示这个视口矩形的宽度和高度，根据窗口的实时变化重绘窗口。
        GLES20.glViewport(0,0,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        //整个窗口清除为黑色
        //参数为相关缓冲区
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        //用黑色清屏
        GLES20.glClearColor(0.0f,0.0f,0.0f,1.0f);
        rendererYUV();
    }

    private void initYUVRenderer(){
        //拿到顶点着色器源码
        String vertexShaderSource = YXShaderUtil.readRawText(context, R.raw.vertex_shader);
        //拿到片面着色器源码
        String fragmentShaderSource = YXShaderUtil.readRawText(context, R.raw.fragment_shader);
        //获取program对象
        program_yuv = YXShaderUtil.createProgram(vertexShaderSource,fragmentShaderSource);
        //获取成功之后，拿到内部属性
        av_position_yuv = GLES20.glGetAttribLocation(program_yuv, "av_Position");
        af_position_yuv = GLES20.glGetAttribLocation(program_yuv, "af_Position");
        sampler_y = GLES20.glGetUniformLocation(program_yuv,"sampler_y");
        sampler_u = GLES20.glGetUniformLocation(program_yuv,"sampler_u");
        sampler_v = GLES20.glGetUniformLocation(program_yuv,"sampler_v");
        textureId_yuv = new int[3];
        //生成三个纹理
        GLES20.glGenTextures(3,textureId_yuv,0);

        for (int i = 0; i < 3; i++) {
            //绑定这个纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureId_yuv[i]);
            //设置环绕和过滤方式
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    public void setYUVRendererData(int width,int height,byte[] y,byte[] u,byte[] v){
        this.width_yuv = width;
        this.height_yuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void rendererYUV(){
        if(width_yuv>0 && height_yuv>0 && y!=null && u!=null && v!=null){

            //使用program_yuv
            GLES20.glUseProgram(program_yuv);
            //把顶点坐标设置成可用状态
            GLES20.glEnableVertexAttribArray(av_position_yuv);
            /**
             * 指定渲染时索引值为 index 的顶点属性数组的数据格式和位置
             * index:指定要修改的顶点属性的索引值.
             * size:指定每个顶点属性的组件数量,必须为1、2、3或者4。初始值为4,(如position是由3个(x,y,z)组成，而颜色是4个(r,g,b,a)).
             * type:指定数组中每个组件的数据类型.
             * normalized:指定当被访问时，固定点数据值是否应该被归一化（GL_TRUE）或者直接转换为固定点值（GL_FALSE).
             * stride:指定连续顶点属性之间的偏移量.
             * pointer:指定第一个组件在数组的第一个顶点属性中的偏移量。该数组与GL_ARRAY_BUFFER绑定，储存于缓冲区中。初始值为0.
             */
            GLES20.glVertexAttribPointer(av_position_yuv,2, GLES20.GL_FLOAT,false,8,vertexBuffer);
            GLES20.glEnableVertexAttribArray(af_position_yuv);
            GLES20.glVertexAttribPointer(af_position_yuv,2, GLES20.GL_FLOAT,false,8,textureBuffer);
            //设置纹理单元
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            //绑定到指定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureId_yuv[0]);
            //根据参数生成指定2D纹理
//        GL_APICALL void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLenum internalformat,
//                GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
//        参数说明：
//        target 指定目标纹理，这个值必须是GL_TEXTURE_2D。
//        level 执行细节级别。0是最基本的图像级别，n表示第N级贴图细化级别。
//        internalformat 指定纹理中的颜色组件。可选的值有GL_ALPHA,GL_RGB,GL_RGBA,GL_LUMINANCE, GL_LUMINANCE_ALPHA 等几种。
//        width 指定纹理图像的宽度，必须是2的n次方。纹理图片至少要支持64个材质元素的宽度
//        height 指定纹理图像的高度，必须是2的m次方。纹理图片至少要支持64个材质元素的高度
//        border 指定边框的宽度。必须为0。
//        format 像素数据的颜色格式, 不需要和internalformatt取值必须相同。可选的值参考internalformat。
//        type 指定像素数据的数据类型。可以使用的值有GL_UNSIGNED_BYTE,GL_UNSIGNED_SHORT_5_6_5,GL_UNSIGNED_SHORT_4_4_4_4,GL_UNSIGNED_SHORT_5_5_5_1。
//        pixels 指定内存中指向图像数据的指针
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,width_yuv,height_yuv,0,GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,width_yuv/2,height_yuv/2,0,GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,width_yuv/2,height_yuv/2,0,GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,v);
            //为当前程序对象指定Uniform变量的值
            GLES20.glUniform1i(sampler_y,0);
            GLES20.glUniform1i(sampler_u,1);
            GLES20.glUniform1i(sampler_v,2);
            //释放资源
            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);
        }

    }
}
