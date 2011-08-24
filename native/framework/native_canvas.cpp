/*
 * Copyright (C) 2011 Havlena Petr, havlenapetr@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Canvas"
#include <utils/Log.h>

#include <android/native_canvas.h>

#include <SkCanvas.h>
#include <SkBitmap.h>
#include <SkPaint.h>
#include <SkRect.h>
#include <SkDevice.h>

#include <jni.h>

using namespace android;

typedef struct ANativeCanvas {
    // actual pen
    SkPaint pen;
    
    // pointer on skia canvas, used only by native_canvas.c
    SkCanvas* canvas;
    
    int width, height;
} ANativeCanvas;

ANativeCanvas* ANativeCanvas_fromSurface(JNIEnv* env, jobject jsurface) {
    LOGI("Obtaining canvas from jsurface");
    
    jclass clazz_surface = env->FindClass("android/view/Surface");
    jfieldID field_jcanvas = env->GetFieldID(clazz_surface, "mCanvas", "Landroid/graphics/Canvas;");
    if(field_jcanvas == NULL) {
        LOGE("Can't obtain java canvas object from surface!");
		return NULL;
	}
    
    // canvas java object in java surface object
    jobject jcanvas = env->GetObjectField(jsurface, field_jcanvas);
    
    jclass clazz_canvas = env->FindClass("android/graphics/Canvas");
	jfieldID field_canvas = env->GetFieldID(clazz_canvas, "mNativeCanvas", "I");
	if(field_canvas == NULL) {
        LOGE("Can't obtain native canvas pointer");
		return NULL;
	}
	
    ANativeCanvas* canvas = (ANativeCanvas *) malloc(sizeof(ANativeCanvas));
    canvas->canvas = (SkCanvas *) env->GetIntField(jcanvas, field_canvas);
    canvas->pen.setColor(ACOLOR_WHITE);
    return canvas;
}

ANativeCanvas* ANativeCanvas_acquire() {
    LOGI("Creating new canvas");
    
    ANativeCanvas* canvas = (ANativeCanvas *) malloc(sizeof(ANativeCanvas));
    canvas->canvas = new SkCanvas();
    canvas->pen.setColor(ACOLOR_WHITE);
    return canvas;
}

void ANativeCanvas_release(ANativeCanvas* canvas) {
    delete canvas->canvas;
}

SkCanvas* ANativeCanvas_getSkCanvas(ANativeCanvas* canvas) {
    return canvas->canvas;
}

/*ANativeCanvas* ANativeCanvas_acquire(ANativeSurface* surface, int w, int h) {
    ANativeCanvas* canvas = (ANativeCanvas *) malloc(sizeof(ANativeCanvas));
    canvas->surface = surface;
    canvas->color = ACOLOR_WHITE;
    
    // create SkCanvas with bitmap as main container
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bitmap.setIsOpaque(true);
    if(!bitmap.allocPixels()) {
        return NULL;
    }
    canvas->canvas = new SkCanvas(bitmap);
    return canvas;
}

void ANativeCanvas_release(ANativeCanvas* canvas) {
    free(canvas->_canvas);
    free(canvas);
}*/

void ANativeCanvas_setWidth(ANativeCanvas* canvas, int32_t width) {
    canvas->width = width;
}

void ANativeCanvas_setHeight(ANativeCanvas* canvas, int32_t height) {
    canvas->height = height;
}

int32_t ANativeCanvas_getWidth(ANativeCanvas* canvas) {
    return canvas->width;
}

int32_t ANativeCanvas_getHeight(ANativeCanvas* canvas) {
    return canvas->height;
}

int32_t ANativeCanvas_drawPolygon(ANativeCanvas* canvas, int* points, int points_size) {
    SkCanvas* c = canvas->canvas;
    SkPath path;
    path.moveTo((SkScalar) points[0], (SkScalar) points[1]);
    for (int i = 2; i < points_size; i += 2) {
        path.lineTo((SkScalar) points[i], (SkScalar) points[i + 1]);
    }
    //pen.setStyle(SkPaint::Style::kStrokeAndFill_Style);
    c->drawPath(path, canvas->pen);
    return 0;
}

int32_t ANativeCanvas_drawText(ANativeCanvas* canvas, const void* text, size_t text_length,
        int text_size, int x, int y, int dx, int dy) {
    SkCanvas* c = canvas->canvas;
    canvas->pen.setTextSize(text_size / 15);
    //pen.setStyle(SkPaint::Style::kStrokeAndFill_Style);
    c->drawText(text, text_length, (SkScalar) x, (SkScalar) y, canvas->pen);
    return 0;
}

int32_t ANativeCanvas_drawPolyline(ANativeCanvas* canvas, int* points, int points_size) {
    SkCanvas* c = canvas->canvas;
    SkPath path;
    path.moveTo((SkScalar) points[0], (SkScalar) points[1]);
    for (int i = 2; i < points_size; i += 2) {
        path.lineTo((SkScalar) points[i], (SkScalar) points[i + 1]);
    }
    //pen.setStyle(SkPaint::Style::kStroke_Style);
    c->drawPath(path, canvas->pen);
    return 0;
}

int32_t ANativeCanvas_drawCircle(ANativeCanvas* canvas, int x, int y, int radius) {
    SkCanvas* c = canvas->canvas;
    //p.setStyle(SkPaint::Style::kStrokeAndFill_Style);
    c->drawCircle((SkScalar)x, (SkScalar)y, (SkScalar)radius, canvas->pen);
    return 0;
}

int32_t ANativeCanvas_drawRectange(ANativeCanvas* canvas, int x, int y, int w, int h) {
    SkPaint p;
    SkCanvas* c = canvas->canvas;
    SkRect rect = SkRect::MakeXYWH(x, y, w, h);
    p.setColor(ACOLOR_WHITE);
    //p.setStyle(SkPaint::Style::kStrokeAndFill_Style);
    c->drawRect(rect, p);
    return 0;
}

/*int32_t ANativeCanvas_draw(ANativeCanvas* canvas) {
    SkCanvas* c = canvas->canvas;
    ANativeSurface_Buffer buffer;
    int ret = ANativeSurface_lock(canvas->surface, &buffer, NULL);
    if(ret < 0) {
        LOGE("Can't lock native surfacedow!");
        return -1;
    }
    
    // copy canvas's bitmap pixels to screen
    SkDevice* device = c->getDevice();
    if(device == NULL) {
        LOGE("Can't obtain SkDevice!");
        return -1;
    }
    
    SkBitmap bitmap = device->accessBitmap(false);
    bitmap.lockPixels();
    int bpp = buffer.stride / buffer.width;
    memcpy(buffer.bits, bitmap.getPixels(), buffer.width * buffer.height * bpp);
    bitmap.unlockPixels();
    
    return ANativeSurface_unlockAndPost(canvas->surface);
}*/