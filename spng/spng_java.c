#include "abex_spng_SPNGEncoder.h"
#define SPNG__BUILD
#include "spng.h"
#include <stdio.h>

static jfieldID SPNGEncoder_ctx = 0;

#define CTX spng_ctx *ctx = (spng_ctx*)((*env)->GetLongField(env, self, SPNGEncoder_ctx))
#define ERRCHECK(...) if (err) {errThrow(env, err); return __VA_ARGS__;}

static void errThrow(JNIEnv *env, int code) {
	char buf[256] = {0};
	snprintf(buf, sizeof(buf), "spng: %d", code);

	jclass clazz = (*env)->FindClass(env, "java/lang/RuntimeException");
	(*env)->ThrowNew(env, clazz, buf);
}

JNIEXPORT jlong JNICALL Java_abex_spng_SPNGEncoder_new0(JNIEnv *env, jclass clazz) {
	SPNGEncoder_ctx = (*env)->GetFieldID(env, clazz, "ctx", "J");
	return (jlong) spng_ctx_new(SPNG_CTX_ENCODER);
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_setIHDR(JNIEnv *env, jobject self, jint width, jint height) {
	CTX;
	struct spng_ihdr ihdr = {0};
	ihdr.width = width;
	ihdr.height = height;
	ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
	ihdr.bit_depth = 8;
	int err = spng_set_ihdr(ctx, &ihdr);
	ERRCHECK();
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_setFile(JNIEnv *env, jobject self, jstring filename) {
	CTX;
	const char *cFilename = (*env)->GetStringUTFChars(env, filename, 0);
	FILE *fi = fopen(cFilename, "wb");
	(*env)->ReleaseStringUTFChars(env, filename, cFilename);
	if (!fi) {
		errThrow(env, -1);
		return;
	}
	int err = spng_set_png_file(ctx, fi);
	if (err) {
		fclose(fi);
		ERRCHECK()
	}
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_setOption(JNIEnv *env, jobject self, jint key, jint value) {
	CTX;
	int err = spng_set_option(ctx, (enum spng_option) key, value);
	ERRCHECK();
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_encodeImage(JNIEnv *env, jobject self, jintArray buf, jint offset, jint stride, jint fmt, jint flags) {
	CTX;
	size_t len = (*env)->GetArrayLength(env, buf) * 4L;
	char *bufPtr = (*env)->GetPrimitiveArrayCritical(env, buf, NULL);
	if (!bufPtr) {
		jclass clazz = (*env)->FindClass(env, "java/lang/RuntimeException");
		(*env)->ThrowNew(env, clazz, "oom");
		return;
	}
	size_t off = (4L * offset);
	int err = spng_encode_image_stride(ctx, bufPtr + off, 4L * stride, len - off, fmt, flags);
	(*env)->ReleasePrimitiveArrayCritical(env, buf, bufPtr, JNI_ABORT);
	ERRCHECK();
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_encodeChunks(JNIEnv *env, jobject self) {
	CTX;
	int err = spng_encode_chunks(ctx);
	ERRCHECK();
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_setState(JNIEnv *env, jobject self, jint state) {
	CTX;
	spng_set_state(ctx, state);
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_writeChunk0(JNIEnv *env, jobject self, jstring hdr, jobject data, jint offset, jint length) {
	CTX;
	const char *cHdr = (*env)->GetStringUTFChars(env, hdr, NULL);
	int err = spng_write_chunk(ctx, cHdr,
		((uint8_t*) (*env)->GetDirectBufferAddress(env, data)) + offset,
		length);
	(*env)->ReleaseStringUTFChars(env, hdr, cHdr);
	ERRCHECK();
}

JNIEXPORT jobject JNICALL Java_abex_spng_SPNGEncoder_getBuffer(JNIEnv *env, jobject self) {
	CTX;
	int err;
	size_t len;
	void *data = spng_get_png_buffer(ctx, &len, &err);
	ERRCHECK(NULL);

	return (*env)->NewDirectByteBuffer(env, data, len);
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_freeBuffer(JNIEnv *env, jclass clazz, jobject buffer) {
	void *buf = (*env)->GetDirectBufferAddress(env, buffer);
	if (buf) {
		free(buf);
	}
}

JNIEXPORT void JNICALL Java_abex_spng_SPNGEncoder_close(JNIEnv *env, jobject self) {
	CTX;
	spng_close_png_file(ctx);
	spng_ctx_free(ctx);
	(*env)->SetLongField(env, self, SPNGEncoder_ctx, 0);
}